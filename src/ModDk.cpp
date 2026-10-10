#include "ModDk.h"

#include "Chat.h"
#include "Config.h"
#include "Creature.h"
#include "DBCStores.h"
#include "DatabaseEnv.h"
#include "Duration.h"
#include "EventProcessor.h"
#include "GridTerrainData.h"
#include "Item.h"
#include "Log.h"
#include "MapMgr.h"
#include "ObjectMgr.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedGossip.h"
#include "SpellAuraEffects.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "SpellScriptLoader.h"
#include "StringFormat.h"
#include "Timer.h"
#include "World.h"

#include <algorithm>
#include <unordered_set>

void Addmod_dkCommands();

namespace
{
    constexpr uint32 QUEST_WHERE_KINGS_WALK            = 13188;
    constexpr uint32 QUEST_WARCHIEFS_BLESSING          = 13189;

    constexpr uint32 MAP_ACHERUS = 609;
    constexpr float DK_START_X   = 2355.84f;
    constexpr float DK_START_Y   = -5664.77f;
    constexpr float DK_START_Z   = 426.028f;
    constexpr float DK_START_O   = 3.65997f;

    constexpr uint32 ACTION_START = 1;
    constexpr uint32 ACTION_CLOSE = 2;

    // npc_text rows from mod_dk_15_reentry_epl.sql.
    constexpr uint32 NPC_TEXT_REENTRY_OFFER = 900000;
    constexpr uint32 NPC_TEXT_REENTRY_REFUSE = 900001;

    constexpr uint32 SPELL_DEATH_GATE = 50977;

    constexpr uint32 SPELL_BLOOD_PRESENCE  = 48266;
    constexpr uint32 SPELL_FROST_PRESENCE  = 48263;
    constexpr uint32 SPELL_UNHOLY_PRESENCE = 48265;
    // Child auras the core spell_dk_presence AuraScript casts while a presence is active.
    // It removes them again when the presence is removed; they can belong to any presence
    // (Improved Blood/Frost/Unholy Presence talents), so only strip them with no presence up.
    constexpr uint32 SPELL_IMPROVED_BLOOD_PRESENCE_TRIGGERED  = 63611;
    constexpr uint32 SPELL_FROST_PRESENCE_TRIGGERED           = 61261;
    constexpr uint32 SPELL_IMPROVED_UNHOLY_PRESENCE_TRIGGERED = 63622;
    constexpr uint32 SPELL_UNHOLY_PRESENCE_TRIGGERED          = 49772;

    constexpr uint32 SPELL_BLOOD_PRESENCE_HEAL  = 50475;
    constexpr uint32 SPELL_DEATH_STRIKE_HEAL    = 45470;
    constexpr uint32 SPELL_DEATH_AND_DECAY_DMG  = 52212;
    constexpr uint32 SPELL_BLOOD_PLAGUE         = 55078;
    constexpr uint32 SPELL_FROST_FEVER          = 55095;

    // Levels where the low level scaling curves reach 1.0.
    constexpr uint8 DK_DAMAGE_SCALING_END_LEVEL   = 55;
    constexpr uint8 DK_PRESENCE_SCALING_END_LEVEL = 40;
    constexpr uint8 DK_THREAT_BONUS_END_LEVEL     = 30;

    // Below this level a DK found in Ebon Hold without Death Gate is sent to their
    // racial start: the reentry NPC only takes 55+ DKs there.
    constexpr uint8 DK_EBON_HOLD_MIN_LEVEL = 55;

    // Ebon Hold intro: the quests of Plaguelands: The Scarlet Enclave plus the final
    // capital city quests are reset when the reentry NPC restarts the intro.
    constexpr int32 ZONE_SCARLET_ENCLAVE = 4298;
    constexpr uint32 QUEST_DK_INTRO_START = 12593; // In Service Of The Lich King

    // conf/mod_dk.conf.dist; reloaded with `.reload config`.
    struct DkConfig
    {
        bool scalingEnabled = true;
        float scalingStrength = 1.0f;
        bool threatBonusEnabled = true;
        bool spellProgressionEnabled = true;
        bool reentryEnabled = true;
        bool ebonHoldRescueEnabled = true;
        bool debug = false;
        int32 startLevel = 1;
        int32 minLevelForHeroicCharacter = 0;
    };

    DkConfig Config;

    void LoadDkConfig()
    {
        Config.scalingEnabled = sConfigMgr->GetOption<bool>("ModDk.Scaling.Enable", true);
        Config.scalingStrength = std::clamp(sConfigMgr->GetOption<float>("ModDk.Scaling.Strength", 1.0f), 0.0f, 1.0f);
        Config.threatBonusEnabled = sConfigMgr->GetOption<bool>("ModDk.ThreatBonus.Enable", true);
        Config.spellProgressionEnabled = sConfigMgr->GetOption<bool>("ModDk.SpellProgression.Enable", true);
        Config.reentryEnabled = sConfigMgr->GetOption<bool>("ModDk.Reentry.Enable", true);
        Config.ebonHoldRescueEnabled = sConfigMgr->GetOption<bool>("ModDk.EbonHoldRescue.Enable", true);
        Config.debug = sConfigMgr->GetOption<bool>("ModDk.Debug", false);
        Config.startLevel = sConfigMgr->GetOption<int32>("ModDk.StartLevel", 1);
        Config.minLevelForHeroicCharacter = sConfigMgr->GetOption<int32>("ModDk.MinLevelForHeroicCharacter", 0);
    }

