#pragma once

namespace PERK
{
namespace PerkForActors
{
void InstallHooks();

struct AddPerk
{
    static void Call(RE::Actor* a_this, RE::BGSPerk* a_perk, std::uint32_t a_rank = 0);
    static inline REL::THookVFT func{RE::Character::VTABLE[0], 0xfb, Call};
};

struct RemovePerk
{
    static void Call(RE::Actor* a_this, RE::BGSPerk* a_perk);
    static inline REL::THookVFT func{RE::Character::VTABLE[0], 0xfc, Call};
};

struct ApplyTempPerk
{
    static void Call(RE::Actor* a_this, RE::BGSPerk* a_perk);
    static inline REL::THookVFT func{RE::Character::VTABLE[0], 0xfd, Call};
};

struct RemoveTempPerk
{
    static void Call(RE::Actor* a_this, RE::BGSPerk* a_perk);
    static inline REL::THookVFT func{RE::Character::VTABLE[0], 0xfe, Call};
};

struct ForEachHook
{
    static void Call(RE::Actor* a_actor, RE::PerkEntryVisitor& a_visitor);
    static inline REL::THookVFT func{RE::Character::VTABLE[0], 0xfa, Call};
};

struct HasPerkConditionStruct
{
    static bool Call(RE::Actor* a_this, RE::BGSPerk* a_perk);
    static inline REL::THook func{REL::ID(21622), 0x28, Call};
};
struct HasPerkConsoleStruct
{
    static bool Call(RE::Actor* a_this, RE::BGSPerk* a_perk);
    static inline REL::THook func{REL::ID(22448), 0x93, Call};
};
struct HasPerk_14074eb88Struct
{
    static bool Call(RE::Actor* a_this, RE::BGSPerk* a_perk);
    static inline REL::THook func{REL::ID(40770), 0x38, Call};
};
struct HasPerkPapyrusStruct
{
    static bool Call(RE::Actor* a_this, RE::BGSPerk* a_perk);
    static inline REL::THook func{REL::ID(54644), 0xf, Call};
};

}; // namespace PerkForActors
} // namespace PERK
