#include "cursor_wapper.h"

WapperCursor::WapperCursor(const CXCursor& cursor) : cursor_(cursor)
{}

std::string WapperCursor::GetCursorSpelling() const
{
	std::string spelling;
	Utils::toString(clang_getCursorSpelling(cursor_), spelling);

	return spelling;
}

std::string WapperCursor::GetCursorDisplayName() const
{
	std::string displayName;
	Utils::toString(clang_getCursorDisplayName(cursor_), displayName);

	return displayName;
}

std::string WapperCursor::GetCursorLocateFile()
{
	auto cursor_range = clang_Cursor_getSpellingNameRange(cursor_, 0, 0);

	auto start_location = clang_getRangeStart(cursor_range);

	CXFile file;
	uint32_t line, column, offset;

	clang_getFileLocation(start_location, &file, &line, &column, &offset);

	std::string localtion_file;
	Utils::toString(clang_getFileName(file), localtion_file);

	std::cout << "========== GetCursorLocateFile ============== \n";
	std::cout << "get location file: " << localtion_file << "\n";

	return localtion_file;
}

void WapperCursor::GetCursorLocateLine(uint32_t& start_line, uint32_t& end_line)
{
	auto cursor_range = clang_getCursorExtent(cursor_);

	CXFile file;
	uint32_t start_column, start_offset;
	uint32_t end_column, end_offset;

	clang_getExpansionLocation(clang_getRangeStart(cursor_range), &file, &start_line, &start_column, &start_offset);
	clang_getExpansionLocation(clang_getRangeEnd(cursor_range), &file, &end_line, &end_column, &end_offset);

	std::string localtion_file;
	Utils::toString(clang_getFileName(file), localtion_file);

	std::cout << "========== GetCursorLocateLine ============== \n";
	std::cout << "get location file: " << localtion_file << "\n";
	std::cout << "start_line: " << start_line << " end_line: " << end_line << "\n";
	std::cout << "start_column: " << start_column << " end_column: " << end_column << "\n";
	std::cout << "=============================================" << "\n";
}

 WapperCursor::List WapperCursor::GetAllChild()
{
	List children;

	auto Visitor = [](CXCursor cursor, CXCursor parent, CXClientData client_data) -> CXChildVisitResult
	{
		auto container = static_cast<List*>(client_data);

		container->emplace_back(cursor);

		if (cursor.kind == CXCursor_LastPreprocessing)
			return CXChildVisit_Break;

		return CXChildVisit_Continue;
	};
	
	clang_visitChildren(cursor_, Visitor, &children);

	return children;
}

void WapperCursor::VisitAllChild(CXCursorVisitor visitor, void* data)
{
	clang_visitChildren(cursor_, visitor, data);
}