    // ModDk.StartLevel and ModDk.MinLevelForHeroicCharacter replace the matching
    // worldserver.conf settings, unless set to -1. This runs while the core loads its
    // config at startup, before the player create data (and its stats check) is loaded.
    void ApplyCoreSettings()
    {
        if (Config.startLevel > 0)
        {
            uint32 level = std::min<uint32>(uint32(Config.startLevel), sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL));
            sWorld->setIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL, level);
        }

        if (Config.minLevelForHeroicCharacter >= 0)
            sWorld->setIntConfig(CONFIG_CHARACTER_CREATING_MIN_LEVEL_FOR_HEROIC_CHARACTER, uint32(Config.minLevelForHeroicCharacter));

        LOG_INFO("server.loading", "mod_dk: New death knights start at level {}; creating one needs a level {} character.",
            sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL), sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_MIN_LEVEL_FOR_HEROIC_CHARACTER));
    }

    // Moves a low level scale toward 1.0 by ModDk.Scaling.Strength (1.0 keeps the curve).
    float ApplyScalingConfig(float scale)
    {
        if (!Config.scalingEnabled)
            return 1.0f;

        return 1.0f - Config.scalingStrength * (1.0f - scale);
    }

    struct CurvePoint
    {
        float level;
        float value;
    };

    // Piecewise linear curves, keep them in sync with scripts/build_runeforge_client_mpq.py.
    constexpr CurvePoint DK_DAMAGE_CURVE[] =
    {
        { 1.0f, 0.08f }, { 4.0f, 0.11f }, { 12.0f, 0.22f }, { 20.0f, 0.37f },
        { 30.0f, 0.55f }, { 40.0f, 0.73f }, { 55.0f, 1.00f }
    };

    constexpr CurvePoint DK_DND_CURVE[] =
    {
        { 1.0f, 0.12f }, { 10.0f, 0.20f }, { 20.0f, 0.40f }, { 30.0f, 0.60f },
        { 40.0f, 0.78f }, { 55.0f, 1.00f }
    };

    constexpr CurvePoint DK_PRESENCE_CURVE[] =
    {
        { 1.0f, 0.60f }, { 40.0f, 1.00f }
    };

    template <size_t N>
    float SampleCurve(CurvePoint const (&curve)[N], uint8 level)
    {
        float l = static_cast<float>(level);
        if (l <= curve[0].level)
            return curve[0].value;

        for (size_t i = 1; i < N; ++i)
        {
            if (l <= curve[i].level)
            {
                CurvePoint const& a = curve[i - 1];
                CurvePoint const& b = curve[i];
                return a.value + (l - a.level) * (b.value - a.value) / (b.level - a.level);
            }
        }

        return curve[N - 1].value;
    }

    // Runeforge spells and the enchants they apply.
    constexpr uint32 RUNEFORGE_SPELLS[] =
    {
        53323, // Rune of Swordshattering
        53331, // Rune of Lichbane
        53341, // Rune of Cinderglacier
        53342, // Rune of Spellshattering
        53343, // Rune of Razorice
        53344, // Rune of the Fallen Crusader
        54446, // Rune of Swordbreaking
        54447, // Rune of Spellbreaking
        62158, // Rune of the Stoneskin Gargoyle
        70164, // Rune of the Nerubian Carapace
    };

    constexpr uint32 RUNEFORGE_ENCHANTS[] =
    {
        3365, // Swordshattering
        3366, // Lichbane
        3367, // Spellshattering
        3368, // Fallen Crusader
        3369, // Cinderglacier
        3370, // Razorice
        3594, // Swordbreaking
        3595, // Spellbreaking
        3847, // Stoneskin Gargoyle
        3883, // Nerubian Carapace
    };

    uint8 GetRacialStartClass(uint8 race)
    {
        return (race == RACE_BLOODELF || race == RACE_DRAENEI) ? 2 : 1;
    }

    // Player::GetCinematicMgr() returns a pointer on older cores and a reference on
    // current AzerothCore; only one of the two is used on a given core.
    [[maybe_unused]] CinematicMgr* AsCinematicMgr(CinematicMgr* cm)
    {
        return cm;
    }

    [[maybe_unused]] CinematicMgr* AsCinematicMgr(CinematicMgr& cm)
    {
        return &cm;
    }

    struct DkSpellRule
    {
        uint32 spellId;
        uint8 minLevel;
    };

    std::vector<DkSpellRule> DkSpellRules;

    bool IsDk(Player const* player)
    {
        return player && player->getClass() == CLASS_DEATH_KNIGHT;
    }

    float GetLowLevelDkScale(Player* player)
    {
        if (!IsDk(player))
            return 1.0f;

        uint8 level = player->GetLevel();
        if (level >= DK_DAMAGE_SCALING_END_LEVEL)
            return 1.0f;

        return ApplyScalingConfig(SampleCurve(DK_DAMAGE_CURVE, level));
    }

    float GetDkNonWeaponScaleModifier(uint32 spellId)
    {
        switch (spellId)
        {
            // Single-target threat builders / runic dumps
            case 45477: // Icy Touch
            case 49896:
            case 49903:
            case 49904:
            case 49909:
                return 0.9f;
            case 47541: // Death Coil
            case 47632:
            case 49892:
            case 49893:
            case 49894:
            case 49895:
                return 0.8f;
            // AoE threat, keep slightly softer early
            case 48721: // Blood Boil
            case 49939:
            case 49940:
            case 49941:
                return 0.8f;
            default:
                return 1.0f;
        }
    }

    // The per spell modifier fades out as the level scale approaches 1.0, so there is
    // no damage jump at the end of the curve.
    float GetDkNonWeaponScale(Player* player, uint32 spellId)
    {
        float scale = GetLowLevelDkScale(player);
        if (scale >= 1.0f)
            return 1.0f;

        float modifier = GetDkNonWeaponScaleModifier(spellId);
        return scale * (1.0f - (1.0f - modifier) * (1.0f - scale));
    }

    float GetLowLevelPresenceScale(Player* player)
    {
        if (!IsDk(player))
            return 1.0f;

        uint8 level = player->GetLevel();
        if (level >= DK_PRESENCE_SCALING_END_LEVEL)
            return 1.0f;

        return ApplyScalingConfig(SampleCurve(DK_PRESENCE_CURVE, level));
    }

    float GetLowLevelDndScale(Player* player)
    {
        if (!IsDk(player))
            return 1.0f;

        uint8 level = player->GetLevel();
        if (level >= DK_DAMAGE_SCALING_END_LEVEL)
            return 1.0f;

        return ApplyScalingConfig(SampleCurve(DK_DND_CURVE, level));
    }

    bool HasTankPresence(Player* player)
    {
        return player && (player->HasAura(SPELL_BLOOD_PRESENCE) || player->HasAura(SPELL_FROST_PRESENCE));
    }

    float GetDkThreatSpellBonus(uint32 spellId)
    {
        switch (spellId)
        {
            case 45477: // Icy Touch
            case 49896:
            case 49903:
            case 49904:
            case 49909:
                return 1.25f;
            case 48721: // Blood Boil
            case 49939:
            case 49940:
            case 49941:
                return 1.15f;
            case 43265: // Death and Decay
            case 49936:
            case 49937:
            case 49938:
            case 52212:
                return 1.10f;
            default:
                return 1.0f;
        }
    }

    float GetLowLevelThreatBonus(Player* player, uint32 spellId)
    {
        if (!Config.threatBonusEnabled || !IsDk(player))
            return 0.0f;

        if (!HasTankPresence(player))
            return 0.0f;

        uint8 level = player->GetLevel();
        if (level >= DK_THREAT_BONUS_END_LEVEL)
            return 0.0f;

        float t = (float(DK_THREAT_BONUS_END_LEVEL) - static_cast<float>(level)) / float(DK_THREAT_BONUS_END_LEVEL - 1);
        float baseBonus = 1.25f * t;
        return baseBonus * GetDkThreatSpellBonus(spellId);
    }

    // Threat lives on the creature's threat list, so the victim adds it for the attacker.
    void AddDkThreat(Player* player, Unit* target, float threat, SpellInfo const* spellInfo)
    {
        if (!player || !target || threat <= 0.0f)
            return;

        target->AddThreat(player, threat, spellInfo->GetSchoolMask(), spellInfo);
    }

    // Spells whose damage or healing a module SpellScript/AuraScript already scales.
    // The generic UnitScript below skips them so they are not scaled twice.
    std::unordered_set<uint32> const& GetScriptScaledSpells()
    {
        static std::unordered_set<uint32> const spells =
        {
            // spell_mod_dk_low_level_damage
            45477, 49896, 49903, 49904, 49909,                      // Icy Touch
            45462, 49917, 49918, 49919, 49920, 49921,               // Plague Strike
            66216, 66988, 66989, 66990, 66991, 66992,
            45902, 49926, 49927, 49928, 49929, 49930,               // Blood Strike
            66215, 66975, 66976, 66977, 66978, 66979,
            47541, 47632, 49892, 49893, 49894, 49895,               // Death Coil
            48721, 49939, 49940, 49941,                             // Blood Boil
            49998, 49999, 45463, 49923, 49924,                      // Death Strike
            66188, 66950, 66951, 66952, 66953,
            49020, 51423, 51424, 51425,                             // Obliterate
            66198, 66972, 66973, 66974,
            // spell_mod_dk_low_level_dot
            SPELL_BLOOD_PLAGUE, SPELL_FROST_FEVER,
            // spell_mod_dk_dnd_scale (scales the aura amount that feeds the damage spell)
            SPELL_DEATH_AND_DECAY_DMG,
            // spell_mod_dk_blood_presence_heal
            SPELL_BLOOD_PRESENCE_HEAL,
        };
        return spells;
    }

    bool IsDkSpell(SpellInfo const* spellInfo)
    {
        return spellInfo && (spellInfo->SpellFamilyName == SPELLFAMILY_DEATHKNIGHT || spellInfo->Id == SPELL_DEATH_STRIKE_HEAL);
    }

    bool IsDkIntroComplete(Player* player)
    {
        uint32 questId = (player->GetTeamId() == TEAM_ALLIANCE) ? QUEST_WHERE_KINGS_WALK : QUEST_WARCHIEFS_BLESSING;
        return player->GetQuestRewardStatus(questId);
    }

    bool CanOfferDkReentry(Player* player)
    {
        if (!player)
            return false;

        if (!Config.reentryEnabled || player->getClass() != CLASS_DEATH_KNIGHT)
            return false;

        if (player->GetLevel() < DK_EBON_HOLD_MIN_LEVEL)
            return false;

        if (IsDkIntroComplete(player))
            return false;

        return true;
    }

    void LoadDkSpellRules()
    {
        DkSpellRules.clear();

        QueryResult result = WorldDatabase.Query("SELECT spell_id, min_level FROM mod_dk_spell_progression");
        if (!result)
        {
            LOG_WARN("module.dkd", "mod_dk: No spell progression data found in mod_dk_spell_progression.");
            return;
        }

        do
        {
            Field* fields = result->Fetch();
            DkSpellRules.push_back({ fields[0].Get<uint32>(), fields[1].Get<uint8>() });
        } while (result->NextRow());

        LOG_INFO("module.dkd", "mod_dk: Loaded {} spell progression rules.", DkSpellRules.size());
    }

    // Lets runeforge spells target low level weapons and makes the rune enchants work at
    // any level, in memory, so no spell_dbc rows or server DBC edits are needed.
    uint32 RuneforgeSpellsPatched = 0;
    uint32 RuneforgeEnchantsPatched = 0;
    uint32 SkillLineAbilitiesPatched = 0;

    constexpr uint32 SPELL_RUNEFORGING = 53428;

    bool IsDkSkillLine(uint32 skillLine)
    {
        // Blood, Frost, Unholy, Runeforging
        return skillLine == 770 || skillLine == 771 || skillLine == 772 || skillLine == 776;
    }

    bool IsAutoLearned(SkillLineAbilityEntry const* ability)
    {
        return ability->AcquireMethod == SKILL_LINE_ABILITY_LEARNED_ON_SKILL_VALUE || ability->AcquireMethod == SKILL_LINE_ABILITY_LEARNED_ON_SKILL_LEARN;
    }

    // Spells of TrainerId 1013 that the core must not hand out on its own. Level 1
    // spells are taught by the progression, and learning Runeforging is what gives the
    // Runeforging skill (Player::learnSpell), so both keep their SkillLineAbility flag.
    std::unordered_set<uint32> GetTrainerOnlySpells()
    {
        std::unordered_set<uint32> spells;
        if (QueryResult result = WorldDatabase.Query("SELECT `SpellId` FROM `trainer_spell` WHERE `TrainerId` = 1013"))
        {
            do
                spells.insert(result->Fetch()[0].Get<uint32>());
            while (result->NextRow());
        }

        for (DkSpellRule const& rule : DkSpellRules)
            if (rule.minLevel <= 1)
                spells.erase(rule.spellId);

        spells.erase(SPELL_RUNEFORGING);
        return spells;
    }

    // The core teaches every DK class spell of a skill line the DK has from
    // SkillLineAbility.dbc (Player::learnSkillRewardedSpells, at creation and on level up,
    // without a level check), which bypasses the trainer. Clearing the flag in memory
    // leaves those spells to the trainer, without editing the server DBC file.
    // Skipped when DKs start at 55 and play the stock intro, which relies on it.
    void ApplySkillLineAbilityCorrections()
    {
        if (sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL) >= DK_EBON_HOLD_MIN_LEVEL)
            return;

        std::unordered_set<uint32> trainerOnly = GetTrainerOnlySpells();
        uint32 patched = 0;
        for (SkillLineAbilityEntry const* ability : sSkillLineAbilityStore)
        {
            if (!IsDkSkillLine(ability->SkillLine) || !IsAutoLearned(ability) || !trainerOnly.count(ability->Spell))
                continue;

            const_cast<SkillLineAbilityEntry*>(ability)->AcquireMethod = 0;
            ++patched;
        }

        SkillLineAbilitiesPatched += patched;
        LOG_INFO("module.dkd", "mod_dk: Auto learning turned off for {} trainer DK spells.", patched);
    }

    void ApplyRuneforgeCorrections()
    {
        uint32 spellsPatched = 0;
        for (uint32 spellId : RUNEFORGE_SPELLS)
        {
            SpellInfo* spellInfo = const_cast<SpellInfo*>(sSpellMgr->GetSpellInfo(spellId));
            if (!spellInfo)
            {
                LOG_WARN("module.dkd", "mod_dk: Runeforge spell {} not found.", spellId);
                continue;
            }

            spellInfo->AttributesEx2 |= SPELL_ATTR2_ALLOW_LOW_LEVEL_BUFF;
            spellInfo->BaseLevel = 1;
            spellInfo->MaxLevel = 0;
            ++spellsPatched;
        }

        uint32 enchantsPatched = 0;
        for (uint32 enchantId : RUNEFORGE_ENCHANTS)
        {
            SpellItemEnchantmentEntry* enchant = const_cast<SpellItemEnchantmentEntry*>(sSpellItemEnchantmentStore.LookupEntry(enchantId));
            if (!enchant)
            {
                LOG_WARN("module.dkd", "mod_dk: Runeforge enchant {} not found.", enchantId);
                continue;
            }

            if (enchant->requiredLevel > 1 || enchant->requiredSkillValue > 1)
            {
                LOG_INFO("module.dkd", "mod_dk: Rune enchant {} required level {} and skill {} rank {}, lowered to level 0 rank 1.",
                    enchantId, enchant->requiredLevel, enchant->requiredSkill, enchant->requiredSkillValue);
            }

            // Player::ApplyEnchantment skips the enchant's stat and equip effects below these.
            enchant->requiredLevel = 0;
            if (enchant->requiredSkillValue > 1)
                enchant->requiredSkillValue = 1;

            // Bind the weapon when the rune is applied.
            enchant->slot |= ENCHANTMENT_CAN_SOULBOUND;
            ++enchantsPatched;
        }

        RuneforgeSpellsPatched = spellsPatched;
        RuneforgeEnchantsPatched = enchantsPatched;
        LOG_INFO("module.dkd", "mod_dk: Runeforge corrections applied to {} spells and {} enchants.", spellsPatched, enchantsPatched);
    }

    void ApplyDkSpellProgression(Player* player)
    {
        if (!Config.spellProgressionEnabled || !IsDk(player) || DkSpellRules.empty())
            return;

        uint8 level = player->GetLevel();
        for (auto const& rule : DkSpellRules)
        {
            // Level 1 spells (Plague Strike) are known from the start, so a fresh DK
            // can fight before finding a trainer.
            if (rule.minLevel <= 1)
            {
                if (!player->HasSpell(rule.spellId))
                    player->learnSpell(rule.spellId, false);
                continue;
            }

            if (level < rule.minLevel)
            {
                if (player->HasSpell(rule.spellId))
                    player->removeSpell(rule.spellId, SPEC_MASK_ALL, false);
                // Also strip any active aura: removeSpell only removes from the spellbook,
                // leaving a previously cast aura (e.g. Blood Presence) still active.
                // Removing a presence also removes its child auras (core spell_dk_presence).
                player->RemoveAurasDueToSpell(rule.spellId);
            }
        }

        // Child auras loaded from character_aura without any presence would linger.
        // Never strip them while a presence is up: Blood Presence's heal is one of them.
        if (!player->HasAura(SPELL_BLOOD_PRESENCE) && !player->HasAura(SPELL_FROST_PRESENCE) && !player->HasAura(SPELL_UNHOLY_PRESENCE))
        {
            player->RemoveAurasDueToSpell(SPELL_IMPROVED_BLOOD_PRESENCE_TRIGGERED);
            player->RemoveAurasDueToSpell(SPELL_FROST_PRESENCE_TRIGGERED);
            player->RemoveAurasDueToSpell(SPELL_IMPROVED_UNHOLY_PRESENCE_TRIGGERED);
            player->RemoveAurasDueToSpell(SPELL_UNHOLY_PRESENCE_TRIGGERED);
        }
    }

    // Presence amounts are scaled when they are calculated; recalculate them when the
    // level changes so the new scale applies without recasting.
    void RefreshPresenceScaling(Player* player)
    {
        if (!IsDk(player))
            return;

        for (uint32 spellId : { SPELL_BLOOD_PRESENCE, SPELL_FROST_PRESENCE, SPELL_FROST_PRESENCE_TRIGGERED })
            if (Aura* aura = player->GetAura(spellId))
                if (!aura->IsRemoved())
                    aura->RecalculateAmountOfEffects();
    }

    std::vector<uint32> DkIntroQuests;

    // Collects every Ebon Hold intro quest, so the reentry NPC resets the whole intro:
    // rewarded quests left over from a half finished intro break the Acherus phasing.
    void LoadDkIntroQuests()
    {
        std::unordered_set<uint32> quests;
        for (auto const& [questId, quest] : sObjectMgr->GetQuestTemplates())
            if (quest->GetZoneOrSort() == ZONE_SCARLET_ENCLAVE)
                quests.insert(questId);

        // The start of the chain (in case the zone data differs) and its capital city end.
        for (uint32 questId : { 12593u, 12619u, 12842u, 12848u, 12636u, 12641u, 12657u, 12850u, 12670u, 12678u,
                                QUEST_WHERE_KINGS_WALK, QUEST_WARCHIEFS_BLESSING })
            if (sObjectMgr->GetQuestTemplate(questId))
                quests.insert(questId);

        DkIntroQuests.assign(quests.begin(), quests.end());
        LOG_INFO("module.dkd", "mod_dk: Loaded {} Ebon Hold intro quests for the reentry NPC.", DkIntroQuests.size());
    }

    // Checked before the reentry teleport, so a full quest log can't leave the DK in
    // Ebon Hold without the first quest. The reset frees the slots of the intro's own quests.
    bool CanRestartDkIntroChain(Player* player)
    {
        Quest const* quest = sObjectMgr->GetQuestTemplate(QUEST_DK_INTRO_START);
        if (!quest)
        {
            LOG_ERROR("module.dkd", "mod_dk: Quest {} is missing, the reentry NPC can't restart the Ebon Hold intro.", QUEST_DK_INTRO_START);
            return false;
        }

        for (uint32 questId : DkIntroQuests)
            if (player->FindQuestSlot(questId) < MAX_QUEST_LOG_SIZE)
                return true;

        return player->CanAddQuest(quest, true);
    }

    void RestartDkIntroChain(Player* player, Object* questGiver)
    {
        if (!IsDk(player))
            return;

        // Clear any existing state so scripts and phasing run from a clean slate.
        for (uint32 qid : DkIntroQuests)
        {
            player->RemoveActiveQuest(qid, true);
            player->RemoveRewardedQuest(qid, true);
            player->SetQuestStatus(qid, QUEST_STATUS_NONE, true);
        }

        // Add the starting quest.
        if (Quest const* q = sObjectMgr->GetQuestTemplate(QUEST_DK_INTRO_START))
        {
            if (!player->CanAddQuest(q, true))
            {
                LOG_ERROR("module.dkd", "mod_dk: Could not give {} quest {} after the reentry teleport.", player->GetName(), QUEST_DK_INTRO_START);
                return;
            }
            player->AddQuest(q, questGiver);
        }
    }

    bool IsFreshLowLevelDk(Player* player, bool isFirstLogin)
    {
        if (!player)
            return false;

        if (!isFirstLogin)
            return false;

        if (player->getClass() != CLASS_DEATH_KNIGHT)
            return false;

        // StartHeroicPlayerLevel 55 (stock) plays the normal Ebon Hold intro instead.
        if (player->GetLevel() >= DK_EBON_HOLD_MIN_LEVEL)
            return false;

        return true;
    }

    // A DK below the reentry level found in Ebon Hold, for example after logging out or
    // disconnecting during the intro cinematic. They can leave (see OnPlayerIsClass), but
    // are sent to their racial start on login so they don't have to find their way out.
    bool IsStrandedInEbonHold(Player* player)
    {
        if (!IsDk(player) || player->IsGameMaster())
            return false;

        if (player->GetMapId() != MAP_ACHERUS || player->HasSpell(SPELL_DEATH_GATE))
            return false;

        return player->GetLevel() < DK_EBON_HOLD_MIN_LEVEL;
    }

    void SetRacialHomebind(Player* player)
    {
        if (!player)
            return;

        uint8 race = player->getRace();
        uint8 startClass = GetRacialStartClass(race);
        PlayerInfo const* info = sObjectMgr->GetPlayerInfo(race, startClass);
        if (!info)
            return;

        player->SetHomebind(
            WorldLocation(info->mapId, info->positionX, info->positionY, info->positionZ, info->orientation),
            info->areaId);
    }

    bool TeleportToRacialStart(Player* player)
    {
        if (!player)
            return false;

        uint8 race = player->getRace();
        uint8 startClass = GetRacialStartClass(race);

        PlayerInfo const* info = sObjectMgr->GetPlayerInfo(race, startClass);

        if (!info)
            return false;

        bool success = player->TeleportTo(
            info->mapId,
            info->positionX,
            info->positionY,
            info->positionZ,
            info->orientation
        );

        if (success)
            player->SetHomebind(WorldLocation(info->mapId, info->positionX, info->positionY, info->positionZ, info->orientation), info->areaId);

        return success;
    }

    // Tells the DK why they are leaving Ebon Hold, then sends them to the racial start.
    // OnPlayerIsClass lets low level DKs leave without Death Gate.
    void SendDkToRacialStart(Player* player, char const* message)
    {
        ChatHandler(player->GetSession()).SendSysMessage(message);
        if (!TeleportToRacialStart(player))
            LOG_ERROR("module.dkd", "mod_dk: Failed to send {} from Ebon Hold to their racial start.", player->GetName());
    }

    constexpr char const* MSG_RELEASED = "The Lich King's grip on you is broken. Return to your homeland, death knight: "
        "Madness, your trainer, waits beside the priests, with a runeforge close by.";
    constexpr char const* MSG_RESCUED = "You awaken far from Ebon Hold. Madness, your trainer, waits beside the priests.";

    class DkTeleportAfterCinematicEvent : public BasicEvent
    {
    public:
        explicit DkTeleportAfterCinematicEvent(ObjectGuid guid, uint32 startMs, bool sawCinematic, bool isFirstLogin)
            : _guid(guid)
            , _startMs(startMs)
            , _sawCinematic(sawCinematic)
            , _isFirstLogin(isFirstLogin)
        {
        }

        bool Execute(uint64 /*execTime*/, uint32 /*diff*/) override
        {
            Player* player = ObjectAccessor::FindPlayer(_guid);
            if (!player)
                return true;

            if (!player->IsInWorld())
            {
                Reschedule(player);
                return true;
            }

            if (!IsFreshLowLevelDk(player, _isFirstLogin))
                return true;

            CinematicMgr* cm = AsCinematicMgr(player->GetCinematicMgr());
            if (!cm)
            {
                Reschedule(player);
                return true;
            }

            bool activeCamera = (cm->GetActiveCinematicCamera() != 0);
            if (activeCamera)
                _sawCinematic = true;

            if (_sawCinematic)
            {
                if (cm->IsOnCinematic() || activeCamera)
                {
                    Reschedule(player);
                    return true;
                }
            }
            else
            {
                if (GetMSTimeDiffToNow(_startMs) < 10000)
                {
                    Reschedule(player);
                    return true;
                }
            }

            if (player->IsBeingTeleported() ||
                player->IsInFlight() ||
                player->IsInCombat())
            {
                Reschedule(player);
                return true;
            }

            SendDkToRacialStart(player, MSG_RELEASED);
            return true;
        }

    private:
        void Reschedule(Player* player)
        {
            player->m_Events.AddEvent(new DkTeleportAfterCinematicEvent(_guid, _startMs, _sawCinematic, _isFirstLogin), player->m_Events.CalculateTime(200));
        }

        ObjectGuid _guid;
        uint32 _startMs;
        bool _sawCinematic;
        bool _isFirstLogin;
    };

    class DkRescueFromEbonHoldEvent : public BasicEvent
    {
    public:
        explicit DkRescueFromEbonHoldEvent(ObjectGuid guid) : _guid(guid) { }

        bool Execute(uint64 /*execTime*/, uint32 /*diff*/) override
        {
            Player* player = ObjectAccessor::FindPlayer(_guid);
            if (!player)
                return true;

            if (!player->IsInWorld() || player->IsBeingTeleported() || player->IsInCombat())
            {
                player->m_Events.AddEvent(new DkRescueFromEbonHoldEvent(_guid), player->m_Events.CalculateTime(500));
                return true;
            }

            if (!IsStrandedInEbonHold(player))
                return true;

            LOG_INFO("module.dkd", "mod_dk: Sending {} (level {}) from Ebon Hold to their racial start.", player->GetName(), player->GetLevel());
            SendDkToRacialStart(player, MSG_RESCUED);
            return true;
        }

    private:
        ObjectGuid _guid;
    };

    void ScheduleDkTeleportAfterCinematic(Player* player)
    {
        bool isFirstLogin = player && player->HasAtLoginFlag(AT_LOGIN_FIRST);
        if (!IsFreshLowLevelDk(player, isFirstLogin))
            return;

        player->m_Events.AddEvent(new DkTeleportAfterCinematicEvent(player->GetGUID(), getMSTime(), false, isFirstLogin),
            player->m_Events.CalculateTime(200));
    }
}

