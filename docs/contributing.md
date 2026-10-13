Contributing
============

Bug reports and suggestions are welcome as
[issues](https://github.com/dudemcbro/mod-dk/issues). To contribute code:

1. Fork the repository and create a branch.
2. Keep the SQL re-runnable (the DB updater applies a file again whenever it
   changes) and add matching steps to
   [`sql/dk_rollback.sql`](../sql/dk_rollback.sql) for anything you change.
3. Never commit Blizzard game data (`.dbc`, `.MPQ`); build it from your own
   client with the tools instead.
4. Build the core with your change and test it in game. CI builds AzerothCore
   `master` with the module and fails on any startup error, so check the
   `worldserver` log for errors your change adds.
5. Open a [pull request](https://github.com/dudemcbro/mod-dk/pulls) that
   explains what changed and why, using
   [Conventional Commits](https://www.conventionalcommits.org/) style commit
   messages (for example `fix(sql): ...`).

Repository layout
-----------------
| Path | Contents |
| --- | --- |
| `src/` | The module's C++ scripts and GM commands; the core builds every `.cpp` file here |
| `conf/mod_dk.conf.dist` | Module settings |
| `data/sql/world/` | World SQL, applied by the DB updater |
| `sql/install.sql`, `sql/rollback.sql` | Manual install and the uninstall |
| `scripts/` | Python tool that builds the client patch |
| `patches/` | Optional core patches |
| `docs/` | This documentation |
| `.github/workflows/` | CI: builds AzerothCore master with the module on every push and pull request |
| `icon.png` | Icon for the AzerothCore module catalogue |
