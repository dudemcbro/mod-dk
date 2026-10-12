-- DK trainer spells for levels 1-80 (TrainerId 1013)
-- Custom 1-60 progression + WotLK 55-80 from stock trainer (TrainerId 13)
-- Note: Runeforge spells require Runeforging (53428) as prereq ability.
-- Costs: 1-60 follow the Warrior trainer's price curve; 55-80 stock rows keep stock prices.
-- Keep the levels in sync with mod_dk_02_spell_progression.sql.

DELETE FROM `trainer_spell` WHERE `TrainerId` = 1013;

INSERT INTO `trainer_spell` (`TrainerId`, `SpellId`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `ReqLevel`, `VerifiedBuild`) VALUES
-- ===== CUSTOM 1-60 PROGRESSION =====
-- Level 1-10
  (1013, 45462, 0, 0, 0, 0, 0, 0, 1, 0),  -- Plague Strike R1
  (1013, 45902, 40, 0, 0, 0, 0, 0, 2, 0),  -- Blood Strike R1
  (1013, 45477, 100, 0, 0, 0, 0, 0, 4, 0),  -- Icy Touch R1
  (1013, 61455, 350, 0, 0, 0, 0, 0, 8, 0),  -- Runic Focus
  (1013, 47541, 350, 0, 0, 0, 0, 0, 8, 0),  -- Death Coil R1
  (1013, 48266, 600, 0, 0, 0, 0, 0, 10, 0),  -- Blood Presence
-- Level 12
  (1013, 46584, 1280, 0, 0, 0, 0, 0, 12, 0),  -- Raise Dead
  (1013, 49917, 1280, 0, 0, 0, 0, 0, 12, 0),  -- Plague Strike R2
  (1013, 49926, 1280, 0, 0, 0, 0, 0, 12, 0),  -- Blood Strike R2
-- Level 14-18
  (1013, 48721, 1960, 0, 0, 0, 0, 0, 14, 0),  -- Blood Boil R1
  (1013, 56222, 2640, 0, 0, 0, 0, 0, 16, 0),  -- Dark Command
  (1013, 49576, 3320, 0, 0, 0, 0, 0, 18, 0),  -- Death Grip
  (1013, 48263, 3320, 0, 0, 0, 0, 0, 18, 0),  -- Frost Presence
-- Level 20
  (1013, 53428, 4000, 0, 0, 0, 0, 0, 20, 0),  -- Runeforging
  (1013, 53343, 4000, 0, 0, 53428, 0, 0, 20, 0),  -- Rune of Razorice
  (1013, 49896, 4000, 0, 0, 0, 0, 0, 20, 0),  -- Icy Touch R2
  (1013, 674, 4000, 0, 0, 0, 0, 0, 20, 0),  -- Dual Wield
-- Level 22
  (1013, 45524, 5750, 0, 0, 0, 0, 0, 22, 0),  -- Chains of Ice
  (1013, 49918, 5750, 0, 0, 0, 0, 0, 22, 0),  -- Plague Strike R3
  (1013, 49927, 5750, 0, 0, 0, 0, 0, 22, 0),  -- Blood Strike R3
-- Level 26-28
  (1013, 43265, 9250, 0, 0, 0, 0, 0, 26, 0),  -- Death and Decay R1
  (1013, 49939, 9250, 0, 0, 0, 0, 0, 26, 0),  -- Blood Boil R2
  (1013, 49998, 11000, 0, 0, 0, 0, 0, 28, 0),  -- Death Strike R1
  (1013, 47528, 11000, 0, 0, 0, 0, 0, 28, 0),  -- Mind Freeze
-- Level 30-32
  (1013, 47476, 12750, 0, 0, 0, 0, 0, 30, 0),  -- Strangulate
  (1013, 50842, 14500, 0, 0, 0, 0, 0, 32, 0),  -- Pestilence
  (1013, 49936, 14500, 0, 0, 0, 0, 0, 32, 0),  -- Death and Decay R2
-- Level 34
  (1013, 49919, 16250, 0, 0, 0, 0, 0, 34, 0),  -- Plague Strike R4
  (1013, 49928, 16250, 0, 0, 0, 0, 0, 34, 0),  -- Blood Strike R4
