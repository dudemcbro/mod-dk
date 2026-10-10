#include "ModDk.h"

#include "Chat.h"
#include "CommandScript.h"
#include "Language.h"
#include "DBCStores.h"
#include "DatabaseEnv.h"
#include "EventProcessor.h"
#include "Log.h"
#include "Creature.h"
#include "GameObject.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "StringFormat.h"
#include "Transport.h"

#include <cmath>
#include <cstdlib>
#include <string>
#include <string_view>

using namespace Acore::ChatCommands;

namespace
{
    constexpr uint32 RUNEFORGE_FOCUS_ENTRY = 190557;   // Runeforge (Spell Focus 1552)
    constexpr uint32 DEFAULT_RUNEFORGE_ENTRY = 202236; // Frostmourne Altar (visual)
    constexpr uint32 RUNEFORGE_AURA_ENTRY = 900010;    // mod-dk: safe blue flame aura (no trap)
    constexpr float RUNEFORGE_FOCUS_Z_OFFSET = -0.60f;
    constexpr float RUNEFORGE_AURA_Z_OFFSET = 0.0f;
    constexpr uint32 DK_TRAINER_ENTRY = ModDk::MADNESS_ENTRY;
    constexpr uint32 LIST_LIMIT = 50;

    using ModDk::CreatureEntryColumn;

    // SaveToDB() writes the spawn in an asynchronous transaction, so an UPDATE queued after
    // it can run first and match no row when WorldDatabase.WorkerThreads is above 1. Tag
    // the spawn once its row exists; the removal commands find spawns by this comment.
    class DkTagSpawnEvent : public BasicEvent
    {
    public:
        DkTagSpawnEvent(Player* player, std::string table, ObjectGuid::LowType guid, std::string comment, uint32 attemptsLeft)
            : _player(player), _table(std::move(table)), _guid(guid), _comment(std::move(comment)), _attemptsLeft(attemptsLeft)
        {
        }

        bool Execute(uint64 /*execTime*/, uint32 /*diff*/) override
        {
            Run(_player, _table, _guid, _comment, _attemptsLeft);
            return true;
        }

        static void Run(Player* player, std::string const& table, ObjectGuid::LowType guid, std::string const& comment, uint32 attemptsLeft)
        {
            if (WorldDatabase.Query("SELECT 1 FROM `{}` WHERE guid={}", table, guid))
            {
                WorldDatabase.DirectExecute("UPDATE `{}` SET Comment='{}' WHERE guid={}", table, comment, guid);
                return;
            }

            if (!attemptsLeft)
            {
                LOG_WARN("module.dkd", "mod_dk: {} spawn {} was not saved in time; set its Comment to '{}' by hand.", table, guid, comment);
                return;
            }

            player->m_Events.AddEvent(new DkTagSpawnEvent(player, table, guid, comment, attemptsLeft - 1), player->m_Events.CalculateTime(250));
        }

    private:
        Player* _player;
        std::string _table;
        ObjectGuid::LowType _guid;
        std::string _comment;
        uint32 _attemptsLeft;
    };

    void TagSpawn(Player* player, std::string const& table, ObjectGuid::LowType guid, std::string const& comment)
    {
        DkTagSpawnEvent::Run(player, table, guid, comment, 40); // retry for up to 10 seconds
    }

