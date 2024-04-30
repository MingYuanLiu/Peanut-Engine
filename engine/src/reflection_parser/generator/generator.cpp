#include "common/precompiled.h"

#include "generator/generator.h"

namespace Generator
{
    void GeneratorInterface::Prepare(const std::string& path)
    {
        if (!fs::exists(path))
        {
            // create output directory
            fs::create_directories(path);
        }
    }

    void GeneratorInterface::GenClassCodeRenderData(std::shared_ptr<MetaClass> class_temp, Mustache::data& class_def)
    {
        size_t base_class_size = class_temp->GetBaseClasses().size();
        class_def.set("class_name", class_temp->GetClassName());
        class_def.set("class_base_class_size", std::to_string(base_class_size));
        class_def.set("class_need_register", true);

        if (base_class_size > 0)
        {
            Mustache::data class_base_class_defines(Mustache::data::type::list);
            class_def.set("class_has_base", true);
            for (int index = 0; index < base_class_size; ++index)
            {
                Mustache::data class_base_class_def;

                std::string base_class_name = class_temp->GetBaseClasses()[index]->GetClassName();
                class_base_class_def.set("class_base_class_name", base_class_name);
                class_base_class_def.set("class_base_class_index", std::to_string(index));
                class_base_class_defines.push_back(class_base_class_def);
            }
            class_def.set("class_base_class_defines", class_base_class_defines);
        }

        Mustache::data class_field_defines = Mustache::data::type::list;
        GenClassFieldCodeRenderData(class_temp, class_field_defines);
        class_def.set("class_field_defines", class_field_defines);

        
        Mustache::data class_method_defines = Mustache::data::type::list;
        GenClassFunctionCodeRenderData(class_temp, class_method_defines);
        class_def.set("class_method_defines", class_method_defines);
    }
    void GeneratorInterface::GenClassFieldCodeRenderData(std::shared_ptr<MetaClass> class_temp, Mustache::data& feild_defs)
    {
        

        for (auto& field : class_temp->GetFields())
        {
            if (!field->ShouldCompiled())
                continue;

            Mustache::data filed_define;

            filed_define.set("class_field_name", field->GetName());
            filed_define.set("class_field_type", field->GetTypeName());
            filed_define.set("class_field_display_name", field->GetDisplayName());
            bool is_vector = field->IsVector();
            filed_define.set("class_field_is_vector", is_vector);
            feild_defs.push_back(filed_define);
        }
    }

    void GeneratorInterface::GenClassFunctionCodeRenderData(std::shared_ptr<MetaClass> class_temp, Mustache::data& method_defs)
    {
       for (auto& method : class_temp->GetFunctions())
        {
            if (!method->ShouldCompiled())
                continue;
            Mustache::data method_define;

            method_define.set("class_method_name", method->GetName());   
            method_defs.push_back(method_define);
        }
    }
} // namespace Generator
