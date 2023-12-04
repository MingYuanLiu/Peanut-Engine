#pragma once

#ifdef PE_PLATFORM_WINDOWS
#ifdef PE_BUILD_DLL
#define PE_API __declspec(dllexport)
#else
#define PE_API __declspec(dllimport)
#endif
#else
#error Peanut Engine now only supports windows
#endif

#define BIT(x) (1 << x)
