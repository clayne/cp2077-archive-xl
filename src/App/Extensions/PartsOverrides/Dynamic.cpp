#include "Dynamic.hpp"
#include "App/Utils/Str.hpp"
#include "Red/AppearanceChanger.hpp"
#include "Red/CharacterCustomization.hpp"
#include "Red/Entity.hpp"

namespace
{
constexpr auto DynamicValueMarker = '*';

constexpr auto VariantMarker = '!';
constexpr auto PartSeparator = '+';
constexpr auto PartNameGlue = '.';
constexpr auto ConditionMarker = '&';
constexpr auto ReferenceMarkers = "!&";

constexpr auto AttrOpen = '{';
constexpr auto AttrClose = '}';

constexpr auto GenderAttr = Red::CName("gender");
constexpr auto CameraAttr = Red::CName("camera");
constexpr auto BodyTypeAttr = Red::CName("body_type");
constexpr auto LegsStateAttr = Red::CName("legs_state");
constexpr auto SkinColorAttr = Red::CName("skin_color");
constexpr auto HairColorAttr = Red::CName("hair_color");
constexpr auto VariantAttr = Red::CName("variant");

constexpr auto GenderSuffix = Red::TweakDBID("itemsFactoryAppearanceSuffix.Gender");
constexpr auto CameraSuffix = Red::TweakDBID("itemsFactoryAppearanceSuffix.Camera");
constexpr auto BodyTypeSuffix = Red::TweakDBID("itemsFactoryAppearanceSuffix.BodyType");
constexpr auto LegsStateSuffix = Red::TweakDBID("itemsFactoryAppearanceSuffix.LegsState");

constexpr auto MaleAttrValue = "m";
constexpr auto FemaleAttrValue = "w";
constexpr auto MaleSuffixValue = "Male";
constexpr auto FemaleSuffixValue = "Female";

constexpr auto DefaultBodyTypeAttrValue = "base"; // FIXME: Better name?
constexpr auto DefaultBodyTypeSuffixValue = "BaseBody";

constexpr auto MaleBodyComponent = Red::CName("t0_000_pma_base__full");
constexpr auto FemaleBodyComponent1 = Red::CName("t0_000_pwa_base__full");
constexpr auto FemaleBodyComponent2 = Red::CName("t0_000_pwa_fpp__torso");

const std::string s_emptyPathStr;

Red::CName ExtractName(const char* aName, size_t aOffset, size_t aSize, bool aRegister = false)
{
    if (!aSize)
        return {};

    if (aRegister)
    {
        std::string nameStr(aName + aOffset, aSize);
        return Red::CNamePool::Add(nameStr.c_str());
    }

    return Red::FNV1a64(reinterpret_cast<const uint8_t*>(aName + aOffset), aSize);
}
}

App::DynamicAppearanceName::DynamicAppearanceName()
    : isDynamic(false)
{
}

App::DynamicAppearanceName::DynamicAppearanceName(Red::CName aAppearance)
    : isDynamic(false)
{
    std::string_view str = aAppearance.ToString();
    auto markerPos = str.find_first_of(VariantMarker);

    if (markerPos != std::string_view::npos)
    {
        {
            auto suffixPos = str.find(ConditionMarker, markerPos);
            if (suffixPos != std::string_view::npos)
            {
                str.remove_suffix(str.size() - suffixPos);
            }
        }

        isDynamic = true;
        name = ExtractName(str.data(), 0, markerPos);

        str.remove_prefix(markerPos + 1);

        if (!str.empty())
        {
            variant = ExtractName(str.data(), 0, str.size(), true);
            parts[VariantAttr] = variant;

            uint8_t partNum[2]{PartNameGlue, '1'};
            size_t skip;

            while (!str.empty())
            {
                markerPos = str.find(PartSeparator);

                if (markerPos == 0)
                {
                    str.remove_prefix(1);
                    continue;
                }

                if (markerPos == std::string_view::npos)
                {
                    markerPos = str.size();
                    skip = str.size();
                }
                else
                {
                    skip = markerPos + 1;
                }

                auto partName = Red::FNV1a64(partNum, 2, VariantAttr);
                auto partValue = ExtractName(str.data(), 0, markerPos, true);
                parts[partName] = partValue;

                str.remove_prefix(skip);
                ++partNum[1];
            }
        }
    }
    else
    {
        name = aAppearance;
    }
}