-- Level 36
  (1013, 49999, 18000, 0, 0, 0, 0, 0, 36, 0),  -- Death Strike R2
  (1013, 53344, 18000, 0, 0, 53428, 0, 0, 36, 0),  -- Rune of the Fallen Crusader
  (1013, 49410, 18000, 0, 0, 0, 0, 0, 36, 0),  -- Forceful Deflection
-- Level 38
  (1013, 49903, 21430, 0, 0, 0, 0, 0, 38, 0),  -- Icy Touch R3
-- Level 40
  (1013, 48792, 24860, 0, 0, 0, 0, 0, 40, 0),  -- Icebound Fortitude
  (1013, 54446, 24860, 0, 0, 53428, 0, 0, 40, 0),  -- Rune of Swordbreaking
  (1013, 53323, 24860, 0, 0, 53428, 0, 0, 40, 0),  -- Rune of Swordshattering
  (1013, 48265, 24860, 0, 0, 0, 0, 0, 40, 0),  -- Unholy Presence
  (1013, 48778, 24860, 0, 0, 0, 0, 0, 40, 0),  -- Acherus Deathcharger
  (1013, 56815, 24860, 0, 0, 0, 0, 0, 40, 0),  -- Rune Strike
  (1013, 750, 24860, 0, 0, 0, 0, 0, 40, 0),  -- Plate Mail (armor proficiency)
-- Level 44-46
  (1013, 48707, 31710, 0, 0, 0, 0, 0, 44, 0),  -- Anti-Magic Shell
  (1013, 49020, 31710, 0, 0, 0, 0, 0, 44, 0),  -- Obliterate R1
  (1013, 49937, 35140, 0, 0, 0, 0, 0, 46, 0),  -- Death and Decay R3
  (1013, 49920, 35140, 0, 0, 0, 0, 0, 46, 0),  -- Plague Strike R5
  (1013, 49929, 35140, 0, 0, 0, 0, 0, 46, 0),  -- Blood Strike R5
-- Level 48
  (1013, 47568, 38570, 0, 0, 0, 0, 0, 48, 0),  -- Empower Rune Weapon
-- Level 50-55
  (1013, 49940, 42000, 0, 0, 0, 0, 0, 50, 0),  -- Blood Boil R3
  (1013, 45463, 46500, 0, 0, 0, 0, 0, 52, 0),  -- Death Strike R3
  (1013, 49904, 51000, 0, 0, 0, 0, 0, 54, 0),  -- Icy Touch R4
  (1013, 70164, 51000, 0, 0, 53428, 0, 0, 54, 0),  -- Rune of the Nerubian Carapace
  (1013, 62158, 53250, 0, 0, 53428, 0, 0, 55, 0),  -- Rune of the Stoneskin Gargoyle
  (1013, 50977, 53250, 0, 0, 0, 0, 0, 55, 0),  -- Death Gate (the reentry NPC also grants it)
-- Level 56-60
  (1013, 49921, 55500, 0, 0, 0, 0, 0, 56, 0),  -- Plague Strike R6
  (1013, 49930, 55500, 0, 0, 0, 0, 0, 56, 0),  -- Blood Strike R6
  (1013, 49892, 60000, 0, 0, 0, 0, 0, 58, 0),  -- Death Coil R2
  (1013, 54447, 60000, 0, 0, 53428, 0, 0, 58, 0),  -- Rune of Spellbreaking
  (1013, 42650, 65000, 0, 0, 0, 0, 0, 60, 0),  -- Army of the Dead
  (1013, 49938, 65000, 0, 0, 0, 0, 0, 60, 0),  -- Death and Decay R4