class npc_dk_reentry : public CreatureScript
{
public:
    npc_dk_reentry() : CreatureScript("npc_dk_reentry") { }

    uint32 GetDialogStatus(Player* player, Creature* /*creature*/) override
    {
        return CanOfferDkReentry(player) ? DIALOG_STATUS_AVAILABLE : DIALOG_STATUS_NONE;
    }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        ClearGossipMenuFor(player);

        if (!CanOfferDkReentry(player))
        {
            SendGossipMenuFor(player, NPC_TEXT_REENTRY_REFUSE, creature->GetGUID());
            return true;
        }

        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I am ready to answer the call.", GOSSIP_SENDER_MAIN, ACTION_START,
            "Return to Acherus and start the Ebon Hold story over? Your Ebon Hold quest progress is reset.", 0, false);
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Not yet.", GOSSIP_SENDER_MAIN, ACTION_CLOSE);
        SendGossipMenuFor(player, NPC_TEXT_REENTRY_OFFER, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action) override
    {
        ClearGossipMenuFor(player);

        if (action == ACTION_CLOSE)
        {
            CloseGossipMenuFor(player);
            return true;
        }

        if (action != ACTION_START || !CanOfferDkReentry(player))
        {
            CloseGossipMenuFor(player);
            return true;
        }

        if (player->IsInCombat() || player->IsInFlight())
        {
            CloseGossipMenuFor(player);
            return true;
        }
        CloseGossipMenuFor(player);

        if (!CanRestartDkIntroChain(player))
            return true;

        // The core only lets DKs leave Ebon Hold with Death Gate (trainable at 55), so a
        // DK who gives up on the intro is never stuck there.
        if (!player->HasSpell(SPELL_DEATH_GATE))
            player->learnSpell(SPELL_DEATH_GATE, false);

        LOG_INFO("module.dkd", "Restarting DK intro chain for {} (race {}).", player->GetName(), player->getRace());

        if (!player->TeleportTo(MAP_ACHERUS, DK_START_X, DK_START_Y, DK_START_Z, DK_START_O))
        {
            LOG_ERROR("module.dkd", "Failed to teleport DK {} (race {}) to Acherus for reentry.", player->GetName(), player->getRace());
            return true;
        }

        // Start DK intro chain after gossip; delay helps ensure UI settles.
        player->m_Events.AddEventAtOffset([player]() {
            if (!player || !player->IsInWorld())
                return;
            RestartDkIntroChain(player, nullptr);
        }, Milliseconds(750));
        return true;
    }

};

