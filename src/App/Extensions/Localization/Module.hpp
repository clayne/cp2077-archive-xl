#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "App/Extensions/Localization/Unit.hpp"

namespace App
{
using TextResource = Red::localization::PersistenceOnScreenEntries;
using TextEntry = Red::localization::PersistenceOnScreenEntry;
using TextEntryList = Red::DynArray<TextEntry>;
using TextEntryMap = Core::Map<uint64_t, uint32_t>;

using SubtitleResource = Red::localization::PersistenceSubtitleMap;
using SubtitleEntry = Red::localization::PersistenceSubtitleMapEntry;
using SubtitleEntryList = Red::DynArray<SubtitleEntry>;

class LocalizationModule : public ConfigurableUnitModule<LocalizationUnit>
{
public:
    std::string_view GetName() override;
    bool Load() override;
    bool Unload() override;
    void Configure() override;

private:
    void OnLoadTexts(Red::Handle<TextResource>& aOnScreens, Red::ResourcePath aPath);
    void OnLoadSubtitles(Red::Handle<SubtitleResource>& aSubtitles, Red::ResourcePath aPath);
    void OnLoadVoiceOvers(void* aContext, uint64_t a2);
    void OnLoadLipsyncs(void* aContext, uint8_t a2);

    static bool MergeTextResource(const std::string& aPath, TextEntryList& aFinalList, TextEntryMap& aUsedKeyMap,
                                  uint32_t aOriginalCount, uint64_t aOriginalMaxKey, bool aFallback);
    static void MergeTextEntry(TextEntryList& aFinalList, TextEntry& aNewEntry, uint32_t aIndex,
                               TextEntryMap& aUsedKeyMap, uint32_t aOriginalCount, uint64_t aOriginalMaxKey,
                               bool aFallback, bool aExtraEntry = false);
    static TextEntry* FindSameTextEntry(TextEntry& aEntry, TextEntryList& aList, uint32_t aCount);
    static bool IsCommentEntry(TextEntry& aEntry);
    static bool MergeSubtitleResource(const std::string& aPath, SubtitleEntryList& aFinalList);
    static bool MergeLipsyncResource(const Red::Handle<Red::animLipsyncMapping>& aSource,
                                     Red::Handle<Red::animLipsyncMapping>& aTarget);

    static inline Red::ResourceToken<Red::animLipsyncMapping>* s_currentLipMap;
    static inline Core::Map<Red::ResourcePath, std::string> s_paths;
};
}
