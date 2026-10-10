Configuration
=============

Settings live in `etc/modules/mod_dk.conf` (copy it from
[`mod_dk.conf.dist`](../conf/mod_dk.conf.dist), which `make install` puts in
`etc/modules/`). Without the file, every option keeps its default. Apply
changes with `.reload config` or a restart; auras that are already active
(presences, diseases) keep their values until they are cast again.

| Option | Default | Effect |
| --- | --- | --- |
| `ModDk.StartLevel` | `1` | Level new DKs start at, replacing `StartHeroicPlayerLevel`; `-1` keeps the `worldserver.conf` value. Restart to change it |
| `ModDk.MinLevelForHeroicCharacter` | `0` | Character level an account needs before it can create a DK, replacing `CharacterCreating.MinLevelForHeroicCharacter`; `-1` keeps the `worldserver.conf` value. Restart to change it |
| `ModDk.Scaling.Enable` | `1` | Low level damage, healing, presence and minion scaling |
| `ModDk.Scaling.Strength` | `1.0` | How much of the scaling applies, from `0.0` (none) to `1.0` (the full curve). The client patch tooltips assume `1.0` |
| `ModDk.ThreatBonus.Enable` | `1` | Extra threat below level 30 in Blood or Frost Presence |
| `ModDk.SpellProgression.Enable` | `1` | Remove DK spells above the character's level and teach the level 1 spells |
| `ModDk.Reentry.Enable` | `1` | The Ebon Herald offers the Ebon Hold intro again |
| `ModDk.EbonHoldRescue.Enable` | `1` | Send DKs below 55 found in Ebon Hold on login to their racial start |
| `ModDk.Debug` | `0` | Log every scaled hit and threat bonus (logger `module`) |

With `ModDk.StartLevel = 55`, new DKs play the stock Ebon Hold intro, the low
level scaling never applies to them and the core's automatic spell learning is
left as it is; they still start with this module's level 1 outfit and action
bar.
