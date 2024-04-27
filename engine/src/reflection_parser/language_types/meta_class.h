#pragma once
#include "cursor/cursor_wapper.h"
#include "language_types/meta_type.h"
#include "language_types/meta_field.h"
#include "language_types/meta_function.h"

class BaseClass
{
    BaseClass(const WapperCursor& cursor);

    std::string base_class_name_;
    WapperCursor cursor_;
};

class MetaClass : public MetaType
{
public:
    MetaClass(const WapperCursor& own_cursor, Namespace current_namespace);

    inline std::string GetClassName() const { return class_name_; }

    template<typename T>
    using SharedPtrVector = std::vector<std::shared_ptr<T> >;

    const SharedPtrVector<BaseClass>& GetBaseClasses() const { return base_classes_; }

    const SharedPtrVector<MetaField>& GetFields() const { return fields_; }

    const SharedPtrVector<MetaFunction> GetFunctions() const { return functions_; }

    virtual bool ShouldCompiled() override;
private:
    std::string class_name_;

    SharedPtrVector<BaseClass> base_classes_;

    SharedPtrVector<MetaField> fields_;

    SharedPtrVector<MetaFunction> functions_;
};
