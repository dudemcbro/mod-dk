#ifndef MOD_DK_H
#define MOD_DK_H

#include "Define.h"

#include <string>
#include <vector>

// Shared between ModDk.cpp and the GM commands in ModDkCommands.cpp.
namespace ModDk
{
    constexpr uint32 MADNESS_ENTRY = 900001;
    constexpr uint32 REENTRY_NPC_ENTRY = 900000;

    struct StatusLine
    {
        bool problem;
        std::string text;
    };

    // The spawn's entry column: creature.id1 on older cores, creature.id on AzerothCore
    // since its 2026_06_16_00 update.
    std::string const& CreatureEntryColumn();

    // Reloads mod_dk_spell_progression and the intro quest list, then applies the
    // progression to every online DK. Returns the number of online DKs updated.
    uint32 Reload();

    // The module's settings and setup checks; `problem` lines are logged at startup.
    std::vector<StatusLine> CollectStatus();
}

#endif