class mod_dk_worldscript : public WorldScript
{
public:
    mod_dk_worldscript() : WorldScript("mod_dk_worldscript") { }

    void OnAfterConfigLoad(bool reload) override
    {
        LoadDkConfig();

        // Only at startup: the DK stats check and the auto learning setup depend on it.
        if (!reload)
            ApplyCoreSettings();
    }

    void OnStartup() override
    {
        LoadDkSpellRules();
        LoadDkIntroQuests();
        ApplyRuneforgeCorrections();
        ApplySkillLineAbilityCorrections();

        bool problems = false;
        for (ModDk::StatusLine const& line : ModDk::CollectStatus())
        {
            if (line.problem)
            {
                LOG_WARN("module.dkd", "mod_dk: Setup check: {}", line.text);
                problems = true;
            }
        }

        if (!problems)
            LOG_INFO("module.dkd", "mod_dk: Setup check passed.");
    }
};

class mod_dk_playerscript : public PlayerScript
{
public:
    mod_dk_playerscript() : PlayerScript("mod_dk_playerscript") { }

    void OnPlayerLogin(Player* player) override
    {
        bool isFirstLogin = player && player->HasAtLoginFlag(AT_LOGIN_FIRST);
        // Set homebind immediately so it is correct even if the player logs out
        // before the post-cinematic teleport event fires (which also calls SetHomebind).
        if (IsFreshLowLevelDk(player, isFirstLogin))
            SetRacialHomebind(player);

        ScheduleDkTeleportAfterCinematic(player);
        ApplyDkSpellProgression(player);
        RefreshPresenceScaling(player);

        if (Config.ebonHoldRescueEnabled && !isFirstLogin && IsStrandedInEbonHold(player))
            player->m_Events.AddEvent(new DkRescueFromEbonHoldEvent(player->GetGUID()), player->m_Events.CalculateTime(1000));
    }

