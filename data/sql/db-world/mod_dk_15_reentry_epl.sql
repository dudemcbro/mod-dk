-- DK reentry NPC near Light's Hope Chapel (neutral)
SET @DK_NPC_ENTRY := 900000;
SET @DK_NPC_GUID := 9000000;
SET @DK_DISPLAY_ID := 25907; -- Death Knight Champion (NPC 29101) display ID from wowhead (WotLK).

-- The spawn's entry column is creature.id1 on older cores and creature.id on
-- AzerothCore since its 2026_06_16_00 update; use whichever this database has.
SET @CREATURE_ENTRY_COL := IF(EXISTS (SELECT 1 FROM information_schema.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'creature' AND COLUMN_NAME = 'id1'), 'id1', 'id');
SET @SQL := CONCAT('DELETE FROM `creature` WHERE `guid` = ? OR `', @CREATURE_ENTRY_COL, '` = ?');
PREPARE stmt FROM @SQL;
EXECUTE stmt USING @DK_NPC_GUID, @DK_NPC_ENTRY;
DEALLOCATE PREPARE stmt;
DELETE FROM `creature_template_model` WHERE `CreatureID` = @DK_NPC_ENTRY;
DELETE FROM `creature_template` WHERE `entry` = @DK_NPC_ENTRY;

INSERT INTO `creature_template` (`entry`, `name`, `subname`, `minlevel`, `maxlevel`, `faction`, `npcflag`, `ScriptName`)
VALUES
(@DK_NPC_ENTRY, 'Ebon Herald', 'Death Knight Call', 55, 55, 35, 3, 'npc_dk_reentry');

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`)
VALUES
(@DK_NPC_ENTRY, 0, @DK_DISPLAY_ID, 1, 1);

INSERT INTO `creature` (`guid`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`, `CreateObject`, `Comment`)
VALUES
(@DK_NPC_GUID, 0, 0, 0, 1, 1, 0, 2245.50, -5319.00, 82.20, 1.60, 120, 0, 0, 1, 0, 0, 0, 0, 0, '', 12340, 1, 'DK reentry NPC - Light''s Hope Chapel');

SET @SQL := CONCAT('UPDATE `creature` SET `', @CREATURE_ENTRY_COL, '` = ? WHERE `guid` = ?');
PREPARE stmt FROM @SQL;
EXECUTE stmt USING @DK_NPC_ENTRY, @DK_NPC_GUID;
DEALLOCATE PREPARE stmt;

-- Gossip texts shown by npc_dk_reentry ($N is the player's name, $B a line break)
DELETE FROM `npc_text` WHERE `ID` IN (900000, 900001);
INSERT INTO `npc_text` (`ID`, `text0_0`, `text0_1`, `Probability0`) VALUES
(900000, 'The Ebon Blade remembers every knight who never answered the Lich King''s call, $N.$B$BI can send you back to Acherus to live the Ebon Hold story from its first quest. Your Ebon Hold quest progress starts over; your talents, gear and gold stay as they are.$B$BYou will know Death Gate when you arrive, so you can leave whenever you wish.', 'The Ebon Blade remembers every knight who never answered the Lich King''s call, $N.$B$BI can send you back to Acherus to live the Ebon Hold story from its first quest. Your Ebon Hold quest progress starts over; your talents, gear and gold stay as they are.$B$BYou will know Death Gate when you arrive, so you can leave whenever you wish.', 1),
(900001, 'Only a death knight of level 55 or higher who never finished the Ebon Hold story can answer the call again.$B$BGo in peace, $N.', 'Only a death knight of level 55 or higher who never finished the Ebon Hold story can answer the call again.$B$BGo in peace, $N.', 1);
