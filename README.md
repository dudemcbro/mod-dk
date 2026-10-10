mod-dk
======

An [AzerothCore](https://www.azerothcore.org/) module that lets you level a
Death Knight from 1 like any other class. DKs learn their spells from a
trainer as they level, and their damage, healing and threat are scaled down
at low levels so they don't outclass everyone else.

- **Author:** [dudemcbro](https://github.com/dudemcbro)
- **License:** [GNU GPL v3.0](LICENSE)
- **Works with:** AzerothCore `master` and the mod-playerbots fork (WoW 3.3.5a, build 12340)

Features
--------
- **A level 1 start.** New DKs watch the intro cinematic, then go to their
  racial starting zone in starter gear, with a 2H sword and only Plague
  Strike known.
- **A trainer next to every priest.** Madness, the DK trainer, waits beside
  the priest trainer in every starting zone and capital city, with a
  runeforge close by. Spells unlock on a 1-60 schedule; 55-80 is stock.
- **Low level balance.** Damage, healing, presences, threat and minions scale
  up to full strength by level 55, so early DKs play like other classes.
- **Runeforging from level 20**, on low level weapons, at forges out in the
  world.
- **A replayable Ebon Hold intro.** The Ebon Herald at Light's Hope Chapel
  lets level 55+ DKs who skipped it play the intro from the start.
- **Hands-off for admins.** The DB updater applies the SQL, the module sets
  the DK start level itself, a config file toggles every feature, and
  `.dk status` checks the setup.

Quick start
-----------
1. Clone into your AzerothCore `modules/` folder (keep the name `mod-dk`):
   ```sh
   cd <azerothcore>/modules
   git clone https://github.com/dudemcbro/mod-dk.git
   ```
2. Re-run CMake, build and install the core.
3. Start `worldserver`. The DB updater applies the module's SQL; check the
   log for `mod_dk: Setup check passed.`
4. Build the client patch and give `patch-d.MPQ` to your players:
   ```sh
   python3 scripts/build_runeforge_client_mpq.py --dbc-in <DataDir>/dbc/Spell.dbc
   ```

The full steps, requirements, updating and uninstalling are in
[docs/installation.md](docs/installation.md); running the module day to day is
in the [server admin guide](docs/admin-guide.md). Share the
[player guide](docs/player-guide.md) with your players.

Documentation
-------------
| Page | What's in it |
| --- | --- |
| [Player guide](docs/player-guide.md) | For players: the client patch, where to train, what you learn when, runeforging, the Ebon Hold story |
| [Server admin guide](docs/admin-guide.md) | For admins: what the module changes, setup decisions, checking it works, managing spawns, customizing |
| [Installation](docs/installation.md) | Requirements, install, the SQL files, updating, uninstalling |
| [The client patch](docs/client-patch.md) | What the player patch changes, building it, the database password |
| [Configuration](docs/configuration.md) | Every option in `mod_dk.conf` |
| [GM commands](docs/gm-commands.md) | `.dk` commands for trainers, runeforges, status and reload |
| [How it works](docs/how-it-works.md) | Level schedule, scaling curves, core hooks, the intro and reentry |
| [Optional core patches](docs/core-patches.md) | Flight path discovery and runeforge debug logging |
| [Troubleshooting](docs/troubleshooting.md) | Common problems and fixes |
| [Contributing](docs/contributing.md) | How to help, and the repository layout |

License
-------
Licensed under the GNU General Public License v3.0. See [LICENSE](LICENSE).
