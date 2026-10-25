#pragma once

#include "engine.hpp"

namespace azalea {

void search(Board&, const SearchLimits&, SearchReporter&, std::stop_token);

} // namespace azalea
