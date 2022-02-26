#pragma once

#ifdef PE_PLATFORM_WINDOWS
	#ifdef PE_BUILD_DLL
		#define PE_API __declspec(dllexport)
	#else
		#define PE_API __declspec(dllimport)
	#endif
#else
	#error Peanut Engine only supports windows
#endif