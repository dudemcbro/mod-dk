Installation, updating and uninstalling
=======================================

- [Requirements](#requirements)
- [1. Get the module](#1-get-the-module)
- [2. Build the core](#2-build-the-core)
- [3. Apply the SQL](#3-apply-the-sql)
- [4. Build and install the client patch](#4-build-and-install-the-client-patch)
- [5. Start and check](#5-start-and-check)
- [Updating](#updating)
- [Uninstalling](#uninstalling)

Requirements
------------
- An AzerothCore (`azerothcore-wotlk`) source tree that you build yourself.
  The module uses the current script hook names (for example
  `OnPlayerLogin` and `OnPlayerIsClass`), so use a recent core. It builds on
  stock `azerothcore-wotlk` `master` and on the mod-playerbots fork
  (`Playerbot` branch), including forks based on it; no core patches are
  needed.
- MySQL or MariaDB, with the `mysql` command line client set as
  `MySQLExecutable` in `worldserver.conf` (the DB updater needs it).
- Python 3.9 or newer to build the client patch with
  [`scripts/build_runeforge_client_mpq.py`](../scripts/build_runeforge_client_mpq.py).
  It only uses the standard library; no `pip` packages are needed.
- A WoW 3.3.5a (build 12340) client for every player, and the server data
  (`DataDir`, with its `dbc/` folder) extracted from it with the AzerothCore
  extractors.

All SQL goes into the world database, called `acore_world` in the examples
below. The module does not change the auth or characters databases.

1. Get the module
-----------------
Clone it into the `modules/` folder of your AzerothCore source tree. Keep the
folder name `mod-dk`: the core derives the script loader name and the SQL
folder it reads from it.

```sh
cd <azerothcore>/modules
git clone https://github.com/dudemcbro/mod-dk.git
```

2. Build the core
-----------------
Re-run CMake so it picks up the new module, then build and install the core
as you normally do. For example, from your build directory:

```sh
cd <azerothcore>/build
cmake .. <your usual options>
make -j"$(nproc)"
make install
```

Modules are built into `worldserver` by default (`-DMODULES=static`). If you
set `-DMODULES=none`, the module is not built. `make install` also copies
[`conf/mod_dk.conf.dist`](../conf/mod_dk.conf.dist) to `etc/modules/`.

3. Apply the SQL
----------------
Nothing to do in most setups. The world SQL lives in
[`data/sql/db-world/`](../data/sql/db-world/), and the AzerothCore DB updater
applies it when `worldserver` starts, as long as the updater covers the world
database (`Updates.EnableDatabases`, default `7`) and can find the source tree
(`SourceDirectory`, empty by default, which means the tree the core was built
from). The updater remembers every file it applied and applies it again when a
module update changes it.

If you run `worldserver` with the updater turned off, or on a machine without
the source tree, apply the same files by hand with
[`sql/install.sql`](../sql/install.sql). It uses `SOURCE` with paths relative to
the AzerothCore source root, so run it from there:

```sh
cd <azerothcore>
mysql -u <user> -p acore_world < modules/mod-dk/sql/install.sql
```

Every file is re-runnable, and re-running them keeps the trainers and
runeforges placed with the GM commands:

| File | What it does |
| --- | --- |
| `mod_dk_01_balance_class_stats_1_60.sql` | DK base stats for levels 1-60 |
| `mod_dk_02_spell_progression.sql` | Creates and fills the `mod_dk_spell_progression` table |
| `mod_dk_03_trainer_spells_1_60.sql` | Spell list of TrainerId 1013 |
| `mod_dk_04_trainer_spawns.sql` | Madness trainer template (entry 900001) and TrainerId 1013 |
| `mod_dk_05_actionbar.sql` | Plague Strike next to Attack on the action bar |
| `mod_dk_06_start_armor_proficiencies.sql` | Cloth, leather and mail at creation |
| `mod_dk_07_start_skills.sql` | No Artisan First Aid, Plate or Dual Wield at creation |
| `mod_dk_08_start_warrior_gear.sql` | Removes DK gear rows from `playercreateinfo_item` |
| `mod_dk_09_spell_scaling_scripts.sql` | Registers the low level scaling spell scripts |
| `mod_dk_10_start_locations.sql` | DKs start in Acherus for the intro |
| `mod_dk_11_fix_rusted_chain_leggings.sql` | Removes the leggings' faction restriction |
| `mod_dk_12_start_outfit_dbc.sql` | CharStartOutfit override for the starter gear |
| `mod_dk_13_runeforge_template.sql` | Runeforge model size and the blue flame object (entry 900010) |
| `mod_dk_15_reentry_epl.sql` | Reentry NPC (entry 900000) and its gossip texts |
| `mod_dk_16_start_2h_only.sql` | 2H only start |
| `mod_dk_17_default_spawns.sql` | Madness and a runeforge in every starting zone and capital |

The DK stats file matters most: stock AzerothCore only has DK stats for levels
55-80, and `worldserver` exits at startup ("initial level does not have stats
data") when DKs start at a level without them.

New DKs start at level 1 and any account may create one; the module sets this
itself through `ModDk.StartLevel` and `ModDk.MinLevelForHeroicCharacter` (see
[Configuration](configuration.md)), which replace `StartHeroicPlayerLevel` and
`CharacterCreating.MinLevelForHeroicCharacter` from `worldserver.conf`. Set
`ModDk.StartLevel = 55` for the stock start: new DKs then play the stock Ebon
Hold intro and the low level scaling never applies, but they start with this
module's level 1 outfit and action bar.

4. Build and install the client patch
-------------------------------------
Players need a patched client `Spell.dbc`. Without it the client refuses
runeforging on low level weapons ("Target is too low level"), shows the stock
spell levels and shows unscaled damage in tooltips. Build it after the SQL is
applied, because the tool reads the trainer levels from the world database:

```sh
cd <azerothcore>/modules/mod-dk
export ACORE_MYSQL_PWD='<world db password>'
python3 scripts/build_runeforge_client_mpq.py --dbc-in <DataDir>/dbc/Spell.dbc
```

Each player copies the resulting `client/patch-d.MPQ` into the client's
`Data/` folder (next to `common.MPQ`) and deletes the client's `Cache/` folder.
See [The client patch](client-patch.md) for what it changes, its options and
the database password; the [Player guide](player-guide.md) walks players
through installing it.

5. Start and check
------------------
Start `worldserver`. The log shows:

```text
mod_dk: Scripts loaded.
mod_dk: New death knights start at level 1; creating one needs a level 0 character.
mod_dk: Loaded <n> spell progression rules.
mod_dk: Loaded <n> Ebon Hold intro quests for the reentry NPC.
mod_dk: Runeforge corrections applied to 10 spells and 10 enchants.
mod_dk: Auto learning turned off for <n> trainer DK spells.
mod_dk: Setup check passed.
```

Instead of `Setup check passed`, the startup check logs a `mod_dk: Setup
check:` warning for each problem it finds, with the file or tool that fixes
it. `.dk status` shows the same check in game, along with the current
settings. See [Troubleshooting](troubleshooting.md) if something is off.

Updating
--------
```sh
cd <azerothcore>/modules/mod-dk
git pull
```

Then:
1. Rebuild and install the core ([step 2](#2-build-the-core)). Compare your
   `etc/modules/mod_dk.conf` with the new `mod_dk.conf.dist` for new options.
2. Restart `worldserver`. The DB updater applies the SQL files that changed.
   (With the updater off, re-run `sql/install.sql` instead.) Trainers and
   runeforges placed by GMs stay.
3. Rebuild and redistribute the client patch ([step 4](#4-build-and-install-the-client-patch))
   if the trainer or scaling tables changed.

The starting skill changes (`mod_dk_07_start_skills.sql`) and the taxi patch
only apply to characters created afterwards. Existing DKs keep the First Aid,
Plate, Dual Wield and flight paths they already have.

Updating from an older version:
- The repository was republished with a fresh history in October 2026. A
  clone made before that cannot `git pull` ("refusing to merge unrelated
  histories"); run `git fetch origin && git reset --hard origin/main` in
  `modules/mod-dk` (this drops local changes in that folder), or clone it
  again.
- The SQL moved from `sql/db_world/base/` to `data/sql/db-world/` with new
  names, so the updater applies all of it once on the next start. That is
  safe, since every file is re-runnable.
- The module now sets the DK start level itself (`ModDk.StartLevel`, default
  1). If you want DKs to start at 55, set `ModDk.StartLevel = 55` or `-1`.
- `tools/patch_skillline_dk.py` and `mod_dk_14_runeforge_spell_dbc.sql` are
  gone: the module makes both changes in memory at startup, and
  `mod_dk_13_runeforge_template.sql` removes the old `spell_dbc` rows. You can
  put back the original server `SkillLineAbility.dbc`
  (`SkillLineAbility.dbc.bak`).

Uninstalling
------------
The rollback deletes the DK stats for levels 1-54, which stock AzerothCore does
not have, so `worldserver` exits at startup if new DKs are still set to start
below 55. Without the module, `StartHeroicPlayerLevel` from `worldserver.conf`
applies again: leave it at `55` (the default) or remove the line.

[`sql/rollback.sql`](../sql/rollback.sql) removes the module's rows, the
`mod_dk_spell_progression` table, every Madness trainer and runeforge (default
and GM placed), the reentry NPC and its texts, and the module's entries in the
DB updater's history, and restores the stock AzerothCore values the module
overwrote (DK start locations, action bars, creation spells and items, weapon
skills and class stats). Run it from the core root:

```sh
cd <azerothcore>
mysql -u <user> -p acore_world < modules/mod-dk/sql/rollback.sql
```

Then, before starting `worldserver` again (otherwise the updater installs
the module's SQL again):
- remove `modules/mod-dk`, rebuild and install the core,
- if an older version's `tools/patch_skillline_dk.py` patched your server
  `SkillLineAbility.dbc`, restore the original (`SkillLineAbility.dbc.bak`),
- revert `patches/dk-taxi-nodes.patch` if you applied it
  (`git apply -R modules/mod-dk/patches/dk-taxi-nodes.patch`),
- have players delete the patch MPQ from `Data/` and clear `Cache/`.

Existing DK characters keep the spells, items and positions they already
have.
