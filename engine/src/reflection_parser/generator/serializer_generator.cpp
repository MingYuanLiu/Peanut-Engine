#include "generator/serializer_generator.h"
#include "common/precompiled.h"
#include "language_types/meta_class.h"
#include "template_manager/template_manager.h"
#include "cursor/cursor_type.h"

namespace Generator
{
    const std::string SerializerGenerator::all_serialize_template_filename = "allSerializer.h";
    const std::string SerializerGenerator::all_serialize_template_impl_filename = "allSerializer.ipp";
    const std::string SerializerGenerator::common_serialize_template_filename = "commonSerializerGenFile";
    
    SerializerGenerator::SerializerGenerator(std::string                             source_directory,
                                             std::function<std::string(std::string)> get_include_function) :
        GeneratorInterface(source_directory + "/_generated_/serializer", source_directory, get_include_function)
    {
        Prepare(out_path_);
    }

    void SerializerGenerator::Prepare(const std::string& path)
    {
        GeneratorInterface::Prepare(path);
    }

    void SerializerGenerator::LoadCodeTemplate()
    {
        TemplateManager::getInstance()->LoadTemplates(root_path_, all_serialize_template_filename);
        TemplateManager::getInstance()->LoadTemplates(root_path_, all_serialize_template_impl_filename);
        TemplateManager::getInstance()->LoadTemplates(root_path_, common_serialize_template_filename);
    }

    std::string SerializerGenerator::ProcessFileName(std::string path)
    {
        auto relativeDir = fs::path(path).filename().replace_extension("serializer.gen.h").string();
        return out_path_ + "/" + relativeDir;
    }
    
    int SerializerGenerator::Generate(std::string path, SchemaMoudle schema)
    {
        std::string file_path = ProcessFileName(path);

        Mustache::data muatache_data;
        Mustache::data include_headfiles(Mustache::data::type::list);
        Mustache::data class_defines(Mustache::data::type::list);

        include_headfiles.push_back(
            Mustache::data("headfile_name", Utils::MakeRelativePath(root_path_, path).string()));
        for (auto class_temp : schema.classes)
        {
            if (!class_temp->ShouldCompiled())
                continue;

            Mustache::data class_def;
            GenClassCodeRenderData(class_temp, class_def);

            // deal base class
            auto& base_classes = class_temp->GetBaseClasses();
            for (int index = 0; index < base_classes.size(); ++index)
            {
                auto include_file = get_include_func_(base_classes[index]->GetClassName());
                if (!include_file.empty())
                {
                    auto include_file_base = ProcessFileName(include_file);
                    if (file_path != include_file_base)
                    {
                        include_headfiles.push_back(Mustache::data(
                            "headfile_name", Utils::MakeRelativePath(root_path_, include_file_base).string()));
                    }
                }
            }
            
            for (const auto& field : class_temp->GetFields())
            {
                if (!field->ShouldCompiled())
                    continue;
                
                // deal vector
                if (field->IsVector())
                {
                    auto include_file = get_include_func_(field->GetName());
                    if (!include_file.empty())
                    {
                        auto include_file_base = ProcessFileName(include_file);
                        if (file_path != include_file_base)
                        {
                            include_headfiles.push_back(Mustache::data(
                                "headfile_name", Utils::MakeRelativePath(root_path_, include_file_base).string()));
                        }
                    }
                }
                // deal normal
            }
            class_defines.push_back(class_def);
            m_class_defines.push_back(class_def);
        }

        muatache_data.set("class_defines", class_defines);
        muatache_data.set("include_headfiles", include_headfiles);
        std::string render_string =
            TemplateManager::getInstance()->RenderByTemplate(common_serialize_template_filename, muatache_data);
        Utils::SaveFile(render_string, file_path);

        m_include_headfiles.push_back(
            Mustache::data("headfile_name", Utils::MakeRelativePath(out_path_, file_path).string()));
        return 0;
    }

    void SerializerGenerator::Finish()
    {
        Mustache::data mustache_data;
        mustache_data.set("class_defines", m_class_defines);
        mustache_data.set("include_headfiles", m_include_headfiles);

        std::string render_string = TemplateManager::getInstance()->RenderByTemplate(all_serialize_template_filename, mustache_data);
        Utils::SaveFile(render_string, out_path_ + "/all_serializer.h");
        render_string = TemplateManager::getInstance()->RenderByTemplate(all_serialize_template_impl_filename, mustache_data);
        Utils::SaveFile(render_string, out_path_ + "/all_serializer.ipp");
    }

    SerializerGenerator::~SerializerGenerator() {}
} // namespace Generator
