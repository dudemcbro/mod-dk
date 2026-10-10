-- DK starting skills that stock AzerothCore hands out at creation.
-- DK class bit: 32 (1 << (6-1)). Only affects characters created afterwards.
--
-- - First Aid: stock gives DKs Artisan rank (max 300) at creation, which skips the
--   level gates other classes have on higher bandages. DKs now train it like everyone.
-- - Plate: stock gives DKs Plate at creation. It is trained at level 40 instead
--   (Plate Mail, spell 750, TrainerId 1013).
-- - Dual Wield: shared with Rogues in stock ((0,40,118)). Only the DK bit is removed;
--   DKs train Dual Wield (spell 674) at level 20 like Warriors.
-- Re-running this file is a no-op.

DELETE FROM `playercreateinfo_skills`
WHERE `classMask` = 32
  AND `skill` IN (129, 293, 118);

UPDATE `playercreateinfo_skills`
SET `classMask` = `classMask` & ~32
WHERE (`classMask` & 32) != 0
  AND `classMask` != 32
  AND `skill` = 118;
