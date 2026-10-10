#include "serial.h"

#include <cstdint>
#include <mutex>
#include <shared_mutex>

#include "RE/B/BSCoreTypes.h"
#include "REX/LOG.h"
#include "REX/TSingleton.h"
#include "SKSE/API.h"
#include "perk-manip.h"

namespace PERK
{
namespace
{
using PerkData  = ActorPerkStorage::SerialisedPerkData;
using serde_map = std::unordered_map<RE::FormID, std::vector<PerkData>>;

} // namespace
void Serialisation::InitialiseSerialisation()
{
    if (auto serde = SKSE::GetSerializationInterface())
    {
        serde->SetUniqueID(ID);
        serde->SetSaveCallback(&SaveCallback);
        serde->SetLoadCallback(&LoadCallback);
        serde->SetRevertCallback(&RevertCallback);
    }
}
void Serialisation::SaveCallback(SKSE::SerializationInterface* a_intfc)
{

    REX::INFO("Start saving to SKSE co-save");

    auto ps = REX::TSingleton<ActorPerkStorage>::GetSingleton();
    ps->OnSave();

    serde_map local{};
    {
        std::shared_lock lock(ps->perk_mutex);
        local = ps->serialisation_map;
    }
    if (!a_intfc->OpenRecord(Type, Version))
    {
        REX::ERROR("Failed to open record on save");
        return;
    }

    const auto size = static_cast<uint32_t>(local.size());
    if (!a_intfc->WriteRecordData(size))
    {
        REX::ERROR("Failed to save map size");
        return;
    }
    for (const auto& [actorID, perkArray] : local)
    {

        const auto perkCount = static_cast<uint32_t>(perkArray.size());

        if (!a_intfc->WriteRecordData(actorID) || !a_intfc->WriteRecordData(perkCount))
        {
            REX::ERROR("Failed to write data for actor {:08x}", actorID);
            return;
        }
        for (const auto& perk : perkArray)
        {
            if (!a_intfc->WriteRecordData(perk.m_perkID) || !a_intfc->WriteRecordData(static_cast<int8_t>(perk.m_rank)))
            {
                REX::ERROR("Failed to write perk {:08X} for actor {:08X}", perk.m_perkID, actorID);
                return;
            }
        }
    }

    REX::INFO("Saved {} actors with perks", size);
}
void Serialisation::LoadCallback(SKSE::SerializationInterface* a_intfc)
{

    uint32_t type, version, length;

    while (a_intfc->GetNextRecordInfo(type, version, length))
    {

        if (type != Type)
        {
            continue;
        }

        if (version != Version)
        {
            REX::ERROR("Unsupported record version {} (expected {})", version, Version);
            continue;
        }

        uint32_t count = 0;
        if (!a_intfc->ReadRecordData(count))
        {
            REX::ERROR("Can not read Actor count from save");
            return;
        }
        serde_map local{};

        for (uint32_t iter = 0; iter < count; ++iter)
        {

            RE::FormID actorID = 0;
            uint32_t perkCount = 0;

            if (!a_intfc->ReadRecordData(actorID) || !a_intfc->ReadRecordData(perkCount))
            {
                REX::ERROR("Failed to read data for actor");
                return;
            }
            std::vector<ActorPerkStorage::SerialisedPerkData> perks;
            for (uint32_t perkIter = 0; perkIter < perkCount; ++perkIter)
            {
                RE::FormID perkID = 0;
                int8_t rank       = 0;

                if (!a_intfc->ReadRecordData(perkID) || !a_intfc->ReadRecordData(rank))
                {
                    REX::ERROR("Failed to read perk data");
                    return;
                }

                RE::FormID resolvedPerk = 0;

                if (!a_intfc->ResolveFormID(perkID, resolvedPerk))
                {
                    REX::WARN("Dropping perk {:08x}: couldn't resolve form (mod removed?)", perkID);
                    continue;
                }
                perks.push_back({resolvedPerk, rank});
            }
            RE::FormID resolvedActor = 0;
            if (!a_intfc->ResolveFormID(actorID, resolvedActor))
            {
                REX::WARN("Dropping actor {:08x}: couldn't resolve actor (mod removed?)", actorID);
                continue;
            }
            if (perks.empty())
            {
                continue;
            }
            local.emplace(resolvedActor, std::move(perks));
        }
        const auto ps            = REX::TSingleton<ActorPerkStorage>::GetSingleton();
        std::size_t loaded_count = 0;
        {
            std::unique_lock lock(ps->perk_mutex);
            ps->serialisation_map = std::move(local);
            loaded_count          = ps->serialisation_map.size();
        }
        ps->OnLoad();
        REX::INFO("Loaded {} actors with perks", loaded_count);
        return;
    }
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