-- Rollback for mod-dk world changes (acore_world)
-- Reverses every file in data/sql/db-world. Where the module overwrote stock rows,
-- the stock AzerothCore values (data/sql/base/db_world) are restored.

-- 1) Remove custom DK reentry NPC and DK trainer (module entries), including
--    the default trainers and any spawns placed with `.dk madness add`
-- The spawn's entry column is creature.id1 on older cores and creature.id on
-- AzerothCore since its 2026_06_16_00 update; use whichever this database has.
SET @CREATURE_ENTRY_COL := IF(EXISTS (SELECT 1 FROM information_schema.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'creature' AND COLUMN_NAME = 'id1'), 'id1', 'id');
SET @SQL := CONCAT('DELETE FROM creature WHERE `', @CREATURE_ENTRY_COL, '` IN (900000, 900001)');
PREPARE stmt FROM @SQL;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;
DELETE FROM creature_template WHERE entry IN (900000, 900001);
DELETE FROM creature_template_model WHERE CreatureID IN (900000, 900001);
DELETE FROM creature_default_trainer WHERE CreatureId = 900001;
DELETE FROM trainer WHERE Id = 1013;

-- 2) Remove trainer spell list for DK trainer (TrainerId 1013)
DELETE FROM trainer_spell WHERE TrainerId = 1013;

-- 3) Remove spell script registrations added by this module
DELETE FROM spell_script_names WHERE ScriptName IN
(
  'spell_mod_dk_low_level_damage',
  'spell_mod_dk_low_level_dot',
  'spell_mod_dk_presence_scale',
  'spell_mod_dk_dnd_scale',
  'spell_mod_dk_blood_presence_heal'
);

-- 4) Remove the module-owned spell progression table
DROP TABLE IF EXISTS mod_dk_spell_progression;

-- 5) Restore stock DK action bars (class 6)
DELETE FROM playercreateinfo_action WHERE class = 6;
INSERT INTO playercreateinfo_action (race, class, button, action, type) VALUES
(1,6,0,6603,0),
(1,6,1,49576,0),
(1,6,2,45477,0),
(1,6,3,45462,0),
(1,6,4,45902,0),
(1,6,5,47541,0),
(1,6,11,59752,0),
(2,6,0,6603,0),
(2,6,1,49576,0),
(2,6,2,45477,0),
(2,6,3,45462,0),
(2,6,4,45902,0),
(2,6,5,47541,0),
(2,6,10,20572,0),
(3,6,0,6603,0),
(3,6,1,49576,0),
(3,6,2,45477,0),
(3,6,3,45462,0),
(3,6,4,45902,0),
(3,6,5,47541,0),
(3,6,10,2481,0),
(4,6,0,6603,0),
(4,6,1,49576,0),
(4,6,2,45477,0),
(4,6,3,45462,0),
(4,6,4,45902,0),
(4,6,5,47541,0),
(4,6,10,58984,0),
(5,6,0,6603,0),
(5,6,1,49576,0),
(5,6,2,45477,0),
(5,6,3,45462,0),
(5,6,4,45902,0),
(5,6,5,47541,0),
(5,6,10,20577,0),
(6,6,0,6603,0),
(6,6,1,49576,0),
(6,6,2,45477,0),
(6,6,3,45462,0),
(6,6,4,45902,0),
(6,6,5,47541,0),
(6,6,10,20549,0),
(7,6,0,6603,0),
(7,6,1,49576,0),
(7,6,2,45477,0),
(7,6,3,45462,0),
(7,6,4,45902,0),
(7,6,5,47541,0),
(7,6,10,20589,0),
(8,6,0,6603,0),
(8,6,1,49576,0),
(8,6,2,45477,0),
(8,6,3,45462,0),
(8,6,4,45902,0),
(8,6,5,47541,0),
(8,6,10,26297,0),
(10,6,0,6603,0),
(10,6,1,49576,0),
(10,6,2,45477,0),
(10,6,3,45462,0),
(10,6,4,45902,0),
(10,6,5,47541,0),
(10,6,6,50613,0),
(11,6,0,6603,0),
(11,6,1,49576,0),
(11,6,2,45477,0),
(11,6,3,45462,0),
(11,6,4,45902,0),
(11,6,5,47541,0),
(11,6,10,59545,0);

-- 6) Remove DK armor proficiencies added for starter gear and restore the
--    stock Blood Presence creation cast
DELETE FROM playercreateinfo_cast_spell
WHERE (classMask & 32) != 0
  AND spell IN (8737, 9077, 9078, 48266);
INSERT INTO playercreateinfo_cast_spell (raceMask, classMask, spell, note) VALUES
(0, 32, 48266, 'Death Knight - Blood Presence');

