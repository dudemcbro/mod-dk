-- Adjust default runeforge model size
UPDATE `gameobject_template`
SET `size` = 0.8
WHERE `entry` = 202236;

-- Create a safe blue flame aura without a linked trap (copy of Scourge Bonfire)
REPLACE INTO `gameobject_template` (
  `entry`, `type`, `displayId`, `name`, `IconName`, `castBarCaption`, `unk1`, `size`,
  `Data0`, `Data1`, `Data2`, `Data3`, `Data4`, `Data5`, `Data6`, `Data7`, `Data8`, `Data9`,
  `Data10`, `Data11`, `Data12`, `Data13`, `Data14`, `Data15`, `Data16`, `Data17`, `Data18`,
  `Data19`, `Data20`, `Data21`, `Data22`, `Data23`, `AIName`, `ScriptName`, `VerifiedBuild`
)
SELECT
  900010, `type`, `displayId`, 'mod-dk Runeforge Aura', `IconName`, `castBarCaption`, `unk1`, `size`,
  `Data0`, `Data1`, 0, `Data3`, `Data4`, `Data5`, `Data6`, `Data7`, `Data8`, `Data9`,
  `Data10`, `Data11`, `Data12`, `Data13`, `Data14`, `Data15`, `Data16`, `Data17`, `Data18`,
  `Data19`, `Data20`, `Data21`, `Data22`, `Data23`, `AIName`, `ScriptName`, `VerifiedBuild`
FROM `gameobject_template`
WHERE `entry` = 188542;

-- Shrink the runeforge aura object (custom entry) to 35% of default size
UPDATE `gameobject_template`
SET `size` = 0.35
WHERE `entry` = 900010;

-- Older versions of this module also overrode the rune spells in spell_dbc. The module
-- now makes those changes in memory at startup, so drop the old rows (stock spell_dbc
-- has none for these IDs).
DELETE FROM `spell_dbc` WHERE `ID` IN (53323, 53331, 53341, 53342, 53343, 53344, 54446, 54447, 62158, 70164);
