#pragma once

#include "App/Extensions/ExtensionBase.hpp"

namespace App
{
struct ResourceFix
{
    [[nodiscard]] inline bool DefinesNameMappings() const
    {
        return !names.empty();
    }

    [[nodiscard]] inline Red::CName GetMappedName(Red::CName aName) const
    {
        const auto& it = names.find(aName);
        return it != names.end() ? it.value() : aName;
    }

    [[nodiscard]] inline bool DefinesContext() const
    {
        return !context.empty();
    }

    [[nodiscard]] inline const Core::Map<Red::CName, std::string>& GetContext() const
    {
        return context;
    }

    Core::Map<Red::CName, Red::CName> names;
    Core::Map<Red::CName, std::string> context;
};

struct ResourceMetaConfig : ExtensionConfig
{
    using ExtensionConfig::ExtensionConfig;

    bool IsDefined() override;
    void LoadYAML(const YAML::Node& aNode) override;
    void LoadScopes(const YAML::Node& aNode);
    void LoadFixes(const YAML::Node& aNode);

    Core::Map<Red::ResourcePath, Core::Set<Red::ResourcePath>> scopes;
    Core::Map<Red::ResourcePath, ResourceFix> fixes;
    Core::Map<Red::ResourcePath, std::string> paths;
};
}
