#include "Extension.hpp"
#include "App/Extensions/ResourceMeta/Extension.hpp"
#include "Red/TweakDB.hpp"

namespace
{
constexpr auto ExtensionName = "PhotoMode";

constexpr uint32_t CharacterSelectorAttribute = 63;
constexpr uint32_t DummyCharacterSlots = 2;

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
    Hook<Raw::PhotoModeSystem::CollectPoses>(&OnCollectPoses).OrThrow();
    Hook<Raw::PhotoModeSystem::PreparePoses>(&OnPreparePoses).OrThrow();
    HookBefore<Raw::PhotoModeMenuController::SetupGridSelector>(&OnSetupGridSelector).OrThrow();
    Hook<Raw::PhotoModeMenuController::SetNpcImageCallback>(&OnSetNpcImage).OrThrow();

    return true;
}

bool App::PhotoModeExtension::Unload()
{
    Unhook<Raw::PhotoModeSystem::Activate>();
    Unhook<Raw::PhotoModeSystem::CollectPoses>();
    Unhook<Raw::PhotoModeSystem::PreparePoses>();
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

    for (const auto& character : Red::GetRecords<Red::gamedataCharacter_Record>())
    {
        auto characterID = character->recordID;
        auto persistentName = Red::GetFlat<Red::CName>({characterID, ".persistentName"});

        if (persistentName != "PhotomodePuppet")
            continue;

        {
            auto exists = false;

            for (const auto& puppetID : puppetList)
            {
                if (characterID == puppetID.c_str())
                {
                    exists = true;
                    break;
                }
            }

            if (exists)
                continue;
        }

        if (!s_dummyCharacterIndex)
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

        {
            uint32_t characterIndex = puppetList.size;
            uint32_t characterSource = 0;
            uint32_t characterType = 1;

            auto visualTags = Red::GetFlat<Red::DynArray<Red::TweakDBID>>({characterID, ".visualTags"});
            for (const auto& visualTag : visualTags)
            {
                if (visualTag == ManAverageTag)
                {
                    characterType = 2;
                    break;
                }
                if (visualTag == ManBigTag)
                {
                    characterSource = 8;
                    characterType = 4;
                    break;
                }
                if (visualTag == ManMassiveTag)
                {
                    characterSource = 9;
                    characterType = 4;
                    break;
                }
                if (visualTag == CatTag)
                {
                    characterSource = 22;
                    characterType = 4;
                    break;
                }
            }

            s_extraCharacters[characterIndex] = {characterSource, characterType};
        }

        auto characterStr = Red::ToStringDebug(characterID);
        auto iconStr = std::string{characterStr.c_str()} + ".icon";

        auto displayName = Red::GetFlat<Red::gamedataLocKeyWrapper>({characterID, ".displayName"});
        auto nameStr = std::string{"LocKey#"} + std::to_string(displayName.primaryKey);

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

    for (const auto& [characterIndex, characterSource] : s_extraCharacters)
    {
        Red::DynArray<void*> placeholder;
        Raw::PhotoModeSystem::AddCharacter(aSystem, characterSource.index, characterList, characterSource.type,
                                           placeholder, placeholder);

        characterList.Back().characterIndex = characterIndex;
    }
}

void App::PhotoModeExtension::OnCollectPoses(Red::gamePhotoModeSystem* aSystem, uint32_t aCharacterIndex, uint32_t a3,
                                             uint64_t a4)
{
    auto extracCharacter = s_extraCharacters.find(aCharacterIndex);
    if (extracCharacter != s_extraCharacters.end())
    {
        aCharacterIndex = extracCharacter->second.index;
    }

    Raw::PhotoModeSystem::CollectPoses(aSystem, aCharacterIndex, a3, a4);
}

void App::PhotoModeExtension::OnPreparePoses(Red::gamePhotoModeSystem* aSystem, uint32_t aCharacterIndex, uint32_t* a3,
                                             uint32_t* a4)
{
    auto extracCharacter = s_extraCharacters.find(aCharacterIndex);
    if (extracCharacter != s_extraCharacters.end())
    {
        aCharacterIndex = extracCharacter->second.index;
    }

    Raw::PhotoModeSystem::PreparePoses(aSystem, aCharacterIndex, a3, a4);
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