    void OnPlayerLevelChanged(Player* player, uint8 /*oldLevel*/) override
    {
        ApplyDkSpellProgression(player);
        RefreshPresenceScaling(player);
    }

    Optional<bool> OnPlayerIsClass(Player const* player, Classes playerClass, ClassContext context) override
    {
        if (playerClass != CLASS_DEATH_KNIGHT || !IsDk(player))
            return std::nullopt;

        switch (context)
        {
            // In Ebon Hold the core gives DKs only (level - 55) + quest talents, which
            // resets the talents of a DK leveled from 1 who logs in or turns in a quest there.
            case CLASS_CONTEXT_TALENT_POINT_CALC:
            // New weapon skills start at 1 like for other classes instead of (level - 1) * 5.
            case CLASS_CONTEXT_SKILL:
            // Only consulted by patches/dk-taxi-nodes.patch: no free old world flight paths.
            case CLASS_CONTEXT_TAXI:
                return false;
            // The core keeps DKs without Death Gate in Ebon Hold until they finish the intro.
            // DKs below 55 skip the intro, so they may always leave (hearthstone included).
            case CLASS_CONTEXT_TELEPORT:
                if (player->GetLevel() < DK_EBON_HOLD_MIN_LEVEL)
                    return false;
                return std::nullopt;
            default:
                return std::nullopt;
        }
    }
};

