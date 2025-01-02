#include "Extension.hpp"
#include "App/Extensions/ResourceMeta/Extension.hpp"
#include "Red/TweakDB.hpp"

namespace
{
constexpr auto ExtensionName = "PhotoMode";

constexpr uint32_t CharacterSelectorAttribute = 63;
constexpr uint32_t DummyCharacterSlots = 1;

constexpr Red::TweakDBID WomanAverageID = "Character.Panam_Puppet_Photomode";
constexpr Red::TweakDBID ManAverageID = "Character.Viktor_Puppet_Photomode";
constexpr Red::TweakDBID ManBigID = "Character.Jackie_Puppet_Photomode";
constexpr Red::TweakDBID ManMassiveID = "Character.AdamSmasher_Puppet_Photomode";
constexpr Red::TweakDBID CatID = "Character.Nibbles_Puppet_Photomode";

constexpr Red::CName ManAverageTag = "ManAverage";
constexpr Red::CName ManBigTag = "ManBig";
constexpr Red::CName ManMassiveTag = "ManMassive";
constexpr Red::CName CatTag = "Cat";
}

std::string_view App::PhotoModeExtension::GetName()
{
    return ExtensionName;
}

bool App::PhotoModeExtension::Load()
{
    HookAfter<Raw::PhotoModeSystem::Activate>(&OnActivatePhotoMode).OrThrow();
    Hook<Raw::PhotoModeSystem::ValidateCharacter>(&OnValidateCharacter).OrThrow();
    Hook<Raw::PhotoModeSystem::RegisterPoses>(&OnRegisterPoses).OrThrow();
    Hook<Raw::PhotoModeSystem::RegisterWeaponPoses>(&OnRegisterWeaponPoses).OrThrow();
    Hook<Raw::PhotoModeSystem::PrepareCategories>(&OnPrepareCategories).OrThrow();
    Hook<Raw::PhotoModeSystem::PreparePoses>(&OnPreparePoses).OrThrow();
    Hook<Raw::PhotoModeSystem::PrepareCameras>(&OnPrepareCameras).OrThrow();
    Hook<Raw::PhotoModeSystem::UpdatePoseDependents>(&OnUpdatePoseDependents).OrThrow();
    HookBefore<Raw::PhotoModeMenuController::SetupGridSelector>(&OnSetupGridSelector).OrThrow();
    Hook<Raw::PhotoModeMenuController::SetNpcImageCallback>(&OnSetNpcImage).OrThrow();

    return true;
}

bool App::PhotoModeExtension::Unload()
{
    Unhook<Raw::PhotoModeSystem::Activate>();
    Unhook<Raw::PhotoModeSystem::ValidateCharacter>();
    Unhook<Raw::PhotoModeSystem::RegisterPoses>();
    Unhook<Raw::PhotoModeSystem::RegisterWeaponPoses>();
    Unhook<Raw::PhotoModeSystem::PrepareCategories>();
    Unhook<Raw::PhotoModeSystem::PreparePoses>();
    Unhook<Raw::PhotoModeSystem::PrepareCameras>();
    Unhook<Raw::PhotoModeSystem::UpdatePoseDependents>();
    Unhook<Raw::PhotoModeMenuController::SetupGridSelector>();
    Unhook<Raw::PhotoModeMenuController::SetNpcImageCallback>();

    return true;
}

void App::PhotoModeExtension::Configure()
{
}

