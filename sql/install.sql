-- Manual installer for mod-dk world SQL.
--
-- The AzerothCore DB updater applies data/sql/db-world/ on its own when
-- worldserver starts (Updates.EnableDatabases includes the world database,
-- which is the default). Use this file only with the updater turned off.
--
-- Run from the AzerothCore source root:
--   mysql -u <user> -p acore_world < modules/mod-dk/sql/install.sql

SOURCE modules/mod-dk/data/sql/db-world/mod_dk_01_balance_class_stats_1_60.sql;
SOURCE modules/mod-dk/data/sql/db-world/mod_dk_02_spell_progression.sql;
SOURCE modules/mod-dk/data/sql/db-world/mod_dk_03_trainer_spells_1_60.sql;
SOURCE modules/mod-dk/data/sql/db-world/mod_dk_04_trainer_spawns.sql;
SOURCE modules/mod-dk/data/sql/db-world/mod_dk_05_actionbar.sql;
SOURCE modules/mod-dk/data/sql/db-world/mod_dk_06_start_armor_proficiencies.sql;
SOURCE modules/mod-dk/data/sql/db-world/mod_dk_07_start_skills.sql;
SOURCE modules/mod-dk/data/sql/db-world/mod_dk_08_start_warrior_gear.sql;
SOURCE modules/mod-dk/data/sql/db-world/mod_dk_09_spell_scaling_scripts.sql;
SOURCE modules/mod-dk/data/sql/db-world/mod_dk_10_start_locations.sql;
SOURCE modules/mod-dk/data/sql/db-world/mod_dk_11_fix_rusted_chain_leggings.sql;
SOURCE modules/mod-dk/data/sql/db-world/mod_dk_12_start_outfit_dbc.sql;
SOURCE modules/mod-dk/data/sql/db-world/mod_dk_13_runeforge_template.sql;
SOURCE modules/mod-dk/data/sql/db-world/mod_dk_15_reentry_epl.sql;
SOURCE modules/mod-dk/data/sql/db-world/mod_dk_16_start_2h_only.sql;
SOURCE modules/mod-dk/data/sql/db-world/mod_dk_17_default_spawns.sql;