-- 7) Undo 2H-only skill adjustments
-- Rows added by this module (current and older versions of mod_dk_16_start_2h_only.sql)
DELETE FROM playercreateinfo_skills
WHERE raceMask = 0 AND classMask = 32 AND skill IN (55, 160) AND comment LIKE 'mod-dk:%';
DELETE FROM playercreateinfo_skills
WHERE raceMask = 0 AND classMask = 37 AND skill IN (55, 160);
-- Put the DK bit back on the shared stock 1H rows (or recreate them if an
-- older version of this module deleted them)
DELETE FROM playercreateinfo_skills WHERE raceMask = 0 AND classMask IN (5, 37) AND skill = 44;
DELETE FROM playercreateinfo_skills WHERE raceMask = 0 AND classMask IN (7, 39) AND skill = 43;
INSERT INTO playercreateinfo_skills (raceMask, classMask, skill, `rank`, comment) VALUES
(0, 37, 44, 0, 'Axes'),
(0, 39, 43, 0, 'Swords');

-- 7b) Undo mod_dk_07_start_skills.sql: Artisan First Aid, Plate and Dual Wield at creation
DELETE FROM playercreateinfo_skills WHERE raceMask = 0 AND classMask = 32 AND skill IN (129, 293);
INSERT INTO playercreateinfo_skills (raceMask, classMask, skill, `rank`, comment) VALUES
(0, 32, 129, 4, 'Death Knight - First Aid'),
(0, 32, 293, 0, 'Plate');
DELETE FROM playercreateinfo_skills WHERE raceMask = 0 AND classMask IN (8, 40) AND skill = 118;
INSERT INTO playercreateinfo_skills (raceMask, classMask, skill, `rank`, comment) VALUES
(0, 40, 118, 0, 'Dual Wield');

-- 8) Restore stock DK start locations (class 6)
REPLACE INTO playercreateinfo (race, class, map, zone, position_x, position_y, position_z, orientation) VALUES
(1,6,609,4298,2355.84,-5664.77,426.028,3.65997),
(2,6,609,4298,2358.44,-5666.9,426.023,3.65997),
(3,6,609,4298,2358.44,-5666.9,426.023,3.65997),
(4,6,609,4298,2356.21,-5662.21,426.026,3.65997),
(5,6,609,4298,2356.21,-5662.21,426.026,3.65997),
(6,6,609,4298,2358.17,-5663.21,426.027,3.65997),
(7,6,609,4298,2355.05,-5661.7,426.026,3.65997),
(8,6,609,4298,2355.05,-5661.7,426.026,3.65997),
(10,6,609,4298,2355.84,-5664.77,426.028,3.65997),
(11,6,609,4298,2358.17,-5663.21,426.027,3.65997);

-- 9) Remove DK starter gear rows from playercreateinfo_item (class 6) and CharStartOutfit overrides
DELETE FROM playercreateinfo_item
WHERE class = 6
  AND itemid IN (2391, 2386, 2388, 2389, 2587, 1194, 6948, 40582);
-- Stock row: strip the Scourgestone from the stock DK outfit
INSERT INTO playercreateinfo_item (race, class, itemid, amount, Note) VALUES
(0, 6, 40582, -1, '[TDB PH] - unsused Scourgestone');

-- CharStartOutfit overrides use IDs 60001-60020 (stock charstartoutfit_dbc has no rows)
DELETE FROM charstartoutfit_dbc WHERE ID BETWEEN 60001 AND 60020;

-- 10) Restore Rusted Chain Leggings faction restriction (stock FlagsExtra = 1)
UPDATE `item_template`
SET `FlagsExtra` = (`FlagsExtra` | 1)
WHERE `entry` = 2388;

-- 11) Restore runeforge template size (stock 1.0)
UPDATE `gameobject_template`
SET `size` = 1
WHERE `entry` = 202236;

-- 11b) Remove the default runeforges, those placed with `.dk runeforge add` and the
--      custom aura template
DELETE FROM `gameobject` WHERE `Comment` LIKE 'mod-dk: runeforge%';
DELETE FROM `gameobject_template`
WHERE `entry` = 900010;

-- 12) Remove the runeforging spell_dbc overrides older versions of the module added
--     (stock spell_dbc has no rows for these IDs, so the server Spell.dbc applies again).
DELETE FROM `spell_dbc`
WHERE `ID` IN (53323, 53331, 53341, 53342, 53343, 53344, 54446, 54447, 62158, 70164);

-- 13) Restore stock DK class stats: stock only defines levels 55-80.
--     Set StartHeroicPlayerLevel back to 55 first, or worldserver exits at startup
--     ("initial level does not have stats data").
DELETE FROM `player_class_stats` WHERE `Class` = 6 AND `Level` BETWEEN 1 AND 54;
REPLACE INTO `player_class_stats` (`Class`, `Level`, `BaseHP`, `BaseMana`, `Strength`, `Agility`, `Stamina`, `Intellect`, `Spirit`) VALUES
(6,55,1359,0,108,73,99,29,42),
(6,56,1421,0,111,75,102,29,43),
(6,57,1485,0,113,76,104,29,43),
(6,58,1551,0,115,77,106,30,44),
(6,59,1619,0,118,79,108,30,44),
(6,60,1689,0,120,80,110,30,45);

-- 14) Remove the reentry NPC's gossip texts
DELETE FROM `npc_text` WHERE `ID` IN (900000, 900001);

-- 15) Forget the module's files in the DB updater's history, so they are applied
--     again if the module is installed again later
DELETE FROM `updates` WHERE `name` LIKE 'mod\_dk\_%.sql';
