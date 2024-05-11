#pragma once
#include "generator/generator.h"

namespace Generator
{
    class SerializerGenerator : public GeneratorInterface
    {
    public:
        SerializerGenerator() = delete;
        SerializerGenerator(std::string source_directory, std::function<std::string(std::string)> get_include_function);

        virtual int Generate(const std::string& path, const SchemaMoudle& schema) override;

        virtual void Finish() override;

        virtual ~SerializerGenerator() override;

    protected:
        virtual void Prepare(const std::string& path) override;

        virtual std::string ProcessFileName(std::string path) override;

    private:
        void LoadCodeTemplate();
        Mustache::data class_defines_ {Mustache::data::type::list};
        Mustache::data include_headfiles_ {Mustache::data::type::list};

        static const std::string all_serialize_template_filename;
        static const std::string all_serialize_template_impl_filename;
        static const std::string common_serialize_template_filename;
    };
} // namespace Generator
