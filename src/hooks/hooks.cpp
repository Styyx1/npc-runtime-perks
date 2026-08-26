#include "hooks.h"

#include "perk-manip.h"
#include "st-hooks.h"

namespace PERK
{
template <class T>
struct HasPerkHook
{
    static bool Call(RE::Actor* a_this, RE::BGSPerk* a_perk)
    {
        return PerkManip::HasPerk(a_this, a_perk, func(a_this, a_perk));
    }
    static inline REL::Relocation<bool (*)(RE::Actor*, RE::BGSPerk*)> func;
};

// So has perk can check runtime added perks as well. this wouldn't work without
// the hooks. as for that numbered struct, i have no idea what it is, but so far
// i didn't have any downsides with it, when i do, i'll remove the hook
// not sure if the HasPerk hooks are still needed, needs more testing, but I'm
// fairly sure they are
struct HasPerkConditionStruct
{
};
struct HasPerkConsoleStruct
{
};
struct HasPerk_14074eb88Struct
{
};
struct HasPerkPapyrusStruct
{
};
using HasPerkCondition  = HasPerkHook<HasPerkConditionStruct>;
using HasPerkConsole    = HasPerkHook<HasPerkConsoleStruct>;
using HasPerk_14074eb88 = HasPerkHook<HasPerk_14074eb88Struct>;
using HasPerkPapyrus    = HasPerkHook<HasPerkPapyrusStruct>;

void PerkForActors::InstallHooks()
{
    // old version hooks. keeping as comment in case things go wrong and i have to
    // use LoadActor and ApplyPerkFromBase again
    /*static REL::Relocation<uintptr_t> charVTABLE{RE::Character::VTABLE[0]};
    _addPerk = charVTABLE.write_vfunc(0xFB, AddPerk);
    _removePerk = charVTABLE.write_vfunc(0xFC, RemovePerk);
    _addTempPerk = charVTABLE.write_vfunc(0xFD, ApplyTempPerk);
    _removeTempPerk = charVTABLE.write_vfunc(0xFE, RemoveTempPerk);
    _loadActor = charVTABLE.write_vfunc(0x06A, LoadActor);
    _addPerkFromBase = charVTABLE.write_vfunc(0x101, ApplyPerkFromBase);*/

    using namespace HookUtils;

    // HasPerk is hooked so whenever the game calls it, it also returns my HasPerk
    // function
    WriteCall5<HasPerkCondition>(RELOCATION_ID(21167, 21622), 0x28);
    WriteCall5<HasPerkConsole>(RELOCATION_ID(21965, 22448), 0x93);
    WriteCall5<HasPerk_14074eb88>(RELOCATION_ID(39677, 40770), OFFSET(0x37, 0x38));
    WriteCall5<HasPerkPapyrus>(RELOCATION_ID(53845, 54644), 0xf);

    WriteVFunc<RE::Character, 0, 0xfb, AddPerk>();
    WriteVFunc<RE::Character, 0, 0xfc, RemovePerk>();
    WriteVFunc<RE::Character, 0, 0xfd, ApplyTempPerk>();
    WriteVFunc<RE::Character, 0, 0xfe, RemoveTempPerk>();
    // recommendation from noah, this seems to be actually the main hook i need
    // basically replaces the ApplyPerkFromBase hook from earlier versions
    WriteVFunc<RE::Character, 0, 0xfa, ForEachHook>();
}

void PerkForActors::ForEachHook::Call(RE::Actor* a_actor, RE::PerkEntryVisitor& a_visitor)
{
    func(a_actor, a_visitor);

    auto* ps = REX::Singleton<ActorPerkStorage>::GetSingleton();

    std::vector<RE::BGSPerk*> perks;

    {
        std::shared_lock lock(ps->perk_mutex);

        auto it = ps->actor_perk_ranks.find(a_actor);
        if (it == ps->actor_perk_ranks.end())
        {
            return;
        }

        for (const auto& data : it->second)
        {
            if (data.perk)
            {
                perks.push_back(data.perk);
            }
        }
    }

    ps->LogPerksFromMap(a_actor);

    // PerkManip::ApplyPerksFromMap(a_actor);

    for (auto* perk : perks)
    {

        if (!perk)
        {
            continue;
        }

        for (auto* entry : perk->perkEntries)
        {
            if (!entry)
            {
                continue;
            }

            if (a_visitor.Visit(entry) == RE::BSContainer::ForEachResult::kStop)
            {
                return;
            }
        }
    }
}
void PerkForActors::AddPerk::Call(RE::Actor* a_this, RE::BGSPerk* a_perk, std::uint32_t a_rank)
{

    PerkManip::AddPerkToActor(a_this, a_perk);
    func(a_this, a_perk, a_rank);
}
void PerkForActors::RemovePerk::Call(RE::Actor* a_this, RE::BGSPerk* a_perk)
{

    PerkManip::RemovePerkFromActor(a_this, a_perk);
    func(a_this, a_perk);
}
void PerkForActors::ApplyTempPerk::Call(RE::Actor* a_this, RE::BGSPerk* a_perk)
{

    PerkManip::AddPerkToActor(a_this, a_perk);
    func(a_this, a_perk);
}
void PerkForActors::RemoveTempPerk::Call(RE::Actor* a_this, RE::BGSPerk* a_perk)
{

    PerkManip::RemovePerkFromActor(a_this, a_perk);
    func(a_this, a_perk);
}
} // namespace PERK
