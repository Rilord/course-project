

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_CORE_FLOATFILE_H
#define PBRT_CORE_FLOATFILE_H

// core/floatfile.h*
#include "pbrt.h"

namespace pbrt {

bool ReadFloatFile(const char *filename, std::vector<Float> *values);

}  // namespace pbrt

#endif  // PBRT_CORE_FLOATFILE_H
