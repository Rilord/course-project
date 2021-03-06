

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_CORE_ERROR_H
#define PBRT_CORE_ERROR_H

// core/error.h*
#include "pbrt.h"

namespace pbrt {

// Error Reporting Declarations

// Setup printf format
#ifdef __GNUG__
#define PRINTF_FUNC __attribute__((__format__(__printf__, 1, 2)))
#else
#define PRINTF_FUNC
#endif  // __GNUG__
void Warning(const char *, ...) PRINTF_FUNC;
void Error(const char *, ...) PRINTF_FUNC;

}  // namespace pbrt

#endif  // PBRT_CORE_ERROR_H
