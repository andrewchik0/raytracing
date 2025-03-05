#pragma once

#ifdef _WIN32
  #define RT_PLATFORM_WINDOWS
#elif defined(__APPLE__)
  #include <TargetConditionals.h>
  #if TARGET_IPHONE_SIMULATOR == 1
    #error "IOS simulator is not supported!"
  #elif TARGET_OS_IPHONE == 1
    #error "IOS is not supported!"
  #elif TARGET_OS_MAC == 1
    #define RT_PLATFORM_MACOS
    #define USE_SDL
  #else
    #error "Unknown Apple platform!"
  #endif
#elif defined(__linux__)
  #define RT_PLATFORM_LINUX
#else
  #error "Unknown platform"
#endif

#if defined(_DEBUG) & defined(RT_PLATFORM_WINDOWS)
  #define _CRTDBG_MAP_ALLOC
  #include <crtdbg.h>
  #define SetDbgMemHooks() \
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF)

  static class __dummy
  {
  public:
    __dummy()
    {
      SetDbgMemHooks();
    }
  } __ooppss;
#endif // _DEBUG
