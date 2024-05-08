#include "common/precompiled.h"
#include "parser/parser.h"

int parse(std::string project_file_name,
          std::string header_file_name_to_generate,
          std::string project_base_directory,
          std::string sys_include_directory,
          std::string module_name,
          std::string show_errors);

int main(int argc, char** argv)
{
    auto start_time = std::chrono::system_clock::now();
    int  result     = 0;

    if (argv[1] != nullptr && argv[2] != nullptr && argv[3] != nullptr && 
        argv[4] != nullptr && argv[5] != nullptr && argv[6] != nullptr)
    {
        std::cout << "parse project file name: " << argv[1] << std::endl;
        std::cout << "the header file path to generate: " << argv[2] << std::endl;
        std::cout << "the project base directory: " << argv[3] << std::endl;
        std::cout << "the system include directory: " << argv[4] << std::endl;
        std::cout << "the module name: " << argv[5] << std::endl;
		std::cout << "show errors" << argv[6] << std::endl;
        ReflectionParser::Prepare();

        result = parse(argv[1], argv[2], argv[3], argv[4], argv[5], argv[6]);

        auto duration_time = std::chrono::system_clock::now() - start_time;
        std::cout << "Completed in " << std::chrono::duration_cast<std::chrono::milliseconds>(duration_time).count()
                  << "ms" << std::endl;
        return result;
    }
    else
    {
        std::cerr << "Arguments parse error!" << std::endl
                  << "Please call the tool like this:" << std::endl
                  << "meta_parser <project_file_names> <header_file_name_to_generate> <project_base_directory> "
                     "<sys_include_directory> <module_name> <show_errors(0 or 1)>"
                  << std::endl
                  << std::endl;
        return -1;
    }

    return 0;
}

int parse(std::string project_file_names,
            std::string header_file_name_to_generate,
            std::string project_base_directory,
            std::string sys_include_directory,
            std::string module_name,
            std::string show_errors)
{
    std::cout << std::endl;
    std::cout << "Parsing meta data for target \"" << module_name << "\"" << std::endl;
    std::fstream input_file;

    bool is_show_errors = "0" != show_errors;

    ReflectionParser parser(
        project_file_names, header_file_name_to_generate, project_base_directory, sys_include_directory, module_name, is_show_errors);

    std::cout << "Parsing in " << project_base_directory << std::endl;
    int result = parser.Parse();
    if (0 != result)
    {
        return result;
    }

    parser.GenerateFiles();

    return 0;
}
