#pragma once

namespace Raw::MappinSystem
{
using CookedMappinResource = Core::OffsetPtr<0x58, Red::Handle<Red::gameMappinResource>>;
using CookedPoiResource = Core::OffsetPtr<0x68, Red::Handle<Red::gamePointOfInterestMappinResource>>;

constexpr auto GetMappinData = Core::RawFunc<
    /* addr = */ Red::AddressLib::MappinSystem_GetMappinData,
    /* type = */ void* (*)(void* aSystem, uint32_t aHash)>();

constexpr auto GetPoiData = Core::RawFunc<
    /* addr = */ Red::AddressLib::MappinSystem_GetPoiData,
    /* type = */ void* (*)(void* aSystem, uint32_t aHash)>();

constexpr auto OnStreamingWorldLoaded = Core::RawFunc<
    /* addr = */ Red::AddressLib::MappinSystem_OnStreamingWorldLoaded,
    /* type = */ void (*)(void* aSystem, Red::worldRuntimeScene*)>();
}
