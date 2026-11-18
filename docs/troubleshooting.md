Troubleshooting
===============

Start with `.dk status` in game (or `dk status` on the console). It lists every
problem it finds with the file or tool that fixes it.

| Symptom | Cause and fix |
| --- | --- |
| `worldserver` exits with "initial level does not have stats data" | DKs start below 55 but `mod_dk_01_balance_class_stats_1_60.sql` is not applied. Check that the DB updater covers the world database, or apply `sql/install.sql` by hand. |
| No `mod_dk_` lines in the DB updater output | The updater only reads modules it was built with and needs the source tree: check `-DMODULES`, `SourceDirectory`, `Updates.EnableDatabases` and `MySQLExecutable`. |
| `git pull` fails with "refusing to merge unrelated histories" | The clone predates the repository's fresh history; see [Updating from an older version](installation.md#updating). |
| "Target is too low level" when runeforging | The player's client has no patch MPQ, or `Cache/` was not cleared after copying it. |
| Players see "You have learned" then "You have unlearned" on level up | A trainer spell is still auto learned; `.dk status` names the count and `.dk reload` fixes it. |
| New DKs start at 55 although `StartHeroicPlayerLevel = 1` | `ModDk.StartLevel` replaces that setting; set it in `mod_dk.conf` (or `-1` to use `worldserver.conf`). |
| Tooltips do not change as the character levels | Rebuild the client patch with `--static-tooltips`; the server scaling is not affected. |
| A default trainer or runeforge stands in a wall | Remove it with `.dk madness remove` / `.dk runeforge remove` and place one with `.dk madness add` / `.dk runeforge add`. |
| Config changes have no effect | The file must be `etc/modules/mod_dk.conf` (not `.dist`); apply with `.reload config`. Active presences and diseases keep their values until cast again. `ModDk.StartLevel` and `ModDk.MinLevelForHeroicCharacter` need a restart. |
