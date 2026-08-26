#include "serial.h"

#include "perk-manip.h"

using serde_map = std::unordered_map<RE::FormID, std::vector<PERK::ActorPerkStorage::SerialisedPerkData>>;

using gp_map = std::unordered_map<RE::Actor*, std::vector<RE::PerkRankData>>;

namespace PERK
{
void Serialisation::InitialiseSerialisation()
{
    if (const auto serialisation = SKSE::GetSerializationInterface())
    {
        REX::INFO("{:=^30}", "SERIALISATION");
        serialisation->SetUniqueID(ID);
        serialisation->SetSaveCallback(&SaveCallback);
        serialisation->SetLoadCallback(&LoadCallback);
        serialisation->SetRevertCallback(&RevertCallback);
        REX::INFO("Initialised serialisation");
    }
}

void Serialisation::SaveCallback(SKSE::SerializationInterface* a_intfc)
{
    REX::INFO("Start saving to SKSE co-save");

    const auto ps = REX::TSingleton<ActorPerkStorage>::GetSingleton();
    ps->OnSave();

    serde_map local{};
    // extra scope for lock and copy map
    {
        std::shared_lock lock(ps->perk_mutex);
        local = ps->serialisation_map;
    }

    if (!a_intfc->OpenRecord(Type, Version))
    {
        REX::ERROR("Failed to open record");
        return;
    }

    const uint32_t count = static_cast<uint32_t>(local.size());

    if (!a_intfc->WriteRecordData(count))
    {
        REX::ERROR("Failed to write map size");
        return;
    }

    for (const auto& [actorID, perkArray] : local)
    {
        if (!a_intfc->WriteRecordData(actorID))
            return;

        const uint32_t perkCount = static_cast<uint32_t>(perkArray.size());

        if (!a_intfc->WriteRecordData(perkCount))
            return;

        for (const auto& perkData : perkArray)
        {
            if (!a_intfc->WriteRecordData(perkData.m_perkID))
                return;

            if (!a_intfc->WriteRecordData(perkData.m_rank))
                return;
        }
    }

    REX::INFO("Saved {} actors with perks", count);
}

void Serialisation::LoadCallback(SKSE::SerializationInterface* a_intfc)
{
    uint32_t type, version, length;

    if (!a_intfc->GetNextRecordInfo(type, version, length))
        return;

    if (type != Type || version != Version)
    {
        REX::ERROR("version or type don't match");
        return;
    }

    const auto ps = REX::TSingleton<ActorPerkStorage>::GetSingleton();

    uint32_t count = 0;
    if (!a_intfc->ReadRecordData(count))
    {
        REX::ERROR("Failed to read actor count");
        return;
    }
    REX::INFO("Loaded Actor Count: {}", count);

    serde_map new_map{};

    for (uint32_t actor_it = 0; actor_it < count; actor_it++)
    {

        RE::FormID actorID;
        if (!a_intfc->ReadRecordData(actorID))
        {
            REX::ERROR("Failed to read ActorID on load");
            return;
        }
        // REX::INFO("ActorID is: {}", actorID);
        uint32_t perk_data_count = 0;
        if (!a_intfc->ReadRecordData(perk_data_count))
        {
            REX::ERROR("Failed to read Perk Count");
            return;
        }

        RE::FormID resolvedActorID;
        if (!a_intfc->ResolveFormID(actorID, resolvedActorID))
        {
            REX::ERROR("Failed to resolve actor ID");
            continue;
        }

        std::vector<ActorPerkStorage::SerialisedPerkData> temp_perks{};
        temp_perks.reserve(perk_data_count);

        for (uint32_t perk_iterator = 0; perk_iterator < perk_data_count; perk_iterator++)
        {

            RE::FormID perk_id;
            int8_t rank;

            if (!a_intfc->ReadRecordData(perk_id))
            {
                REX::ERROR("Failed to read PerkID");
                return;
            }
            if (!a_intfc->ReadRecordData(rank))
            {
                REX::ERROR("Failed to read Rank");
                return;
            }

            RE::FormID resolvedPerkID;

            if (!a_intfc->ResolveFormID(perk_id, resolvedPerkID))
            {
                REX::WARN("Failed to Resolve PerkID");
                continue;
            }
            temp_perks.push_back({resolvedPerkID, rank});
        }
        if (!temp_perks.empty())
        {
            new_map.emplace(resolvedActorID, std::move(temp_perks));
        }
    }

    {
        std::unique_lock lock(ps->perk_mutex);
        ps->serialisation_map = std::move(new_map);
    }
    ps->OnLoad();
    REX::INFO("Loaded {} actors with perks", ps->serialisation_map.size());
}

void Serialisation::RevertCallback([[maybe_unused]] SKSE::SerializationInterface* a_intfc)
{
    REX::INFO("Reverting actors with perks");

    const auto ps = REX::TSingleton<ActorPerkStorage>::GetSingleton();

    std::unique_lock lock(ps->perk_mutex);
    ps->actor_perk_ranks.clear();
    ps->serialisation_map.clear();
}
} // namespace PERK