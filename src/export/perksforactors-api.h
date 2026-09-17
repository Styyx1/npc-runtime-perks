/*

Copyright (c) <2026> <Styyx>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the “Software”), to deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#pragma once

#include <cstdint>
#include <vector>

inline constexpr const char* ACTOR_PERKS_DLL_NAME = "styyx-npc-perks.dll";
inline constexpr const char* ACTOR_PERKS_API_FUNC = "GetNPCPerkAPI";
inline constexpr uint32_t ACTOR_PERKS_API_VERSION = 2;

struct ActorPerksAPI
{
    uint32_t apiVersion;

    bool (*HasRuntimePerk)(RE::Actor*, RE::BGSPerk*);
    bool (*GetRuntimeAddedPerks)(RE::Actor*, std::vector<RE::BGSPerk*>&);
    bool (*GetAllActorPerks)(RE::Actor*, std::vector<RE::BGSPerk*>&);
    //new in API version 2
    bool (*_HasPerk)(RE::Actor*, RE::BGSPerk*);
};

inline ActorPerksAPI* RequestActorPerksAPI()
{
    using GetAPIFn = ActorPerksAPI* (*)();

    const auto dll = REX::W32::GetModuleHandleA(ACTOR_PERKS_DLL_NAME);
    if (!dll)
    {
        return nullptr;
    }

    const auto getter = reinterpret_cast<GetAPIFn>(REX::W32::GetProcAddress(dll, ACTOR_PERKS_API_FUNC));

    if (!getter)
    {
        return nullptr;
    }

    auto* api = getter();
    if (!api || api->apiVersion != ACTOR_PERKS_API_VERSION)
    {
        return nullptr;
    }

    return api;
}

// convenient way of getting the api. Call on kPostLoad or kDataLoaded, NOT on plugin load
inline ActorPerksAPI* GetActorPerksAPI()
{
    static ActorPerksAPI* api = nullptr;

    if (!api)
    {
        api = RequestActorPerksAPI();
    }

    return api;
}
