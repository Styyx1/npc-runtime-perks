#pragma once

namespace PERK
{

namespace MENU
{

struct CurrentTrackedActors
{
    RE::FormID id;
    std::string name;
    std::vector<std::pair<std::string, RE::FormID>> perk_names;
};
inline std::vector<CurrentTrackedActors> tracked_actors;
inline int selected_index = -1;
inline bool bShowAllPerksInMenu;
inline std::chrono::steady_clock::time_point last_update_time{};
inline constexpr auto update_cooldown = std::chrono::seconds(2);

} // namespace MENU
namespace FUNCS
{

void OnMenuOpen();
void OnMenuClose();
void UpdatePerkList();
} // namespace FUNCS
namespace TITLE
{

inline std::string MOD_NAME       = "Perks For NPCs";
inline std::string SETTINGS_TITLE = "Perks For NPCs";
inline std::string GROUP          = "Styyx Configs";

} // namespace TITLE
namespace LAB
{

inline std::string ACTOR_SEARCH = "Select Actor";
inline std::string LABEL_ACTOR  = "Actor";
inline std::string BASE_PERKS   = "Base Perks?";
inline std::string UPDATE_LIST  = "Update List";
inline std::string PERK_LABEL   = "Perks";

} // namespace LAB
namespace TOOL
{

inline std::string ACTOR_SEARCH = "Select Actor to view perks from";
inline std::string BASE_PERKS   = "Show Perks inherited from the actor base";
inline std::string UPDATE_LIST  = "Update the perk list";
} // namespace TOOL

} // namespace PERK