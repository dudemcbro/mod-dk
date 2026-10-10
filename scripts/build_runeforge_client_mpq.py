#!/usr/bin/env python3
"""Patch client Spell.dbc for DK trainer spells, runeforging, and low-level damage scaling.

Patch logic (applied in order, first match wins per spell):
  1. Runeforge enchant spells: set ALLOW_LOW_LEVEL_ATTR, BaseLevel=1, MaxLevel=0, and
     drop "in Ebon Hold" from the English descriptions (runeforges stand in the world).
  2. All other DK trainer spells (TrainerId=1013):
     a. Always: set SpellLevel = max(1, trainer_level).
     b. Below level 55, make the damage tooltip follow the server scaling at the
        player's level. The client computes a tooltip value as
            BasePoints + 1 + RealPointsPerLevel * (min(level, MaxLevel) - SpellLevel)
        so BasePoints is set to the server value at trainer_level, RealPointsPerLevel to
        the climb to the full value at 55, and BaseLevel/MaxLevel to trainer_level/55.
        Weapon strikes scale their flat bonus by level / 55 (exact), other damage spells
        follow GetDkNonWeaponScale and Death and Decay GetLowLevelDndScale (both close to
        linear above level 12). With --static-tooltips the values stay at trainer_level.
"""
import argparse
import os
import struct
import subprocess
from pathlib import Path

# Runeforge enchant spells — need low-level access regardless of character level.
RUNEFORGE_SPELLS = {
    53323,  # Rune of Swordshattering
    53331,  # Rune of Lichbane
    53341,  # Rune of Cinderglacier
    53342,  # Rune of Spellshattering
    53343,  # Rune of Razorice
    53344,  # Rune of the Fallen Crusader
    54446,  # Rune of Swordbreaking
    54447,  # Rune of Spellbreaking
    62158,  # Rune of the Stoneskin Gargoyle
    70164,  # Rune of the Nerubian Carapace
}

ALLOW_LOW_LEVEL_ATTR = 0x8  # SPELL_ATTR2_ALLOW_LOW_LEVEL_BUFF

# Effect types that indicate a weapon strike spell (from SharedDefines.h)
WEAPON_DMG_EFFECTS = {17, 31, 58, 121}

# Flat bonus effects among them: WEAPON_DAMAGE_NOSCHOOL (17), WEAPON_DAMAGE (58),
# NORMALIZED_WEAPON_DMG (121). WEAPON_PERCENT_DAMAGE (31) is the weapon %, left alone.
WEAPON_FLAT_EFFECTS = {17, 58, 121}

# Effect types for direct school-damage spells (Icy Touch, Blood Boil, Death Coil)
SCHOOL_DMG_EFFECTS = {2, 3}  # SPELL_EFFECT_SCHOOL_DAMAGE, SPELL_EFFECT_DUMMY

# Per-spell extra multiplier applied server-side by GetDkNonWeaponScaleModifier
NON_WEAPON_EXTRA_SCALE = {
    45477: 0.9, 49896: 0.9, 49903: 0.9, 49904: 0.9, 49909: 0.9,  # Icy Touch
    47541: 0.8, 47632: 0.8, 49892: 0.8, 49893: 0.8, 49894: 0.8, 49895: 0.8,  # Death Coil
    48721: 0.8, 49939: 0.8, 49940: 0.8, 49941: 0.8,               # Blood Boil
}

# Death and Decay spell IDs — use GetLowLevelDndScale instead of the main scale
DND_SPELL_IDS = {43265, 49936, 49937, 49938}

DK_TRAINER_ID = 1013


DK_DAMAGE_SCALING_END_LEVEL = 55

# Mirrors of the curves in ModDk.cpp (DK_DAMAGE_CURVE, DK_DND_CURVE); keep them in sync.
DK_DAMAGE_CURVE = [(1, 0.08), (4, 0.11), (12, 0.22), (20, 0.37), (30, 0.55), (40, 0.73), (55, 1.00)]
DK_DND_CURVE = [(1, 0.12), (10, 0.20), (20, 0.40), (30, 0.60), (40, 0.78), (55, 1.00)]


def sample_curve(curve: list[tuple[int, float]], level: int) -> float:
    if level <= curve[0][0]:
        return curve[0][1]
    for (l0, v0), (l1, v1) in zip(curve, curve[1:]):
        if level <= l1:
            return v0 + (level - l0) * (v1 - v0) / (l1 - l0)
    return curve[-1][1]


