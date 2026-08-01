#pragma once

namespace Papyrus
{
using VM      = RE::BSScript::Internal::VirtualMachine;
using StackID = RE::VMStackID;

bool Bind(VM* a_vm);

namespace Functions
{
constexpr uint32_t kVersion = 1;

uint32_t GetVersion(VM*, StackID, RE::StaticFunctionTag*);
std::vector<RE::BGSPerk*> GetAllPerksFromActor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor);
std::vector<RE::BGSPerk*> GetAllRuntimePerksFromActor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*,
                                                      RE::Actor* a_actor);

void Bind(VM& a_vm);
} // namespace Functions
} // namespace Papyrus