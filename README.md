mod-dk
======

An [AzerothCore](https://www.azerothcore.org/) module that turns the Death
Knight into a class you level from 1 to 60 (and on to 80) like any other:
with the core set to create DKs at level 1, they learn their spells from a
trainer and have their damage, healing and threat scaled down at low levels so
they do not outclass other classes early on.

- **Author:** [dudemcbro](https://github.com/dudemcbro)
- **License:** [GNU GPL v3.0](LICENSE)
- **Works with:** AzerothCore `master` and the mod-playerbots fork (WoW 3.3.5a, build 12340)

Contents
--------
- [Features](#features)
- [Requirements](#requirements)
- [Installation](#installation)
- [Configuration](#configuration)
- [GM commands](#gm-commands)
- [How it works](#how-it-works)
- [Python tools and the database password](#python-tools-and-the-database-password)
- [Updating](#updating)
- [Uninstalling](#uninstalling)
- [Optional core patches](#optional-core-patches)
- [Troubleshooting](#troubleshooting)
- [Repository layout](#repository-layout)
- [Contributing](#contributing)
- [License](#license)

Features
--------
Character creation and start:
- DK creation starts in Acherus so the core intro cinematic plays. Once it ends
  or is skipped, the DK is told where to find their trainer and sent to their
  racial starting zone, with the hearthstone bound there.
- Starter outfit (through a CharStartOutfit override, without duplicate bag
  items): Rusted Chain gloves, vest, leggings and boots, a long sleeved Gray
  Woolen Shirt, a Bastard Sword and a Hearthstone. Only the DK gear rows are
  removed from `playercreateinfo_item`, so starter items from other modules
  are kept.
- DKs start with cloth, leather and mail. Plate is trained at level 40.
- No Artisan First Aid or Dual Wield at creation: DKs train First Aid like
  everyone and Dual Wield at level 20.
- 2H only start: the one handed weapon skills are removed at creation.
- Plague Strike is known from the start and sits in the second action bar
  slot, next to Attack.
- DKs below level 55 may always leave Ebon Hold, and one found there on login
  (for example after logging out during the intro cinematic) is sent to their
  racial start.

Trainers and runeforges:
- Madness, the DK trainer, stands next to a priest trainer in every DK
  starting zone and capital city, with a runeforge close by. GMs can place more
  with `.dk madness add` and `.dk runeforge add`.
- Training costs gold on the Warrior price curve; the level 55-80 spells keep
  their stock prices.

Leveling and balance:
- DK base stats for levels 1-60 on the Warrior/Paladin curve (stock
  AzerothCore has no DK stats below 55).
- A spell progression table gates DK spells by level, and the Madness trainer
  (TrainerId 1013) follows the same schedule. Death Gate is trainable at 55.
- Low level damage and healing scaling (levels 1-54) so DKs do not outclass
  other classes, covering trained spells, talent abilities and the DK's minions.
- Low level Blood and Frost Presence scaling (levels 1-39), Death and Decay
  scaling and a tank threat bonus (levels 1-29) to keep early tanking in line
  with other classes.
- DKs keep their normal talent points everywhere, including Ebon Hold, and new
  weapon skills start at 1 like for other classes.

Runeforging:
- Runeforging works on low item level weapons and every rune takes effect at
  the level it is trained (server side at startup, client side through the
  [client patch](#4-build-and-install-the-client-patch)). Runes bind the weapon.

Reentry:
- The Ebon Herald at Light's Hope Chapel offers level 55+ DKs who never
  finished the Ebon Hold intro to play it from the start. It explains what
  happens, asks for confirmation, resets every intro quest, checks for a free
  quest log slot before the teleport and grants Death Gate, so a DK who gives
  up on the intro can still leave Ebon Hold.

For server admins:
- Install is clone, build, start: the AzerothCore DB updater applies the world
  SQL, the module sets the DK start level itself and changes the server data it
  needs in memory, so there is no `worldserver.conf` edit and no server DBC file
  to patch. Re-applying the SQL keeps the trainers and runeforges placed by GMs.
- A [config file](#configuration) turns features on and off and tunes the
  scaling without a rebuild.
- `.dk status` and a startup check report missing or outdated pieces of the
  setup; `.dk reload` reloads the progression table without a restart; list
  commands show every trainer and runeforge (see [GM commands](#gm-commands)).

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
  [`scripts/build_runeforge_client_mpq.py`](scripts/build_runeforge_client_mpq.py).
  It only uses the standard library; no `pip` packages are needed.
- A WoW 3.3.5a (build 12340) client for every player, and the server data
  (`DataDir`, with its `dbc/` folder) extracted from it with the AzerothCore
  extractors.

All SQL goes into the world database, called `acore_world` in the examples
below. The module does not change the auth or characters databases.

Installation
------------

### 1. Get the module

Clone it into the `modules/` folder of your AzerothCore source tree. Keep the
folder name `mod-dk`: the core derives the script loader name and the SQL
folder it reads from it.

```sh
cd <azerothcore>/modules
git clone https://github.com/dudemcbro/mod-dk.git
```

### 2. Build the core

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
[`conf/mod_dk.conf.dist`](conf/mod_dk.conf.dist) to `etc/modules/`.

### 3. Apply the SQL

Nothing to do in most setups. The world SQL lives in
[`data/sql/db-world/`](data/sql/db-world/), and the AzerothCore DB updater
applies it when `worldserver` starts, as long as the updater covers the world
database (`Updates.EnableDatabases`, default `7`) and can find the source tree
(`SourceDirectory`, empty by default, which means the tree the core was built
from). The updater remembers every file it applied and applies it again when a
module update changes it.

If you run `worldserver` with the updater turned off, or on a machine without
the source tree, apply the same files by hand with
[`sql/install.sql`](sql/install.sql). It uses `SOURCE` with paths relative to
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
[Configuration](#configuration)), which replace `StartHeroicPlayerLevel` and
`CharacterCreating.MinLevelForHeroicCharacter` from `worldserver.conf`. Set
`ModDk.StartLevel = 55` for the stock start: new DKs then play the stock Ebon
Hold intro and the low level scaling never applies, but they start with this
module's level 1 outfit and action bar.

### 4. Build and install the client patch

Players need a patched client `Spell.dbc`. Without it the client refuses
runeforging on low level weapons ("Target is too low level"), shows the stock
spell levels and shows unscaled damage in tooltips. The patched file is built
from your own game data and is not shipped with this module.

Build it after the SQL is applied, because the tool reads the trainer levels
from the world database:

```sh
cd <azerothcore>/modules/mod-dk
export ACORE_MYSQL_PWD='<world db password>'
python3 scripts/build_runeforge_client_mpq.py --dbc-in <DataDir>/dbc/Spell.dbc
```

Use the unmodified `Spell.dbc` that the extractor took from your client. The
file carries the spell text of that client's language, so build the patch from
the same locale your players use.

This writes (both are ignored by git):

- `client/patch-d.MPQ`: the file to give to players
- `client/DBFilesClient/Spell.dbc`: the patched DBC inside it

Change the output paths with `--mpq-out` and `--dbc-out`, and the database
connection with `--mysql`.

The patch:
- lets the client cast runeforge enchants on low level weapons,
- removes "in Ebon Hold" from the English rune descriptions, since runeforges
  stand in the world,
- shows the module's trainer levels as the spell level,
- makes the damage tooltips of the trained spells below level 55 follow the
  server scaling at the player's current level. Weapon strike tooltips match
  the server exactly; Icy Touch, Death Coil, Blood Boil and Death and Decay
  follow a straight line from the training level to 55, within a few percent
  of the server value. The client computes these numbers from per level fields
  in `Spell.dbc`; if a tooltip does not change as you level, build the patch
  with `--static-tooltips`, which keeps the value of the training level.

Talent abilities and minions are scaled on the server only; their tooltips
show full values.

Each player copies `patch-d.MPQ` into the client's `Data/` folder (next to
`common.MPQ`) and deletes the client's `Cache/` folder. If a `patch-d.MPQ`
already exists there, rename this one to another free patch letter, for
example `patch-e.MPQ`.

Do not copy the patched `Spell.dbc` into the server `DataDir`: the server
already scales the damage, so it would be scaled twice.

Rebuild and redistribute the patch whenever the trainer or scaling tables
change.

### 5. Start and check

Start `worldserver`. The log shows:

```text
mod_dk: Scripts loaded.
mod_dk: Loaded <n> spell progression rules.
mod_dk: Loaded <n> Ebon Hold intro quests for the reentry NPC.
mod_dk: Runeforge corrections applied to 10 spells and 10 enchants.
mod_dk: Auto learning turned off for <n> trainer DK spells.
mod_dk: Setup check passed.
```

Instead of `Setup check passed`, the startup check logs a `mod_dk: Setup
check:` warning for each problem it finds, with the file or tool that fixes
it. `.dk status` shows the same check in game, along with the current
settings.

Configuration
-------------
Settings live in `etc/modules/mod_dk.conf` (copy it from
`mod_dk.conf.dist`). Without the file, every option keeps its default. Apply
changes with `.reload config` or a restart; auras that are already active
(presences, diseases) keep their values until they are cast again.

| Option | Default | Effect |
| --- | --- | --- |
| `ModDk.StartLevel` | `1` | Level new DKs start at, replacing `StartHeroicPlayerLevel`; `-1` keeps the `worldserver.conf` value. Restart to change it |
| `ModDk.MinLevelForHeroicCharacter` | `0` | Character level an account needs before it can create a DK, replacing `CharacterCreating.MinLevelForHeroicCharacter`; `-1` keeps the `worldserver.conf` value. Restart to change it |
| `ModDk.Scaling.Enable` | `1` | Low level damage, healing, presence and minion scaling |
| `ModDk.Scaling.Strength` | `1.0` | How much of the scaling applies, from `0.0` (none) to `1.0` (the full curve). The client patch tooltips assume `1.0` |
| `ModDk.ThreatBonus.Enable` | `1` | Extra threat below level 30 in Blood or Frost Presence |
| `ModDk.SpellProgression.Enable` | `1` | Remove DK spells above the character's level and teach the level 1 spells |
| `ModDk.Reentry.Enable` | `1` | The Ebon Herald offers the Ebon Hold intro again |
| `ModDk.EbonHoldRescue.Enable` | `1` | Send DKs below 55 found in Ebon Hold on login to their racial start |
| `ModDk.Debug` | `0` | Log every scaled hit and threat bonus (logger `module`) |

GM commands
-----------
| Command | Alias | Level | Effect |
| --- | --- | --- | --- |
| `.dk runeforge add [entry]` | `.dk rf add` | GM (2) | Spawn a permanent runeforge at your position (default visual entry 202236) |
| `.dk runeforge remove` | `.dk rf remove` | GM (2) | Remove the targeted runeforge, or the nearest one within 15 yards |
| `.dk runeforge list` | `.dk rf list` | GM (2) | List the runeforges, nearest first |
| `.dk madness add` | `.dk md add` | GM (2) | Spawn a permanent Madness trainer at your position |
| `.dk madness remove` | `.dk md remove` | GM (2) | Remove the targeted Madness trainer, or the nearest one within 15 yards |
| `.dk madness list` | `.dk md list` | GM (2) | List the Madness trainers, nearest first |
| `.dk status` | | GM (2) | Settings and the setup check; also works from the console |
| `.dk reload` | | Admin (3) | Reload the spell progression table and the intro quest list and apply them to online DKs; also works from the console |

Spawns are saved to the world database and appear at once, no reload needed.
The list commands show up to 50 spawns with their GUID, map, position,
distance and whether they are a default spawn; go to one with
`.go creature <guid>` or `.go gameobject <guid>`.

A runeforge is three objects: a hidden runeforge focus (entry 190557) that
satisfies the "Requires Runeforge" check, the Frostmourne Altar model
(entry 202236, scaled to 0.8) and a harmless blue flame (entry 900010).
Runeforge removal only touches spawns tagged `mod-dk: runeforge`.

How it works
------------
Trainers and runeforges:
- Madness (creature entry 900001) uses TrainerId 1013.
  `mod_dk_17_default_spawns.sql` places one 2 yards to the right of a priest
  trainer, with a runeforge 2.5 yards further on, in Northshire, Coldridge
  Valley, Shadowglen, Ammen Vale, the Valley of Trials, Deathknell, Sunstrider
  Isle and Camp Narache, and in Stormwind, Ironforge, Darnassus, the Exodar,
  Orgrimmar, the Undercity, Thunder Bluff and Silvermoon City. Default spawns
  are tagged `(default)` in their `Comment`; re-applying the file replaces
  only those, so a default spawn removed in game comes back when the file
  changes in a module update.
- [mod-trainers](https://github.com/dudemcbro/mod-trainers) also points
  its Death Knight trainers at TrainerId 1013, so it needs this module's SQL.
- DKs do not auto-learn spells; they must train. The core would teach every
  DK class spell from `SkillLineAbility.dbc` at creation and on level up, so
  at startup the module turns that off in memory for the trainer spells
  (except Runeforging, whose flag grants the Runeforging skill). The DBC file
  itself stays untouched. With `ModDk.StartLevel = 55` the stock behavior is
  kept. Spells with `min_level` 1 in the progression table (Plague Strike) are
  taught on login.

Level schedule (1-60, TrainerId 1013):

| Level | Spells |
| --- | --- |
| 1-10 | Plague Strike (1), Blood Strike (2), Icy Touch (4), Runic Focus, Death Coil (8), Blood Presence (10) |
| 12-18 | Raise Dead, Plague/Blood Strike R2 (12), Blood Boil (14), Dark Command (16), Death Grip, Frost Presence (18) |
| 20-28 | Runeforging, Rune of Razorice, Icy Touch R2, Dual Wield (20), Chains of Ice, Plague/Blood Strike R3 (22), Death and Decay, Blood Boil R2 (26), Death Strike, Mind Freeze (28) |
| 30-38 | Strangulate (30), Pestilence, Death and Decay R2 (32), Plague/Blood Strike R4 (34), Death Strike R2, Rune of the Fallen Crusader, Forceful Deflection (36), Icy Touch R3 (38) |
| 40-48 | Icebound Fortitude, Runes of Swordbreaking and Swordshattering, Unholy Presence, Acherus Deathcharger, Rune Strike, Plate Mail (40), Anti-Magic Shell, Obliterate (44), Death and Decay R3, Plague/Blood Strike R5 (46), Empower Rune Weapon (48) |
| 50-60 | Blood Boil R3 (50), Death Strike R3 (52), Icy Touch R4, Rune of the Nerubian Carapace (54), Rune of the Stoneskin Gargoyle, Rune of Cinderglacier, Death Gate (55), Plague/Blood Strike R6 (56), Rune of Spellshattering (57), Death Coil R2, Rune of Spellbreaking (58), Army of the Dead, Death and Decay R4 (60) |

From 55 to 80 the trainer teaches the stock DK spells at their stock levels.
DKs learn riding at the normal riding trainers.

Scaling:
- `mod_dk_spell_progression` is enforced on login and level up: spells and
  auras above the character's level are removed.
- Low level damage scaling follows a curve from 0.08 at level 1 to 1.0 at
  level 55 (0.37 at 20, 0.55 at 30, 0.73 at 40) and applies to every target of
  AoE spells.
- Weapon strikes (Plague Strike, Blood Strike, Death Strike, Obliterate) keep
  their weapon damage and scale only the flat bonus by level / 55.
- Icy Touch, Death Coil and Blood Boil get an extra multiplier (0.9, 0.8, 0.8)
  that fades out as the level scale reaches 1.0.
- Diseases and Death and Decay are scaled when their amount is calculated, so a
  refreshed disease or a Death and Decay over a pack keep the scaled value.
- Everything else from a DK below 55 is scaled by the same curve: talent
  abilities (Heart Strike, Scourge Strike, Frost Strike, Howling Blast, Corpse
  Explosion, ...), DK heals such as Death Strike and Rune Tap, and the DK's
  minions (ghoul, Army of the Dead, Bloodworms, gargoyle).
- Blood and Frost Presence (including the Frost Presence stamina aura) scale
  from 0.6 at level 1 to 1.0 at level 40; threat effects stay at full strength.
  The Blood Presence heal follows the same curve, and below level 30 its flat
  DBC fallback heal (about 30 HP, older cores) is suppressed.
- Below level 30 in Blood or Frost Presence, DK damage spells add threat that
  tapers off by level. Icy Touch and Blood Boil get a little extra, and Death
  and Decay ticks get a small bonus to keep early AoE pulls stable.
- `ModDk.Scaling.Strength` moves every scale toward 1.0.

Runeforging:
- Runeforge spells can target low level weapons, and rune enchants have their
  level and Runeforging rank requirements removed at startup, so stat runes
  (parry, defense, spell deflection) work at the level they are trained.

Core hooks (`OnPlayerIsClass`):
- DKs use the normal talent point formula everywhere; the stock Ebon Hold
  formula would reset the talents of a DK leveled from 1.
- New weapon skills start at 1.
- DKs below level 55 may leave Ebon Hold without Death Gate (the core
  otherwise keeps a DK there until the intro is done), hearthstone included.
- With `patches/dk-taxi-nodes.patch`, DKs discover flight paths like other
  classes.

The intro and Ebon Hold:
- The module does not change the core DK intro. It waits for the cinematic to
  finish (or be skipped), tells the DK where Madness waits and sends them to
  their racial start.
- Starter armor proficiencies are applied through
  `playercreateinfo_cast_spell`, so `PlayerStart.CustomSpells` is not needed.
- The Ebon Herald (entry 900000) resets every quest of the Scarlet Enclave
  zone plus the capital city quests that end the intro before it sends a DK
  back to Acherus.

Python tools and the database password
--------------------------------------
| Tool | Purpose |
| --- | --- |
| [`scripts/build_runeforge_client_mpq.py`](scripts/build_runeforge_client_mpq.py) | Build the client patch ([step 4](#4-build-and-install-the-client-patch)) |

Run it with `--help` for all options.

The tool queries the world database through the `mysql` client. By default it
runs:

```sh
mysql -u acore -D acore_world -N -B
```

Set the password in the `ACORE_MYSQL_PWD` environment variable instead of
writing it into a command:

```sh
export ACORE_MYSQL_PWD='<world db password>'
```

The tool hands it to the client as `MYSQL_PWD`, so it never appears on the
`mysql` command line, where other users on the machine could read it from the
process list.

For another user, host or database, pass your own command with `--mysql`.
Keep `-N -B` (the tool parses tab separated output without headers) and leave
`-p` out, so the password still comes from the variable:

```sh
python3 scripts/build_runeforge_client_mpq.py --dbc-in <DataDir>/dbc/Spell.dbc \
    --mysql 'mysql -h 127.0.0.1 -u <user> -D acore_world -N -B'
```

You can also keep the credentials in a MySQL option file and pass
`--defaults-extra-file=<file>` as the first option of your `--mysql` command.

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

[`sql/rollback.sql`](sql/rollback.sql) removes the module's rows, the
`mod_dk_spell_progression` table, every Madness trainer and runeforge (default
and GM placed), the reentry NPC and its texts, and the module's entries in the
DB updater's history, and restores the stock AzerothCore values the module
overwrote (DK start locations, action bars, creation spells, weapon skills
and class stats). Run it from the core root:

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

Optional core patches
---------------------
Stock AzerothCore gives every Death Knight all Eastern Kingdoms and Kalimdor
flight paths, without asking any script.
[`patches/dk-taxi-nodes.patch`](patches/dk-taxi-nodes.patch) routes that
through `OnPlayerIsClass(CLASS_CONTEXT_TAXI)`, which this module answers so
DKs discover flight paths like other classes. It only affects characters
created after it is applied (known nodes are saved). Apply it from the root of
your AzerothCore source tree and rebuild:

```sh
cd <azerothcore>
git apply --check modules/mod-dk/patches/dk-taxi-nodes.patch
git apply modules/mod-dk/patches/dk-taxi-nodes.patch
```

[`patches/runeforge-fail-logging.patch`](patches/runeforge-fail-logging.patch)
logs DK enchant item cast failures, to find the runeforge spell behind a
"Target is too low level" error. It is a debugging aid and not needed for
normal use. Apply it from the root of your AzerothCore source tree and
rebuild:

```sh
cd <azerothcore>
git apply --check modules/mod-dk/patches/runeforge-fail-logging.patch
git apply modules/mod-dk/patches/runeforge-fail-logging.patch
```

It logs at warning level under the `spells.runeforge` logger. The default
`worldserver.conf` only logs errors for loggers it does not list, so add:

```ini
Logger.spells.runeforge=3,Console Server
```

Remove it again with `git apply -R modules/mod-dk/patches/runeforge-fail-logging.patch`.

Troubleshooting
---------------
Start with `.dk status` in game (or `dk status` on the console). It lists every
problem it finds with the file or tool that fixes it.

| Symptom | Cause and fix |
| --- | --- |
| `worldserver` exits with "initial level does not have stats data" | DKs start below 55 but `mod_dk_01_balance_class_stats_1_60.sql` is not applied. Check that the DB updater covers the world database, or apply `sql/install.sql` by hand. |
| No `mod_dk_` lines in the DB updater output | The updater only reads modules it was built with and needs the source tree: check `-DMODULES`, `SourceDirectory`, `Updates.EnableDatabases` and `MySQLExecutable`. |
| "Target is too low level" when runeforging | The player's client has no patch MPQ, or `Cache/` was not cleared after copying it. |
| Players see "You have learned" then "You have unlearned" on level up | A trainer spell is still auto learned; `.dk status` names the count and `.dk reload` fixes it. |
| New DKs start at 55 although `StartHeroicPlayerLevel = 1` | `ModDk.StartLevel` replaces that setting; set it in `mod_dk.conf` (or `-1` to use `worldserver.conf`). |
| Tooltips do not change as the character levels | Rebuild the client patch with `--static-tooltips`; the server scaling is not affected. |
| A default trainer or runeforge stands in a wall | Remove it with `.dk madness remove` / `.dk runeforge remove` and place one with `.dk madness add` / `.dk runeforge add`. |
| Config changes have no effect | The file must be `etc/modules/mod_dk.conf` (not `.dist`); apply with `.reload config`. Active presences and diseases keep their values until cast again. |

Repository layout
-----------------
| Path | Contents |
| --- | --- |
| `src/` | The module's C++ scripts and GM commands |
| `conf/mod_dk.conf.dist` | Module settings |
| `data/sql/db-world/` | World SQL, applied by the DB updater |
| `sql/install.sql`, `sql/rollback.sql` | Manual install and the uninstall |
| `scripts/` | Python tool that builds the client patch |
| `.github/workflows/` | CI: builds AzerothCore master with the module on every push and pull request |
| `patches/` | Optional core patches |
| `icon.png` | Icon for the AzerothCore module catalogue |

The core builds every `.cpp` file in `src/`; the module needs no CMake file.

Contributing
------------
Bug reports and suggestions are welcome as
[issues](https://github.com/dudemcbro/mod-dk/issues). To contribute code:

1. Fork the repository and create a branch.
2. Keep the SQL re-runnable (the DB updater applies a file again whenever it
   changes) and add matching steps to
   [`sql/dk_rollback.sql`](sql/dk_rollback.sql) for anything you change.
3. Never commit Blizzard game data (`.dbc`, `.MPQ`); build it from your own
   client with the tools instead.
4. Build the core with your change and test it in game.
5. Open a [pull request](https://github.com/dudemcbro/mod-dk/pulls) that
   explains what changed and why, using
   [Conventional Commits](https://www.conventionalcommits.org/) style commit
   messages (for example `fix(sql): ...`).

License
-------
Licensed under the GNU General Public License v3.0. See [LICENSE](LICENSE).