def get_low_level_dk_scale(level: int) -> float:
    """Mirror of ModDk.cpp GetLowLevelDkScale(). Returns damage multiplier for level."""
    if level >= DK_DAMAGE_SCALING_END_LEVEL:
        return 1.0
    return sample_curve(DK_DAMAGE_CURVE, level)


def get_dk_non_weapon_scale(level: int, spell_id: int) -> float:
    """Mirror of ModDk.cpp GetDkNonWeaponScale(): the per spell modifier fades out with level."""
    scale = get_low_level_dk_scale(level)
    if scale >= 1.0:
        return 1.0
    modifier = NON_WEAPON_EXTRA_SCALE.get(spell_id, 1.0)
    return scale * (1.0 - (1.0 - modifier) * (1.0 - scale))


def get_weapon_strike_flat_scale(level: int) -> float:
    """Mirror of ModDk.cpp ScaleWeaponStrikeHit(): flat bonus scales by level / 55."""
    if level >= DK_DAMAGE_SCALING_END_LEVEL:
        return 1.0
    return level / DK_DAMAGE_SCALING_END_LEVEL


def get_low_level_dnd_scale(level: int) -> float:
    """Mirror of ModDk.cpp GetLowLevelDndScale(). Used for Death and Decay tooltip scaling."""
    if level >= DK_DAMAGE_SCALING_END_LEVEL:
        return 1.0
    return sample_curve(DK_DND_CURVE, level)


def mysql_env() -> dict:
    """Pass ACORE_MYSQL_PWD to the client as MYSQL_PWD, so the password is not on
    the mysql command line, where other users can see it in the process list."""
    env = os.environ.copy()
    if env.get("ACORE_MYSQL_PWD") and "MYSQL_PWD" not in env:
        env["MYSQL_PWD"] = env["ACORE_MYSQL_PWD"]
    return env


def run_mysql_rows(cmd: str) -> list[list[str]]:
    out = subprocess.check_output(cmd, shell=True, text=True, env=mysql_env())
    rows = []
    for line in out.splitlines():
        if line.strip():
            rows.append(line.rstrip("\n").split("\t"))
    return rows


def resolve_spell_indices(mysql_cmd: str) -> dict[str, int]:
    """Query INFORMATION_SCHEMA to map DBC column names -> 0-based field indices."""
    col_query = (
        "SELECT COLUMN_NAME FROM INFORMATION_SCHEMA.COLUMNS "
        "WHERE TABLE_SCHEMA=DATABASE() AND TABLE_NAME='spell_dbc' "
        "ORDER BY ORDINAL_POSITION"
    )
    rows = run_mysql_rows(f'{mysql_cmd} -e "{col_query}"')
    if not rows:
        raise SystemExit("Failed to read spell_dbc columns. Check DB connection.")
    columns = [r[0] for r in rows]

    def idx(name: str) -> int:
        return columns.index(name)

    idxs = {
        "id":          idx("ID"),
        "attr_ex2":    idx("AttributesEx2"),
        "base_level":  idx("BaseLevel"),
        "max_level":   idx("MaxLevel"),
        "spell_level": idx("SpellLevel"),
        "eff1":        idx("Effect_1"),
        "eff2":        idx("Effect_2"),
        "eff3":        idx("Effect_3"),
        "bp1":         idx("EffectBasePoints_1"),
        "bp2":         idx("EffectBasePoints_2"),
        "bp3":         idx("EffectBasePoints_3"),
        "die1":        idx("EffectDieSides_1"),
        "die2":        idx("EffectDieSides_2"),
        "die3":        idx("EffectDieSides_3"),
        "rpl1":        idx("EffectRealPointsPerLevel_1"),
        "rpl2":        idx("EffectRealPointsPerLevel_2"),
        "rpl3":        idx("EffectRealPointsPerLevel_3"),
    }
    # Every localized description column (not the *_Lang_Mask flags column).
    for c in columns:
        if c.startswith("Description_Lang_") and not c.endswith("_Mask"):
            idxs["desc:" + c] = columns.index(c)
    return idxs


