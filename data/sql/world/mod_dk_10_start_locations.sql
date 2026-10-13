-- mod-dk: Keep DK creation in Acherus for the core intro cinematic.
-- The module teleports fresh DKs to racial starts after the cinematic ends.

UPDATE playercreateinfo
SET map = 609,
    zone = 0,
    position_x = 2355.84,
    position_y = -5664.77,
    position_z = 426.028,
    orientation = 3.65997
WHERE class = 6;
