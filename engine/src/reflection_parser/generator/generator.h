#pragma once
#include "common/schema_module.h"
#include "language_types/meta_class.h"

#include <functional>
#include <string>
namespace Generator
{
    class GeneratorInterface
    {
    public:
        GeneratorInterface(std::string                             out_path,
                           std::string                             root_path,
                           std::function<std::string(std::string)> get_include_func) :
            out_path_(out_path),
            root_path_(root_path), get_include_func_(get_include_func)
        {}

        /**
        * Generate code for the schema
        * @param: path
        * @param: schema
        */
        virtual int  Generate(std::string path, SchemaMoudle schema) = 0;
        virtual void Finish() {};

        virtual ~GeneratorInterface() {};

    protected:
        virtual void Prepare(const std::string& path);

        // Generate render data for class code template generation
        virtual void GenClassCodeRenderData(std::shared_ptr<MetaClass> class_temp, Mustache::data& class_def);
        
        // Generate render data for class field code template generation
        virtual void GenClassFieldCodeRenderData(std::shared_ptr<MetaClass> class_temp, Mustache::data& feild_defs);
        
        // Generate render data for class function code template generation
        virtual void GenClassFunctionCodeRenderData(std::shared_ptr<MetaClass> class_temp, Mustache::data& method_defs);

        virtual std::string ProcessFileName(std::string path) = 0;

        std::string                             out_path_ {"gen_src"};
        std::string                             root_path_;
        std::function<std::string(std::string)> get_include_func_;
    };
} // namespace Generator
