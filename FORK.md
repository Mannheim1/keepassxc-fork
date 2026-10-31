# Fork notes

This fork adds three things to KeePassXC. The database format and encryption are unchanged, so a database saved here opens normally in stock KeePassXC.

- **Tags column:** right-click the entry table header and tick **Tags**.
- **Custom field column:** right-click the header, choose **Choose custom field...**, and pick or type a field name such as `Email`. Click the column header to sort. Protected fields show as dots.
- **Bulk edit:** select two or more entries, right-click, and choose **Bulk Edit Entries…**. Only ticked fields change. Each changed entry gets a history item, so an edit can be reverted per entry from the entry's History tab.

## Building on Windows

Requires Visual Studio 2022 C++ build tools. Dependencies live outside the repo in `C:\Dev\kpxc-deps`:

- `vcpkg`: a clone of https://github.com/microsoft/vcpkg, bootstrapped with `bootstrap-vcpkg.bat`
- `qt`: Qt for MSVC, installed with `aqt install-qt windows desktop <version> win64_msvc2022_64 -m qtimageformats -O qt`

```bash
utils\build-fork-windows.bat
```

The app is placed in `C:\Dev\kpxc-deps\build\dist`.

## Updating to a new KeePassXC release

The fork's commits sit on top of a stable release tag. To move them to a new release, where `2.8.0` is the current base and `2.8.1` is the new one:

```bash
git fetch upstream --tags
```

```bash
git rebase --onto 2.8.1 2.8.0 fork/entry-table-features
```

Then rebuild and run the tests. Most of the feature code is in new files (`src/core/EntryBulkEdit.*`, `src/gui/entry/BulkEditEntriesDialog.*`), so conflicts should be limited to small hooks in `EntryModel`, `EntryView`, `MainWindow`, and `DatabaseWidget`. If upstream adds its own column numbered 17 or 18, renumber `Tags` and `CustomField` to follow it, and bump the count in `EntryModel::columnCount()`.
