#include "perk-manip.h"

#include <algorithm>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "RE/B/BGSAbilityPerkEntry.h"
#include "RE/B/BGSPerkEntry.h"
#include "RE/Offsets_RTTI.h"
#include "SKSE/API.h"

namespace PERK
{

void ActorPerkStorage::OnSave()
{
    RebuildSerialisationMap();
}
void ActorPerkStorage::OnLoad()
{
    RebuildGameplayMap();
}
std::vector<RE::BGSPerk*> ActorPerkStorage::GetPerks(RE::Actor* a_actor)
{
    std::vector<RE::BGSPerk*> result;

    if (!a_actor)
        return result;

    std::shared_lock lock(perk_mutex);

    auto it = actor_perk_ranks.find(a_actor);
    if (it == actor_perk_ranks.end())
        return result;

    result.reserve(it->second.size());

    for (const auto& data : it->second)
    {
        if (data.perk)
            result.push_back(data.perk);
    }
    return result;
}
void ActorPerkStorage::AddRuntimePerksToVector(RE::Actor* a_actor, std::vector<RE::BGSPerk*>& a_vectorOut)
{
    if (!a_actor)
    {
        return;
    }
    std::shared_lock lock(perk_mutex);
    auto it = actor_perk_ranks.find(a_actor);
    if (it == actor_perk_ranks.end())
    {
        return;
    }

    for (const auto& data : it->second)
    {
        if (data.perk)
        {
            a_vectorOut.push_back(data.perk);
        }
    }
}

bool ActorPerkStorage::HasAnyPerk(RE::Actor* a_actor)
{
    if (!a_actor)
    {
        return false;
    }
    auto it = actor_perk_ranks.find(a_actor);
    return it != actor_perk_ranks.end() && !it->second.empty();
}
bool ActorPerkStorage::HasRuntimeAddedPerk(RE::Actor* a_actor, RE::BGSPerk* a_perk)
{
    if (!a_actor || !a_perk)
    {
        return false;
    }
    auto it = actor_perk_ranks.find(a_actor);
    if (it == actor_perk_ranks.end())
    {
        return false;
    }

    return std::ranges::any_of(it->second,
                               [a_perk](RE::PerkRankData& a_data) { return a_data.perk && a_data.perk == a_perk; });
}
void ActorPerkStorage::LogPerksFromMap(RE::Actor* a_actor)
{

    if (!a_actor)
    {
        return;
    }
    auto it = actor_perk_ranks.find(a_actor);
    if (it != actor_perk_ranks.end())
    {
        for (auto& data : it->second)
        {
            if (!data.perk)
            {
                continue;
            }
        }
    }
}


void ActorPerkStorage::RebuildSerialisationMap()
{

    serialisation_map.clear();
    std::unordered_map<RE::Actor*, std::vector<RE::PerkRankData>> temp_map{};

    {
        std::shared_lock lock(perk_mutex);
        temp_map = actor_perk_ranks;
    }

    for (auto& [actor, perkranks] : temp_map)
    {

        if (!actor)
        {
            continue;
        }

        logs::info("saving for actor: {}", actor->GetName());
        std::vector<SerialisedPerkData> temp_data{};
        temp_data.reserve(perkranks.size());
        for (auto& perkData : perkranks)
        {
            if (!perkData.perk)
                continue;

            logs::info("saving perk: {}", perkData.perk->GetName());

            SerialisedPerkData data{perkData.perk->GetFormID(), perkData.currentRank};

            temp_data.push_back(data);
        }
        if (!temp_data.empty())
            serialisation_map.emplace(actor->GetFormID(), std::move(temp_data));
    }
}
void ActorPerkStorage::RebuildGameplayMap()
{
    actor_perk_ranks.clear();

    std::unordered_map<RE::FormID, std::vector<SerialisedPerkData>> temp_map;

    {
        std::shared_lock lock(perk_mutex);
        temp_map = serialisation_map;
    }

    for (auto& [actorID, perkranks] : temp_map)
    {
        const auto act_form = RE::TESForm::LookupByID(actorID);
        const auto actor    = act_form ? act_form->As<RE::Actor>() : nullptr;

        if (!actor)
            continue;

        std::vector<RE::PerkRankData> temp_data;
        temp_data.reserve(perkranks.size());

        for (auto& perkData : perkranks)
        {
            const auto form = RE::TESForm::LookupByID(perkData.m_perkID);
            const auto perk = form ? form->As<RE::BGSPerk>() : nullptr;

            if (!perk)
                continue;

            temp_data.emplace_back(perk, perkData.m_rank);
        }

        actor_perk_ranks[actor] = std::move(temp_data);
    }
}
void PerkManip::AddPerkToActor(RE::Actor* a_actor, RE::BGSPerk* a_perk)
{

    if (!a_actor || !a_perk)
    {
        return;
    }
    // ActorPerkStorage::GetSingleton() is completely fine but clangd complains
    // and shows anything used from that scruct as error unless gotten explicitely
    // the magic of developing on linux
    const auto ps = REX::Singleton<ActorPerkStorage>::GetSingleton();

    bool apply = false;
    {
        auto& perks = ps->actor_perk_ranks[a_actor];

        auto perkIt =
            std::ranges::find_if(perks, [a_perk](const RE::PerkRankData& data) { return data.perk == a_perk; });

        if (perkIt != perks.end())
        {
            if (perkIt->currentRank < a_perk->data.numRanks)
            {
                ++perkIt->currentRank;
            }
            return;
        }
        perks.emplace_back(a_perk, 1);
        apply = true;
    }

    if (apply)
    {
        for (const auto entry : a_perk->perkEntries)
        {
            if (entry)
            {
                SKSE::GetTaskInterface()->AddTask(
                    [entry, a_actor]()
                    {
                        entry->ApplyPerkEntry(a_actor);
                        a_actor->OnArmorActorValueChanged();
                    });
            }
        }
    }
}
void PerkManip::RemovePerkFromActor(RE::Actor* a_actor, RE::BGSPerk* a_perk)
{
    if (!a_actor || !a_perk)
    {
        return;
    }

    auto* ps = REX::Singleton<ActorPerkStorage>::GetSingleton();

    bool removeEntries = false;

    {
        auto actorIt = ps->actor_perk_ranks.find(a_actor);
        if (actorIt == ps->actor_perk_ranks.end())
        {
            return;
        }

        auto& perks = actorIt->second;

        auto perkIt =
            std::ranges::find_if(perks, [a_perk](const RE::PerkRankData& data) { return data.perk == a_perk; });

        if (perkIt == perks.end())
        {
            return;
        }

        if (perkIt->currentRank > 1)
        {
            --perkIt->currentRank;
            return;
        }

        perks.erase(perkIt);

        if (perks.empty())
        {
            ps->actor_perk_ranks.erase(actorIt);
        }

        removeEntries = true;
    }

    if (!removeEntries)
    {
        return;
    }

    for (auto* entry : a_perk->perkEntries)
    {
        if (!entry)
        {
            continue;
        }

        switch (entry->GetType())
        {
            case RE::PERK_ENTRY_TYPE::kEntryPoint:
                SKSE::GetTaskInterface()->AddTask(
                    [entry, a_actor]()
                    {
                        entry->RemovePerkEntry(a_actor);
                        a_actor->OnArmorActorValueChanged();
                    });

                break;
            case RE::PERK_ENTRY_TYPE::kAbility:
            {
                RE::BGSAbilityPerkEntry* ab = static_cast<RE::BGSAbilityPerkEntry*>(entry);
                if (ab)
                {
                    SKSE::GetTaskInterface()->AddTask(
                        [ab, a_actor]()
                        {
                            ab->RemovePerkEntry(a_actor);
                            a_actor->OnArmorActorValueChanged();
                        });
                }
                else
                {
                    logs::warn("CAST FAILED");
                }
            }
            break;
            default:
                break;
        }
        a_actor->OnArmorActorValueChanged();
    }
}
bool PerkManip::HasPerk(RE::Actor* a_actor, RE::BGSPerk* a_perk, bool original)
{
    auto ret = HasPerk_Unsafe(a_actor, a_perk);
    return ret || original;
}
void PerkManip::ApplyPerksFromMap(RE::Actor* a_actor)
{

    if (!a_actor)
    {
        return;
    }

    auto ps = REX::Singleton<ActorPerkStorage>::GetSingleton();
    std::vector<RE::PerkRankData> perks;

    auto it = ps->actor_perk_ranks.find(a_actor);
    if (it == ps->actor_perk_ranks.end())
        return;

    perks = it->second;

    std::size_t applications = 0;

    for (auto perkData : perks)
    {
        RE::BGSPerk* t_perk = perkData.perk;
        int8_t ranks        = perkData.currentRank;

        while (ranks > 0)
        {
            for (auto& entry : t_perk->perkEntries)
            {
                ++applications;
                SKSE::GetTaskInterface()->AddTask(
                    [entry, a_actor]()
                    {
                        entry->ApplyPerkEntry(a_actor);
                        a_actor->OnArmorActorValueChanged();
                    });
            }
            ranks--;
        }
    }
    logs::info("Applied {} perk entries to {}", applications, a_actor->GetName());
}
bool PerkManip::IsGameplayMapEmpty()
{

    auto ps = REX::Singleton<ActorPerkStorage>::GetSingleton();
    return !ps->actor_perk_ranks.empty();
}
bool PerkManip::IsSerialisationMapEmpty()
{
    auto ps = REX::Singleton<ActorPerkStorage>::GetSingleton();
    return !ps->serialisation_map.empty();
}
bool PerkManip::HasPerk_Unsafe(RE::Actor* a_actor, RE::BGSPerk* a_perk)
{

    if (!a_actor || !a_perk)
    {
        return false;
    }

    auto ps = REX::Singleton<ActorPerkStorage>::GetSingleton();

    auto it = ps->actor_perk_ranks.find(a_actor);
    if (it == ps->actor_perk_ranks.end())
    {
        return false;
    }

    return std::ranges::any_of(it->second,
                               [a_perk](const RE::PerkRankData& perkData) { return perkData.perk == a_perk; });
}
void PerkManip::ApplyPerkToActor(RE::Actor* a_actor, RE::BGSPerk* a_perk)
{

    if (!a_actor || !a_perk)
    {
        return;
    }
    for (auto& entry : a_perk->perkEntries)
    {
        entry->ApplyPerkEntry(a_actor);
    }
};

} // namespace PERK