#pragma once

#include "common/namespace.h"
#include "cursor/cursor_wapper.h"
#include "cursor/cursor_type.h"

namespace Utils
{

    void toString(const CXString& str, std::string& output);

    std::string GetQualifiedName(const CursorType& type);

    std::string GetQualifiedName(const std::string& display_name, const Namespace& current_namespace);

    std::string GetQualifiedName(const WapperCursor& cursor, const Namespace& current_namespace);

    std::string FormatQualifiedName(std::string& source_string);

    fs::path MakeRelativePath(const fs::path& from, const fs::path& to);

    void fatalError(const std::string& error);

    template<typename A, typename B>
    bool rangeEqual(A startA, A endA, B startB, B endB);

    std::vector<std::string> split(std::string input, std::string pat);

    std::string GetFileName(std::string path);

    std::string getNameWithoutFirstM(std::string& name);

    std::string getTypeNameWithoutNamespace(const CursorType& type);

    std::string GetTypeNameOfVector(std::string name);

    std::string getStringWithoutQuot(std::string input);

    std::string replace(std::string& source_string, std::string sub_string, const std::string new_string);

    std::string replace(std::string& source_string, char taget_char, const char new_char);

    std::string ToUpper(std::string& source_string);

    std::string join(std::vector<std::string> context_list, std::string separator);

    std::string trim(std::string& source_string, const std::string trim_chars);

    std::string loadFile(std::string path);

    void SaveFile(const std::string& outpu_string, const std::string& output_file);

    void ReplaceAll(std::string& resource_str, std::string sub_str, std::string new_str);

    unsigned long formatPathString(const std::string& path_string, std::string& out_string);

    std::string ConvertNameToUpperCamelCase(const std::string& name, std::string pat);

} // namespace Utils

#include "meta_utils.hpp"
