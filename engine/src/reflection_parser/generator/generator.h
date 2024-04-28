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
            m_out_path(out_path),
            m_root_path(root_path), m_get_include_func(get_include_func)
        {}
        virtual int  Generate(std::string path, SchemaMoudle schema) = 0;
        virtual void Finish() {};

        virtual ~GeneratorInterface() {};

    protected:
        virtual void PrepareStatus(std::string path);

        // Generate render data for class code template generation
        virtual void GenClassCodeRenderData(std::shared_ptr<MetaClass> class_temp, Mustache::data& class_def);
        
        // Generate render data for class field code template generation
        virtual void GenClassFieldCodeRenderData(std::shared_ptr<MetaClass> class_temp, Mustache::data& feild_defs);
        
        // Generate render data for class function code template generation
        virtual void GenClassFunctionCodeRenderData(std::shared_ptr<MetaClass> class_temp, Mustache::data& method_defs);

        virtual std::string ProcessFileName(std::string path) = 0;

        std::string                             m_out_path {"gen_src"};
        std::string                             m_root_path;
        std::function<std::string(std::string)> m_get_include_func;
    };
} // namespace Generator
