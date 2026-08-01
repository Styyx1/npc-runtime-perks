#pragma once

namespace PERK::Serialisation
{
void InitialiseSerialisation();
static constexpr uint32_t Version = 1;
static constexpr uint32_t ID      = 'PENP';
static constexpr uint32_t Type    = 'PFNP';
void SaveCallback(SKSE::SerializationInterface* a_intfc);
void LoadCallback(SKSE::SerializationInterface* a_intfc);
void RevertCallback([[maybe_unused]] SKSE::SerializationInterface* a_intfc);
} // namespace PERK::Serialisation