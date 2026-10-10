-- Madness, the DK trainer (creature entry 900001, TrainerId 1013).
-- Spawns are not touched here, so re-running this file (or the core DB updater
-- re-applying it) keeps trainers placed with `.dk madness add`. Default spawns
-- next to the priest trainers are in mod_dk_17_default_spawns.sql.
SET @DK_TRAINER_ENTRY := 900001;
SET @DK_TRAINER_DISPLAY := 25907;

DELETE FROM `creature_template_model` WHERE `CreatureID` = @DK_TRAINER_ENTRY;
DELETE FROM `creature_template` WHERE `entry` = @DK_TRAINER_ENTRY;
DELETE FROM `creature_default_trainer` WHERE `CreatureId` = @DK_TRAINER_ENTRY;
DELETE FROM `trainer` WHERE `Id` = 1013;

INSERT INTO `creature_template` (`entry`, `name`, `subname`, `minlevel`, `maxlevel`, `faction`, `npcflag`, `ScriptName`) VALUES
(@DK_TRAINER_ENTRY, 'Madness', 'Death Knight Trainer', 55, 55, 35, 16, '');

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`) VALUES
(@DK_TRAINER_ENTRY, 0, @DK_TRAINER_DISPLAY, 1, 1);

INSERT INTO `creature_default_trainer` (`CreatureId`, `TrainerId`) VALUES
(@DK_TRAINER_ENTRY, 1013);

INSERT INTO `trainer` (`Id`, `Type`, `Requirement`, `Greeting`) VALUES
(1013, 0, 6, 'Hello, death knight! Ready for some training?');
