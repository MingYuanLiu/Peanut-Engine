#include "file_helper.h"
#include <filesystem>
#include <fstream>

#include "core/base/logger.h"
#include "core/log/peanut_log.h"


namespace peanut
{
    bool FileHelper::IsFileExist(const std::string& file_path)
    {
        return std::filesystem::is_regular_file(file_path) && std::filesystem::exists(file_path);
    }

    bool FileHelper::IsDirectoryExist(const std::string& dir_path)
    {
        return std::filesystem::is_directory(dir_path) && std::filesystem::exists(dir_path);
    }
    
    std::string FileHelper::ReadFile(const std::string& file_path)
    {
        std::ifstream file(file_path, std::ios::binary);
        if (!file.is_open())
        {
            PEANUT_LOG_WARN("file {0} not exist", file_path);
            return "";
        }
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        return content;
    }

    bool FileHelper::WriteFile(const std::string& file_path, const std::string& content)
    {
        std::ofstream file(file_path, std::ios::binary);
        if (!file.is_open())
        {
            PEANUT_LOG_WARN("file {0} not exist", file_path);
            return false;
        }
        
        file.write(content.c_str(), static_cast<std::streamsize>(content.size()));
        file.close();
        return true;
    }

    bool FileHelper::ReadFileBinary(const std::string& file_path, std::vector<char>& contents)
    {
        std::ifstream file(file_path, std::ios::binary | std::ios::ate);
        if (!file.is_open())
        {
            PEANUT_LOG_WARN("file {0} not exist", file_path);
            return false;
        }
        
        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);
        contents.resize(size);
        file.read(contents.data(), static_cast<std::streamsize>(size));
        file.close();
        return true;
    }

    std::string FileHelper::CombinePath(const std::string& path1, const std::string& path2)
    {
        std::filesystem::path path_combined(path1);
        path_combined.append(path2);
        return path_combined.generic_string();
    }

    bool FileHelper::MakeDirectory(const std::string& dir_path)
    {
        return std::filesystem::create_directories(dir_path);
    }

    bool FileHelper::DeleteDirectory(const std::string& dir_path)
    {
        return std::filesystem::remove_all(dir_path);
    }

    std::string FileHelper::GetExtension(const std::string& file_path)
    {
        return std::filesystem::path(file_path).extension().generic_string();
    }

    std::string FileHelper::GetFileName(const std::string& file_path)
    {
        return std::filesystem::path(file_path).filename().generic_string();
    }

    std::string FileHelper::GetBaseNameWithoutExtension(const std::string& file_path)
    {
        return std::filesystem::path(file_path).stem().generic_string();
    }

    std::string FileHelper::GetFileModifiedTime(const std::string& file_path)
    {
        auto time = std::filesystem::last_write_time(file_path).time_since_epoch();
        return std::to_string(std::chrono::duration_cast<std::chrono::seconds>(time).count());
    }

    bool FileHelper::RemoveFile(const std::string& file_path)
    {
        return std::filesystem::remove(file_path);
    }

    bool FileHelper::RenameFile(const std::string& old_file_path, const std::string& new_file_path)
    {
        std::filesystem::rename(old_file_path, new_file_path);
        return true;
    }

    std::vector<std::string> FileHelper::TranverseDirectory(const std::string& dir_path, bool is_recursive, std::function<void(const std::string&, bool)> callback)
    {
        std::vector<std::string> file_list;
        if (!IsDirectoryExist(dir_path))
        {
            PEANUT_LOG_INFO("directory {0} not exist", dir_path);
            return file_list;
        }

        if (is_recursive)
        {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(dir_path))
            {
                if (!entry.is_directory())
                {
                    file_list.push_back(entry.path().generic_string());
                }
                
                if (callback)
                {
                    callback(entry.path().generic_string(), entry.is_directory());
                }
            }
        }
        else
        {
            for (const auto& entry : std::filesystem::directory_iterator(dir_path))
            {
                file_list.push_back(entry.path().generic_string());
                
                if (callback)
                {
                    callback(entry.path().generic_string(), entry.is_directory());
                }
            }
        }
        
        return file_list;
    }

    std::string FileHelper::GetCurrentWorkDir()
    {
        std::filesystem::path work_dir = std::filesystem::current_path();
        return work_dir.generic_string();
    }

}   