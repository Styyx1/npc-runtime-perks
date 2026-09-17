#include "configs/config.h"
#include "export/papyrus.h"
#include "export/perksforactors-api.h"
#include "hooks/hooks.h"
#include "menu/ingame-menu.h"
#include "perk-manip.h"
#include "serial.h"

namespace
{

bool HasRuntimePerkImpl(RE::Actor* actor, RE::BGSPerk* perk)
{
    return REX::Singleton<PERK::ActorPerkStorage>::GetSingleton()->HasRuntimeAddedPerk(actor, perk);
}

bool GetRuntimeAddedPerksImpl(RE::Actor* actor, std::vector<RE::BGSPerk*>& out)
{
    REX::Singleton<PERK::ActorPerkStorage>::GetSingleton()->AddRuntimePerksToVector(actor, out);
    return !out.empty();
}

bool GetAllActorPerksImpl(RE::Actor* actor, std::vector<RE::BGSPerk*>& out)
{
    ActorUtil::GetPerksFromBaseActor(actor, out);
    REX::Singleton<PERK::ActorPerkStorage>::GetSingleton()->AddRuntimePerksToVector(actor, out);
    return !out.empty();
}

bool _HasPerk(RE::Actor* actor, RE::BGSPerk* perk)
{
    return PERK::PerkManip::HasPerk(actor, perk, actor->HasPerk(perk));
}

} // namespace

extern "C" DLLEXPORT ActorPerksAPI* GetNPCPerkAPI()
{
    static ActorPerksAPI api{ACTOR_PERKS_API_VERSION, HasRuntimePerkImpl, GetRuntimeAddedPerksImpl,
                             GetAllActorPerksImpl, _HasPerk};

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
    Init(skse);
    SKSE::AllocTrampoline(14 * 4);
    PERK::Serialisation::InitialiseSerialisation();
    Config::UpdateConfig();
    PERK::PerkForActors::InstallHooks();
    SKSE::GetMessagingInterface()->RegisterListener(Listener);
    SKSE::GetPapyrusInterface()->Register(Papyrus::Bind);

    return true;
}