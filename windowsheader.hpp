#pragma once
#ifdef _WIN32
#ifndef _WINDOWS_

#ifdef _MSC_VER
#define NOMINMAX
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <cstdint>

using socklen_t = int32_t;

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "WS2_32.lib")
#endif