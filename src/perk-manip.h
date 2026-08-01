#pragma once

#include <shared_mutex>
#include <unordered_map>


namespace PERK
{
struct ActorPerkStorage : REX::Singleton<ActorPerkStorage>
{

    // mimic PerkRankData for serialisation cause i don't think BGSPerk* can be
    // serialised
    struct SerialisedPerkData
    {
        RE::FormID m_perkID;
        int8_t m_rank;
    };

    // gameplay map
    // FormID lookup isn't great during gameplay i think
    // may need some testing but this will do for now
    std::unordered_map<RE::Actor*, std::vector<RE::PerkRankData>> actor_perk_ranks;
    // serialisation map.
    std::unordered_map<RE::FormID, std::vector<SerialisedPerkData>> serialisation_map;

    void OnSave();
    void OnLoad();
    std::shared_mutex perk_mutex;
    std::vector<RE::BGSPerk*> GetPerks(RE::Actor* a_actor);
    void AddRuntimePerksToVector(RE::Actor* a_actor, std::vector<RE::BGSPerk*>& a_vectorOut);
    bool HasAnyPerk(RE::Actor* a_actor);
    bool HasRuntimeAddedPerk(RE::Actor* a_actor, RE::BGSPerk* a_perk);

  private:
    void RebuildSerialisationMap();
    void RebuildGameplayMap();
};

// perk functions
struct PerkManip
{
    // original Actor function takes rank as arg but it's most likely unused
    static void AddPerkToActor(RE::Actor* a_actor, RE::BGSPerk* a_perk);

    static void RemovePerkFromActor(RE::Actor* a_actor, RE::BGSPerk* a_perk);
    // ActorHasPerk == original
    static bool HasPerk(RE::Actor* a_actor, RE::BGSPerk* a_perk, bool original = true);
    static void ApplyPerksFromMap(RE::Actor* a_actor);
    static bool IsGameplayMapEmpty();
    static bool IsSerialisationMapEmpty();

  private:
    static bool HasPerk_Unsafe(RE::Actor* a_actor, RE::BGSPerk* a_perk);
    static void ApplyPerkToActor(RE::Actor* a_actor, RE::BGSPerk* a_perk);
};

} // namespace PERK