#pragma once

namespace NativeProperty
{
    const auto All = "All";

    const auto Fields = "Fields";
    const auto Methods = "Methods";

    const auto Enable  = "Enable";
    const auto Disable = "Disable";

    const auto WhiteListFields = "WhiteListFields";
    const auto WhiteListMethods = "WhiteListMethods";

} // namespace NativeProperty

namespace MetaFlag
{
    const std::string ClassEnable = "meta_class";

    const std::string StructEnable = "meta_struct";

    const std::string PropertyEnable = "meta_property";

    const std::string FunctionEnable = "meta_function";

    const std::string EnumEnable = "meta_enum";
}
