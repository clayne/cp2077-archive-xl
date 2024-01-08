#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"

namespace Red
{
enum class CharacterBodyPartState : uint32_t
{
    Undefined = 0,
    Hidden = 1,
    Visible = 2,
};
}

namespace Raw::CharacterCustomizationHairstyleController
{
constexpr auto OnDetach = Core::RawFunc<
    /* addr = */ Red::Addresses::CharacterCustomizationHairstyleController_OnDetach,
    /* type = */ void (*)(Red::game::ui::CharacterCustomizationHairstyleController* aComponent, uintptr_t a2)>();

constexpr auto CheckState = Core::RawFunc<
    /* addr = */ Red::Addresses::CharacterCustomizationHairstyleController_CheckState,
    /* type = */ void (*)(Red::game::ui::CharacterCustomizationHairstyleController* aComponent,
                          Red::CharacterBodyPartState& aHairState)>();
}

namespace Raw::CharacterCustomizationGenitalsController
{
constexpr auto OnAttach = Core::RawFunc<
    /* addr = */ Red::Addresses::CharacterCustomizationGenitalsController_OnAttach,
    /* type = */ void (*)(Red::game::ui::CharacterCustomizationGenitalsController* aComponent)>();

constexpr auto CheckState = Core::RawFunc<
    /* addr = */ Red::Addresses::CharacterCustomizationGenitalsController_CheckState,
    /* type = */ void (*)(Red::game::ui::CharacterCustomizationGenitalsController* aComponent,
                          Red::CharacterBodyPartState& aUpperState,
                          Red::CharacterBodyPartState& aBottomState)>();
}

namespace Raw::CharacterCustomizationFeetController
{
// constexpr auto GetOwner = Core::RawFunc<
//     /* addr = */ Red::Addresses::CharacterCustomizationFeetController_GetOwner,
//     /* type = */ void (*)(Red::game::ui::CharacterCustomizationFeetController* aComponent,
//                           Red::Handle<Red::Entity>& aOwner)>();

constexpr auto CheckState = Core::RawFunc<
    /* addr = */ Red::Addresses::CharacterCustomizationFeetController_CheckState,
    /* type = */ void (*)(Red::game::ui::CharacterCustomizationFeetController* aComponent,
                          Red::CharacterBodyPartState& aLiftedState,
                          Red::CharacterBodyPartState& aFlatState)>();
}

namespace Raw::CharacterCustomizationSystem
{
using FemaleResource = Core::OffsetPtr<0x58, Red::SharedPtr<Red::ResourceToken<Red::gameuiCharacterCustomizationInfoResource>>>;
using MaleResource = Core::OffsetPtr<0x48, Red::SharedPtr<Red::ResourceToken<Red::gameuiCharacterCustomizationInfoResource>>>;

constexpr auto Initialize = Core::RawFunc<
    /* addr = */ Red::Addresses::CharacterCustomizationSystem_Initialize,
    /* type = */ void (*)(Red::gameuiICharacterCustomizationSystem* aSystem,
                          Red::Handle<Red::game::Puppet>& aPuppet, bool aIsMale, uintptr_t a4)>();

constexpr auto Uninitialize = Core::RawFunc<
    /* addr = */ Red::Addresses::CharacterCustomizationSystem_Uninitialize,
    /* type = */ void (*)(Red::gameuiICharacterCustomizationSystem* aSystem)>();

constexpr auto EnsureState = Core::RawFunc<
    /* addr = */ Red::Addresses::CharacterCustomizationSystem_EnsureState,
    /* type = */ void (*)(Red::gameuiICharacterCustomizationSystem* aSystem,
                          Red::Handle<Red::game::ui::CharacterCustomizationState>& aState)>();

constexpr auto InitializeAppOption = Core::RawFunc<
    /* addr = */ Red::Addresses::CharacterCustomizationSystem_InitializeAppOption,
    /* type = */ void (*)(Red::gameuiICharacterCustomizationSystem* aSystem,
                          Red::game::ui::CharacterCustomizationPart aPartType,
                          Red::Handle<Red::game::ui::CharacterCustomizationOption>& aOption,
                          Red::SortedUniqueArray<Red::CName>& aStateOptions,
                          Red::Map<Red::CName, Red::Handle<Red::game::ui::CharacterCustomizationOption>>& aUiSlots)>();

constexpr auto InitializeMorphOption = Core::RawFunc<
    /* addr = */ Red::Addresses::CharacterCustomizationSystem_InitializeMorphOption,
    /* type = */ void (*)(Red::gameuiICharacterCustomizationSystem* aSystem,
                          Red::Handle<Red::game::ui::CharacterCustomizationOption>& aOption,
                          Red::SortedUniqueArray<Red::CName>& aStateOptions,
                          Red::Map<Red::CName, Red::Handle<Red::game::ui::CharacterCustomizationOption>>& aUiSlots)>();

constexpr auto InitializeSwitcherOption = Core::RawFunc<
    /* addr = */ Red::Addresses::CharacterCustomizationSystem_InitializeSwitcherOption,
    /* type = */ bool (*)(Red::gameuiICharacterCustomizationSystem* aSystem,
                          Red::game::ui::CharacterCustomizationPart aPartType,
                          Red::Handle<Red::game::ui::CharacterCustomizationOption>& aOption,
                          int32_t aCurrentIndex,
                          uint64_t a5,
                          Red::Map<Red::CName, Red::Handle<Red::game::ui::CharacterCustomizationOption>>& aUiSlots)>();
}

namespace Raw::CharacterCustomizationHelper
{
constexpr auto GetHairColor = Core::RawFunc<
    /* addr = */ Red::Addresses::CharacterCustomizationHelper_GetHairColor,
    /* type = */ void (*)(Red::CName& aOut, Red::WeakHandle<Red::ISerializable>& aSystem, bool aIsMale)>();
}
