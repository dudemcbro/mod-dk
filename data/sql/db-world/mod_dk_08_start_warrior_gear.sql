-- DK starting gear now comes from CharStartOutfit (DBC/DB override).
-- Remove only the DK starter gear items to avoid duplicates; keep other starter items intact.
DELETE FROM `playercreateinfo_item`
WHERE `class` = 6
  AND `itemid` IN (2391, 2386, 2388, 2389, 2587, 1194, 6948);
