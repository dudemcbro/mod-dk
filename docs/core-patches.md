Optional core patch
===================

The patch is not needed to run the module.

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
