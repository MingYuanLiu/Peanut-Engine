#pragma once
#include "cursor/cursor_wapper.h"
#include "language_types/meta_type.h"
#include "language_types/meta_field.h"
#include "language_types/meta_function.h"

class BaseClass
{
public:
    BaseClass(const WapperCursor& cursor);

    std::string GetClassName() const { return base_class_name_; }

private:
    std::string base_class_name_;
    WapperCursor cursor_;
};

class MetaClass : public MetaType
{
public:
    MetaClass(const WapperCursor& own_cursor, Namespace current_namespace);

    ~MetaClass() {}

    inline std::string GetClassName() const { return class_name_; }

    const SharedPtrVector<BaseClass>& GetBaseClasses() const { return base_classes_; }

    const SharedPtrVector<MetaField>& GetFields() const { return fields_; }

    const SharedPtrVector<MetaFunction> GetFunctions() const { return functions_; }

    virtual bool ShouldCompiled() override;

private:
    std::string class_name_;

    std::string qualified_name_;

    SharedPtrVector<BaseClass> base_classes_;

    SharedPtrVector<MetaField> fields_;

    SharedPtrVector<MetaFunction> functions_;
};
