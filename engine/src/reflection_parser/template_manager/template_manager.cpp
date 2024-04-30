
#include "template_manager.h"

void TemplateManager::LoadTemplates(std::string path, std::string template_name)
{
    template_pool_.insert_or_assign(template_name,
                                     Utils::loadFile(path + "/../template/" + template_name + ".mustache"));
}

std::string TemplateManager::RenderByTemplate(std::string template_name, Mustache::data& template_data)
{
    if (template_pool_.end() == template_pool_.find(template_name))
    {
        return "";
    }
    Mustache::mustache tmpl(template_pool_[template_name]);
    return tmpl.render(template_data);
}