def get_dk_trainer_spells(mysql_cmd: str) -> dict[int, int]:
    """Return {spell_id: req_level} for all spells in trainer 1013."""
    rows = run_mysql_rows(
        f'{mysql_cmd} -e "SELECT SpellId, ReqLevel FROM trainer_spell WHERE TrainerId={DK_TRAINER_ID}"'
    )
    return {int(r[0]): int(r[1]) for r in rows if len(r) == 2}


EFFECT_KEYS = [("eff1", "bp1", "die1", "rpl1"), ("eff2", "bp2", "die2", "rpl2"), ("eff3", "bp3", "die3", "rpl3")]

# Rune descriptions say runeforging needs "a Rune Forge in Ebon Hold"; this module puts
# runeforges in the world, so the English text drops the location.
EBON_HOLD_TEXT = ("Rune Forge in Ebon Hold", "Rune Forge")


def patch_spell_dbc(
    src: Path,
    dst: Path,
    idxs: dict[str, int],
    dk_trainer_spells: dict[int, int],
    static_tooltips: bool = False,
) -> None:
    with src.open("rb") as f:
        if f.read(4) != b"WDBC":
            raise SystemExit(f"Not a DBC file: {src}")
        record_count, field_count, record_size, string_block_size = struct.unpack("<4I", f.read(16))
        data = bytearray(f.read(record_count * record_size))
        string_block = bytearray(f.read(string_block_size))

    fields = record_size // 4
    max_idx = max(idxs.values())
    if max_idx >= fields:
        raise SystemExit(f"Column index {max_idx} exceeds DBC field count {fields}. Wrong schema?")

    def rdi(off: int, field: str) -> int:
        return struct.unpack_from("<i", data, off + idxs[field] * 4)[0]

    def rdu(off: int, field: str) -> int:
        return struct.unpack_from("<I", data, off + idxs[field] * 4)[0]

    def wri(off: int, field: str, val: int) -> None:
        struct.pack_into("<i", data, off + idxs[field] * 4, val)

    def wru(off: int, field: str, val: int) -> None:
        struct.pack_into("<I", data, off + idxs[field] * 4, val)

    def wrf(off: int, field: str, val: float) -> None:
        struct.pack_into("<f", data, off + idxs[field] * 4, val)

    desc_keys = [k for k in idxs if k.startswith("desc:")]

    def drop_ebon_hold(off: int) -> bool:
        changed = False
        for key in desc_keys:
            str_off = rdu(off, key)
            if not str_off:
                continue
            end = string_block.find(b"\x00", str_off)
            text = bytes(string_block[str_off:end]).decode("utf-8", errors="replace")
            if EBON_HOLD_TEXT[0] not in text:
                continue
            new_off = len(string_block)
            string_block.extend(text.replace(*EBON_HOLD_TEXT).encode("utf-8") + b"\x00")
            wru(off, key, new_off)
            changed = True
        return changed

    def scale_effects(off: int, effect_types: set[int], curve, trainer_level: int, positive_only: bool) -> None:
        """Scale the matching effects. curve(level) is the server scale at that level."""
        start_scale = curve(trainer_level)
        if not static_tooltips:
            # The client takes min(level, MaxLevel) and max(level, BaseLevel).
            wri(off, "base_level", trainer_level)
            wri(off, "max_level", DK_DAMAGE_SCALING_END_LEVEL)
        for eff_key, bp_key, die_key, rpl_key in EFFECT_KEYS:
            if rdi(off, eff_key) not in effect_types:
                continue
            bp = rdi(off, bp_key)
            if positive_only and bp <= 0:
                continue
            full = bp + 1
            start = max(1, round(full * start_scale))
            wri(off, bp_key, start - 1)
            die = rdi(off, die_key)
            if die > 1:
                wri(off, die_key, max(1, round(die * start_scale)))
            if not static_tooltips:
                levels = DK_DAMAGE_SCALING_END_LEVEL - trainer_level
                wrf(off, rpl_key, (full - start) / levels if levels > 0 else 0.0)

    runeforge_patched = 0
    text_patched = 0
    trainer_patched = 0
    trainer_scaled = 0

    for i in range(record_count):
        off = i * record_size
        spell_id = rdu(off, "id")

        # --- Path 1: Runeforge enchants ---
        if spell_id in RUNEFORGE_SPELLS:
            attr = rdu(off, "attr_ex2")
            wru(off, "attr_ex2", attr | ALLOW_LOW_LEVEL_ATTR)
            wri(off, "base_level", 1)
            wri(off, "max_level", 0)
            runeforge_patched += 1
            if drop_ebon_hold(off):
                text_patched += 1
            continue  # don't also do trainer patching

        # --- Path 2: DK trainer spells ---
        if spell_id not in dk_trainer_spells:
            continue

        if drop_ebon_hold(off):
            text_patched += 1

        trainer_level = max(1, dk_trainer_spells[spell_id])

        # Always fix SpellLevel so tooltip shows correct requirement
        wri(off, "spell_level", trainer_level)
        trainer_patched += 1

        if trainer_level >= DK_DAMAGE_SCALING_END_LEVEL:
            continue

        effects = {rdi(off, "eff1"), rdi(off, "eff2"), rdi(off, "eff3")}

        if spell_id in DND_SPELL_IDS:
            # Death and Decay is a persistent area aura (27); its periodic amount feeds
            # the damage spell, so every positive bp is scaled.
            scale_effects(off, effects, get_low_level_dnd_scale, trainer_level, True)
            trainer_scaled += 1
        elif effects.intersection(WEAPON_DMG_EFFECTS):
            # Weapon strike: scale the flat bonus only; the weapon % stays as it is.
            scale_effects(off, WEAPON_FLAT_EFFECTS, get_weapon_strike_flat_scale, trainer_level, True)
            trainer_scaled += 1
        elif spell_id in NON_WEAPON_EXTRA_SCALE and effects.intersection(SCHOOL_DMG_EFFECTS):
            # School-damage / dummy spells (Icy Touch, Death Coil, Blood Boil). Limited to
            # these: other trainer spells use dummy effects for values that are not damage.
            scale_effects(off, SCHOOL_DMG_EFFECTS, lambda level: get_dk_non_weapon_scale(level, spell_id), trainer_level, True)
            trainer_scaled += 1

    expected_runeforge = len(RUNEFORGE_SPELLS)
    if runeforge_patched != expected_runeforge:
        print(f"WARNING: Patched {runeforge_patched}/{expected_runeforge} runeforge spells.")
    print(
        f"Patched: {runeforge_patched} runeforge, "
        f"{trainer_patched} trainer SpellLevel, "
        f"{trainer_scaled} damage spells scaled"
        f"{' (static tooltips)' if static_tooltips else ' (tooltips follow the player level)'}, "
        f"{text_patched} descriptions without 'in Ebon Hold'."
    )

    dst.parent.mkdir(parents=True, exist_ok=True)
    with dst.open("wb") as f:
        f.write(b"WDBC")
        f.write(struct.pack("<4I", record_count, field_count, record_size, len(string_block)))
        f.write(data)
        f.write(string_block)