    bool SpawnRuneforgeGameObject(Player* player, uint32 entry, std::string_view comment, float zOffset, ObjectGuid::LowType& outGuid)
    {
        if (!player)
            return false;

        GameObjectTemplate const* objectInfo = sObjectMgr->GetGameObjectTemplate(entry);
        if (!objectInfo)
            return false;

        if (objectInfo->displayId && !sGameObjectDisplayInfoStore.LookupEntry(objectInfo->displayId))
            return false;

        Map* map = player->GetMap();
        if (!map)
            return false;

        float x = player->GetPositionX();
        float y = player->GetPositionY();
        float z = player->GetPositionZ();
        float o = player->GetOrientation();

        float snappedZ = z;
        float height = map->GetHeight(player->GetPhaseMask(), x, y, z, true);
        if (height > -50000.0f)
            snappedZ = height;
        z = snappedZ + 0.10f + zOffset;

        GameObject* object = sObjectMgr->IsGameObjectStaticTransport(objectInfo->entry) ? new StaticTransport() : new GameObject();
        ObjectGuid::LowType guidLow = map->GenerateLowGuid<HighGuid::GameObject>();

        if (!object->Create(guidLow, objectInfo->entry, map, player->GetPhaseMaskForSpawn(), x, y, z, o, G3D::Quat(), 0, GO_STATE_READY))
        {
            delete object;
            return false;
        }

        int32 spawntimesecs = 300;
        object->SetRespawnTime(spawntimesecs);

        object->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()), player->GetPhaseMaskForSpawn());
        guidLow = object->GetSpawnId();

        delete object;

        object = sObjectMgr->IsGameObjectStaticTransport(objectInfo->entry) ? new StaticTransport() : new GameObject();
        if (!object->LoadGameObjectFromDB(guidLow, map, true))
        {
            delete object;
            return false;
        }

        sObjectMgr->AddGameobjectToGrid(guidLow, sObjectMgr->GetGameObjectData(guidLow));

        TagSpawn(player, "gameobject", guidLow, std::string(comment));

        outGuid = guidLow;
        return true;
    }

    bool HandleDkRuneforgeAdd(ChatHandler* handler, char const* args)
    {
        Player* player = handler ? handler->GetPlayer() : nullptr;
        if (!player)
            return false;

        uint32 entry = 0;
        if (args && *args)
            entry = static_cast<uint32>(std::strtoul(args, nullptr, 10));

        if (!entry)
            entry = DEFAULT_RUNEFORGE_ENTRY;

        ObjectGuid::LowType focusGuid = 0;
        if (!SpawnRuneforgeGameObject(player, RUNEFORGE_FOCUS_ENTRY, "mod-dk: runeforge-focus", RUNEFORGE_FOCUS_Z_OFFSET, focusGuid))
        {
            handler->PSendSysMessage("Runeforge: focus template {} not found or invalid.", RUNEFORGE_FOCUS_ENTRY);
            handler->SetSentErrorMessage(true);
            return false;
        }

        ObjectGuid::LowType mainGuid = 0;
        if (!SpawnRuneforgeGameObject(player, entry, "mod-dk: runeforge-visual", 0.0f, mainGuid))
            handler->PSendSysMessage("Runeforge: visual template {} not found or invalid.", entry);

        ObjectGuid::LowType auraGuid = 0;
        if (!SpawnRuneforgeGameObject(player, RUNEFORGE_AURA_ENTRY, "mod-dk: runeforge-aura", RUNEFORGE_AURA_Z_OFFSET, auraGuid))
        {
            handler->PSendSysMessage("Runeforge: failed to spawn aura object (entry {}).", RUNEFORGE_AURA_ENTRY);
        }

        handler->PSendSysMessage(
            "Runeforge: saved permanent spawn. focus={} guid={} visual={} guid={} aura={} guid={}.",
            RUNEFORGE_FOCUS_ENTRY, focusGuid, entry, mainGuid, RUNEFORGE_AURA_ENTRY, auraGuid
        );

        return true;
    }

    bool IsModDkRuneforgeSpawn(uint32 guidLow)
    {
        QueryResult result = WorldDatabase.Query("SELECT Comment FROM gameobject WHERE guid={}", guidLow);
        if (!result)
            return false;

        Field* fields = result->Fetch();
        if (!fields)
            return false;

        std::string comment = fields[0].Get<std::string>();
        return comment.rfind("mod-dk: runeforge", 0) == 0;
    }

    void DeleteRuneforgeSpawnsNear(ChatHandler* handler, uint32 mapId, float x, float y, float z, float radius)
    {
        QueryResult result = WorldDatabase.Query(
            "SELECT guid FROM gameobject "
            "WHERE Comment LIKE 'mod-dk: runeforge%' AND map={} "
            "AND POW(position_x-{},2)+POW(position_y-{},2)+POW(position_z-{},2) <= {}",
            mapId, x, y, z, radius * radius
        );

        if (!result)
            return;

        do
        {
            Field* fields = result->Fetch();
            uint32 guidLow = fields[0].Get<uint32>();
            GameObject* go = handler ? handler->GetObjectFromPlayerMapByDbGuid(guidLow) : nullptr;
            if (go)
            {
                // Same sequence as the core `.gobject delete` command: despawn, then drop
                // the spawn from ObjectMgr and the DB so it does not come back on grid reload.
                go->SetRespawnTime(0);
                go->Delete();
                go->DeleteFromDB();
            }
            else
            {
                sObjectMgr->DeleteGOData(guidLow);
                WorldDatabase.Execute("DELETE FROM gameobject WHERE guid={} AND Comment LIKE 'mod-dk: runeforge%'", guidLow);
            }
        } while (result->NextRow());
    }

    bool HandleDkRuneforgeRemove(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler ? handler->GetPlayer() : nullptr;
        if (!player)
            return false;

        ObjectGuid sel = player->GetTarget();
        if (!sel || sel.GetHigh() != HighGuid::GameObject)
        {
            float px = player->GetPositionX();
            float py = player->GetPositionY();
            float pz = player->GetPositionZ();
            uint32 mapId = player->GetMapId();
            float maxDist = 15.0f;

            QueryResult result = WorldDatabase.Query(
                "SELECT guid, position_x, position_y, position_z "
                "FROM gameobject "
                "WHERE Comment LIKE 'mod-dk: runeforge%' AND map={} "
                "ORDER BY POW(position_x-{},2)+POW(position_y-{},2)+POW(position_z-{},2) ASC "
                "LIMIT 1",
                mapId, px, py, pz
            );

            if (!result)
            {
                handler->SendSysMessage("Runeforge: no nearby mod-dk runeforge found (try targeting one).");
                handler->SetSentErrorMessage(true);
                return false;
            }

            Field* fields = result->Fetch();
            float gx = fields[1].Get<float>();
            float gy = fields[2].Get<float>();
            float gz = fields[3].Get<float>();

            float dx = gx - px;
            float dy = gy - py;
            float dz = gz - pz;
            float dist2 = dx * dx + dy * dy + dz * dz;
            if (dist2 > maxDist * maxDist)
            {
                handler->PSendSysMessage("Runeforge: nearest mod-dk runeforge is too far ({:.1f} yards). Move closer or target it.", std::sqrt(dist2));
                handler->SetSentErrorMessage(true);
                return false;
            }

            DeleteRuneforgeSpawnsNear(handler, mapId, gx, gy, gz, 5.0f);
            handler->PSendSysMessage("Runeforge: removed nearest mod-dk spawn group.");
            return true;
        }

        GameObject* go = ObjectAccessor::GetGameObject(*player, sel);
        if (!go)
        {
            handler->SendSysMessage("Runeforge: selected object not found (try getting closer).");
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 guidLow = go->GetSpawnId();
        if (!guidLow)
        {
            handler->SendSysMessage("Runeforge: selected object does not have a DB spawn id.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!IsModDkRuneforgeSpawn(guidLow))
        {
            handler->SendSysMessage("Runeforge: selected object is not a mod-dk runeforge spawn.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        DeleteRuneforgeSpawnsNear(handler, go->GetMapId(), go->GetPositionX(), go->GetPositionY(), go->GetPositionZ(), 5.0f);
        handler->PSendSysMessage("Runeforge: removed selected mod-dk spawn group.");
        return true;
    }

    bool SpawnMadnessTrainer(Player* player, ObjectGuid::LowType& outGuid)
    {
        if (!player)
            return false;

        CreatureTemplate const* creatureInfo = sObjectMgr->GetCreatureTemplate(DK_TRAINER_ENTRY);
        if (!creatureInfo)
            return false;

        Map* map = player->GetMap();
        if (!map)
            return false;

        float x = player->GetPositionX();
        float y = player->GetPositionY();
        float z = player->GetPositionZ();
        float o = player->GetOrientation();

        Creature* creature = new Creature();
        if (!creature->Create(map->GenerateLowGuid<HighGuid::Unit>(), map, player->GetPhaseMaskForSpawn(), DK_TRAINER_ENTRY, 0, x, y, z, o))
        {
            delete creature;
            return false;
        }

        creature->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()), player->GetPhaseMaskForSpawn());
        ObjectGuid::LowType spawnId = creature->GetSpawnId();

        creature->CleanupsBeforeDelete();
        delete creature;

        creature = new Creature();
        if (!creature->LoadCreatureFromDB(spawnId, map, true, true))
        {
            delete creature;
            return false;
        }

        sObjectMgr->AddCreatureToGrid(spawnId, sObjectMgr->GetCreatureData(spawnId));

        TagSpawn(player, "creature", spawnId, "mod-dk: madness");
        outGuid = spawnId;
        return true;
    }

    void DeleteMadnessSpawn(ChatHandler* handler, ObjectGuid::LowType guidLow)
    {
        if (Creature* creature = handler ? handler->GetCreatureFromPlayerMapByDbGuid(guidLow) : nullptr)
        {
            creature->CombatStop();
            creature->DeleteFromDB();
            creature->AddObjectToRemoveList();
            return;
        }

        sObjectMgr->DeleteCreatureData(guidLow);
        WorldDatabase.Execute("DELETE FROM creature WHERE guid={}", guidLow);
    }

    bool HandleDkMadnessAdd(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler ? handler->GetPlayer() : nullptr;
        if (!player)
            return false;

        ObjectGuid::LowType guidLow = 0;
        if (!SpawnMadnessTrainer(player, guidLow))
        {
            handler->SendSysMessage("Madness: failed to spawn trainer (template 900001 missing?).");
            handler->SetSentErrorMessage(true);
            return false;
        }

        handler->PSendSysMessage("Madness: spawned trainer (guid={}).", guidLow);
        return true;
    }

    bool HandleDkMadnessRemove(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler ? handler->GetPlayer() : nullptr;
        if (!player)
            return false;

        Creature* selected = handler->getSelectedCreature();
        if (selected && selected->GetEntry() == DK_TRAINER_ENTRY)
        {
            DeleteMadnessSpawn(handler, selected->GetSpawnId());
            handler->SendSysMessage("Madness: removed selected trainer.");
            return true;
        }

        float px = player->GetPositionX();
        float py = player->GetPositionY();
        float pz = player->GetPositionZ();
        uint32 mapId = player->GetMapId();
        float maxDist = 15.0f;

        QueryResult result = WorldDatabase.Query(
            "SELECT guid, position_x, position_y, position_z "
            "FROM creature "
            "WHERE `{}`={} AND map={} "
            "ORDER BY POW(position_x-{},2)+POW(position_y-{},2)+POW(position_z-{},2) ASC "
            "LIMIT 1",
            CreatureEntryColumn(), DK_TRAINER_ENTRY, mapId, px, py, pz
        );

        if (!result)
        {
            handler->SendSysMessage("Madness: no nearby trainer found (try targeting one).");
            handler->SetSentErrorMessage(true);
            return false;
        }

        Field* fields = result->Fetch();
        uint32 guidLow = fields[0].Get<uint32>();
        float gx = fields[1].Get<float>();
        float gy = fields[2].Get<float>();
        float gz = fields[3].Get<float>();

        float dx = gx - px;
        float dy = gy - py;
        float dz = gz - pz;
        float dist2 = dx * dx + dy * dy + dz * dz;
        if (dist2 > maxDist * maxDist)
        {
            handler->PSendSysMessage("Madness: nearest trainer is too far ({:.1f} yards). Move closer or target it.", std::sqrt(dist2));
            handler->SetSentErrorMessage(true);
            return false;
        }

        DeleteMadnessSpawn(handler, guidLow);
        handler->SendSysMessage("Madness: removed nearest trainer.");
        return true;
    }

    std::string MapName(ChatHandler* handler, uint32 mapId)
    {
        MapEntry const* map = sMapStore.LookupEntry(mapId);
        if (!map)
            return Acore::StringFormat("map {}", mapId);

        char const* name = map->name[handler->GetSessionDbcLocale()];
        return (name && *name) ? name : map->name[0];
    }

    // Lists the spawns in table `from` matching `where`, nearest first: the ones on the
    // player's map, then the rest by map.
    void ListSpawns(ChatHandler* handler, std::string const& what, std::string const& goCommand, std::string const& from, std::string const& where)
    {
        Player* player = handler->GetPlayer();
        uint32 mapId = player->GetMapId();
        float px = player->GetPositionX();
        float py = player->GetPositionY();
        float pz = player->GetPositionZ();

        uint32 total = 0;
        if (QueryResult result = WorldDatabase.Query("SELECT COUNT(*) FROM {} WHERE {}", from, where))
            total = result->Fetch()[0].Get<uint32>();

        if (!total)
        {
            handler->PSendSysMessage("No {} found.", what);
            return;
        }

        handler->PSendSysMessage("{} {} (nearest first, showing up to {}). Go there with {} <guid>.", total, what, LIST_LIMIT, goCommand);

        QueryResult result = WorldDatabase.Query(
            "SELECT guid, map, position_x, position_y, position_z, COALESCE(Comment, '') FROM {} WHERE {} "
            "ORDER BY (map = {}) DESC, map, POW(position_x-{},2)+POW(position_y-{},2)+POW(position_z-{},2) ASC LIMIT {}",
            from, where, mapId, px, py, pz, LIST_LIMIT);
        if (!result)
            return;

        do
        {
            Field* fields = result->Fetch();
            uint32 guid = fields[0].Get<uint32>();
            uint32 map = fields[1].Get<uint32>();
            float x = fields[2].Get<float>();
            float y = fields[3].Get<float>();
            float z = fields[4].Get<float>();
            bool isDefault = fields[5].Get<std::string>().find("(default)") != std::string::npos;

            std::string distance;
            if (map == mapId)
                distance = Acore::StringFormat(", {:.0f} yd away", std::sqrt((x - px) * (x - px) + (y - py) * (y - py) + (z - pz) * (z - pz)));

            handler->PSendSysMessage("guid {}: {} ({:.1f}, {:.1f}, {:.1f}){}, {}", guid, MapName(handler, map), x, y, z, distance,
                isDefault ? "default spawn" : "placed by a GM");
        } while (result->NextRow());
    }

    bool HandleDkRuneforgeList(ChatHandler* handler, char const* /*args*/)
    {
        if (!handler->GetPlayer())
            return false;

        // One focus object per runeforge group.
        ListSpawns(handler, "mod-dk runeforges", ".go gameobject", "gameobject", "Comment LIKE 'mod-dk: runeforge-focus%'");
        return true;
    }

    bool HandleDkMadnessList(ChatHandler* handler, char const* /*args*/)
    {
        if (!handler->GetPlayer())
            return false;

        ListSpawns(handler, "Madness trainers", ".go creature", "creature", Acore::StringFormat("`{}` = {}", CreatureEntryColumn(), DK_TRAINER_ENTRY));
        return true;
    }

    bool HandleDkReload(ChatHandler* handler, char const* /*args*/)
    {
        uint32 updated = ModDk::Reload();
        handler->PSendSysMessage("mod-dk: reloaded the spell progression table and the Ebon Hold intro quests; applied to {} online death knights.", updated);
        handler->SendSysMessage("Settings from mod_dk.conf reload with .reload config.");
        return true;
    }

    bool HandleDkStatus(ChatHandler* handler, char const* /*args*/)
    {
        uint32 problems = 0;
        for (ModDk::StatusLine const& line : ModDk::CollectStatus())
        {
            if (line.problem)
            {
                ++problems;
                handler->PSendSysMessage("PROBLEM: {}", line.text);
            }
            else
                handler->SendSysMessage(line.text);
        }

        handler->PSendSysMessage(problems ? "mod-dk: {} problem(s) found." : "mod-dk: no problems found.", problems);
        return true;
    }
}

class mod_dk_commandscript : public CommandScript
{
public:
    mod_dk_commandscript() : CommandScript("mod_dk_commandscript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable dkRuneforgeTable =
        {
            { "add", HandleDkRuneforgeAdd, SEC_GAMEMASTER, Console::No },
            { "remove", HandleDkRuneforgeRemove, SEC_GAMEMASTER, Console::No },
            { "list", HandleDkRuneforgeList, SEC_GAMEMASTER, Console::No }
        };

        static ChatCommandTable dkMadnessTable =
        {
            { "add", HandleDkMadnessAdd, SEC_GAMEMASTER, Console::No },
            { "remove", HandleDkMadnessRemove, SEC_GAMEMASTER, Console::No },
            { "list", HandleDkMadnessList, SEC_GAMEMASTER, Console::No }
        };

        static ChatCommandTable dkTable =
        {
            { "runeforge", dkRuneforgeTable },
            { "rf", dkRuneforgeTable },
            { "madness", dkMadnessTable },
            { "md", dkMadnessTable },
            { "reload", HandleDkReload, SEC_ADMINISTRATOR, Console::Yes },
            { "status", HandleDkStatus, SEC_GAMEMASTER, Console::Yes }
        };

        static ChatCommandTable rootTable =
        {
            { "dk", dkTable }
        };

        return rootTable;
    }
};

void Addmod_dkCommands()
{
    new mod_dk_commandscript();
}
