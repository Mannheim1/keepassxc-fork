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

#include "EntryBulkEdit.h"

#include "core/Entry.h"

void EntryBulkEdit::apply(const QList<Entry*>& entries) const
{
    for (auto entry : entries) {
        // Each changed entry gets a history item, so a bulk edit can be reverted per entry
        entry->beginUpdate();

        if (title) {
            entry->setTitle(*title);
        }
        if (username) {
            entry->setUsername(*username);
        }
        if (url) {
            entry->setUrl(*url);
        }
        if (notes) {
            entry->setNotes(*notes);
        }
        if (tags) {
            switch (tagMode) {
            case TagMode::Replace:
                entry->setTags(tags->join(";"));
                break;
            case TagMode::Add:
                for (const auto& tag : *tags) {
                    entry->addTag(tag);
                }
                break;
            case TagMode::Remove:
                for (const auto& tag : *tags) {
                    entry->removeTag(tag);
                }
                break;
            }
        }
        if (expires) {
            entry->setExpires(*expires);
            if (*expires) {
                entry->setExpiryTime(expiryTime);
            }
        }
        if (autoTypeEnabled) {
            entry->setAutoTypeEnabled(*autoTypeEnabled);
        }
        for (const auto& field : customFields) {
            auto attributes = entry->attributes();
            if (field.remove) {
                attributes->remove(field.name);
            } else {
                // A field that is already protected stays protected
                attributes->set(field.name, field.value, field.protect || attributes->isProtected(field.name));
            }
        }

        entry->endUpdate();
    }
}