# ── MPQ builder (pure-Python, no external tools) ──────────────────────────────

def build_crypt_table() -> list[int]:
    crypt = [0] * 0x500
    seed = 0x00100001
    for i in range(0x100):
        for j in range(5):
            seed = (seed * 125 + 3) % 0x2AAAAB
            temp1 = (seed & 0xFFFF) << 16
            seed = (seed * 125 + 3) % 0x2AAAAB
            temp2 = seed & 0xFFFF
            crypt[i + j * 0x100] = (temp1 | temp2) & 0xFFFFFFFF
    return crypt


CRYPT_TABLE = build_crypt_table()


def hash_string(s: str, hash_type: int) -> int:
    seed1, seed2 = 0x7FED7FED, 0xEEEEEEEE
    for ch in s.upper().replace("/", "\\"):
        value = CRYPT_TABLE[(hash_type << 8) + ord(ch)]
        seed1 = (value ^ (seed1 + seed2)) & 0xFFFFFFFF
        seed2 = (ord(ch) + seed1 + seed2 + ((seed2 << 5) & 0xFFFFFFFF) + 3) & 0xFFFFFFFF
    return seed1


def encrypt_table(data: bytes, key: int) -> bytes:
    seed1, seed2 = key & 0xFFFFFFFF, 0xEEEEEEEE
    out = bytearray()
    for i in range(0, len(data), 4):
        val = struct.unpack_from("<I", data, i)[0]
        seed2 = (seed2 + CRYPT_TABLE[0x400 + (seed1 & 0xFF)]) & 0xFFFFFFFF
        enc = (val ^ (seed1 + seed2)) & 0xFFFFFFFF
        out += struct.pack("<I", enc)
        seed1 = (((~seed1) << 21) + 0x11111111 | (seed1 >> 11)) & 0xFFFFFFFF
        seed2 = (val + seed2 + ((seed2 << 5) & 0xFFFFFFFF) + 3) & 0xFFFFFFFF
    return bytes(out)


