-- Rollback helper for mod-dk.
-- Run from the AzerothCore source root:
--   mysql -u <user> -p acore_world < modules/mod-dk/sql/rollback.sql

SOURCE modules/mod-dk/sql/dk_rollback.sql;
