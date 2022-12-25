//
// Created by Adrien COURNAND on 24/12/2022.
//

#ifdef WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __attribute__((visibility("default")))
#endif

#include <cstdio>

extern "C" DLL_EXPORT void hotReloadDraw(void)
{
}

