#pragma once
#include "generator/generator.h"
namespace Generator
{
    class ReflectionGenerator : public GeneratorInterface
    {
    public:
        ReflectionGenerator() = delete;
        ReflectionGenerator(std::string source_directory, std::function<std::string(const std::string&)> get_include_function);
        virtual int  Generate(const std::string& path, const SchemaMoudle& schema) override;
        virtual void Finish() override;
        virtual ~ReflectionGenerator() override;

    protected:
        virtual void        Prepare(const std::string& path) override;
        virtual std::string ProcessFileName(std::string path) override;

    private:
        void LoadCodeTemplate();
        std::vector<std::string> head_file_list_;
        std::vector<std::string> sourcefile_list_;

        static const std::string common_reflection_template_filename;
        static const std::string all_reflection_template_filename;
    };
} // namespace Generator
