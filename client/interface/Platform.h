#pragma once

#include <stdexcept>

#pragma warning(disable:4251)
#pragma warning(disable:4275)
//windows defines
#if defined(CLIENT_INTERFACE_BUILD)
#define CLIENT_INTERFACE_API __declspec(dllexport)
#else
#define CLIENT_INTERFACE_API __declspec(dllimport)
#endif
