#pragma once

namespace PERK
{
namespace PerkForActors
{
void InstallHooks();

struct AddPerk
{
    static void Call(RE::Actor* a_this, RE::BGSPerk* a_perk, std::uint32_t a_rank = 0);
    static inline REL::Relocation<decltype(Call)> func;
};

struct RemovePerk
{
    static void Call(RE::Actor* a_this, RE::BGSPerk* a_perk);
    static inline REL::Relocation<decltype(Call)> func;
};

struct ApplyTempPerk
{
    static void Call(RE::Actor* a_this, RE::BGSPerk* a_perk);
    static inline REL::Relocation<decltype(Call)> func;
};

struct RemoveTempPerk
{
    static void Call(RE::Actor* a_this, RE::BGSPerk* a_perk);
    static inline REL::Relocation<decltype(Call)> func;
};

struct ForEachHook
{
    static void Call(RE::Actor* a_actor, RE::PerkEntryVisitor& a_visitor);
    static inline REL::Relocation<decltype(Call)> func;
};
}; // namespace PerkForActors
} // namespace PERK
