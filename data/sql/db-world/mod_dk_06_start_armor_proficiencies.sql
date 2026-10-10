-- DK armor proficiencies for level 1 starter gear
-- Ensures DKs can equip cloth/leather/mail at creation.
-- Plate is trained later (see mod_dk_03_trainer_spells_1_60.sql).

DELETE FROM `playercreateinfo_cast_spell`
WHERE (classmask & 32) != 0
  AND `Spell` IN (8737, 9077, 9078);

INSERT IGNORE INTO `playercreateinfo_cast_spell` (`racemask`, `classmask`, `spell`) VALUES
-- All DK races
(0, 32, 9078),
(0, 32, 9077),
(0, 32, 8737);

-- Remove Blood Presence from DK creation auto-cast.
-- It is trained at level 10; must not be force-applied at character creation.
DELETE FROM `playercreateinfo_cast_spell`
WHERE (classmask & 32) != 0
  AND `spell` = 48266;
