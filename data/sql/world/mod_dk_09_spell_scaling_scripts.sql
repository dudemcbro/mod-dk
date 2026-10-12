-- DK low-level damage scaling scripts
DELETE FROM `spell_script_names`
WHERE `ScriptName` IN ('spell_mod_dk_low_level_damage', 'spell_mod_dk_low_level_dot', 'spell_mod_dk_presence_scale', 'spell_mod_dk_dnd_scale', 'spell_mod_dk_blood_presence_heal');

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
-- Direct damage scaling
(45477, 'spell_mod_dk_low_level_damage'), -- Icy Touch
(49896, 'spell_mod_dk_low_level_damage'),
(49903, 'spell_mod_dk_low_level_damage'),
(49904, 'spell_mod_dk_low_level_damage'),
(49909, 'spell_mod_dk_low_level_damage'),
(45462, 'spell_mod_dk_low_level_damage'), -- Plague Strike
(49917, 'spell_mod_dk_low_level_damage'),
(49918, 'spell_mod_dk_low_level_damage'),
(49919, 'spell_mod_dk_low_level_damage'),
(49920, 'spell_mod_dk_low_level_damage'),
(49921, 'spell_mod_dk_low_level_damage'),
(66216, 'spell_mod_dk_low_level_damage'), -- Plague Strike triggered damage
(66988, 'spell_mod_dk_low_level_damage'),
(66989, 'spell_mod_dk_low_level_damage'),
(66990, 'spell_mod_dk_low_level_damage'),
(66991, 'spell_mod_dk_low_level_damage'),
(66992, 'spell_mod_dk_low_level_damage'),
(45902, 'spell_mod_dk_low_level_damage'), -- Blood Strike
(49926, 'spell_mod_dk_low_level_damage'),
(49927, 'spell_mod_dk_low_level_damage'),
(49928, 'spell_mod_dk_low_level_damage'),
(49929, 'spell_mod_dk_low_level_damage'),
(49930, 'spell_mod_dk_low_level_damage'),
(66215, 'spell_mod_dk_low_level_damage'), -- Blood Strike triggered damage
(66975, 'spell_mod_dk_low_level_damage'),
(66976, 'spell_mod_dk_low_level_damage'),
(66977, 'spell_mod_dk_low_level_damage'),
(66978, 'spell_mod_dk_low_level_damage'),
(66979, 'spell_mod_dk_low_level_damage'),
(47541, 'spell_mod_dk_low_level_damage'), -- Death Coil
(47632, 'spell_mod_dk_low_level_damage'),
(49892, 'spell_mod_dk_low_level_damage'),
(49893, 'spell_mod_dk_low_level_damage'),
(49894, 'spell_mod_dk_low_level_damage'),
(49895, 'spell_mod_dk_low_level_damage'),
(48721, 'spell_mod_dk_low_level_damage'), -- Blood Boil
(49939, 'spell_mod_dk_low_level_damage'),
(49940, 'spell_mod_dk_low_level_damage'),
(49941, 'spell_mod_dk_low_level_damage'),
(49998, 'spell_mod_dk_low_level_damage'), -- Death Strike
(49999, 'spell_mod_dk_low_level_damage'),
(45463, 'spell_mod_dk_low_level_damage'),
(49923, 'spell_mod_dk_low_level_damage'),
(49924, 'spell_mod_dk_low_level_damage'),
(66188, 'spell_mod_dk_low_level_damage'), -- Death Strike triggered damage
(66950, 'spell_mod_dk_low_level_damage'),
(66951, 'spell_mod_dk_low_level_damage'),
(66952, 'spell_mod_dk_low_level_damage'),
(66953, 'spell_mod_dk_low_level_damage'),
(49020, 'spell_mod_dk_low_level_damage'), -- Obliterate
(51423, 'spell_mod_dk_low_level_damage'),
(51424, 'spell_mod_dk_low_level_damage'),
(51425, 'spell_mod_dk_low_level_damage'),
(66198, 'spell_mod_dk_low_level_damage'), -- Obliterate triggered damage
(66972, 'spell_mod_dk_low_level_damage'),
(66973, 'spell_mod_dk_low_level_damage'),
(66974, 'spell_mod_dk_low_level_damage'),

-- Periodic damage scaling
(55078, 'spell_mod_dk_low_level_dot'), -- Blood Plague
(55095, 'spell_mod_dk_low_level_dot'), -- Frost Fever
(43265, 'spell_mod_dk_dnd_scale'), -- Death and Decay
(49936, 'spell_mod_dk_dnd_scale'),
(49937, 'spell_mod_dk_dnd_scale'),
(49938, 'spell_mod_dk_dnd_scale'),

-- Presence scaling
(48266, 'spell_mod_dk_presence_scale'), -- Blood Presence
(48263, 'spell_mod_dk_presence_scale'), -- Frost Presence
(61261, 'spell_mod_dk_presence_scale'), -- Frost Presence stamina (triggered by Frost Presence)

-- Blood Presence heal fallback suppression (see ModDk.cpp)
(50475, 'spell_mod_dk_blood_presence_heal'); -- Blood Presence heal
