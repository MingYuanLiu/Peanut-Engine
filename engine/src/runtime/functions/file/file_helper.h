#pragma once

#include <filesystem>
#include <functional>
#include <string>

namespace peanut
{
    class FileHelper
    {
    public:
        static bool IsFileExist(const std::string& file_path);
        static bool IsDirectoryExist(const std::string& dir_path);
        static bool MakeDirectory(const std::string& dir_path);
        static bool DeleteDirectory(const std::string& dir_path);
        static bool RemoveFile(const std::string& file_path);
        static bool RenameFile(const std::string& old_file_path, const std::string& new_file_path);
        
        static std::vector<std::string> TranverseDirectory(const std::string& dir_path, bool is_recursive = true, std::function<void(const std::string&, bool)> callback = nullptr);
        static std::string GetExtension(const std::string& file_path);
        static std::string GetFileName(const std::string& file_path);
        static std::string GetBaseNameWithoutExtension(const std::string& file_path);
        static std::string CombinePath(const std::string& path1, const std::string& path2);

        static std::string ReadFile(const std::string& file_path);
        static bool ReadFileBinary(const std::string& file_path, std::vector<char>& contents);
        static bool WriteFile(const std::string& file_path, const std::string& content);
        static std::string GetFileModifiedTime(const std::string& file_path);

        static std::string GetCurrentWorkDir();
    };
}

