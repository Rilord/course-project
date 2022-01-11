

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_CORE_SOBOLMATRICES_H
#define PBRT_CORE_SOBOLMATRICES_H

// core/sobolmatrices.h*
#include "pbrt.h"

namespace pbrt {

// Sobol Matrix Declarations
static PBRT_CONSTEXPR int NumSobolDimensions = 1024;
static PBRT_CONSTEXPR int SobolMatrixSize = 52;
extern const uint32_t SobolMatrices32[NumSobolDimensions * SobolMatrixSize];
extern const uint64_t SobolMatrices64[NumSobolDimensions * SobolMatrixSize];
extern const uint64_t VdCSobolMatrices[][SobolMatrixSize];
extern const uint64_t VdCSobolMatricesInv[][SobolMatrixSize];

}  // namespace pbrt

#endif  // PBRT_CORE_SOBOLMATRICES_H
