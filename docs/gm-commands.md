GM commands
===========

| Command | Alias | Level | Effect |
| --- | --- | --- | --- |
| `.dk runeforge add [entry]` | `.dk rf add` | GM (2) | Spawn a permanent runeforge at your position (default visual entry 202236) |
| `.dk runeforge remove` | `.dk rf remove` | GM (2) | Remove the targeted runeforge, or the nearest one within 15 yards |
| `.dk runeforge list` | `.dk rf list` | GM (2) | List the runeforges, nearest first |
| `.dk madness add` | `.dk md add` | GM (2) | Spawn a permanent Madness trainer at your position |
| `.dk madness remove` | `.dk md remove` | GM (2) | Remove the targeted Madness trainer, or the nearest one within 15 yards |
| `.dk madness list` | `.dk md list` | GM (2) | List the Madness trainers, nearest first |
| `.dk status` | | GM (2) | Settings and the setup check; also works from the console |
| `.dk reload` | | Admin (3) | Reload the spell progression table and the intro quest list and apply them to online DKs; also works from the console |

Spawns are saved to the world database and appear at once, no reload needed.
The list commands show up to 50 spawns with their GUID, map, position,
distance and whether they are a default spawn; go to one with
`.go creature <guid>` or `.go gameobject <guid>`.

A runeforge is three objects: a hidden runeforge focus (entry 190557) that
satisfies the "Requires Runeforge" check, the Frostmourne Altar model
(entry 202236, scaled to 0.8) and a harmless blue flame (entry 900010).
Runeforge removal only touches spawns tagged `mod-dk: runeforge`.

Settings from `mod_dk.conf` reload with the core command `.reload config`
(see [Configuration](configuration.md)).
