#include "perk-manip.h"

#include <algorithm>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "RE/B/BGSAbilityPerkEntry.h"
#include "RE/B/BGSPerk.h"
#include "RE/B/BGSPerkEntry.h"
#include "RE/Offsets_RTTI.h"
#include "SKSE/API.h"

namespace PERK
{

void EntryTaskStuff(RE::Actor* a_actor, RE::BGSPerkEntry* a_entry, bool a_applyIt)
{
    if (!a_actor || !a_entry)
    {
        return;
    }
    // attempt to keep actor alive till task is run to prevent reported crash
    // decompilation of the dll makes me assume the task was the issue.
    const RE::NiPointer<RE::Actor> actor{a_actor};
    SKSE::GetTaskInterface()->AddTask(
        [actor, a_entry, a_applyIt]
        {
            if (actor->IsDeleted())
                return;

            if (a_applyIt)
            {
                a_entry->ApplyPerkEntry(actor.get());
            }
            else
            {
                a_entry->RemovePerkEntry(actor.get());
            }
            actor->OnArmorActorValueChanged();
        });
}

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

    std::vector<RE::BGSPerk*> result{};

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

    std::shared_lock lock(perk_mutex);
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

void ActorPerkStorage::RebuildSerialisationMap()
{

    std::unordered_map<RE::Actor*, std::vector<RE::PerkRankData>> snap;

    {
        std::shared_lock lock(perk_mutex);
        snap = actor_perk_ranks;
    }

    std::unordered_map<RE::FormID, std::vector<SerialisedPerkData>> new_map;

    for (auto& [actor, perkranks] : snap)
    {

        if (!actor)
        {
            continue;
        }

        std::vector<SerialisedPerkData> temp_data{};
        temp_data.reserve(perkranks.size());
        for (auto& perkData : perkranks)
        {
            if (!perkData.perk)
                continue;

            SerialisedPerkData data{perkData.perk->GetFormID(), perkData.currentRank};

            temp_data.push_back(data);
        }
        if (!temp_data.empty())
            new_map.emplace(actor->GetFormID(), std::move(temp_data));
    }
    {
        std::unique_lock lock(perk_mutex);
        serialisation_map = std::move(new_map);
    }
}

void ActorPerkStorage::RebuildGameplayMap()
{
    std::unordered_map<RE::FormID, std::vector<SerialisedPerkData>> snap;

    {
        std::shared_lock lock(perk_mutex);
        snap = serialisation_map;
    }

    std::unordered_map<RE::Actor*, std::vector<RE::PerkRankData>> new_map;

    for (auto& [actorID, perkranks] : snap)
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

        new_map[actor] = std::move(temp_data);
    }
    {
        std::unique_lock lock(perk_mutex);
        actor_perk_ranks = std::move(new_map);
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
    const auto ps = REX::TSingleton<ActorPerkStorage>::GetSingleton();

    bool apply = false;
    {
        std::unique_lock lock(ps->perk_mutex);
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
        for (auto& entry : a_perk->perkEntries)
        {
            if (entry)
            {
                EntryTaskStuff(a_actor, entry, true);
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

    auto* ps = REX::TSingleton<ActorPerkStorage>::GetSingleton();

    bool removeEntries = false;

    {
        std::unique_lock lock(ps->perk_mutex);
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
        if (entry)
        {
            EntryTaskStuff(a_actor, entry, false);
        }
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
                        if (!entry || !a_actor)
                        {
                            return;
                        }
                        entry->ApplyPerkEntry(a_actor);
                        a_actor->OnArmorActorValueChanged();
                    });
            }
            ranks--;
        }
    }
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
    std::shared_lock lock(ps->perk_mutex);
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
