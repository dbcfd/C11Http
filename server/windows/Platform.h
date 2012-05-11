#pragma once

#include <stdexcept>

#pragma warning(disable:4251)
#pragma warning(disable:4275)
//windows defines
#if defined(TCP_WINDOWS_BUILD)
#define TCP_WINDOWS_API __declspec(dllexport)
#else
#define TCP_WINDOWS_API __declspec(dllimport)
#endif
