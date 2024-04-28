#include "common/precompiled.h"

#include "cursor_wapper.h"
#include "cursor_type.h"

CursorType::CursorType(const CXType& handle) : handle_(handle) {}

std::string CursorType::GetDisplayName(void) const
{
    std::string display_name;

    Utils::toString(clang_getTypeSpelling(handle_), display_name);

    return display_name;
}

int CursorType::GetArgumentCount(void) const { return clang_getNumArgTypes(handle_); }

CursorType CursorType::GetArgument(unsigned index) const { return clang_getArgType(handle_, index); }

CursorType CursorType::GetCanonicalType(void) const { return clang_getCanonicalType(handle_); }

WapperCursor CursorType::GetDeclaration(void) const { return clang_getTypeDeclaration(handle_); }

CXTypeKind CursorType::GetKind(void) const { return handle_.kind; }

bool CursorType::IsConst(void) const { return clang_isConstQualifiedType(handle_) ? true : false; }

bool CursorType::IsVolatile(void) const { return clang_isVolatileQualifiedType(handle_) ? true : false; }