App::DynamicReference::DynamicReference(Red::CName aReference)
    : isDynamic(false)
    , isConditional(false)
{
    std::string_view str = aReference.ToString();
    auto markerPos = str.find_first_of(ReferenceMarkers);

    if (markerPos != std::string_view::npos)
    {
        isDynamic = true;
        name = ExtractName(str.data(), 0, markerPos);

        str.remove_prefix(markerPos);

        if (str.size() > 1)
        {
            if (str[0] == VariantMarker)
            {
                str.remove_prefix(1);

                while (!str.empty())
                {
                    markerPos = str.find_first_of(ReferenceMarkers);

                    if (markerPos == 0 && str[0] == ConditionMarker)
                    {
                        break;
                    }

                    if (markerPos == std::string_view::npos)
                    {
                        variants.insert(ExtractName(str.data(), 0, str.size()));
                        // don't remove prefix for condition marker check
                        break;
                    }

                    variants.insert(ExtractName(str.data(), 0, markerPos));

                    if (str[markerPos] != VariantMarker)
                    {
                        str.remove_prefix(markerPos);
                        break;
                    }

                    str.remove_prefix(markerPos + 1);
                }
            }

            if (str[0] == ConditionMarker)
            {
                str.remove_prefix(1);

                while (!str.empty())
                {
                    markerPos = str.find(ConditionMarker);

                    if (markerPos == std::string_view::npos)
                    {
                        conditions.insert(ExtractName(str.data(), 0, str.size()));
                        break;
                    }

                    conditions.insert(ExtractName(str.data(), 0, markerPos));

                    str.remove_prefix(markerPos + 1);
                }
            }

            weight = conditions.size() + (variants.empty() ? 0 : 100);
            isConditional = weight > 0;
        }
    }
    else
    {
        name = aReference;
    }
}

bool App::DynamicReference::Match(Red::CName aVariant) const
{
    return variants.contains(aVariant);
}

bool App::DynamicReference::Match(const DynamicTagList& aConditions) const
{
    for (const auto& condition : conditions)
    {
        if (!aConditions.contains(condition))
            return false;
    }

    return true;
}

App::DynamicAppearanceName App::DynamicAppearanceController::ParseAppearance(Red::CName aAppearance) const
{
    return DynamicAppearanceName(aAppearance);
}

App::DynamicReference App::DynamicAppearanceController::ParseReference(Red::CName aReference) const
{
    return DynamicReference(aReference);
}

bool App::DynamicAppearanceController::MatchReference(Red::Entity* aEntity, Red::CName aVariant,
                                                      const DynamicReference& aReference) const
{
    if (!aReference.variants.empty() )
    {
        if (!aReference.Match(aVariant))
            return false;
    }

    if (!aReference.conditions.empty())
    {
        const auto stateIt = m_states.find(aEntity);

        if (stateIt == m_states.end())
            return false;

        const auto& state = stateIt.value();

        if (!aReference.Match(state.conditions))
            return false;
    }

    return true;
}

Red::CName App::DynamicAppearanceController::ResolveName(Red::Entity* aEntity, const DynamicPartList& aVariant,
                                                         Red::CName aName) const
{
    const auto nameStr = aName.ToString();

    if (!IsDynamicValue(nameStr))
        return aName;

    const auto stateIt = m_states.find(aEntity);

    if (stateIt == m_states.end())
        return aName;

    const auto& state = stateIt.value();
    const auto result = ProcessString(state.values, aVariant, nameStr);

    if (!result.valid)
        return aName;

    return result.value.data();
}

