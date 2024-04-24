#include "parser.h"

void MetaParser::Prepare(void)
{
    arguments_ = {"-x","c++", "-std=c++11", "-D__REFLECTION_PARSER__",
                  "-DNDEBUG", "-D__clang__", "-w", "-MG", "-M", "-ferror-limit=0",
                  "-o clangLog.txt"};
}

MetaParser::MetaParser(
    const std::string project_input_file,
    const std::string include_file_generate_path,
    const std::string project_base_dir,
    const std::string include_sys,
    const std::string module_name,
    bool              is_show_errors
)
    : project_input_file_(project_input_file)
    , include_file_generate_path(include_file_generate_path)
    , project_base_dir_(project_base_dir)
    , include_sys_(include_sys)
    , module_name_(module_name)
    , is_show_errors_(is_show_errors)
{



}

MetaParser::~MetaParser(void)
{
}

void MetaParser::Finish(void)
{
}

int MetaParser::Parse(void)
{
	return 0;
}

void MetaParser::GenerateFiles(void)
{
}