class spell_mod_dk_low_level_damage : public SpellScriptLoader
{
public:
    spell_mod_dk_low_level_damage() : SpellScriptLoader("spell_mod_dk_low_level_damage") { }

    class spell_mod_dk_low_level_damage_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_mod_dk_low_level_damage_SpellScript);

        bool IsWeaponStrikeSpell(uint32 spellId) const
        {
            switch (spellId)
            {
                case 45462: // Plague Strike
                case 49917:
                case 49918:
                case 49919:
                case 49920:
                case 49921:
                case 66216: // Plague Strike triggered hit
                case 66988:
                case 66989:
                case 66990:
                case 66991:
                case 66992:
                case 45902: // Blood Strike
                case 49926:
                case 49927:
                case 49928:
                case 49929:
                case 49930:
                case 66215: // Blood Strike triggered hit
                case 66975:
                case 66976:
                case 66977:
                case 66978:
                case 66979:
                case 49998: // Death Strike
                case 49999:
                case 45463:
                case 49923:
                case 49924:
                case 66188: // Death Strike triggered hit
                case 66950:
                case 66951:
                case 66952:
                case 66953:
                case 49020: // Obliterate
                case 51423:
                case 51424:
                case 51425:
                case 66198: // Obliterate triggered hit
                case 66972:
                case 66973:
                case 66974:
                    return true;
                default:
                    return false;
            }
        }

        // Weapon strikes keep their weapon damage and scale only the flat bonus by
        // level / 55. The core builds the damage in DBC effect order (Spell::EffectWeaponDmg),
        // so a % effect after the flat bonus effect gives (weapon + flat) * pct. The ratio is
        // taken on the average weapon damage so it does not depend on the hit's own roll.
        void ScaleWeaponStrikeHit(Player* player)
        {
            uint8 level = player->GetLevel();
            if (level >= DK_DAMAGE_SCALING_END_LEVEL)
                return;

            int32 baseHit = GetHitDamage();
            if (baseHit <= 0)
                return;

            SpellInfo const* info = GetSpellInfo();

            float weaponPct = 1.0f;
            float flatBonus = 0.0f;
            bool normalized = false;
            bool seenFlat = false;
            bool pctAfterFlat = false;

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            {
                switch (info->Effects[i].Effect)
                {
                    case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
                        weaponPct = float(info->Effects[i].CalcValue(player)) / 100.0f;
                        pctAfterFlat = seenFlat;
                        break;
                    case SPELL_EFFECT_WEAPON_DAMAGE:
                    case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
                        flatBonus += float(info->Effects[i].CalcValue(player));
                        seenFlat = true;
                        break;
                    case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
                        flatBonus += float(info->Effects[i].CalcValue(player));
                        normalized = true;
                        seenFlat = true;
                        break;
                    default:
                        break;
                }
            }

            if (flatBonus <= 0.0f)
                return;

            WeaponAttackType attType = info->HasAttribute(SPELL_ATTR3_REQUIRES_OFF_HAND_WEAPON) ? OFF_ATTACK : BASE_ATTACK;

            float minDamage = 0.0f;
            float maxDamage = 0.0f;
            player->CalculateMinMaxDamage(attType, normalized, true, minDamage, maxDamage, 0);
            float avgWeapon = (minDamage + maxDamage) / 2.0f;

            // The core multiplies the flat bonus by the weapon's total pct modifiers.
            float flatPct = player->GetPctModifierValue(attType == OFF_ATTACK ? UNIT_MOD_DAMAGE_OFFHAND : UNIT_MOD_DAMAGE_MAINHAND, TOTAL_PCT);

            auto expected = [&](float flat)
            {
                flat *= flatPct;
                return pctAfterFlat ? (avgWeapon + flat) * weaponPct : avgWeapon * weaponPct + flat;
            };

            float levelScale = ApplyScalingConfig(float(level) / float(DK_DAMAGE_SCALING_END_LEVEL));
            float full = expected(flatBonus);
            if (full <= 0.0f)
                return;

            float ratio = expected(flatBonus * levelScale) / full;
            int32 newDamage = std::max(1, int32(float(baseHit) * ratio));
            SetHitDamage(newDamage);

            if (Config.debug)
            {
                LOG_INFO("module", "mod_dk: Weapon strike hit {} -> {} for spell {} (caster {}, level {}, weaponPct {}, flat {}, pctAfterFlat {}).",
                    baseHit, newDamage, info->Id, player->GetName(), level, weaponPct, flatBonus, pctAfterFlat);
            }
        }

        void ScaleHitDamage()
        {
            Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
            if (!IsDk(player))
                return;

            uint32 spellId = GetSpellInfo()->Id;
            if (IsWeaponStrikeSpell(spellId))
            {
                ScaleWeaponStrikeHit(player);
                return;
            }

            float scale = GetDkNonWeaponScale(player, spellId);
            if (scale >= 1.0f)
                return;

            int32 damage = GetHitDamage();
            if (damage <= 0)
                return;

            int32 scaled = std::max(1, int32(float(damage) * scale));
            SetHitDamage(scaled);

            if (Config.debug)
            {
                LOG_INFO("module", "mod_dk: Scaled hit {} -> {} for spell {} (caster {}, level {}).",
                    damage, scaled, spellId, player->GetName(), player->GetLevel());
            }
        }

        void ApplyLowLevelThreatBonus()
        {
            Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
            Unit* target = GetHitUnit();
            if (!player || !target)
                return;

            float bonus = GetLowLevelThreatBonus(player, GetSpellInfo()->Id);
            if (bonus <= 0.0f)
                return;

            int32 damage = GetHitDamage();
            if (damage <= 0)
                return;

            float extraThreat = float(damage) * bonus;
            AddDkThreat(player, target, extraThreat, GetSpellInfo());

            if (Config.debug)
            {
                LOG_INFO("module", "mod_dk: Added low-level threat {} for spell {} (caster {}, level {}).",
                    extraThreat, GetSpellInfo()->Id, player->GetName(), player->GetLevel());
            }
        }

        // OnHit runs once per target with that target's damage, so AoE spells such as
        // Blood Boil are scaled on every target.
        void HandleHit()
        {
            ScaleHitDamage();
            ApplyLowLevelThreatBonus();
        }

        void Register() override
        {
            OnHit += SpellHitFn(spell_mod_dk_low_level_damage_SpellScript::HandleHit);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_mod_dk_low_level_damage_SpellScript();
    }
};

// The aura scripts below scale in DoEffectCalcAmount, which runs once per aura effect and
// again whenever the amount is recalculated (disease refresh, loading from the DB). Scaling
// in OnEffectApply instead ran once per target of area auras, was skipped on refresh and
// was applied again on top of the saved amount at login.

class spell_mod_dk_low_level_dot : public SpellScriptLoader
{
public:
    spell_mod_dk_low_level_dot() : SpellScriptLoader("spell_mod_dk_low_level_dot") { }

    class spell_mod_dk_low_level_dot_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_mod_dk_low_level_dot_AuraScript);

        void CalcAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
        {
            Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
            float scale = GetLowLevelDkScale(player);
            if (scale < 1.0f)
                amount = int32(float(amount) * scale);
        }

        void Register() override
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mod_dk_low_level_dot_AuraScript::CalcAmount, EFFECT_ALL, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_mod_dk_low_level_dot_AuraScript();
    }
};

class spell_mod_dk_presence_scale : public SpellScriptLoader
{
public:
    spell_mod_dk_presence_scale() : SpellScriptLoader("spell_mod_dk_presence_scale") { }

