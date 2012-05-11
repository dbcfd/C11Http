#pragma once

#include <stdexcept>

#pragma warning(disable:4251)
#pragma warning(disable:4275)
//windows defines
#if defined(TCP_BUILD)
#define TCP_API __declspec(dllexport)
#else
#define TCP_API __declspec(dllimport)
#endif