Red::ResourcePath App::DynamicAppearanceController::ResolvePath(Red::Entity* aEntity, const DynamicPartList& aVariant,
                                                                Red::ResourcePath aPath) const
{
    const auto pathStr = GetPathStr(aPath);

    if (!IsDynamicValue(pathStr))
        return aPath;

    const auto stateIt = m_states.find(aEntity);

    if (stateIt == m_states.end())
        return aPath;

    const auto& state = stateIt.value();
    auto result = ProcessString(state.values, aVariant, pathStr.data());

    if (!result.valid)
        return aPath;

    Red::ResourcePath finalPath = result.value.data();

    if (result.attributes.contains(BodyTypeAttr))
    {
        if (!Red::ResourceDepot::Get()->ResourceExists(finalPath))
        {
            result = ProcessString(state.fallback, aVariant, pathStr.data());
            finalPath = result.value.data();
        }
    }

    return finalPath;
}

App::DynamicAppearanceController::DynamicString App::DynamicAppearanceController::ProcessString(
    const DynamicAttrList& aAttrs, const DynamicPartList& aVariant, const char* aInput) const
{
    constexpr auto MaxLength = 512;

    DynamicString result{};

    char buffer[MaxLength + 1];
    char* out = buffer;
    const char* max = buffer + MaxLength;
    const char* str = aInput;

    if (*str == DynamicValueMarker)
    {
        ++str;
    }

    while (str && *str)
    {
        auto* attrOpen = strchr(str, AttrOpen);

        if (!attrOpen)
        {
            if (out == buffer)
                return result;

            while (*str && out < max)
            {
                *out = *str;
                ++out;
                ++str;
            }
            break;
        }

        auto* attrClose = strchr(str, AttrClose);

        if (!attrClose)
        {
            return result;
        }

        while (str != attrOpen && out < max)
        {
            *out = *str;
            ++out;
            ++str;
        }

        if (out == max)
        {
            break;
        }

        str = attrClose + 1;

        const auto attr = Red::FNV1a64(reinterpret_cast<const uint8_t*>(attrOpen + 1), attrClose - attrOpen - 1);
        const char* value = nullptr;

        // Simple check based on the fact that there's no attribute starting with 'v'
        if (*(attrOpen + 1) == 'v')
        {
            const auto it = aVariant.find(attr);
            if (it != aVariant.end())
            {
                value = it.value().ToString();
            }
        }
        else
        {
            const auto it = aAttrs.find(attr);
            if (it != aAttrs.end())
            {
                value = it.value().value.data();
                result.attributes.insert(attr);
            }
        }

        if (value)
        {
            while (value && *value && out < max)
            {
                *out = *value;
                ++out;
                ++value;
            }

            if (out == max)
            {
                break;
            }
        }
    }

    *out = '\0';

    result.valid = true;
    result.value = buffer;

    return result;
}

void App::DynamicAppearanceController::UpdateState(Red::Entity* aEntity)
{
    auto& state = m_states[aEntity];

    state.values[GenderAttr] = GetSuffixData(aEntity, GenderSuffix);
    state.values[CameraAttr] = GetSuffixData(aEntity, CameraSuffix);
    state.values[BodyTypeAttr] = GetSuffixData(aEntity, BodyTypeSuffix);
    state.values[LegsStateAttr] = GetSuffixData(aEntity, LegsStateSuffix);
    state.values[SkinColorAttr] = GetSkinColorData(aEntity);
    state.values[HairColorAttr] = GetHairColorData(aEntity);

    state.fallback = state.values;
    state.fallback[BodyTypeAttr] = {DefaultBodyTypeAttrValue, DefaultBodyTypeSuffixValue};

    state.conditions.clear();
    for (const auto& [_, attribute] : state.values)
    {
        state.conditions.insert(attribute.suffix.data());
    }
}

void App::DynamicAppearanceController::RemoveState(Red::Entity* aEntity)
{
    m_states.erase(aEntity);
}

App::DynamicAppearanceController::AttributeData App::DynamicAppearanceController::GetAttributeData(
    Red::Entity* aEntity, Red::CName aAttribute) const
{
    switch (aAttribute)
    {
    case GenderAttr:
        return GetSuffixData(aEntity, GenderSuffix);
    case CameraAttr:
        return GetSuffixData(aEntity, CameraSuffix);
    case BodyTypeAttr:
        return GetSuffixData(aEntity, BodyTypeSuffix);
    case LegsStateAttr:
        return GetSuffixData(aEntity, LegsStateSuffix);
    case SkinColorAttr:
        return GetSkinColorData(aEntity);
    case HairColorAttr:
        return GetHairColorData(aEntity);
    }

    return {};
}

