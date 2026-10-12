-- DK starting gear now comes from CharStartOutfit (DBC/DB override).
-- Remove only the DK starter gear items to avoid duplicates; keep other starter items intact.
-- Stock also asks the core to strip the Scourgestone (40582, amount -1) from the stock DK
-- outfit; the module's outfit has no Scourgestone, so the core would log a startup error
-- for every race and gender.
DELETE FROM `playercreateinfo_item`
WHERE `class` = 6
  AND `itemid` IN (2391, 2386, 2388, 2389, 2587, 1194, 6948, 40582);
