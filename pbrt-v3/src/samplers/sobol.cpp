


// samplers/sobol.cpp*
#include "samplers/sobol.h"
#include "lowdiscrepancy.h"
#include "paramset.h"

namespace pbrt {

// SobolSampler Method Definitions
int64_t SobolSampler::GetIndexForSample(int64_t sampleNum) const {
    return SobolIntervalToIndex(log2Resolution, sampleNum,
                                Point2i(currentPixel - sampleBounds.pMin));
}

Float SobolSampler::SampleDimension(int64_t index, int dim) const {
    if (dim >= NumSobolDimensions)
        LOG(FATAL) << StringPrintf("SobolSampler can only sample up to %d "
                                   "dimensions! Exiting.",
                                   NumSobolDimensions);
    Float s = SobolSample(index, dim);
    // Remap Sobol$'$ dimensions used for pixel samples
    if (dim == 0 || dim == 1) {
        s = s * resolution + sampleBounds.pMin[dim];
        s = Clamp(s - currentPixel[dim], (Float)0, OneMinusEpsilon);
    }
    return s;
}

std::unique_ptr<Sampler> SobolSampler::Clone(int seed) {
    return std::unique_ptr<Sampler>(new SobolSampler(*this));
}

SobolSampler *CreateSobolSampler(const ParamSet &params,
                                 const Bounds2i &sampleBounds) {
    int nsamp = params.FindOneInt("pixelsamples", 16);
    if (PbrtOptions.quickRender) nsamp = 1;
    return new SobolSampler(nsamp, sampleBounds);
}

}  // namespace pbrt