App::DynamicAppearanceController::AttributeData App::DynamicAppearanceController::GetSuffixData(
    Red::Entity* aEntity, Red::TweakDBID aSuffixID) const
{
    AttributeData data;

    {
        auto* handle = reinterpret_cast<Red::Handle<Red::GameObject>*>(&aEntity->ref);

        Red::CString suffixValue;
        Raw::AppearanceChanger::GetSuffixValue({}, 1ull, *handle, aSuffixID, suffixValue);

        data.suffix = suffixValue.c_str();
    }

    if (aSuffixID == GenderSuffix)
    {
        data.value = data.suffix == MaleSuffixValue ? MaleAttrValue : FemaleAttrValue;
    }
    else if (aSuffixID == BodyTypeSuffix && data.suffix.empty())
    {
        data.suffix = DefaultBodyTypeSuffixValue;
        data.value = DefaultBodyTypeAttrValue;
    }
    else
    {
        data.value = Str::SnakeCase(data.suffix);
    }

    return data;
}

App::DynamicAppearanceController::AttributeData App::DynamicAppearanceController::GetSkinColorData(
    Red::Entity* aEntity) const
{
    const auto& components = Raw::Entity::ComponentsStorage(aEntity)->components;
    for (const auto& component : components | std::views::reverse)
    {
        switch (component->name)
        {
        case MaleBodyComponent:
        case FemaleBodyComponent1:
        case FemaleBodyComponent2:
            const auto appearance = component.GetPtr<Red::ent::MorphTargetSkinnedMeshComponent>()->meshAppearance;
            if (appearance)
            {
                return {appearance.ToString() , ""};
            }
        }
    }

    return {};
}

App::DynamicAppearanceController::AttributeData App::DynamicAppearanceController::GetHairColorData(
    Red::Entity* aEntity) const
{
    static auto system = Red::GetGameSystem<Red::game::ui::ICharacterCustomizationSystem>();

    Red::CName hairColor;
    Raw::CharacterCustomizationHelper::GetHairColor(hairColor, system->ref, IsMale(aEntity));

    return {hairColor.ToString(), ""};
}

bool App::DynamicAppearanceController::IsMale(Red::Entity* aEntity) const
{
    for (const auto& component : Raw::Entity::GetComponents(aEntity) | std::views::reverse)
    {
        switch (component->name)
        {
        case MaleBodyComponent:
            return true;
        case FemaleBodyComponent1:
        case FemaleBodyComponent2:
            return false;
        }
    }

    return false;
}

bool App::DynamicAppearanceController::IsDynamicValue(const char* aValue) const
{
    return aValue && aValue[0] == DynamicValueMarker;
}

bool App::DynamicAppearanceController::IsDynamicValue(const std::string& aValue) const
{
    return aValue.starts_with(DynamicValueMarker);
}

bool App::DynamicAppearanceController::IsDynamicValue(const std::string_view& aValue) const
{
    return aValue.starts_with(DynamicValueMarker);
}

void App::DynamicAppearanceController::RegisterPath(Red::ResourcePath aPath, const char* aPathStr)
{
    m_paths.insert_or_assign(aPath, aPathStr);
}

void App::DynamicAppearanceController::RegisterPath(Red::ResourcePath aPath, const std::string& aPathStr)
{
    m_paths.insert_or_assign(aPath, aPathStr);
}

void App::DynamicAppearanceController::RegisterPath(Red::ResourcePath aPath, const std::string_view& aPathStr)
{
    m_paths.insert_or_assign(aPath, aPathStr);
}

const std::string& App::DynamicAppearanceController::GetPathStr(Red::ResourcePath aPath) const
{
    if (!aPath)
        return s_emptyPathStr;

    auto it = m_paths.find(aPath);

    if (it == m_paths.end())
        return s_emptyPathStr;

    return it.value();
}