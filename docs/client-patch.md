The client patch
================

Players need a patched client `Spell.dbc`. Without it the client refuses
runeforging on low level weapons ("Target is too low level"), shows the stock
spell levels and shows unscaled damage in tooltips. The patched file is built
from your own game data with
[`scripts/build_runeforge_client_mpq.py`](../scripts/build_runeforge_client_mpq.py)
and is not shipped with this module.

- [Building it](#building-it)
- [What it changes](#what-it-changes)
- [Giving it to players](#giving-it-to-players)
- [The database password](#the-database-password)

Building it
-----------
Build it after the SQL is applied, because the tool reads the trainer levels
from the world database:

```sh
cd <azerothcore>/modules/mod-dk
export ACORE_MYSQL_PWD='<world db password>'
python3 scripts/build_runeforge_client_mpq.py --dbc-in <DataDir>/dbc/Spell.dbc
```

Use the unmodified `Spell.dbc` that the extractor took from your client. The
file carries the spell text of that client's language, so build the patch from
the same locale your players use.

This writes (both are ignored by git):

- `client/patch-d.MPQ`: the file to give to players
- `client/DBFilesClient/Spell.dbc`: the patched DBC inside it

Change the output paths with `--mpq-out` and `--dbc-out`, and the database
connection with `--mysql`. Run the tool with `--help` for all options.

What it changes
---------------
- Lets the client cast runeforge enchants on low level weapons.
- Removes "in Ebon Hold" from the English rune descriptions, since runeforges
  stand in the world.
- Shows the module's trainer levels as the spell level.
- Makes the damage tooltips of the trained spells below level 55 follow the
  server scaling at the player's current level. Weapon strike tooltips match
  the server exactly; Icy Touch, Death Coil, Blood Boil and Death and Decay
  follow a straight line from the training level to 55, within a few percent
  of the server value. The client computes these numbers from per level fields
  in `Spell.dbc`; if a tooltip does not change as you level, build the patch
  with `--static-tooltips`, which keeps the value of the training level.

Talent abilities and minions are scaled on the server only; their tooltips
show full values. The tooltips assume `ModDk.Scaling.Strength = 1.0`.

Giving it to players
--------------------
Each player copies `patch-d.MPQ` into the client's `Data/` folder (next to
`common.MPQ`) and deletes the client's `Cache/` folder. If a `patch-d.MPQ`
already exists there, rename this one to another free patch letter, for
example `patch-e.MPQ`.

Do not copy the patched `Spell.dbc` into the server `DataDir`: the server
already scales the damage, so it would be scaled twice.

Rebuild and redistribute the patch whenever the trainer or scaling tables
change.

The database password
---------------------
The tool queries the world database through the `mysql` client. By default it
runs:

```sh
mysql -u acore -D acore_world -N -B
```

Set the password in the `ACORE_MYSQL_PWD` environment variable instead of
writing it into a command:

```sh
export ACORE_MYSQL_PWD='<world db password>'
```

The tool hands it to the client as `MYSQL_PWD`, so it never appears on the
`mysql` command line, where other users on the machine could read it from the
process list.

For another user, host or database, pass your own command with `--mysql`.
Keep `-N -B` (the tool parses tab separated output without headers) and leave
`-p` out, so the password still comes from the variable:

```sh
python3 scripts/build_runeforge_client_mpq.py --dbc-in <DataDir>/dbc/Spell.dbc \
    --mysql 'mysql -h 127.0.0.1 -u <user> -D acore_world -N -B'
```

You can also keep the credentials in a MySQL option file and pass
`--defaults-extra-file=<file>` as the first option of your `--mysql` command.
