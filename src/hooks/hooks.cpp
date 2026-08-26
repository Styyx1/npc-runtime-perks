#include "hooks.h"

#include "perk-manip.h"
namespace PERK
{


void PerkForActors::ForEachHook::Call(RE::Actor* a_actor, RE::PerkEntryVisitor& a_visitor)
{

    func(a_actor, a_visitor);
    auto* ps = REX::TSingleton<ActorPerkStorage>::GetSingleton();

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
bool PerkForActors::HasPerkConditionStruct::Call(RE::Actor* a_this, RE::BGSPerk* a_perk)
{
    return PerkManip::HasPerk(a_this, a_perk, func(a_this, a_perk));
}
bool PerkForActors::HasPerkConsoleStruct::Call(RE::Actor* a_this, RE::BGSPerk* a_perk)
{
    return PerkManip::HasPerk(a_this, a_perk, func(a_this, a_perk));
}
bool PerkForActors::HasPerk_14074eb88Struct::Call(RE::Actor* a_this, RE::BGSPerk* a_perk)
{
    return PerkManip::HasPerk(a_this, a_perk, func(a_this, a_perk));
}
bool PerkForActors::HasPerkPapyrusStruct::Call(RE::Actor* a_this, RE::BGSPerk* a_perk)
{
    return PerkManip::HasPerk(a_this, a_perk, func(a_this, a_perk));
}
} // namespace PERK
