#include "parser.h"

#include "generator/reflection_generator.h"

void ReflectionParser::Prepare(void)
{
}

ReflectionParser::ReflectionParser(
    const std::string& header_desc_file,
    const std::string& header_file_generate_path,
    const std::string& project_base_dir,
    const std::string& include_sys,
    const std::string& module_name,
    bool              is_show_errors
)
    : header_desc_file_(header_desc_file)
    , header_file_generate_path_(header_file_generate_path)
    , project_base_dir_(project_base_dir)
    , include_sys_(include_sys)
    , module_name_(module_name)
    , is_show_errors_(is_show_errors)
{
    work_paths_ = Utils::split(project_base_dir_, ",");
    generators_.emplace_back(std::make_shared<Generator::ReflectionGenerator>(project_base_dir_,
        std::bind(&ReflectionParser::GetIncludeFile, this, std::placeholders::_1)));

}

ReflectionParser::~ReflectionParser(void)
{
    
}

void ReflectionParser::Finish(void)
{
    
}

int ReflectionParser::Parse(void)
{
    if (!GenerateIncludeFiles())
    {
        return -1;
    }
    
     // include system header file in argument
    std::vector<const char*> temp_arguments;
    std::string include_flag = "-I";
    if (project_base_dir_ != "*")
    {
        std::string include_path = include_flag + project_base_dir_;
        arguments_.emplace_back(std::move(include_path));
    }

    for (std::string& path : work_paths_)
    {
        std::string include_path = include_flag + path;
        arguments_.emplace_back(std::move(include_path));
    }

    fs::path generated_include_file(header_file_generate_path_);
    if (!fs::exists(generated_include_file))
    {
        std::cerr << "Parse header file not exist: " << std::endl;
        return -1;
    }

    // get clang translate unit
    int show_error = is_show_errors_ ? 1 : 0;
    cursor_index_ = clang_createIndex(0, show_error);

    std::vector<const char*> translate_arguments;
    for (const std::string& arg : arguments_)
    {
        translate_arguments.push_back(arg.c_str());
    }

    // gather ast
    CXTranslationUnit unit = clang_createTranslationUnitFromSourceFile(cursor_index_, header_file_generate_path_.c_str(),
        static_cast<int>(translate_arguments.size()), translate_arguments.data(), 0, nullptr);
    cursor = clang_getTranslationUnitCursor(unit);

    Namespace tmp_namespace;
    BuildAST(cursor, tmp_namespace);
    
	return 0;
}

void ReflectionParser::GenerateFiles()
{
    
}


bool ReflectionParser::GenerateIncludeFiles(void)
{
    // read header description files
    std::ifstream desc_file_read_stream(header_desc_file_.c_str(), std::ios::in);
    if (desc_file_read_stream.fail())
    {
        std::cerr << "Can not read project description file: " << header_desc_file_ << std::endl;
        return false;
    }

    std::stringstream desc_file_context;
    desc_file_context << desc_file_read_stream.rdbuf();
    std::string desc_file_content(desc_file_context.str());

    std::vector<std::string> header_files = Utils::split(desc_file_content, ";");

    // generate parse_all_headers.h
    std::ofstream parse_header_out_stream(header_file_generate_path_.c_str(), std::ios::out);
    if (!parse_header_out_stream.is_open())
    {
        std::cerr << "Failed to open header generated file: " << header_file_generate_path_ << std::endl;
        return false;
    }

    std::string generated_header_file_name = Utils::GetFileName(header_file_generate_path_);
    if (generated_header_file_name.empty())
    {
        generated_header_file_name = "REFLECTION_HEADER_PARSE_H";
    }
    else
    {
        Utils::ReplaceAll(generated_header_file_name, ".", "_");
        Utils::ReplaceAll(generated_header_file_name, " ", "_");
        Utils::ToUpper(generated_header_file_name);
    }

    parse_header_out_stream << "#ifndef __" << generated_header_file_name << "__ \n";
    parse_header_out_stream << "#define __" << generated_header_file_name << "-- \n";
    parse_header_out_stream << "\n";

    // write include files
    for (const std::string& header_file : header_files)
    {
        std::string tmp(header_file);
        Utils::replace(tmp, "\\", "/");
        parse_header_out_stream << "#include  \"" << header_file << "\" \n";
    }

    parse_header_out_stream << "\n";
    parse_header_out_stream << "#endif";

    parse_header_out_stream.flush();
    desc_file_read_stream.close();

    return true;
}

void ReflectionParser::BuildAST(const CXCursor& cursor, Namespace& current_namespace)
{
    
}




