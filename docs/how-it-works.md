How it works
============

- [Character creation and the intro](#character-creation-and-the-intro)
- [Trainers and runeforges](#trainers-and-runeforges)
- [Level schedule](#level-schedule)
- [Scaling](#scaling)
- [Runeforging](#runeforging)
- [Core hooks](#core-hooks)
- [The reentry NPC](#the-reentry-npc)

Character creation and the intro
--------------------------------
- DK creation starts in Acherus, so the core intro cinematic plays. The module
  does not change the core DK intro: it waits for the cinematic to finish (or
  be skipped), tells the DK where Madness waits and sends them to their racial
  start, with the hearthstone bound there.
- Starter outfit (through a CharStartOutfit override, without duplicate bag
  items): Rusted Chain gloves, vest, leggings and boots, a long sleeved Gray
  Woolen Shirt, a Bastard Sword and a Hearthstone. Only the DK gear rows are
  removed from `playercreateinfo_item`, so starter items from other modules
  are kept.
- DKs start with cloth, leather and mail; plate is trained at level 40. The
  armor proficiencies are applied through `playercreateinfo_cast_spell`, so
  `PlayerStart.CustomSpells` is not needed.
- No Artisan First Aid or Dual Wield at creation: DKs train First Aid like
  everyone and Dual Wield at level 20. The one handed weapon skills are
  removed at creation (2H only start).
- Plague Strike is known from the start and sits in the second action bar
  slot, next to Attack.
- DK base stats for levels 1-60 follow the Warrior/Paladin curve; stock
  AzerothCore has no DK stats below 55.
- A DK below level 55 found in Ebon Hold on login (for example after logging
  out during the intro cinematic) is sent to their racial start.

Trainers and runeforges
-----------------------
- Madness (creature entry 900001) uses TrainerId 1013.
  `mod_dk_17_default_spawns.sql` places one 2 yards to the right of a priest
  trainer, with a runeforge 2.5 yards further on, in Northshire, Coldridge
  Valley, Shadowglen, Ammen Vale, the Valley of Trials, Deathknell, Sunstrider
  Isle and Camp Narache, and in Stormwind, Ironforge, Darnassus, the Exodar,
  Orgrimmar, the Undercity, Thunder Bluff and Silvermoon City. Default spawns
  are tagged `(default)` in their `Comment`; re-applying the file replaces
  only those, so a default spawn removed in game comes back when the file
  changes in a module update.
- Training costs gold on the Warrior price curve; the level 55-80 spells keep
  their stock prices.
- [mod-trainers](https://github.com/dudemcbro/mod-trainers) also points
  its Death Knight trainers at TrainerId 1013, so it needs this module's SQL.
- DKs do not auto-learn spells; they must train. The core would teach every
  DK class spell from `SkillLineAbility.dbc` at creation and on level up, so
  at startup the module turns that off in memory for the trainer spells
  (except Runeforging, whose flag grants the Runeforging skill). The DBC file
  itself stays untouched. With `ModDk.StartLevel = 55` the stock behavior is
  kept. Spells with `min_level` 1 in the progression table (Plague Strike) are
  taught on login.

Level schedule
--------------
TrainerId 1013, levels 1-60:

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

Scaling
-------
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

Runeforging
-----------
Runeforge spells can target low level weapons, and rune enchants have their
level and Runeforging rank requirements removed at startup, so stat runes
(parry, defense, spell deflection) work at the level they are trained. Runes
bind the weapon. All of this happens in memory, without `spell_dbc` rows; the
client side comes from the [client patch](client-patch.md).

Core hooks
----------
The module answers the core `OnPlayerIsClass` hook:
- DKs use the normal talent point formula everywhere; the stock Ebon Hold
  formula would reset the talents of a DK leveled from 1.
- New weapon skills start at 1.
- DKs below level 55 may leave Ebon Hold without Death Gate (the core
  otherwise keeps a DK there until the intro is done), hearthstone included.
- With [`patches/dk-taxi-nodes.patch`](core-patches.md), DKs discover flight
  paths like other classes.

`ModDk.StartLevel` and `ModDk.MinLevelForHeroicCharacter` are applied while
the core loads its configuration at startup, before the player create data
and its stats check are loaded.

The reentry NPC
---------------
The Ebon Herald (entry 900000) stands at Light's Hope Chapel. It offers level
55+ DKs who never finished the Ebon Hold intro to play it from the start: it
explains what happens in its own gossip text, asks for confirmation, checks for
a free quest log slot before the teleport, grants Death Gate (so a DK who gives
up on the intro can still leave Ebon Hold) and resets every quest of the
Scarlet Enclave zone plus the capital city quests that end the intro. Other
players get a text explaining who can answer the call.
