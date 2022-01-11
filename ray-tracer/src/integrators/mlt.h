

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_INTEGRATORS_MLT_H
#define PBRT_INTEGRATORS_MLT_H

// integrators/mlt.h*
#include "pbrt.h"
#include "integrator.h"
#include "sampler.h"
#include "spectrum.h"
#include "film.h"
#include "rng.h"
#include <unordered_map>

namespace pbrt {

// MLTSampler Declarations
class MLTSampler : public Sampler {
  public:
    // MLTSampler Public Methods
    MLTSampler(int mutationsPerPixel, int rngSequenceIndex, Float sigma,
               Float largeStepProbability, int streamCount)
        : Sampler(mutationsPerPixel),
          rng(rngSequenceIndex),
          sigma(sigma),
          largeStepProbability(largeStepProbability),
          streamCount(streamCount) {}
    Float Get1D();
    Point2f Get2D();
    std::unique_ptr<Sampler> Clone(int seed);
    void StartIteration();
    void Accept();
    void Reject();
    void StartStream(int index);
    int GetNextIndex() { return streamIndex + streamCount * sampleIndex++; }

  protected:
    // MLTSampler Private Declarations
    struct PrimarySample {
        Float value = 0;
        // PrimarySample Public Methods
        void Backup() {
            valueBackup = value;
            modifyBackup = lastModificationIteration;
        }
        void Restore() {
            value = valueBackup;
            lastModificationIteration = modifyBackup;
        }

        // PrimarySample Public Data
        int64_t lastModificationIteration = 0;
        Float valueBackup = 0;
        int64_t modifyBackup = 0;
    };

    // MLTSampler Private Methods
    void EnsureReady(int index);

    // MLTSampler Private Data
    RNG rng;
    const Float sigma, largeStepProbability;
    const int streamCount;
    std::vector<PrimarySample> X;
    int64_t currentIteration = 0;
    bool largeStep = true;
    int64_t lastLargeStepIteration = 0;
    int streamIndex, sampleIndex;
};

// MLT Declarations
class MLTIntegrator : public Integrator {
  public:
    // MLTIntegrator Public Methods
    MLTIntegrator(std::shared_ptr<const Camera> camera, int maxDepth,
                  int nBootstrap, int nChains, int mutationsPerPixel,
                  Float sigma, Float largeStepProbability)
        : camera(camera),
          maxDepth(maxDepth),
          nBootstrap(nBootstrap),
          nChains(nChains),
          mutationsPerPixel(mutationsPerPixel),
          sigma(sigma),
          largeStepProbability(largeStepProbability) {}
    void Render(const Scene &scene);
    Spectrum L(const Scene &scene, MemoryArena &arena,
               const std::unique_ptr<Distribution1D> &lightDistr,
               const std::unordered_map<const Light *, size_t> &lightToIndex,
               MLTSampler &sampler, int k, Point2f *pRaster);

  private:
    // MLTIntegrator Private Data
    std::shared_ptr<const Camera> camera;
    const int maxDepth;
    const int nBootstrap;
    const int nChains;
    const int mutationsPerPixel;
    const Float sigma, largeStepProbability;
};

MLTIntegrator *CreateMLTIntegrator(const ParamSet &params,
                                   std::shared_ptr<const Camera> camera);

}  // namespace pbrt

#endif  // PBRT_INTEGRATORS_MLT_H
