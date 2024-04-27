#include "meta_data.h"

MetaData::MetaData(const WapperCursor& cursor)
{
	for (const auto& child : cursor.GetAllChild())
	{
		// parse annotate attribute data
		if (child.GetCursorKind() == CXCursor_AnnotateAttr)
		{
			CXString display_name = clang_getCursorDisplayName(child.GetCursor());
			std::string display_name_str;
			Utils::toString(display_name, display_name_str);

			if (!display_name_str.empty())
				properties_.emplace_back(display_name_str);
		}
	}
}

bool MetaData::GetPropertyList(MetaData::PropertyList& out_list) const
{
	if (properties_.empty())
		return false;

	out_list.clear();
	if (properties_.size() == 1)
		return true;
		
	out_list.resize(properties_.size());

	std::copy(properties_.begin() + 1, properties_.end(), out_list.begin());

	return true;
}

std::string MetaData::GetFlag() const
{
	if (properties_.empty())
		return "";

	return *(properties_.begin());
}
