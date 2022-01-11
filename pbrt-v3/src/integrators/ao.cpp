

// integrators/ao.cpp*
#include "integrators/ao.h"
#include "sampling.h"
#include "interaction.h"
#include "paramset.h"
#include "camera.h"
#include "film.h"
#include "scene.h"

namespace pbrt {

// AOIntegrator Method Definitions
AOIntegrator::AOIntegrator(bool cosSample, int ns,
                           std::shared_ptr<const Camera> camera,
                           std::shared_ptr<Sampler> sampler,
                           const Bounds2i &pixelBounds)
    : SamplerIntegrator(camera, sampler, pixelBounds),
      cosSample(cosSample) {
    nSamples = sampler->RoundCount(ns);
    if (ns != nSamples)
        Warning("Taking %d samples, not %d as specified", nSamples, ns);
    sampler->Request2DArray(nSamples);
}

Spectrum AOIntegrator::Li(const RayDifferential &r, const Scene &scene,
                          Sampler &sampler, MemoryArena &arena,
                          int depth) const {
    ProfilePhase p(Prof::SamplerIntegratorLi);
    Spectrum L(0.f);
    RayDifferential ray(r);

    // Intersect _ray_ with scene and store intersection in _isect_
    SurfaceInteraction isect;
 retry:
    if (scene.Intersect(ray, &isect)) {
        isect.ComputeScatteringFunctions(ray, arena, true);
        if (!isect.bsdf) {
            VLOG(2) << "Skipping intersection due to null bsdf";
            ray = isect.SpawnRay(ray.d);
            goto retry;
        }

        // Compute coordinate frame based on true geometry, not shading
        // geometry.
        Normal3f n = Faceforward(isect.n, -ray.d);
        Vector3f s = Normalize(isect.dpdu);
        Vector3f t = Cross(isect.n, s);

        const Point2f *u = sampler.Get2DArray(nSamples);
        for (int i = 0; i < nSamples; ++i) {
            Vector3f wi;
            Float pdf;
            if (cosSample) {
                wi = CosineSampleHemisphere(u[i]);
                pdf = CosineHemispherePdf(std::abs(wi.z));
            } else {
                wi = UniformSampleHemisphere(u[i]);
                pdf = UniformHemispherePdf();
            }

            // Transform wi from local frame to world space.
            wi = Vector3f(s.x * wi.x + t.x * wi.y + n.x * wi.z,
                          s.y * wi.x + t.y * wi.y + n.y * wi.z,
                          s.z * wi.x + t.z * wi.y + n.z * wi.z);

            if (!scene.IntersectP(isect.SpawnRay(wi)))
                L += Dot(wi, n) / (pdf * nSamples);
        }
    }
    return L;
}

AOIntegrator *CreateAOIntegrator(const ParamSet &params,
                                 std::shared_ptr<Sampler> sampler,
                                 std::shared_ptr<const Camera> camera) {
    int np;
    const int *pb = params.FindInt("pixelbounds", &np);
    Bounds2i pixelBounds = camera->film->GetSampleBounds();
    if (pb) {
        if (np != 4)
            Error("Expected four values for \"pixelbounds\" parameter. Got %d.",
                  np);
        else {
            pixelBounds = Intersect(pixelBounds,
                                    Bounds2i{{pb[0], pb[2]}, {pb[1], pb[3]}});
            if (pixelBounds.Area() == 0)
                Error("Degenerate \"pixelbounds\" specified.");
        }
    }
    bool cosSample = params.FindOneBool("cossample", true);
    int nSamples = params.FindOneInt("nsamples", 64);
    if (PbrtOptions.quickRender) nSamples = 1;
    return new AOIntegrator(cosSample, nSamples, camera, sampler, pixelBounds);
}

}  // namespace pbrt
