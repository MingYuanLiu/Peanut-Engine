#pragma once

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <clang-c/Index.h>

namespace fs = std::filesystem;

#include "meta/meta_data_config.h"
#include "meta/meta_utils.h"
#include "cursor/cursor_type.h"

#include "mustache.hpp"

namespace Mustache = kainjow::mustache;

template<typename T>
using SharedPtrVector = std::vector<std::shared_ptr<T> >;
