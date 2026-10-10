Server admin guide
==================

This guide covers running mod-dk on a server: what it changes, the decisions
to make, and the day-to-day tasks. The step-by-step install is in
[Installation](installation.md); this page links to the reference pages for
details.

- [What the module changes](#what-the-module-changes)
- [First-time setup](#first-time-setup)
- [Decisions to make](#decisions-to-make)
- [Checking that everything works](#checking-that-everything-works)
- [Managing trainers and runeforges](#managing-trainers-and-runeforges)
- [The client patch](#the-client-patch)
- [Customizing spell levels and other data](#customizing-spell-levels-and-other-data)
- [Updating](#updating)
- [Other modules](#other-modules)
- [Uninstalling](#uninstalling)

What the module changes
-----------------------
Only the world database and the running server; the auth and characters
databases are untouched.

- **New world data:** the Madness trainer (creature 900001, TrainerId 1013 and
  its spell list), the Ebon Herald reentry NPC (900000, with two gossip texts),
  a blue flame object (gameobject 900010), the `mod_dk_spell_progression`
  table, and default Madness and runeforge spawns in every DK starting zone and
  capital.
- **Changed stock data, DK class only:** start location (Acherus), action bar,
  starting outfit, creation spells and skills, starting items, and DK base
  stats for levels 1-60. Starter items from other modules are kept.
- **Spell scripts** registered in `spell_script_names` for the low level
  scaling.
- **In memory at startup:** the DK start level and the account level needed to
  create one (`ModDk.StartLevel`, `ModDk.MinLevelForHeroicCharacter`), the
  rune spell and enchant requirements, and the core's automatic learning of the
  trainer spells. No server DBC file is edited.

[How it works](how-it-works.md) explains each of these.

First-time setup
----------------
1. Clone into `modules/mod-dk`, re-run CMake, build and install.
2. Copy `etc/modules/mod_dk.conf.dist` to `mod_dk.conf` if you want to change a
   setting (see [Decisions to make](#decisions-to-make)).
3. Start `worldserver`; the DB updater applies the SQL.
4. Build the client patch and share it with your players.
5. Run `.dk status` in game.

Details, requirements and the manual SQL route: [Installation](installation.md).

Decisions to make
-----------------
All in `mod_dk.conf`; see [Configuration](configuration.md) for every option.

| Question | Setting | Default |
| --- | --- | --- |
| What level do new DKs start at? | `ModDk.StartLevel` (`55` for the stock intro, `-1` to use `worldserver.conf`) | `1` |
| Does an account need a high level character to create a DK? | `ModDk.MinLevelForHeroicCharacter` | `0` (no) |
| How strong is the low level scaling? | `ModDk.Scaling.Strength` (`0.0` to `1.0`) or `ModDk.Scaling.Enable` | full curve |
| Extra low level threat for tanks? | `ModDk.ThreatBonus.Enable` | on |
| Offer the Ebon Hold story to 55+ DKs? | `ModDk.Reentry.Enable` | on |
| Keep the default trainers and runeforges? | Remove the ones you don't want in game | all placed |

If you lower `ModDk.Scaling.Strength`, the client patch tooltips no longer
match the server; they assume `1.0`.

Checking that everything works
------------------------------
- **At startup**, the log shows `mod_dk: Setup check passed.`, or a
  `mod_dk: Setup check:` warning per problem with the file or tool that fixes it.
- **In game or on the console**, `.dk status` shows the settings, the DK start
  level, the progression and trainer data, the spell scripts, the automatic
  learning fix, the runeforge corrections, and how many trainers, runeforges
  and reentry NPCs are spawned.

[Troubleshooting](troubleshooting.md) lists common problems.

Managing trainers and runeforges
--------------------------------
- `.dk madness list` and `.dk runeforge list` show every spawn, nearest first,
  marked as default or GM placed. Go to one with `.go creature <guid>` or
  `.go gameobject <guid>`.
- `.dk madness add` / `.dk runeforge add` place one at your position;
  `.dk madness remove` / `.dk runeforge remove` remove the targeted one or the
  nearest within 15 yards.
- The default spawns were placed from the priest trainers' positions, so check
  a few in game; move any that clip into a wall by removing it and adding one.
- Default spawns come back when a module update changes
  `mod_dk_17_default_spawns.sql`. GM placed spawns are never touched by updates.

All commands: [GM commands](gm-commands.md).

The client patch
----------------
Players need `patch-d.MPQ`, built on your server from your own game data with
`scripts/build_runeforge_client_mpq.py`. Point your players at the
[Player guide](player-guide.md), which explains how to install it. Rebuild and
share it again whenever a module update changes the trainer levels or the
scaling, or after you change spell levels yourself. Building it and the
database password are covered in [The client patch](client-patch.md).

Customizing spell levels and other data
---------------------------------------
The module's SQL files are re-applied by the DB updater whenever an update
changes them, which overwrites edits made to their tables. To change data
yourself:

- Put your changes in your own SQL file instead of editing the module's files.
  The updater applies files in name order, so name it to sort after the
  module's files (for example `zz_mod_dk_custom.sql` in the core's
  `data/sql/custom/db_world/`).
- After a module update, check the updater output for re-applied `mod_dk_`
  files and re-apply your file if they touch the same tables.
- Spell levels live in two places that must agree: `mod_dk_spell_progression`
  (what DKs may keep) and `trainer_spell` for TrainerId 1013 (what Madness
  teaches). Apply the change, then run `.dk reload` (no restart needed) and
  rebuild the client patch.

Updating
--------
`git pull` in `modules/mod-dk`, rebuild and install, restart. The updater
applies changed SQL; GM placed spawns stay. Compare `mod_dk.conf` with the new
`.dist` for new options. A clone made before the repository's fresh history in
October 2026 needs `git fetch origin && git reset --hard origin/main` once.
Full notes: [Installation: Updating](installation.md#updating).

Other modules
-------------
- [mod-trainers](https://github.com/dudemcbro/mod-trainers) points its Death
  Knight trainers at TrainerId 1013, so they teach this module's schedule.
- Other modules' starter items in `playercreateinfo_item` are kept; only the
  DK gear rows are replaced.
- [Optional core patches](core-patches.md): stop DKs from knowing every old
  world flight path, or log runeforge cast failures.

Uninstalling
------------
Leave `StartHeroicPlayerLevel` in `worldserver.conf` at `55` (or remove it),
run `sql/rollback.sql`, then remove the module and rebuild before starting
`worldserver` again, or the updater installs the SQL again. Full steps:
[Installation: Uninstalling](installation.md#uninstalling).
