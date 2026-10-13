-- DK start with 2H only, 1H trainable later
-- DK class bit: 32 (1 << (6-1))
--
-- Stock rows for 1H skills are shared with other classes, for example
-- (0,37,44,'Axes') covers Warrior+Hunter+DK and (0,39,43,'Swords') covers
-- Warrior+Paladin+Hunter+DK. Only the DK bit is removed so the other classes
-- keep their starting skills. Re-running this file is a no-op.

-- Shared rows: strip the DK bit only
UPDATE playercreateinfo_skills
SET classMask = classMask & ~32
WHERE (classMask & 32) != 0
  AND classMask != 32
  AND skill IN (43,44,54);

-- DK-only rows (if any): remove
DELETE FROM playercreateinfo_skills
WHERE classMask = 32
  AND skill IN (43,44,54);

-- Ensure DKs start with 2H maces and 2H swords (they already have 2H axes)
INSERT IGNORE INTO playercreateinfo_skills (raceMask, classMask, skill, `rank`, comment) VALUES
(0, 32, 160, 0, 'mod-dk: Two-Handed Maces'),
(0, 32, 55, 0, 'mod-dk: Two-Handed Swords');
