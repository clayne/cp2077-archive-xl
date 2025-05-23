#pragma once

#include "App/Extensions/ExtensionBase.hpp"
#include "Red/AppearanceChanger.hpp"

namespace App
{
class TransmogExtension : public Extension
{
public:
    bool Load() override;
    bool Unload() override;
    std::string_view GetName() override;

private:
    static bool OnLoadTemplate(Red::ItemFactoryAppearanceChangeRequest* aRequest);
    static void* OnSelectAppearance(Red::CName* aOut,
                                    const Red::Handle<Red::TweakDBRecord>& aItemRecord,
                                    const Red::ItemID& aItemID,
                                    const Red::Handle<Red::AppearanceResource>& aAppearanceResource,
                                    uint64_t a5,
                                    Red::CName aAppearanceName);
};
}
