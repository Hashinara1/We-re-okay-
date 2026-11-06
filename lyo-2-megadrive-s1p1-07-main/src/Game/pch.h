#pragma once

#ifdef _DEBUG
#pragma comment(lib, "../../lib/cpulib/lib/cpu-core-debug.lib")
#pragma comment(lib, "../../lib/cpulib/lib/cpu-render-debug.lib")
#pragma comment(lib, "../../lib/cpulib/lib/cpu-engine-debug.lib")
#else
#pragma comment(lib, "../../lib/cpulib/lib/cpu-core.lib")
#pragma comment(lib, "../../lib/cpulib/lib/cpu-render.lib")
#pragma comment(lib, "../../lib/cpulib/lib/cpu-engine.lib")
#endif

#include "../../lib/cpulib/include/cpu-engine/cpu-engine.h"

class Table;
class PlayerStriker;
class AIStriker;
class Puck;

#include "App.h"