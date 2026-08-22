#include "menu-defines.h"

#include "configs/config.h"
#include "perk-manip.h"
#include "st-actor.h"

namespace PERK
{

void FUNCS::OnMenuOpen()
{
    UpdatePerkList();
}
void FUNCS::OnMenuClose()
{
    Config::UpdateConfig(true);
}
void FUNCS::UpdatePerkList()
{

    MENU::selected_index      = -1;
    MENU::bShowAllPerksInMenu = Config::show_all_perks.GetValue();
    MENU::tracked_actors.clear();

    auto storage = REX::Singleton<PERK::ActorPerkStorage>::GetSingleton();

    std::vector<RE::Actor*> actors;
    {
        std::shared_lock lock(storage->perk_mutex);
        actors.reserve(storage->actor_perk_ranks.size());
        for (const auto& [actor, perks] : storage->actor_perk_ranks)
        {
            if (!actor)
                continue;

            if (actor->IsDeleted() || actor->IsMarkedForDeletion())
                continue;

            if (!RE::TESForm::LookupByID<RE::Actor>(actor->GetFormID()))
                continue;

            actors.push_back(actor);
        }
    }

    for (auto actor : actors)
    {
        MENU::CurrentTrackedActors tracked;
        tracked.id   = actor->GetFormID();
        tracked.name = actor->GetName();

        const auto add_perk = [&](const RE::BGSPerk* a_perk)
        {
            if (!a_perk)
            {
                return;
            }
            std::string perk_name = a_perk->GetName();
            if (perk_name.empty())
            {
                perk_name = "Unnamed Perk";
            }
            tracked.perk_names.emplace_back(perk_name, a_perk->GetLocalFormID());
        };
        if (MENU::bShowAllPerksInMenu)
        {
            std::vector<RE::BGSPerk*> base_perks;
            StyyxUtil::ActorUtil::GetPerksFromBaseActor(actor, base_perks);

            for (auto perk : base_perks)
            {
                add_perk(perk);
            }
        }

        auto perks = storage->GetPerks(actor);
        for (auto perk : perks)
        {
            add_perk(perk);
        }
        MENU::tracked_actors.emplace_back(std::move(tracked));
    }
}

} // namespace PERK