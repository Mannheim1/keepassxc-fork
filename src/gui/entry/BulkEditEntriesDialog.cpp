/*
 *  Copyright (C) 2026 KeePassXC Team <team@keepassxc.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 or (at your option)
 *  version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "BulkEditEntriesDialog.h"
#include "ui_BulkEditEntriesDialog.h"

#include <QSet>

#include "core/Clock.h"
#include "core/Entry.h"
#include "gui/MessageBox.h"

namespace
{
    enum CustomFieldColumn
    {
        NameColumn,
        ActionColumn,
        ValueColumn,
        ProtectColumn
    };

    enum CustomFieldAction
    {
        SetAction,
        DeleteAction
    };
} // namespace

BulkEditEntriesDialog::BulkEditEntriesDialog(const QList<Entry*>& entries,
                                             const QStringList& knownTags,
                                             QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::BulkEditEntriesDialog)
{
    m_ui->setupUi(this);

    m_ui->summaryLabel->setText(tr("Only ticked fields change. Selected entries: %1").arg(entries.size()));
    m_ui->tagsEdit->completion(knownTags);
    m_ui->expiryEdit->setDateTime(Clock::currentDateTime().addYears(1));

    QSet<QString> names;
    for (const auto entry : entries) {
        for (const auto& key : entry->attributes()->customKeys()) {
            names.insert(key);
        }
    }
    m_customFieldNames = names.values();
    m_customFieldNames.sort(Qt::CaseInsensitive);

    auto header = m_ui->customFieldsTable->horizontalHeader();
    header->setSectionResizeMode(NameColumn, QHeaderView::Stretch);
    header->setSectionResizeMode(ValueColumn, QHeaderView::Stretch);

    const QList<QPair<QCheckBox*, QWidget*>> fields = {{m_ui->titleCheck, m_ui->titleEdit},
                                                       {m_ui->usernameCheck, m_ui->usernameEdit},
                                                       {m_ui->urlCheck, m_ui->urlEdit},
                                                       {m_ui->tagsCheck, m_ui->tagsWidget},
                                                       {m_ui->notesCheck, m_ui->notesEdit},
                                                       {m_ui->expiryCheck, m_ui->expiryWidget},
                                                       {m_ui->autoTypeCheck, m_ui->autoTypeCombo}};
    for (const auto& field : fields) {
        field.second->setEnabled(false);
        connect(field.first, &QCheckBox::toggled, field.second, &QWidget::setEnabled);
    }

    connect(m_ui->addFieldButton, &QPushButton::clicked, this, &BulkEditEntriesDialog::addCustomFieldRow);
    connect(m_ui->removeFieldButton, &QPushButton::clicked, this, &BulkEditEntriesDialog::removeCustomFieldRow);
    connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, &BulkEditEntriesDialog::accept);
    connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &BulkEditEntriesDialog::reject);
}

BulkEditEntriesDialog::~BulkEditEntriesDialog() = default;

void BulkEditEntriesDialog::addCustomFieldRow()
{
    auto table = m_ui->customFieldsTable;
    const int row = table->rowCount();
    table->insertRow(row);

    auto nameCombo = new QComboBox(table);
    nameCombo->setEditable(true);
    nameCombo->addItems(m_customFieldNames);
    nameCombo->setCurrentText({});
    table->setCellWidget(row, NameColumn, nameCombo);

    auto actionCombo = new QComboBox(table);
    actionCombo->insertItem(SetAction, tr("Set value"));
    actionCombo->insertItem(DeleteAction, tr("Delete field"));
    table->setCellWidget(row, ActionColumn, actionCombo);

    table->setItem(row, ValueColumn, new QTableWidgetItem());

    auto protectItem = new QTableWidgetItem();
    protectItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    protectItem->setCheckState(Qt::Unchecked);
    table->setItem(row, ProtectColumn, protectItem);
}

void BulkEditEntriesDialog::removeCustomFieldRow()
{
    m_ui->customFieldsTable->removeRow(m_ui->customFieldsTable->currentRow());
}

QString BulkEditEntriesDialog::customFieldName(int row) const
{
    auto nameCombo = qobject_cast<QComboBox*>(m_ui->customFieldsTable->cellWidget(row, NameColumn));
    return nameCombo->currentText().trimmed();
}

void BulkEditEntriesDialog::accept()
{
    for (int row = 0; row < m_ui->customFieldsTable->rowCount(); ++row) {
        const auto name = customFieldName(row);
        if (name.isEmpty() || EntryAttributes::isDefaultAttribute(name) || EntryAttributes::isPasskeyAttribute(name)) {
            MessageBox::warning(this,
                                tr("Invalid custom field"),
                                tr("\"%1\" cannot be used as a custom field name here.").arg(name));
            return;
        }
    }

    QDialog::accept();
}

EntryBulkEdit BulkEditEntriesDialog::edit() const
{
    EntryBulkEdit edit;

    if (m_ui->titleCheck->isChecked()) {
        edit.title = m_ui->titleEdit->text();
    }
    if (m_ui->usernameCheck->isChecked()) {
        edit.username = m_ui->usernameEdit->text();
    }
    if (m_ui->urlCheck->isChecked()) {
        edit.url = m_ui->urlEdit->text();
    }
    if (m_ui->notesCheck->isChecked()) {
        edit.notes = m_ui->notesEdit->toPlainText();
    }
    if (m_ui->tagsCheck->isChecked()) {
        auto tags = m_ui->tagsEdit->tags();
        tags.removeAll({});
        edit.tags = tags;
        edit.tagMode = static_cast<EntryBulkEdit::TagMode>(m_ui->tagsModeCombo->currentIndex());
    }
    if (m_ui->expiryCheck->isChecked()) {
        edit.expires = m_ui->expiresCombo->currentIndex() == 1;
        edit.expiryTime = m_ui->expiryEdit->dateTime().toUTC();
    }
    if (m_ui->autoTypeCheck->isChecked()) {
        edit.autoTypeEnabled = m_ui->autoTypeCombo->currentIndex() == 0;
    }

    auto table = m_ui->customFieldsTable;
    for (int row = 0; row < table->rowCount(); ++row) {
        EntryBulkEdit::CustomField field;
        field.name = customFieldName(row);
        field.value = table->item(row, ValueColumn)->text();
        field.protect = table->item(row, ProtectColumn)->checkState() == Qt::Checked;
        field.remove = qobject_cast<QComboBox*>(table->cellWidget(row, ActionColumn))->currentIndex() == DeleteAction;
        edit.customFields.append(field);
    }

    return edit;
}
