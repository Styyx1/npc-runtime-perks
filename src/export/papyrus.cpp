#include "papyrus.h"

#include "REX/REX/Singleton.h"
#include "hooks/hooks.h"
#include "perk-manip.h"

namespace Papyrus
{
bool Bind(VM* a_vm)
{
    if (!a_vm)
    {
        logs::critical("couldn't get VM State"sv);
        return false;
    }

    logs::info("{:=^30}", "PAPYRUS"sv);

    Functions::Bind(*a_vm);

    return true;
}

void Functions::Bind(VM& a_vm)
{
    constexpr auto script_name = "Styyx_PerksForNPC"sv;

    a_vm.RegisterFunction("GetVersion", script_name, GetVersion, true);
    a_vm.RegisterFunction("GetAllPerksFromActor", script_name, GetAllPerksFromActor);
    a_vm.RegisterFunction("GetAllRuntimePerksFromActor", script_name, GetAllRuntimePerksFromActor);

    logs::info("Registered papyrus functions");
}

uint32_t Functions::GetVersion(VM*, StackID, RE::StaticFunctionTag*)
{
    return kVersion;
}

std::vector<RE::BGSPerk*> Functions::GetAllPerksFromActor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*,
                                                          RE::Actor* a_actor)
{
    logs::info("GetAllRuntimePerksFromActor"sv);
    std::vector<RE::BGSPerk*> perks{};
    if (!a_actor)
    {
        a_vm->TraceStack("Actor is None", a_stackID);
        return perks;
    }

    perks = REX::Singleton<PERK::ActorPerkStorage>::GetSingleton()->GetPerks(a_actor);
    ActorUtil::GetPerksFromBaseActor(a_actor, perks);
    return perks;
}

std::vector<RE::BGSPerk*> Functions::GetAllRuntimePerksFromActor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*,
                                                                 RE::Actor* a_actor)
{
    std::vector<RE::BGSPerk*> perks{};
    if (!a_actor)
    {
        a_vm->TraceStack("Actor is None", a_stackID);
        return perks;
    }

    perks = REX::Singleton<PERK::ActorPerkStorage>::GetSingleton()->GetPerks(a_actor);
    return perks;
}
} // namespace Papyrus
