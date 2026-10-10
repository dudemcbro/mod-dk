Optional core patches
=====================

Neither patch is needed to run the module.

Flight paths
------------
Stock AzerothCore gives every Death Knight all Eastern Kingdoms and Kalimdor
flight paths, without asking any script.
[`patches/dk-taxi-nodes.patch`](../patches/dk-taxi-nodes.patch) routes that
through `OnPlayerIsClass(CLASS_CONTEXT_TAXI)`, which this module answers so
DKs discover flight paths like other classes. It only affects characters
created after it is applied (known nodes are saved). Apply it from the root of
your AzerothCore source tree and rebuild:

```sh
cd <azerothcore>
git apply --check modules/mod-dk/patches/dk-taxi-nodes.patch
git apply modules/mod-dk/patches/dk-taxi-nodes.patch
```

Revert it with `git apply -R modules/mod-dk/patches/dk-taxi-nodes.patch`.

Runeforge cast failure logging
------------------------------
[`patches/runeforge-fail-logging.patch`](../patches/runeforge-fail-logging.patch)
logs DK enchant item cast failures, to find the runeforge spell behind a
"Target is too low level" error. It is a debugging aid and not needed for
normal use. Apply it from the root of your AzerothCore source tree and
rebuild:

```sh
cd <azerothcore>
git apply --check modules/mod-dk/patches/runeforge-fail-logging.patch
git apply modules/mod-dk/patches/runeforge-fail-logging.patch
```

It logs at warning level under the `spells.runeforge` logger. The default
`worldserver.conf` only logs errors for loggers it does not list, so add:

```ini
Logger.spells.runeforge=3,Console Server
```

Remove it again with `git apply -R modules/mod-dk/patches/runeforge-fail-logging.patch`.
