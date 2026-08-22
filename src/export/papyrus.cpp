#include "papyrus.h"

#include <st-actor.h>

#include "REX/REX/Singleton.h"
#include "hooks/hooks.h"
#include "perk-manip.h"

namespace Papyrus
{
bool Bind(VM* a_vm)
{
    if (!a_vm)
    {
        REX::CRITICAL("couldn't get VM State");
        return false;
    }

    REX::INFO("{:=^30}", "PAPYRUS");

    Functions::Bind(*a_vm);

    return true;
}

void Functions::Bind(VM& a_vm)
{
    constexpr auto script_name = "Styyx_PerksForNPC";

    a_vm.RegisterFunction("GetVersion", script_name, GetVersion, true);
    a_vm.RegisterFunction("GetAllPerksFromActor", script_name, GetAllPerksFromActor);
    a_vm.RegisterFunction("GetAllRuntimePerksFromActor", script_name, GetAllRuntimePerksFromActor);

    REX::INFO("Registered papyrus functions");
}

uint32_t Functions::GetVersion(VM*, StackID, RE::StaticFunctionTag*)
{
    return kVersion;
}

std::vector<RE::BGSPerk*> Functions::GetAllPerksFromActor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*,
                                                          RE::Actor* a_actor)
{
    REX::INFO("GetAllRuntimePerksFromActor");
    std::vector<RE::BGSPerk*> perks{};
    if (!a_actor)
    {
        a_vm->TraceStack("Actor is None", a_stackID);
        return perks;
    }

    perks = REX::TSingleton<PERK::ActorPerkStorage>::GetSingleton()->GetPerks(a_actor);
    StyyxUtil::ActorUtil::GetPerksFromBaseActor(a_actor, perks);
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

    perks = REX::TSingleton<PERK::ActorPerkStorage>::GetSingleton()->GetPerks(a_actor);
    return perks;
}
} // namespace Papyrus
