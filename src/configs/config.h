#pragma once

namespace Config
{
inline constexpr auto TOML_PATH_DEF   = "Data/SKSE/Plugins/styyx-npc-perks.toml";
inline constexpr auto TOML_PATH_CUS   = "Data/SKSE/Plugins/NPCPerks_custom.toml";
inline constexpr auto SETTINGS        = "Settings";
inline REX::TOML::Bool show_all_perks = {SETTINGS, "bShowAllPerksInMenu", true};

inline void UpdateConfig(const bool a_save = false)
{
    const auto toml = REX::Singleton<REX::TOML::SettingStore>::GetSingleton();
    toml->Init(TOML_PATH_DEF, TOML_PATH_CUS);
    if (!a_save)
        toml->Load();
    else
        toml->Save();
}
} // namespace Config