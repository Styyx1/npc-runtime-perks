#include "ingame-menu.h"

#include "configs/config.h"
#include "menu-defines.h"

namespace PERK
{
void FUCKMenu::RegisterFMenu()
{
    if (FUCK::Connect("styyx-npc-perks", 1))
    {
        FUCK::RegisterTool(&g_perkMenuTool);
    }
}
void FUCKMenu::PerkShower::OnClose()
{
    Config::UpdateConfig(true);
}
void FUCKMenu::PerkShower::OnOpen()
{
    FUNCS::UpdatePerkList();
}
void FUCKMenu::PerkShower::Draw()
{
    DrawPerkShower();
}
void FUCKMenu::PerkShower::DrawPerkShower()
{

    DrawOptions();
    DrawActorSelector();

    DrawSelectedPerks();
}
void FUCKMenu::PerkShower::DrawActorSelector()
{

    std::vector<std::string> actorLabels;
    std::vector<const char*> actorPtrs;

    actorLabels.reserve(MENU::tracked_actors.size());
    actorPtrs.reserve(MENU::tracked_actors.size());

    for (const auto& actor : MENU::tracked_actors)
    {
        actorLabels.push_back(std::format("{} (0x{:X})", actor.name, actor.id));
    }

    for (const auto& str : actorLabels)
    {
        actorPtrs.push_back(str.c_str());
    }

    FUCK::ComboWithFilter(LAB::LABEL_ACTOR.c_str(), &MENU::selected_index, actorPtrs.data(),
                          static_cast<int>(actorPtrs.size()));
}
void FUCKMenu::PerkShower::DrawOptions()
{

    FUCK::SameLine();
    FUCKUtil::Checkbox(LAB::BASE_PERKS.c_str(), MENU::bShowAllPerksInMenu, Config::show_all_perks,
                       TOOL::BASE_PERKS.c_str());


    const auto now     = std::chrono::steady_clock::now();
    const auto elapsed = now - MENU::last_update_time;

    if (elapsed < MENU::update_cooldown)
    {
        const auto remaining = std::chrono::duration_cast<std::chrono::seconds>(MENU::update_cooldown - elapsed);
        FUCK::BeginDisabled();
        FUCK::Button(std::format("{} ({})", LAB::UPDATE_LIST, remaining).c_str());
        FUCK::EndDisabled();
    }

    else
    {
        if (FUCK::Button(LAB::UPDATE_LIST.c_str()))
        {
            FUNCS::UpdatePerkList();
            MENU::last_update_time = now;
        }
    }
}
void FUCKMenu::PerkShower::DrawSelectedPerks()
{

    if (MENU::selected_index >= 0 && MENU::selected_index < MENU::tracked_actors.size())
    {
        const auto& selected = MENU::tracked_actors[MENU::selected_index];

        FUCK::Separator();
        FUCK::Text(LAB::PERK_LABEL.c_str());

        for (const auto& [perkName, perkID] : selected.perk_names)
        {
            FUCK::Text(std::format("{} (0x{:X})", perkName, perkID).c_str());
        }
    }
}
} // namespace PERK