-- ===== WOTLK 55-80 PROGRESSION (from stock trainer 13) =====
-- No duplicates with custom 1-60 section above
  (1013, 53341, 55000, 0, 0, 53428, 0, 0, 55, 0),  -- Rune of Cinderglacier
  (1013, 53342, 5700, 0, 0, 53428, 0, 0, 57, 0),  -- Rune of Spellshattering
  (1013, 55258, 5900, 0, 0, 0, 0, 0, 59, 0),  -- Heart Strike R2
  (1013, 51325, 300, 0, 0, 0, 0, 0, 60, 0),  -- Howling Blast R2
  (1013, 53331, 6000, 0, 0, 53428, 0, 0, 60, 0),  -- Rune of Lichbane
  (1013, 3714, 61000, 0, 0, 0, 0, 0, 61, 0),  -- Path of Frost
  (1013, 45529, 64000, 0, 0, 0, 0, 0, 64, 0),  -- Blood Tap
  (1013, 55259, 3200, 0, 0, 0, 0, 0, 64, 0),  -- Heart Strike R3
  (1013, 51417, 3250, 0, 0, 0, 0, 0, 65, 0),  -- Frost Strike R3
  (1013, 57330, 65000, 0, 0, 0, 0, 0, 65, 0),  -- Horn of Winter R1
  (1013, 48743, 66000, 0, 0, 0, 0, 0, 66, 0),  -- Death Pact
  (1013, 51423, 67000, 0, 0, 0, 0, 0, 67, 0),  -- Obliterate R2
  (1013, 55265, 18000, 0, 0, 0, 0, 0, 67, 0),  -- Corpse Explosion R2
  (1013, 49893, 68000, 0, 0, 0, 0, 0, 68, 0),  -- Death Coil R3
  (1013, 55260, 3450, 0, 0, 0, 0, 0, 69, 0),  -- Heart Strike R4
  (1013, 51326, 18000, 0, 0, 0, 0, 0, 70, 0),  -- Howling Blast R3
  (1013, 51409, 6500, 0, 0, 0, 0, 0, 70, 0),  -- Scourge Strike R2
  (1013, 51418, 18000, 0, 0, 0, 0, 0, 70, 0),  -- Frost Strike R4
  (1013, 61999, 360000, 0, 0, 0, 0, 0, 72, 0),  -- Raise Ally
  (1013, 51424, 360000, 0, 0, 0, 0, 0, 73, 0),  -- Obliterate R3
  (1013, 55270, 18000, 0, 0, 0, 0, 0, 73, 0),  -- Corpse Explosion R3
  (1013, 55261, 18000, 0, 0, 0, 0, 0, 74, 0),  -- Heart Strike R5
  (1013, 49923, 360000, 0, 0, 0, 0, 0, 75, 0),  -- Death Strike R4
  (1013, 51327, 18000, 0, 0, 0, 0, 0, 75, 0),  -- Howling Blast R4
  (1013, 51410, 10000, 0, 0, 0, 0, 0, 75, 0),  -- Scourge Strike R3
  (1013, 51419, 18000, 0, 0, 0, 0, 0, 75, 0),  -- Frost Strike R5
  (1013, 57623, 360000, 0, 0, 0, 0, 0, 75, 0),  -- Horn of Winter R2
  (1013, 49894, 360000, 0, 0, 0, 0, 0, 76, 0),  -- Death Coil R4
  (1013, 49909, 360000, 0, 0, 0, 0, 0, 78, 0),  -- Icy Touch R5
  (1013, 49941, 360000, 0, 0, 0, 0, 0, 78, 0),  -- Blood Boil R4
  (1013, 51425, 360000, 0, 0, 0, 0, 0, 79, 0),  -- Obliterate R4
  (1013, 55271, 18000, 0, 0, 0, 0, 0, 79, 0),  -- Corpse Explosion R4
  (1013, 49895, 360000, 0, 0, 0, 0, 0, 80, 0),  -- Death Coil R5
  (1013, 49924, 360000, 0, 0, 0, 0, 0, 80, 0),  -- Death Strike R5
  (1013, 51328, 18000, 0, 0, 0, 0, 0, 80, 0),  -- Howling Blast R5
  (1013, 51411, 10000, 0, 0, 0, 0, 0, 80, 0),  -- Scourge Strike R4
  (1013, 55262, 18000, 0, 0, 0, 0, 0, 80, 0),  -- Heart Strike R6
  (1013, 55268, 18000, 0, 0, 0, 0, 0, 80, 0);  -- Frost Strike R6
