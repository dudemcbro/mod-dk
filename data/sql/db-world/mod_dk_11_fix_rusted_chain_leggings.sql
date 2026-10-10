-- Fix Rusted Chain Leggings faction restriction so DKs can equip them
UPDATE `item_template`
SET `FlagsExtra` = (`FlagsExtra` & 0xFFFFFFFE)
WHERE `entry` = 2388;
