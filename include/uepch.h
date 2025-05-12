#pragma once

#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>
#include <array>
#include <fstream>
#include <cstdint>
#include <queue>
#include <map>
#include <list>

#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#ifdef UE_PLATFORM_WINDOWS
    #ifndef NOMINMAX
    # define NOMINMAX
    #endif
    #include <Windows.h>
#endif