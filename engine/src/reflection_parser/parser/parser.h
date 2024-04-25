#pragma once

#include "common/precompiled.h"

#include "common/namespace.h"
#include "common/schema_module.h"

#include "cursor/cursor.h"

#include "generator/generator.h"
#include "template_manager/template_manager.h"

#include <string>

class Class;

class ReflectionParser
{
public:
    static void Prepare(void);

    ReflectionParser(const std::string& header_desc_file_,
               const std::string& include_file_generate_path,
               const std::string& project_base_dir,
               const std::string& include_sys,
               const std::string& module_name,
               bool              is_show_errors);
    ~ReflectionParser(void);

    void Finish(void);

    // 解析include_file_generate_path
    int Parse(void);

    // 根据project_input_file生成包含全部项目头文件的头文件路径
    bool GenerateIncludeFiles(void);

    void GenerateFiles(void);

    std::string GetIncludeFile(const std::string& file_name);

private:
    void BuildAST(const CXCursor& cursor, Namespace& current_namespace);


private:
    std::vector<std::string> arguments_ =
        {"-x","c++", "-std=c++11", "-D__REFLECTION_PARSER__",
        "-DNDEBUG", "-D__clang__", "-w", "-MG", "-M", "-ferror-limit=0",
        "-o clangLog.txt"};

private:
    std::string header_desc_file_; // text file which contains all header file path
    std::string header_file_generate_path_;
    std::string project_base_dir_;
    std::string include_sys_;
    std::string module_name_;
    bool        is_show_errors_;

    CXIndex cursor_index_;
    CXCursor cursor;

    std::vector<std::string> work_paths_;

    std::unordered_map<std::string, SchemaMoudle> schema_modules_;
    std::vector<std::shared_ptr<Generator::GeneratorInterface> > generators_;
};
