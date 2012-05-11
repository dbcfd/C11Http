#pragma once

#include <stdexcept>

#pragma warning(disable:4251)
#pragma warning(disable:4275)
//windows defines
#if defined(WORKERS_BUILD)
#define WORKERS_API __declspec(dllexport)
#else
#define WORKERS_API __declspec(dllimport)
#endif
