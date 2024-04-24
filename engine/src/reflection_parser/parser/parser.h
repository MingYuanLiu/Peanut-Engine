#pragma once

#include "common/precompiled.h"

#include "common/namespace.h"
#include "common/schema_module.h"

#include "cursor/cursor.h"

#include "generator/generator.h"
#include "template_manager/template_manager.h"

#include <string>

class Class;

class MetaParser
{
public:
    static void Prepare(void);

    MetaParser(const std::string project_input_file,
               const std::string include_file_generate_path,
               const std::string project_base_dir,
               const std::string include_sys,
               const std::string module_name,
               bool              is_show_errors);
    ~MetaParser(void);

    void Finish(void);

    // ����include_file_generate_path
    int  Parse(void);

    // ����project_input_file���ɰ�����Ŀȫ��ͷ�ļ���ͷ�ļ�·��
    void GenerateFiles(void);

private:
    void BuildAST(void);


private:
    static std::vector<const char*> arguments_;

private:
    std::string project_input_file_;
    std::string include_file_generate_path;
    std::string project_base_dir_;
    std::string include_sys_;
    std::string module_name_;
    bool        is_show_errors_;

    std::unordered_map<std::string, SchemaMoudle> schema_modules_;
    std::unordered_map<std::string, std::shared_ptr<Generator::GeneratorInterface> > generators_;
};
