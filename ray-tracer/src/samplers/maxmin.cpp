


// samplers/maxmin.cpp*
#include "samplers/maxmin.h"
#include "paramset.h"
#include "stats.h"

namespace pbrt {

// MaxMinDistSampler Method Definitions
void MaxMinDistSampler::StartPixel(const Point2i &p) {
    ProfilePhase _(Prof::StartPixel);
    Float invSPP = (Float)1 / samplesPerPixel;
    for (int i = 0; i < samplesPerPixel; ++i)
        samples2D[0][i] = Point2f(i * invSPP, SampleGeneratorMatrix(CPixel, i));
    Shuffle(&samples2D[0][0], samplesPerPixel, 1, rng);
    // Generate remaining samples for _MaxMinDistSampler_
    for (size_t i = 0; i < samples1D.size(); ++i)
        VanDerCorput(1, samplesPerPixel, &samples1D[i][0], rng);

    for (size_t i = 1; i < samples2D.size(); ++i)
        Sobol2D(1, samplesPerPixel, &samples2D[i][0], rng);

    for (size_t i = 0; i < samples1DArraySizes.size(); ++i) {
        int count = samples1DArraySizes[i];
        VanDerCorput(count, samplesPerPixel, &sampleArray1D[i][0], rng);
    }

    for (size_t i = 0; i < samples2DArraySizes.size(); ++i) {
        int count = samples2DArraySizes[i];
        Sobol2D(count, samplesPerPixel, &sampleArray2D[i][0], rng);
    }
    PixelSampler::StartPixel(p);
}

std::unique_ptr<Sampler> MaxMinDistSampler::Clone(int seed) {
    MaxMinDistSampler *mmds = new MaxMinDistSampler(*this);
    mmds->rng.SetSequence(seed);
    return std::unique_ptr<Sampler>(mmds);
}

MaxMinDistSampler *CreateMaxMinDistSampler(const ParamSet &params) {
    int nsamp = params.FindOneInt("pixelsamples", 16);
    int sd = params.FindOneInt("dimensions", 4);
    if (PbrtOptions.quickRender) nsamp = 1;
    return new MaxMinDistSampler(nsamp, sd);
}

}  // namespace pbrt
