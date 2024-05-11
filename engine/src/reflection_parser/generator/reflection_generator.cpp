#include "common/precompiled.h"

#include "generator/reflection_generator.h"

#include "language_types/meta_class.h"
#include "template_manager/template_manager.h"
#include "cursor/cursor_type.h"

#include <map>
#include <set>

namespace Generator
{
    const std::string ReflectionGenerator::common_reflection_template_filename = "commonReflectionTempl";
    const std::string ReflectionGenerator::all_reflection_template_filename = "allReflectionTempl";


    ReflectionGenerator::ReflectionGenerator(std::string                             source_directory,
                                             std::function<std::string(const std::string&)> get_include_function) 
        : GeneratorInterface(source_directory + "/_generated_/reflection", source_directory, get_include_function)
    {
        Prepare(out_path_);
    }

    void ReflectionGenerator::Prepare(const std::string& path)
    {
        GeneratorInterface::Prepare(path);
        LoadCodeTemplate();
        return;
    }

    void ReflectionGenerator::LoadCodeTemplate()
    {
        TemplateManager::getInstance()->LoadTemplates(root_path_, common_reflection_template_filename);
        TemplateManager::getInstance()->LoadTemplates(root_path_, all_reflection_template_filename);
    }

    std::string ReflectionGenerator::ProcessFileName(std::string path)
    {
        auto relative_dir = fs::path(path).filename().replace_extension("reflection.gen.h").string();
        return out_path_ + "/" + relative_dir;
    }

    int ReflectionGenerator::Generate(const std::string& path, const SchemaMoudle& schema)
    {
        static const std::string vector_prefix = "std::vector<";

        std::string    file_path = ProcessFileName(path);

        Mustache::data mustache_data;
        Mustache::data include_headfiles(Mustache::data::type::list);
        Mustache::data class_defines(Mustache::data::type::list);

        include_headfiles.push_back(
            Mustache::data("headfile_name", Utils::MakeRelativePath(root_path_, path).string()));

        // class defs in single header file
        for (auto class_temp : schema.classes)
        {
            if (!class_temp->ShouldCompiled())
                continue;

            std::vector<std::string>                                   field_names;
            std::map<std::string, std::pair<std::string, std::string>> vector_map;

            Mustache::data class_def;
            Mustache::data vector_defines(Mustache::data::type::list);

            GenClassCodeRenderData(class_temp, class_def);
            for (auto& field : class_temp->GetFields())
            {
                if (!field->ShouldCompiled())
                    continue;

                field_names.emplace_back(field->GetName());
                bool is_array = field->IsVector();
                if (is_array)
                {
                    std::string array_useful_name = field->GetTypeName();

                    Utils::FormatQualifiedName(array_useful_name);

                    std::string item_type = field->GetTypeName();

                    item_type = Utils::GetTypeNameOfVector(item_type);

                    vector_map[field->GetTypeName()] = std::make_pair(array_useful_name, item_type);
                }
            }

            if (vector_map.size() > 0)
            {
                if (nullptr == class_def.get("vector_exist"))
                {
                    class_def.set("vector_exist", true);
                }
                for (auto vector_item : vector_map)
                {
                    std::string    array_useful_name = vector_item.second.first;
                    std::string    item_type         = vector_item.second.second;
                    Mustache::data vector_define;
                    vector_define.set("vector_useful_name", array_useful_name);
                    vector_define.set("vector_type_name", vector_item.first);
                    vector_define.set("vector_element_type_name", item_type);
                    vector_defines.push_back(vector_define);
                }
            }
            class_def.set("vector_defines", vector_defines);
            class_defines.push_back(class_def);
        }

        mustache_data.set("class_defines", class_defines);
        mustache_data.set("include_headfiles", include_headfiles);

        std::string tmp = Utils::ConvertNameToUpperCamelCase(fs::path(path).stem().string(), "_");
        mustache_data.set("sourefile_name_upper_camel_case", tmp);

        std::string render_string =
            TemplateManager::getInstance()->RenderByTemplate(common_reflection_template_filename, mustache_data);
        Utils::SaveFile(render_string, file_path);

        sourcefile_list_.emplace_back(tmp);

        head_file_list_.emplace_back(Utils::MakeRelativePath(root_path_, file_path).string());
        return 0;
    }

    void ReflectionGenerator::Finish()
    {
        Mustache::data mustache_data;
        Mustache::data include_headfiles = Mustache::data::type::list;
        Mustache::data sourefile_names    = Mustache::data::type::list;

        for (auto& head_file : head_file_list_)
        {
            include_headfiles.push_back(Mustache::data("headfile_name", head_file));
        }

        for (auto& sourefile_name_upper_camel_case : sourcefile_list_)
        {
            sourefile_names.push_back(Mustache::data("sourefile_name_upper_camel_case", sourefile_name_upper_camel_case));
        }

        mustache_data.set("include_headfiles", include_headfiles);
        mustache_data.set("sourefile_names", sourefile_names);
        std::string render_string =
            TemplateManager::getInstance()->RenderByTemplate(all_reflection_template_filename, mustache_data);
        Utils::SaveFile(render_string, out_path_ + "/all_reflection.h");
    }

    ReflectionGenerator::~ReflectionGenerator() {}
} // namespace Generator