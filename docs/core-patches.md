Optional core patch
===================

The patch is not needed to run the module.

Flight paths
------------
Stock AzerothCore gives every Death Knight all Eastern Kingdoms and Kalimdor
flight paths, without asking any script. The core already has a class context
for this, `CLASS_CONTEXT_TAXI`, but `Player::InitTaxiNodesForLevel()` does not
consult it.

[`patches/dk-taxi-nodes.patch`](../patches/dk-taxi-nodes.patch) makes
`Player::InitTaxiNodesForLevel()` ask `IsClass(CLASS_DEATH_KNIGHT,
CLASS_CONTEXT_TAXI)` before it grants those flight paths. This module answers
`false` for DKs, so they discover flight paths like other classes. The patch
adds no hook and changes nothing when no script answers that context.

It only affects characters created after it is applied: known nodes are saved,
so existing DKs keep the flight paths they already have.

The same change is proposed upstream to AzerothCore in
[azerothcore-wotlk#28470](https://github.com/azerothcore/azerothcore-wotlk/pull/28470).
Once a core release includes it, the patch is no longer needed. It applies to current
`azerothcore-wotlk` `master` and to the mod-playerbots fork (`Playerbot`
branch). Apply it from the root of your AzerothCore source tree and rebuild:

```sh
cd <azerothcore>
git apply --check modules/mod-dk/patches/dk-taxi-nodes.patch
git apply modules/mod-dk/patches/dk-taxi-nodes.patch
```

Revert it with `git apply -R modules/mod-dk/patches/dk-taxi-nodes.patch`.

If you applied an older version of this patch (the one-line change in
`Player.h`), revert that one with the old patch file first, then apply this
one.

Why not do it in the module
---------------------------
The core adds every old continent flight path back to a DK's taxi mask on each
login, level up and `.reset` command, and `PlayerTaxi` has no way to remove a
node. A module-only version would have to keep its own copy of every node the
DK discovered in a characters table and rewrite the mask after each of those
events, and it could not tell discovered nodes from granted ones on characters
that already exist. Asking the existing class context in the core is one
small change instead.
