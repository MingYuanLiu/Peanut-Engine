#pragma once
#include "cursor/cursor_wapper.h"
#include "meta/meta_data.h"

class MetaType
{
public:
    MetaType(const WapperCursor& own_cursor, Namespace current_namespace);

    MetaData GetMetaData() const { return meta_data_; }

    std::string GetSourceFile() const;

    inline Namespace GetCurrentNameSpace() { return current_namespace_; }

    inline bool IsEnable() { return enable_; }

    virtual bool ShouldCompiled() { return enable_; }

    virtual std::string GetName() { return name_; }

protected:
    MetaData meta_data_;

    Namespace current_namespace_;

    bool enable_;
 
    WapperCursor own_cursor_;

    std::string name_;
};
