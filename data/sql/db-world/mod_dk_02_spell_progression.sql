-- DK spell progression for levels 1-80 (module controlled)
-- Controls which spells are USABLE at each level.
-- WotLK 55-80 spells are gated by trainer ReqLevel; only custom 1-60 spells
-- and spells moved below their stock level need entries here.
-- Keep the levels in sync with mod_dk_03_trainer_spells_1_60.sql.
CREATE TABLE IF NOT EXISTS `mod_dk_spell_progression` (
  `spell_id` INT UNSIGNED NOT NULL,
  `min_level` TINYINT UNSIGNED NOT NULL,
  `note` VARCHAR(100) NULL,
  PRIMARY KEY (`spell_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DELETE FROM `mod_dk_spell_progression`;

INSERT INTO `mod_dk_spell_progression` (`spell_id`, `min_level`, `note`) VALUES
-- ===== CORE 1-60 PROGRESSION =====
(45462,  1, 'Plague Strike R1'),
(45902,  2, 'Blood Strike R1'),
(45477,  4, 'Icy Touch R1'),
(61455,  8, 'Runic Focus'),
(47541,  8, 'Death Coil R1'),
(48266, 10, 'Blood Presence'),
(46584, 12, 'Raise Dead'),
(49917, 12, 'Plague Strike R2'),
(49926, 12, 'Blood Strike R2'),
(48721, 14, 'Blood Boil R1'),
(56222, 16, 'Dark Command'),
(49576, 18, 'Death Grip'),
(48263, 18, 'Frost Presence'),
(53428, 20, 'Runeforging'),
(53343, 20, 'Rune of Razorice'),
(49896, 20, 'Icy Touch R2'),
(674,   20, 'Dual Wield'),
(45524, 22, 'Chains of Ice'),
(49918, 22, 'Plague Strike R3'),
(49927, 22, 'Blood Strike R3'),
(43265, 26, 'Death and Decay R1'),
(49939, 26, 'Blood Boil R2'),
(49998, 28, 'Death Strike R1'),
(47528, 28, 'Mind Freeze'),
(47476, 30, 'Strangulate'),
(50842, 32, 'Pestilence'),
(49936, 32, 'Death and Decay R2'),
(49919, 34, 'Plague Strike R4'),
(49928, 34, 'Blood Strike R4'),
(49999, 36, 'Death Strike R2'),
(53344, 36, 'Rune of the Fallen Crusader'),
(49410, 36, 'Forceful Deflection'),
(49903, 38, 'Icy Touch R3'),
(48792, 40, 'Icebound Fortitude'),
(54446, 40, 'Rune of Swordbreaking'),
(53323, 40, 'Rune of Swordshattering'),
(48265, 40, 'Unholy Presence'),
(48778, 40, 'Acherus Deathcharger'),
(56815, 40, 'Rune Strike'),
(750,   40, 'Plate Mail (armor proficiency)'),
(48707, 44, 'Anti-Magic Shell'),
(49020, 44, 'Obliterate R1'),
(49937, 46, 'Death and Decay R3'),
(49920, 46, 'Plague Strike R5'),
(49929, 46, 'Blood Strike R5'),
(47568, 48, 'Empower Rune Weapon'),
(49940, 50, 'Blood Boil R3'),
(45463, 52, 'Death Strike R3'),
(49904, 54, 'Icy Touch R4'),
(70164, 54, 'Rune of the Nerubian Carapace'),
(62158, 55, 'Rune of the Stoneskin Gargoyle'),
(50977, 55, 'Death Gate'),
(49921, 56, 'Plague Strike R6'),
(49930, 56, 'Blood Strike R6'),
(49892, 58, 'Death Coil R2'),
(54447, 58, 'Rune of Spellbreaking'),
(42650, 60, 'Army of the Dead'),
(49938, 60, 'Death and Decay R4');