    class spell_mod_dk_presence_scale_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_mod_dk_presence_scale_AuraScript);

        static bool IsScaledAuraType(AuraType type)
        {
            switch (type)
            {
                // Threat stays at full strength so low level tanking still works.
                case SPELL_AURA_MOD_THREAT:
                case SPELL_AURA_MOD_TOTAL_THREAT:
                case SPELL_AURA_MOD_SHAPESHIFT:
                case SPELL_AURA_DUMMY:
                case SPELL_AURA_PERIODIC_DUMMY:
                case SPELL_AURA_PROC_TRIGGER_SPELL:
                case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
                case SPELL_AURA_ADD_FLAT_MODIFIER:
                case SPELL_AURA_ADD_PCT_MODIFIER:
                    return false;
                default:
                    return true;
            }
        }

        void CalcAmount(AuraEffect const* aurEff, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (!amount || !IsScaledAuraType(aurEff->GetAuraType()))
                return;

            Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
            float scale = GetLowLevelPresenceScale(player);
            if (scale < 1.0f)
                amount = int32(float(amount) * scale);
        }

        void Register() override
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mod_dk_presence_scale_AuraScript::CalcAmount, EFFECT_ALL, SPELL_AURA_ANY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_mod_dk_presence_scale_AuraScript();
    }
};

class spell_mod_dk_dnd_scale : public SpellScriptLoader
{
public:
    spell_mod_dk_dnd_scale() : SpellScriptLoader("spell_mod_dk_dnd_scale") { }

    class spell_mod_dk_dnd_scale_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_mod_dk_dnd_scale_AuraScript);

        void CalcAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
        {
            Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
            float scale = GetLowLevelDndScale(player);
            if (scale < 1.0f)
                amount = int32(float(amount) * scale);
        }

        void HandlePeriodic(AuraEffect const* aurEff)
        {
            Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
            Unit* target = GetTarget();
            if (!player || !target)
                return;

            float bonus = GetLowLevelThreatBonus(player, GetSpellInfo()->Id);
            if (bonus <= 0.0f)
                return;

            int32 amount = aurEff ? aurEff->GetAmount() : 0;
            if (amount <= 0)
                return;

            float extraThreat = float(amount) * bonus;
            AddDkThreat(player, target, extraThreat, GetSpellInfo());

            if (Config.debug)
            {
                LOG_INFO("module", "mod_dk: Added low-level DnD threat {} for spell {} (caster {}, level {}).",
                    extraThreat, GetSpellInfo()->Id, player->GetName(), player->GetLevel());
            }
        }

        void Register() override
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mod_dk_dnd_scale_AuraScript::CalcAmount, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_mod_dk_dnd_scale_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_mod_dk_dnd_scale_AuraScript();
    }
};

// 50475 - Blood Presence Heal (triggered by the 63611 proc on damage)
//
// The heal is a % of all damage dealt, including white hits that the module does not scale,
// so it follows the presence curve below level 40.
//
// Older cores cast it without custom basepoints when 4% of a small hit truncates to 0,
// which falls back to the DBC value (about 30 HP). Current AzerothCore sends basepoints 0
// instead. Below level 30, a heal exactly equal to that DBC value is treated as the fallback
// and suppressed: 4% of the damage reachable at those levels stays well under it.
class spell_mod_dk_blood_presence_heal : public SpellScriptLoader
{
public:
    spell_mod_dk_blood_presence_heal() : SpellScriptLoader("spell_mod_dk_blood_presence_heal") { }

    class spell_mod_dk_blood_presence_heal_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_mod_dk_blood_presence_heal_SpellScript);

        void HandleHit()
        {
            Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
            if (!IsDk(player))
                return;

            int32 heal = GetHitHeal();
            if (heal <= 0)
                return;

            if (player->GetLevel() < 30 && heal == GetSpellInfo()->Effects[EFFECT_0].CalcValue(player))
            {
                LOG_DEBUG("module.dkd", "mod_dk: Suppressed Blood Presence fallback heal {} (level {}).", heal, player->GetLevel());
                SetHitHeal(0);
                return;
            }

            float scale = GetLowLevelPresenceScale(player);
            if (scale < 1.0f)
                SetHitHeal(int32(float(heal) * scale));
        }

        void Register() override
        {
            OnHit += SpellHitFn(spell_mod_dk_blood_presence_heal_SpellScript::HandleHit);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_mod_dk_blood_presence_heal_SpellScript();
    }
};

// Low level scaling for everything the spell scripts above don't list: talent abilities
// (Heart Strike, Scourge Strike, Frost Strike, Howling Blast, Corpse Explosion, Unholy
// Blight, ...), DK heals (Death Strike, Rune Tap, ...) and the DK's minions (ghoul,
// Army of the Dead, Bloodworms, Dancing Rune Weapon, gargoyle).
class mod_dk_unitscript : public UnitScript
{
public:
    mod_dk_unitscript() : UnitScript("mod_dk_unitscript", true, {
        UNITHOOK_MODIFY_MELEE_DAMAGE,
        UNITHOOK_MODIFY_SPELL_DAMAGE_TAKEN,
        UNITHOOK_MODIFY_PERIODIC_DAMAGE_AURAS_TICK,
        UNITHOOK_MODIFY_HEAL_RECEIVED
    }) { }

    // Returns the DK whose damage this is, or nullptr when it should not be scaled.
    static Player* GetScaledDk(Unit* attacker, SpellInfo const* spellInfo)
    {
        if (!attacker)
            return nullptr;

        Player* owner = attacker->GetCharmerOrOwnerPlayerOrPlayerItself();
        if (!IsDk(owner) || owner->GetLevel() >= DK_DAMAGE_SCALING_END_LEVEL)
            return nullptr;

        // Minions: everything they do.
        if (attacker != owner)
            return owner;

        // The DK: only DK spells that no module script scales already.
        if (!IsDkSpell(spellInfo) || GetScriptScaledSpells().count(spellInfo->Id))
            return nullptr;

        return owner;
    }

    template <typename T>
    static void Scale(T& value, Player* dk)
    {
        float scale = GetLowLevelDkScale(dk);
        if (scale < 1.0f && value > 0)
            value = std::max<T>(1, T(float(value) * scale));
    }

    void ModifyMeleeDamage(Unit* /*target*/, Unit* attacker, uint32& damage) override
    {
        // White hits of the DK itself are weapon damage and stay as they are.
        if (!attacker || attacker->IsPlayer())
            return;

        if (Player* dk = GetScaledDk(attacker, nullptr))
            Scale(damage, dk);
    }

    void ModifySpellDamageTaken(Unit* /*target*/, Unit* attacker, int32& damage, SpellInfo const* spellInfo) override
    {
        if (Player* dk = GetScaledDk(attacker, spellInfo))
            Scale(damage, dk);
    }

    void ModifyPeriodicDamageAurasTick(Unit* /*target*/, Unit* attacker, uint32& damage, SpellInfo const* spellInfo) override
    {
        // Also called for periodic heals, which ModifyHealReceived handles.
        if (!spellInfo || spellInfo->IsPositive())
            return;

        if (Player* dk = GetScaledDk(attacker, spellInfo))
            Scale(damage, dk);
    }

    void ModifyHealReceived(Unit* target, Unit* healer, uint32& heal, SpellInfo const* spellInfo) override
    {
        if (!IsDkSpell(spellInfo) || GetScriptScaledSpells().count(spellInfo->Id))
            return;

        // The core passes the caster in either argument depending on the heal path.
        Player* dk = healer ? healer->GetCharmerOrOwnerPlayerOrPlayerItself() : nullptr;
        if (!IsDk(dk))
            dk = target ? target->GetCharmerOrOwnerPlayerOrPlayerItself() : nullptr;

        if (IsDk(dk) && dk->GetLevel() < DK_DAMAGE_SCALING_END_LEVEL)
            Scale(heal, dk);
    }
};

