#include "configs/config.h"
#include "export/papyrus.h"
#include "export/perksforactors-api.h"
#include "hooks/hooks.h"
#include "menu/ingame-menu.h"
#include "perk-manip.h"
#include "serial.h"
#include "st-actor.h"

namespace
{

bool HasRuntimePerkImpl(RE::Actor* actor, RE::BGSPerk* perk)
{
    return REX::TSingleton<PERK::ActorPerkStorage>::GetSingleton()->HasRuntimeAddedPerk(actor, perk);
}

bool GetRuntimeAddedPerksImpl(RE::Actor* actor, std::vector<RE::BGSPerk*>& out)
{
    REX::TSingleton<PERK::ActorPerkStorage>::GetSingleton()->AddRuntimePerksToVector(actor, out);
    return !out.empty();
}

bool GetAllActorPerksImpl(RE::Actor* actor, std::vector<RE::BGSPerk*>& out)
{
    StyyxUtil::ActorUtil::GetPerksFromBaseActor(actor, out);
    REX::TSingleton<PERK::ActorPerkStorage>::GetSingleton()->AddRuntimePerksToVector(actor, out);
    return !out.empty();
}
} // namespace

extern "C" DLLEXPORT ActorPerksAPI* GetNPCPerkAPI()
{
    static ActorPerksAPI api{ACTOR_PERKS_API_VERSION, HasRuntimePerkImpl, GetRuntimeAddedPerksImpl,
                             GetAllActorPerksImpl};

    return &api;
}

void Listener(SKSE::MessagingInterface::Message* a_msg)
{
    switch (a_msg->type)
    {
        case SKSE::MessagingInterface::kDataLoaded:
            PERK::FUCKMenu::RegisterFMenu();
            break;
        default:
            break;
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    Init(skse, {.trampoline = true, .trampolineSize = 14 * 4});
    PERK::Serialisation::InitialiseSerialisation();
    Config::UpdateConfig();
    SKSE::GetMessagingInterface()->RegisterListener(Listener);
    SKSE::GetPapyrusInterface()->Register(Papyrus::Bind);

    return true;
}