def build_mpq(mpq_path: Path, dbc_path: Path) -> None:
    filename = "DBFilesClient\\Spell.dbc"
    file_bytes = dbc_path.read_bytes()

    # WoW 3.3.5a MPQ loader requires >= 16 hash table entries.
    HASH_TABLE_ENTRIES = 16

    header_size = 32
    hash_table_offset = header_size
    block_table_offset = hash_table_offset + HASH_TABLE_ENTRIES * 16
    file_offset = block_table_offset + 16
    archive_size = file_offset + len(file_bytes)

    # 16-entry hash table; empty slots use the 0xFF sentinel.
    hash_raw = bytearray(b'\xff' * (HASH_TABLE_ENTRIES * 16))
    slot = hash_string(filename, 0) % HASH_TABLE_ENTRIES
    entry = struct.pack("<IIHHI",
                        hash_string(filename, 1),
                        hash_string(filename, 2),
                        0, 0, 0)
    hash_raw[slot * 16:(slot + 1) * 16] = entry
    hash_table = encrypt_table(bytes(hash_raw), hash_string("(hash table)", 3))

    flags = 0x80000000 | 0x01000000  # MPQ_FILE_EXISTS | MPQ_FILE_SINGLE_UNIT
    block_entry = struct.pack("<IIII", file_offset, len(file_bytes), len(file_bytes), flags)
    block_table = encrypt_table(block_entry, hash_string("(block table)", 3))

    header = struct.pack(
        "<4sIIHHIIII",
        b"MPQ\x1A",
        header_size,
        archive_size,
        0,                   # format version
        3,                   # sector size shift (4 KB)
        hash_table_offset,
        block_table_offset,
        HASH_TABLE_ENTRIES,
        1,                   # block table entries
    )

    mpq_path.parent.mkdir(parents=True, exist_ok=True)
    with mpq_path.open("wb") as f:
        f.write(header)
        f.write(hash_table)
        f.write(block_table)
        f.write(file_bytes)
    print(f"Built MPQ: {mpq_path}  ({len(file_bytes):,} byte DBC, file at hash slot {slot}/{HASH_TABLE_ENTRIES})")


MODULE_DIR = Path(__file__).resolve().parent.parent


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument("--dbc-in", required=True,
                        help="unmodified server Spell.dbc (e.g. <DataDir>/dbc/Spell.dbc)")
    parser.add_argument("--dbc-out", default=str(MODULE_DIR / "client/DBFilesClient/Spell.dbc"),
                        help="patched Spell.dbc output (default: %(default)s)")
    parser.add_argument("--mpq-out", default=str(MODULE_DIR / "client/patch-d.MPQ"),
                        help="client MPQ output (default: %(default)s)")
    parser.add_argument(
        "--mysql",
        default="mysql -u acore -D acore_world -N -B",
        help="mysql client command for the world DB; the password is read from "
             "$ACORE_MYSQL_PWD (passed as MYSQL_PWD) (default: %(default)s)",
    )
    parser.add_argument("--static-tooltips", action="store_true",
                        help="keep damage tooltips at the value of the level the spell is trained at "
                             "instead of following the player level")
    args = parser.parse_args()

    dbc_in  = Path(args.dbc_in)
    dbc_out = Path(args.dbc_out)
    mpq_out = Path(args.mpq_out)

    if not dbc_in.exists():
        raise SystemExit(f"DBC not found: {dbc_in}")

    print("Resolving DBC column indices from DB schema...")
    idxs = resolve_spell_indices(args.mysql)

    print("Fetching DK trainer spells from DB...")
    dk_trainer_spells = get_dk_trainer_spells(args.mysql)
    print(f"  Found {len(dk_trainer_spells)} spells in trainer {DK_TRAINER_ID}")

    print("Patching Spell.dbc...")
    patch_spell_dbc(dbc_in, dbc_out, idxs, dk_trainer_spells, args.static_tooltips)

    print("Building MPQ...")
    build_mpq(mpq_out, dbc_out)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