namespace ModDk
{
    std::string const& CreatureEntryColumn()
    {
        static std::string const column = WorldDatabase.Query("SHOW COLUMNS FROM `creature` LIKE 'id1'") ? "id1" : "id";
        return column;
    }

    uint32 Reload()
    {
        LoadDkSpellRules();
        LoadDkIntroQuests();
        ApplySkillLineAbilityCorrections();

        uint32 updated = 0;
        for (auto const& [guid, player] : ObjectAccessor::GetPlayers())
        {
            if (!IsDk(player) || !player->IsInWorld())
                continue;

            ApplyDkSpellProgression(player);
            RefreshPresenceScaling(player);
            ++updated;
        }

        return updated;
    }

    std::vector<StatusLine> CollectStatus()
    {
        std::vector<StatusLine> lines;
        auto info = [&](std::string text) { lines.push_back({ false, std::move(text) }); };
        auto problem = [&](std::string text) { lines.push_back({ true, std::move(text) }); };
        auto onOff = [](bool value) { return value ? "on" : "off"; };

        info(Acore::StringFormat("Settings: scaling {} (strength {:.2f}), threat bonus {}, spell progression {}, reentry {}, Ebon Hold rescue {}, debug {}.",
            onOff(Config.scalingEnabled), Config.scalingStrength, onOff(Config.threatBonusEnabled), onOff(Config.spellProgressionEnabled),
            onOff(Config.reentryEnabled), onOff(Config.ebonHoldRescueEnabled), onOff(Config.debug)));

        uint32 startLevel = sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL);
        char const* startSource = Config.startLevel > 0 ? "ModDk.StartLevel" : "worldserver.conf";
        if (startLevel >= DK_EBON_HOLD_MIN_LEVEL)
            info(Acore::StringFormat("New DKs start at level {} (from {}): they play the stock Ebon Hold intro and the low level features do not apply to them.", startLevel, startSource));
        else
            info(Acore::StringFormat("New DKs start at level {} (from {}).", startLevel, startSource));
        info(Acore::StringFormat("Creating a DK needs a level {} character on the account (from {}).",
            sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_MIN_LEVEL_FOR_HEROIC_CHARACTER),
            Config.minLevelForHeroicCharacter >= 0 ? "ModDk.MinLevelForHeroicCharacter" : "worldserver.conf"));

        if (DkSpellRules.empty())
            problem("mod_dk_spell_progression is empty or missing; apply data/sql/db-world/mod_dk_02_spell_progression.sql.");
        else
            info(Acore::StringFormat("Spell progression rules: {}.", DkSpellRules.size()));

        uint32 trainerSpells = 0;
        if (QueryResult result = WorldDatabase.Query("SELECT COUNT(*) FROM `trainer_spell` WHERE `TrainerId` = 1013"))
            trainerSpells = result->Fetch()[0].Get<uint32>();
        if (!trainerSpells)
            problem("TrainerId 1013 has no spells; apply data/sql/db-world/mod_dk_03_trainer_spells_1_60.sql.");
        else
            info(Acore::StringFormat("Trainer 1013 spells: {}.", trainerSpells));

        if (!sObjectMgr->GetCreatureTemplate(MADNESS_ENTRY))
            problem("The Madness trainer template (creature entry 900001) is missing; apply data/sql/db-world/mod_dk_04_trainer_spawns.sql.");

        std::unordered_set<std::string> scripts;
        if (QueryResult result = WorldDatabase.Query("SELECT DISTINCT `ScriptName` FROM `spell_script_names` WHERE `ScriptName` LIKE 'spell\\_mod\\_dk\\_%'"))
        {
            do
                scripts.insert(result->Fetch()[0].Get<std::string>());
            while (result->NextRow());
        }
        for (char const* name : { "spell_mod_dk_low_level_damage", "spell_mod_dk_low_level_dot", "spell_mod_dk_presence_scale",
                                  "spell_mod_dk_dnd_scale", "spell_mod_dk_blood_presence_heal" })
        {
            if (!scripts.count(name))
                problem(Acore::StringFormat("Spell script {} is not registered; apply data/sql/db-world/mod_dk_09_spell_scaling_scripts.sql.", name));
        }

        if (startLevel < DK_EBON_HOLD_MIN_LEVEL)
        {
            // Anything still auto learned would be taught on level up and removed again.
            std::unordered_set<uint32> trainerOnly = GetTrainerOnlySpells();
            uint32 autoLearned = 0;
            for (SkillLineAbilityEntry const* ability : sSkillLineAbilityStore)
                if (IsDkSkillLine(ability->SkillLine) && IsAutoLearned(ability) && trainerOnly.count(ability->Spell))
                    ++autoLearned;

            if (autoLearned)
                problem(Acore::StringFormat("{} trainer DK spells are still auto learned; run .dk reload.", autoLearned));
            else
                info(Acore::StringFormat("Auto learning is off for the trainer DK spells ({} SkillLineAbility entries changed).", SkillLineAbilitiesPatched));
        }

        uint32 runeforgeSpells = std::size(RUNEFORGE_SPELLS);
        uint32 runeforgeEnchants = std::size(RUNEFORGE_ENCHANTS);
        if (RuneforgeSpellsPatched < runeforgeSpells || RuneforgeEnchantsPatched < runeforgeEnchants)
            problem(Acore::StringFormat("Runeforge corrections applied to {}/{} spells and {}/{} enchants; check Spell.dbc and SpellItemEnchantment.dbc in DataDir.",
                RuneforgeSpellsPatched, runeforgeSpells, RuneforgeEnchantsPatched, runeforgeEnchants));
        else
            info(Acore::StringFormat("Runeforge corrections: {} spells, {} enchants.", RuneforgeSpellsPatched, RuneforgeEnchantsPatched));

        auto countSpawns = [](uint32 entry)
        {
            QueryResult result = WorldDatabase.Query("SELECT COUNT(*) FROM `creature` WHERE `{}` = {}", CreatureEntryColumn(), entry);
            return result ? result->Fetch()[0].Get<uint32>() : 0;
        };

        uint32 trainers = countSpawns(MADNESS_ENTRY);
        if (!trainers)
            problem("No Madness trainers are spawned; apply data/sql/db-world/mod_dk_17_default_spawns.sql or place one with .dk madness add.");
        else
            info(Acore::StringFormat("Madness trainers spawned: {}.", trainers));

        uint32 runeforges = 0;
        if (QueryResult result = WorldDatabase.Query("SELECT COUNT(*) FROM `gameobject` WHERE `Comment` LIKE 'mod-dk: runeforge-focus%'"))
            runeforges = result->Fetch()[0].Get<uint32>();
        info(Acore::StringFormat("Runeforges spawned: {}.", runeforges));

        if (Config.reentryEnabled)
        {
            if (!countSpawns(REENTRY_NPC_ENTRY))
                problem("The reentry NPC (creature entry 900000) is not spawned; apply data/sql/db-world/mod_dk_15_reentry_epl.sql.");
            if (DkIntroQuests.empty())
                problem("No Ebon Hold intro quests were found for the reentry NPC.");
            else
                info(Acore::StringFormat("Ebon Hold intro quests for the reentry NPC: {}.", DkIntroQuests.size()));
        }

        return lines;
    }
}

void Addmod_dkScripts()
{
    LOG_INFO("server.loading", "mod_dk: Scripts loaded.");
    Addmod_dkCommands();
    new npc_dk_reentry();
    new mod_dk_playerscript();
    new mod_dk_worldscript();
    new mod_dk_unitscript();
    new spell_mod_dk_low_level_damage();
    new spell_mod_dk_low_level_dot();
    new spell_mod_dk_presence_scale();
    new spell_mod_dk_dnd_scale();
    new spell_mod_dk_blood_presence_heal();
}
