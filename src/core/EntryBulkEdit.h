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

#pragma once

#include <QDateTime>
#include <QList>
#include <QStringList>

#include <optional>

class Entry;

/**
 * A set of changes to apply to many entries at once. Fields left unset keep
 * the value each entry already has. The password is deliberately not editable.
 */
struct EntryBulkEdit
{
    enum class TagMode
    {
        Replace,
        Add,
        Remove
    };

    struct CustomField
    {
        QString name;
        QString value;
        bool protect = false;
        bool remove = false;
    };

    std::optional<QString> title;
    std::optional<QString> username;
    std::optional<QString> url;
    std::optional<QString> notes;
    std::optional<QStringList> tags;
    TagMode tagMode = TagMode::Replace;
    std::optional<bool> expires;
    QDateTime expiryTime;
    std::optional<bool> autoTypeEnabled;
    QList<CustomField> customFields;

    void apply(const QList<Entry*>& entries) const;
};
