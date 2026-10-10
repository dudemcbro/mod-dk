-- Default Madness trainers (entry 900001) and runeforges, one of each in every DK
-- starting zone and capital city. Madness stands right next to a priest trainer.
--
-- Re-runnable: only the rows tagged "(default)" are replaced, so trainers and
-- runeforges placed with `.dk madness add` / `.dk runeforge add` stay. The spawns
-- get fresh GUIDs above the highest one in use. Remove or move a default spawn
-- in game with `.dk madness remove` / `.dk runeforge remove`; re-running this file
-- (or the DB updater re-applying it after a module update) puts it back.
--
-- Positions come from the priest's spawn: Madness 2 yards to the priest's right,
-- the runeforge group (focus, Frostmourne Altar, blue flame) 4.5 yards to the right.

DELETE FROM `creature` WHERE `Comment` = 'mod-dk: madness (default)';
DELETE FROM `gameobject` WHERE `Comment` LIKE 'mod-dk: runeforge-% (default)';

SET @CGUID := (SELECT COALESCE(MAX(`guid`), 0) FROM `creature`);
SET @OGUID := (SELECT COALESCE(MAX(`guid`), 0) FROM `gameobject`);

INSERT INTO `creature` (`guid`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`, `CreateObject`, `Comment`) VALUES
(@CGUID+1, 0, 0, 0, 1, 1, 0, -8852.472, -191.678, 82.116, 2.548, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0, 0, 'mod-dk: madness (default)'), -- Northshire (Human start), next to Priestess Anetta
(@CGUID+2, 0, 0, 0, 1, 1, 0, -6057.770, 395.262, 392.843, 3.683, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0, 0, 'mod-dk: madness (default)'), -- Coldridge Valley (Dwarf and Gnome start), next to Branstock Khalder
(@CGUID+3, 1, 0, 0, 1, 1, 0, 10457.653, 803.261, 1346.840, 3.752, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0, 0, 'mod-dk: madness (default)'), -- Shadowglen (Night Elf start), next to Shanda
(@CGUID+4, 530, 0, 0, 1, 1, 0, -4120.011, -13763.844, 73.588, 0.512, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0, 0, 'mod-dk: madness (default)'), -- Ammen Vale (Draenei start), next to Zalduun
(@CGUID+5, 1, 0, 0, 1, 1, 0, -619.370, -4202.713, 38.297, 4.869, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0, 0, 'mod-dk: madness (default)'), -- Valley of Trials (Orc and Troll start), next to Ken'jai
(@CGUID+6, 0, 0, 0, 1, 1, 0, 1847.938, 1629.593, 97.017, 3.334, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0, 0, 'mod-dk: madness (default)'), -- Deathknell (Undead start), next to Dark Cleric Duesten
(@CGUID+7, 530, 0, 0, 1, 1, 0, 10372.018, -6426.877, 38.615, 3.334, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0, 0, 'mod-dk: madness (default)'), -- Sunstrider Isle (Blood Elf start), next to Matron Arena
(@CGUID+8, 1, 0, 0, 1, 1, 0, -2893.775, -251.339, 56.770, 3.124, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0, 0, 'mod-dk: madness (default)'), -- Camp Narache (Tauren start), next to the Priest Trainer
(@CGUID+9, 0, 0, 0, 1, 1, 0, -8513.258, 863.919, 109.927, 3.822, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0, 0, 'mod-dk: madness (default)'), -- Stormwind Cathedral, next to High Priestess Laurena
(@CGUID+10, 0, 0, 0, 1, 1, 0, -4614.124, -908.349, 501.146, 4.328, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0, 0, 'mod-dk: madness (default)'), -- Ironforge Mystic Ward, next to High Priest Rohan
(@CGUID+11, 1, 0, 0, 1, 1, 0, 9653.016, 2538.734, 1331.600, 3.927, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0, 0, 'mod-dk: madness (default)'), -- Darnassus Temple of the Moon, next to Jandria
(@CGUID+12, 530, 0, 0, 1, 1, 0, -3969.718, -11471.853, -135.113, 4.102, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0, 0, 'mod-dk: madness (default)'), -- The Exodar, next to Caedmos
(@CGUID+13, 1, 0, 0, 1, 1, 0, 1451.980, -4181.769, 44.377, 6.056, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0, 0, 'mod-dk: madness (default)'), -- Orgrimmar Valley of Spirits, next to Ur'kyo
(@CGUID+14, 0, 0, 0, 1, 1, 0, 1766.468, 401.188, -57.131, 4.398, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0, 0, 'mod-dk: madness (default)'), -- Undercity, next to Aelthalyste
(@CGUID+15, 1, 0, 0, 1, 1, 0, -958.006, 253.357, 97.876, 4.224, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0, 0, 'mod-dk: madness (default)'), -- Thunder Bluff Spirit Rise, next to Miles Welsh
(@CGUID+16, 530, 0, 0, 1, 1, 0, 9939.802, -7071.764, 47.799, 1.484, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0, 0, 'mod-dk: madness (default)'); -- Silvermoon City, next to Lotheolan

-- The spawn's entry column is creature.id1 on older cores and creature.id on
-- AzerothCore since its 2026_06_16_00 update; use whichever this database has.
SET @CREATURE_ENTRY_COL := IF(EXISTS (SELECT 1 FROM information_schema.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'creature' AND COLUMN_NAME = 'id1'), 'id1', 'id');
SET @SQL := CONCAT('UPDATE `creature` SET `', @CREATURE_ENTRY_COL, '` = 900001 WHERE `Comment` = ''mod-dk: madness (default)''');
PREPARE stmt FROM @SQL;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `VerifiedBuild`, `Comment`) VALUES
(@OGUID+1, 190557, 0, 0, 0, 1, 1, -8851.488, -189.326, 81.616, 2.548, 0, 0, 0.956305, 0.292372, 300, 0, 1, '', 0, 'mod-dk: runeforge-focus (default)'), -- Northshire (Human start), next to Priestess Anetta
(@OGUID+2, 202236, 0, 0, 0, 1, 1, -8851.488, -189.326, 82.216, 2.548, 0, 0, 0.956305, 0.292372, 300, 0, 1, '', 0, 'mod-dk: runeforge-visual (default)'),
(@OGUID+3, 900010, 0, 0, 0, 1, 1, -8851.488, -189.326, 82.216, 2.548, 0, 0, 0.956305, 0.292372, 300, 0, 1, '', 0, 'mod-dk: runeforge-aura (default)'),
(@OGUID+4, 190557, 0, 0, 0, 1, 1, -6059.486, 397.148, 392.343, 3.683, 0, 0, 0.963630, -0.267241, 300, 0, 1, '', 0, 'mod-dk: runeforge-focus (default)'), -- Coldridge Valley (Dwarf and Gnome start), next to Branstock Khalder
(@OGUID+5, 202236, 0, 0, 0, 1, 1, -6059.486, 397.148, 392.943, 3.683, 0, 0, 0.963630, -0.267241, 300, 0, 1, '', 0, 'mod-dk: runeforge-visual (default)'),
(@OGUID+6, 900010, 0, 0, 0, 1, 1, -6059.486, 397.148, 392.943, 3.683, 0, 0, 0.963630, -0.267241, 300, 0, 1, '', 0, 'mod-dk: runeforge-aura (default)'),
(@OGUID+7, 190557, 1, 0, 0, 1, 1, 10455.809, 805.022, 1346.340, 3.752, 0, 0, 0.953717, -0.300707, 300, 0, 1, '', 0, 'mod-dk: runeforge-focus (default)'), -- Shadowglen (Night Elf start), next to Shanda
(@OGUID+8, 202236, 1, 0, 0, 1, 1, 10455.809, 805.022, 1346.940, 3.752, 0, 0, 0.953717, -0.300707, 300, 0, 1, '', 0, 'mod-dk: runeforge-visual (default)'),
(@OGUID+9, 900010, 1, 0, 0, 1, 1, 10455.809, 805.022, 1346.940, 3.752, 0, 0, 0.953717, -0.300707, 300, 0, 1, '', 0, 'mod-dk: runeforge-aura (default)'),
(@OGUID+10, 190557, 530, 0, 0, 1, 1, -4118.350, -13765.779, 73.088, 0.512, 0, 0, 0.253104, 0.967439, 300, 0, 1, '', 0, 'mod-dk: runeforge-focus (default)'), -- Ammen Vale (Draenei start), next to Zalduun
(@OGUID+11, 202236, 530, 0, 0, 1, 1, -4118.350, -13765.779, 73.688, 0.512, 0, 0, 0.253104, 0.967439, 300, 0, 1, '', 0, 'mod-dk: runeforge-visual (default)'),
(@OGUID+12, 900010, 530, 0, 0, 1, 1, -4118.350, -13765.779, 73.688, 0.512, 0, 0, 0.253104, 0.967439, 300, 0, 1, '', 0, 'mod-dk: runeforge-aura (default)'),
(@OGUID+13, 190557, 1, 0, 0, 1, 1, -621.761, -4203.598, 37.797, 4.869, 0, 0, 0.649448, -0.760406, 300, 0, 1, '', 0, 'mod-dk: runeforge-focus (default)'), -- Valley of Trials (Orc and Troll start), next to Ken'jai
(@OGUID+14, 202236, 1, 0, 0, 1, 1, -621.761, -4203.598, 38.397, 4.869, 0, 0, 0.649448, -0.760406, 300, 0, 1, '', 0, 'mod-dk: runeforge-visual (default)'),
(@OGUID+15, 900010, 1, 0, 0, 1, 1, -621.761, -4203.598, 38.397, 4.869, 0, 0, 0.649448, -0.760406, 300, 0, 1, '', 0, 'mod-dk: runeforge-aura (default)'),
(@OGUID+16, 190557, 0, 0, 0, 1, 1, 1846.971, 1631.952, 96.517, 3.334, 0, 0, 0.995396, -0.095846, 300, 0, 1, '', 0, 'mod-dk: runeforge-focus (default)'), -- Deathknell (Undead start), next to Dark Cleric Duesten
(@OGUID+17, 202236, 0, 0, 0, 1, 1, 1846.971, 1631.952, 97.117, 3.334, 0, 0, 0.995396, -0.095846, 300, 0, 1, '', 0, 'mod-dk: runeforge-visual (default)'),
(@OGUID+18, 900010, 0, 0, 0, 1, 1, 1846.971, 1631.952, 97.117, 3.334, 0, 0, 0.995396, -0.095846, 300, 0, 1, '', 0, 'mod-dk: runeforge-aura (default)'),
(@OGUID+19, 190557, 530, 0, 0, 1, 1, 10371.051, -6424.518, 38.115, 3.334, 0, 0, 0.995396, -0.095846, 300, 0, 1, '', 0, 'mod-dk: runeforge-focus (default)'), -- Sunstrider Isle (Blood Elf start), next to Matron Arena
(@OGUID+20, 202236, 530, 0, 0, 1, 1, 10371.051, -6424.518, 38.715, 3.334, 0, 0, 0.995396, -0.095846, 300, 0, 1, '', 0, 'mod-dk: runeforge-visual (default)'),
(@OGUID+21, 900010, 530, 0, 0, 1, 1, 10371.051, -6424.518, 38.715, 3.334, 0, 0, 0.995396, -0.095846, 300, 0, 1, '', 0, 'mod-dk: runeforge-aura (default)'),
(@OGUID+22, 190557, 1, 0, 0, 1, 1, -2894.231, -248.831, 56.270, 3.124, 0, 0, 0.999962, 0.008726, 300, 0, 1, '', 0, 'mod-dk: runeforge-focus (default)'), -- Camp Narache (Tauren start), next to the Priest Trainer
(@OGUID+23, 202236, 1, 0, 0, 1, 1, -2894.231, -248.831, 56.870, 3.124, 0, 0, 0.999962, 0.008726, 300, 0, 1, '', 0, 'mod-dk: runeforge-visual (default)'),
(@OGUID+24, 900010, 1, 0, 0, 1, 1, -2894.231, -248.831, 56.870, 3.124, 0, 0, 0.999962, 0.008726, 300, 0, 1, '', 0, 'mod-dk: runeforge-aura (default)'),
(@OGUID+25, 190557, 0, 0, 0, 1, 1, -8515.220, 865.547, 109.427, 3.822, 0, 0, 0.942687, -0.333679, 300, 0, 1, '', 0, 'mod-dk: runeforge-focus (default)'), -- Stormwind Cathedral, next to High Priestess Laurena
(@OGUID+26, 202236, 0, 0, 0, 1, 1, -8515.220, 865.547, 110.027, 3.822, 0, 0, 0.942687, -0.333679, 300, 0, 1, '', 0, 'mod-dk: runeforge-visual (default)'),
(@OGUID+27, 900010, 0, 0, 0, 1, 1, -8515.220, 865.547, 110.027, 3.822, 0, 0, 0.942687, -0.333679, 300, 0, 1, '', 0, 'mod-dk: runeforge-aura (default)'),
(@OGUID+28, 190557, 0, 0, 0, 1, 1, -4616.630, -907.876, 500.646, 4.328, 0, 0, 0.829037, -0.559194, 300, 0, 1, '', 0, 'mod-dk: runeforge-focus (default)'), -- Ironforge Mystic Ward, next to High Priest Rohan
(@OGUID+29, 202236, 0, 0, 0, 1, 1, -4616.630, -907.876, 501.246, 4.328, 0, 0, 0.829037, -0.559194, 300, 0, 1, '', 0, 'mod-dk: runeforge-visual (default)'),
(@OGUID+30, 900010, 0, 0, 0, 1, 1, -4616.630, -907.876, 501.246, 4.328, 0, 0, 0.829037, -0.559194, 300, 0, 1, '', 0, 'mod-dk: runeforge-aura (default)'),
(@OGUID+31, 190557, 1, 0, 0, 1, 1, 9650.894, 2540.148, 1331.100, 3.927, 0, 0, 0.923880, -0.382683, 300, 0, 1, '', 0, 'mod-dk: runeforge-focus (default)'), -- Darnassus Temple of the Moon, next to Jandria
(@OGUID+32, 202236, 1, 0, 0, 1, 1, 9650.894, 2540.148, 1331.700, 3.927, 0, 0, 0.923880, -0.382683, 300, 0, 1, '', 0, 'mod-dk: runeforge-visual (default)'),
(@OGUID+33, 900010, 1, 0, 0, 1, 1, 9650.894, 2540.148, 1331.700, 3.927, 0, 0, 0.923880, -0.382683, 300, 0, 1, '', 0, 'mod-dk: runeforge-aura (default)'),
(@OGUID+34, 190557, 530, 0, 0, 1, 1, -3972.053, -11470.828, -135.613, 4.102, 0, 0, 0.887012, -0.461747, 300, 0, 1, '', 0, 'mod-dk: runeforge-focus (default)'), -- The Exodar, next to Caedmos
(@OGUID+35, 202236, 530, 0, 0, 1, 1, -3972.053, -11470.828, -135.013, 4.102, 0, 0, 0.887012, -0.461747, 300, 0, 1, '', 0, 'mod-dk: runeforge-visual (default)'),
(@OGUID+36, 900010, 530, 0, 0, 1, 1, -3972.053, -11470.828, -135.013, 4.102, 0, 0, 0.887012, -0.461747, 300, 0, 1, '', 0, 'mod-dk: runeforge-aura (default)'),
(@OGUID+37, 190557, 1, 0, 0, 1, 1, 1451.905, -4184.317, 43.877, 6.056, 0, 0, 0.113204, -0.993572, 300, 0, 1, '', 0, 'mod-dk: runeforge-focus (default)'), -- Orgrimmar Valley of Spirits, next to Ur'kyo
(@OGUID+38, 202236, 1, 0, 0, 1, 1, 1451.905, -4184.317, 44.477, 6.056, 0, 0, 0.113204, -0.993572, 300, 0, 1, '', 0, 'mod-dk: runeforge-visual (default)'),
(@OGUID+39, 900010, 1, 0, 0, 1, 1, 1451.905, -4184.317, 44.477, 6.056, 0, 0, 0.113204, -0.993572, 300, 0, 1, '', 0, 'mod-dk: runeforge-aura (default)'),
(@OGUID+40, 190557, 0, 0, 0, 1, 1, 1763.936, 401.485, -57.631, 4.398, 0, 0, 0.809017, -0.587785, 300, 0, 1, '', 0, 'mod-dk: runeforge-focus (default)'), -- Undercity, next to Aelthalyste
(@OGUID+41, 202236, 0, 0, 0, 1, 1, 1763.936, 401.485, -57.031, 4.398, 0, 0, 0.809017, -0.587785, 300, 0, 1, '', 0, 'mod-dk: runeforge-visual (default)'),
(@OGUID+42, 900010, 0, 0, 0, 1, 1, 1763.936, 401.485, -57.031, 4.398, 0, 0, 0.809017, -0.587785, 300, 0, 1, '', 0, 'mod-dk: runeforge-aura (default)'),
(@OGUID+43, 190557, 1, 0, 0, 1, 1, -960.448, 254.089, 97.376, 4.224, 0, 0, 0.857166, -0.515039, 300, 0, 1, '', 0, 'mod-dk: runeforge-focus (default)'), -- Thunder Bluff Spirit Rise, next to Miles Welsh
(@OGUID+44, 202236, 1, 0, 0, 1, 1, -960.448, 254.089, 97.975, 4.224, 0, 0, 0.857166, -0.515039, 300, 0, 1, '', 0, 'mod-dk: runeforge-visual (default)'),
(@OGUID+45, 900010, 1, 0, 0, 1, 1, -960.448, 254.089, 97.975, 4.224, 0, 0, 0.857166, -0.515039, 300, 0, 1, '', 0, 'mod-dk: runeforge-aura (default)'),
(@OGUID+46, 190557, 530, 0, 0, 1, 1, 9942.336, -7071.484, 47.299, 1.484, 0, 0, 0.675590, 0.737277, 300, 0, 1, '', 0, 'mod-dk: runeforge-focus (default)'), -- Silvermoon City, next to Lotheolan
(@OGUID+47, 202236, 530, 0, 0, 1, 1, 9942.336, -7071.484, 47.899, 1.484, 0, 0, 0.675590, 0.737277, 300, 0, 1, '', 0, 'mod-dk: runeforge-visual (default)'),
(@OGUID+48, 900010, 530, 0, 0, 1, 1, 9942.336, -7071.484, 47.899, 1.484, 0, 0, 0.675590, 0.737277, 300, 0, 1, '', 0, 'mod-dk: runeforge-aura (default)');