void App::PhotoModeExtension::ApplyTweaks()
{
    auto iconList = Red::GetFlat<Red::DynArray<Red::CString>>("photo_mode.npcs.npcRecordID");
    auto puppetList = Red::GetFlat<Red::DynArray<Red::CString>>("photo_mode.general.recordIDForPhotoModePuppet");
    auto nameList = Red::GetFlat<Red::DynArray<Red::CString>>("photo_mode.general.localizedNameForPhotoModePuppet");
    auto factList = Red::GetFlat<Red::DynArray<Red::CString>>("photo_mode.general.questFactIDForPhotoModePuppet");
    auto enabledList = Red::GetFlat<Red::DynArray<bool>>("photo_mode.general.enabledPhotoModePuppet");
    auto customizationList = Red::GetFlat<Red::DynArray<bool>>("photo_mode.general.customizablePhotoModePuppet");
    auto expressionList = Red::GetFlat<Red::DynArray<bool>>("photo_mode.general.expressionChangingPhotoModePuppet");
    auto collisionRadiusList = Red::GetFlat<Red::DynArray<float>>("photo_mode.general.collisionRadiusForPhotoModePuppet");
    auto collisionHeightList = Red::GetFlat<Red::DynArray<float>>("photo_mode.general.collisionHeightForPhotoModePuppet");

    for (uint32_t characterIndex = 0; characterIndex < puppetList.size; ++characterIndex)
    {
        s_characterIndexMap[puppetList[characterIndex].c_str()] = characterIndex;
    }

    Core::SortedMap<std::string, Red::Handle<Red::gamedataCharacter_Record>> addedCharacterRecords;

    for (const auto& characterRecord : Red::GetRecords<Red::gamedataCharacter_Record>())
    {
        auto characterID = characterRecord->recordID;
        auto persistentName = Red::GetFlat<Red::CName>({characterID, ".persistentName"});

        if (persistentName != "PhotomodePuppet")
            continue;

        if (s_characterIndexMap.contains(characterID))
            continue;

        auto characterStr = Red::ToStringDebug(characterID);

        addedCharacterRecords[characterStr.c_str()] = characterRecord;
    }

    if (addedCharacterRecords.empty())
        return;

    {
        s_dummyCharacterIndex = puppetList.size;

        auto dummyName = "PhotoModeNpcs.DoNotTouch";
        Red::CreateRecord(dummyName, "PhotoModeSticker");

        for (auto i = DummyCharacterSlots; i > 0; --i)
        {
            iconList.PushBack(dummyName);
            puppetList.PushBack("");
            nameList.PushBack("");
            factList.PushBack("");
            enabledList.PushBack(false);
            customizationList.PushBack(false);
            expressionList.PushBack(false);
            collisionRadiusList.PushBack(0.0);
            collisionHeightList.PushBack(0.0);
        }
    }

    for (const auto& [characterStr, characterRecord] : addedCharacterRecords)
    {
        auto characterID = characterRecord->recordID;
        auto persistentName = Red::GetFlat<Red::CName>({characterID, ".persistentName"});

        auto characterIndex = puppetList.size;
        auto characterSource = s_characterIndexMap[WomanAverageID];
        auto characterType = Red::PhotoModeCharacterType::NPC;

        auto visualTags = Red::GetFlat<Red::DynArray<Red::TweakDBID>>({characterID, ".visualTags"});
        for (const auto& visualTag : visualTags)
        {
            if (visualTag == ManAverageTag)
            {
                characterSource = s_characterIndexMap[ManAverageID];
                break;
            }
            if (visualTag == ManBigTag)
            {
                characterSource = s_characterIndexMap[ManBigID];
                break;
            }
            if (visualTag == ManMassiveTag)
            {
                characterSource = s_characterIndexMap[ManMassiveID];
                break;
            }
            if (visualTag == CatTag)
            {
                characterSource = s_characterIndexMap[CatID];
                break;
            }
        }

        s_extraCharacters[characterIndex] = {characterSource, characterType};
        s_characterIndexMap[characterID] = characterIndex;

        auto displayName = Red::GetFlat<Red::gamedataLocKeyWrapper>({characterID, ".displayName"});
        auto nameStr = std::string{"LocKey#"} + std::to_string(displayName.primaryKey);
        auto iconStr = characterStr + ".icon";

        iconList.PushBack(iconStr);
        puppetList.PushBack(characterStr);
        nameList.PushBack(nameStr);
        factList.PushBack("");
        enabledList.PushBack(true);
        customizationList.PushBack(false);
        expressionList.PushBack(true);
        collisionRadiusList.PushBack(0.35);
        collisionHeightList.PushBack(1.2);
    }

    Red::SetFlat("photo_mode.npcs.npcRecordID", iconList);
    Red::SetFlat("photo_mode.general.recordIDForPhotoModePuppet", puppetList);
    Red::SetFlat("photo_mode.general.localizedNameForPhotoModePuppet", nameList);
    Red::SetFlat("photo_mode.general.questFactIDForPhotoModePuppet", factList);
    Red::SetFlat("photo_mode.general.enabledPhotoModePuppet", enabledList);
    Red::SetFlat("photo_mode.general.customizablePhotoModePuppet", customizationList);
    Red::SetFlat("photo_mode.general.expressionChangingPhotoModePuppet", expressionList);
    Red::SetFlat("photo_mode.general.collisionRadiusForPhotoModePuppet", collisionRadiusList);
    Red::SetFlat("photo_mode.general.collisionHeightForPhotoModePuppet", collisionHeightList);
}

void App::PhotoModeExtension::OnActivatePhotoMode(Red::gamePhotoModeSystem* aSystem)
{
    auto& characterList = Raw::PhotoModeSystem::CharacterList::Ref(aSystem);

    if (!characterList.size)
        return;

    for (const auto& [characterIndex, characterSource] : s_extraCharacters)
    {
        Red::DynArray<Red::gamedataItemType> itemTypes;
        Red::DynArray<Red::Handle<Red::gameItemObject>> clothingItems;

        FillWeaponTypes(itemTypes);

        Raw::PhotoModeSystem::RegisterCharacter(aSystem, characterIndex, characterList,
                                                Red::PhotoModeCharacterType::NPC, itemTypes, clothingItems);
    }
}

bool App::PhotoModeExtension::OnValidateCharacter(Red::gamePhotoModeSystem* aSystem, uint32_t aCharacterIndex)
{
    auto extracCharacter = s_extraCharacters.find(aCharacterIndex);
    if (extracCharacter != s_extraCharacters.end())
    {
        aCharacterIndex = extracCharacter->second.index;
    }

    return Raw::PhotoModeSystem::ValidateCharacter(aSystem, aCharacterIndex);
}

