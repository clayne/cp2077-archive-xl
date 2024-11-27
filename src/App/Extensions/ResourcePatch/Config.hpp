#pragma once

#include "App/Extensions/ExtensionBase.hpp"

namespace App
{
struct ResourcePatchScope
{
    Core::Set<Red::ResourcePath> includes;
    Core::Set<Red::ResourcePath> excludes;
};

struct ResourcePatchConfig : ExtensionConfig
{
    using ExtensionConfig::ExtensionConfig;

    bool IsDefined() override;
    void LoadYAML(const YAML::Node& aNode) override;

    Core::Map<Red::ResourcePath, ResourcePatchScope> patches;
    Core::Map<Red::ResourcePath, std::string> paths;
};
}
