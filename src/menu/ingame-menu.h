#pragma once

#include <st-fui.h>

#include "API/FUCK_API.h"
#include "menu-defines.h"

namespace PERK
{

namespace FUCKMenu
{

void RegisterFMenu();

struct PerkShower : FUCK::ITool
{

    const char* PluginName() const override { return "styyx-npc-perks"; }
    const char* Name() const override { return "Perks for NPCs"; };
    const char* Group() const override { return "Styyx Mods"; };
    void OnClose() override;
    void OnOpen() override;
    void Draw() override;

  private:
    void DrawPerkShower();
    void DrawActorSelector();
    void DrawOptions();
    void DrawSelectedPerks();
};
inline PerkShower g_perkMenuTool;

} // namespace FUCKMenu

} // namespace PERK