void App::PhotoModeExtension::OnRegisterPoses(Red::gamePhotoModeSystem* aSystem, uint32_t aCharacterIndex,
                                              Red::PhotoModeCharacterType aCharacterType)
{
    if (!s_extraCharacters.contains(aCharacterIndex))
    {
        Raw::PhotoModeSystem::RegisterPoses(aSystem, aCharacterIndex, aCharacterType);
    }
}

void App::PhotoModeExtension::OnRegisterWeaponPoses(Red::gamePhotoModeSystem* aSystem, uint32_t aCharacterIndex,
                                                    Red::DynArray<Red::gamedataItemType>& aItemTypes)
{
    auto extracCharacter = s_extraCharacters.find(aCharacterIndex);
    if (extracCharacter != s_extraCharacters.end())
    {
        aCharacterIndex = extracCharacter->second.index;
    }

    if (aItemTypes.size == 0)
    {
        FillWeaponTypes(aItemTypes);
    }

    Raw::PhotoModeSystem::RegisterWeaponPoses(aSystem, aCharacterIndex, aItemTypes);
}

void App::PhotoModeExtension::OnPrepareCategories(Red::gamePhotoModeSystem* aSystem, uint32_t aCharacterIndex,
                                                  uint64_t a3)
{
    auto extracCharacter = s_extraCharacters.find(aCharacterIndex);
    if (extracCharacter != s_extraCharacters.end())
    {
        aCharacterIndex = extracCharacter->second.index;
    }

    Raw::PhotoModeSystem::PrepareCategories(aSystem, aCharacterIndex, a3);
}

void App::PhotoModeExtension::OnPreparePoses(Red::gamePhotoModeSystem* aSystem, uint32_t aCharacterIndex,
                                             uint32_t aCategoryIndex, uint64_t a4)
{
    auto extracCharacter = s_extraCharacters.find(aCharacterIndex);
    if (extracCharacter != s_extraCharacters.end())
    {
        aCharacterIndex = extracCharacter->second.index;
    }

    Raw::PhotoModeSystem::PreparePoses(aSystem, aCharacterIndex, aCategoryIndex, a4);
}

void App::PhotoModeExtension::OnPrepareCameras(Red::gamePhotoModeSystem* aSystem, uint32_t aCharacterIndex,
                                               uint32_t* a3, uint32_t* a4)
{
    auto extracCharacter = s_extraCharacters.find(aCharacterIndex);
    if (extracCharacter != s_extraCharacters.end())
    {
        aCharacterIndex = extracCharacter->second.index;
    }

    Raw::PhotoModeSystem::PrepareCameras(aSystem, aCharacterIndex, a3, a4);
}

void App::PhotoModeExtension::OnUpdatePoseDependents(Red::gamePhotoModeSystem* aSystem, uint32_t aCategoryIndex,
                                                     uint32_t aPoseIndex, Red::PhotoModeCharacter* aCharacter)
{
    auto extracCharacter = s_extraCharacters.find(aCharacter->characterIndex);
    if (extracCharacter != s_extraCharacters.end())
    {
        aCharacter->characterIndex = extracCharacter->second.index;
    }

    Raw::PhotoModeSystem::UpdatePoseDependents(aSystem, aCategoryIndex, aPoseIndex, aCharacter);

    if (extracCharacter != s_extraCharacters.end())
    {
        aCharacter->characterIndex = extracCharacter->first;
    }
}

void App::PhotoModeExtension::OnSetupGridSelector(Red::gameuiPhotoModeMenuController* aController,
                                                  Red::CName aEventName, uint8_t& a3, uint32_t& aAttribute,
                                                  Red::DynArray<Red::gameuiPhotoModeOptionGridButtonData>& aGridData,
                                                  uint32_t& aElementsCount, uint32_t& aElementsInRow)
{
    if (aAttribute == CharacterSelectorAttribute && s_dummyCharacterIndex && aGridData.size >= s_dummyCharacterIndex)
    {
        aElementsCount -= DummyCharacterSlots;

        for (auto i = DummyCharacterSlots; i > 0; --i)
        {
            aGridData.RemoveAt(s_dummyCharacterIndex - 1);
        }
    }
}

void App::PhotoModeExtension::OnSetNpcImage(void* aCallback, uint32_t aCharacterIndex, Red::ResourcePath aAtlasPath,
                                            Red::CName aImagePart, int32_t aImageIndex)
{
    if (aImageIndex >= s_dummyCharacterIndex)
    {
        aImageIndex -= DummyCharacterSlots;
    }

    Raw::PhotoModeMenuController::SetNpcImageCallback(aCallback, aCharacterIndex, aAtlasPath, aImagePart, aImageIndex);
}

void App::PhotoModeExtension::FillWeaponTypes(Red::DynArray<Red::gamedataItemType>& aItemTypes)
{
    auto itemTypeCount = static_cast<uint32_t>(Red::gamedataItemType::Count);
    aItemTypes.Reserve(itemTypeCount);

    for (auto itemType = 0; itemType < itemTypeCount; ++itemType)
    {
        aItemTypes.PushBack(static_cast<Red::gamedataItemType>(itemType));
    }
}
