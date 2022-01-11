

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_MATERIALS_KDSUBSURFACE_H
#define PBRT_MATERIALS_KDSUBSURFACE_H

// materials/kdsubsurface.h*
#include "pbrt.h"
#include "reflection.h"
#include "material.h"
#include "bssrdf.h"

namespace pbrt {

// KdSubsurfaceMaterial Declarations
class KdSubsurfaceMaterial : public Material {
  public:
    // KdSubsurfaceMaterial Public Methods
    KdSubsurfaceMaterial(Float scale,
                         const std::shared_ptr<Texture<Spectrum>> &Kd,
                         const std::shared_ptr<Texture<Spectrum>> &Kr,
                         const std::shared_ptr<Texture<Spectrum>> &Kt,
                         const std::shared_ptr<Texture<Spectrum>> &mfp, Float g,
                         Float eta,
                         const std::shared_ptr<Texture<Float>> &uRoughness,
                         const std::shared_ptr<Texture<Float>> &vRoughness,
                         const std::shared_ptr<Texture<Float>> &bumpMap,
                         bool remapRoughness)
        : scale(scale),
          Kd(Kd),
          Kr(Kr),
          Kt(Kt),
          mfp(mfp),
          uRoughness(uRoughness),
          vRoughness(vRoughness),
          bumpMap(bumpMap),
          eta(eta),
          remapRoughness(remapRoughness),
          table(100, 64) {
        ComputeBeamDiffusionBSSRDF(g, eta, &table);
    }
    void ComputeScatteringFunctions(SurfaceInteraction *si, MemoryArena &arena,
                                    TransportMode mode,
                                    bool allowMultipleLobes) const;

  private:
    // KdSubsurfaceMaterial Private Data
    Float scale;
    std::shared_ptr<Texture<Spectrum>> Kd, Kr, Kt, mfp;
    std::shared_ptr<Texture<Float>> uRoughness, vRoughness;
    std::shared_ptr<Texture<Float>> bumpMap;
    Float eta;
    bool remapRoughness;
    BSSRDFTable table;
};

KdSubsurfaceMaterial *CreateKdSubsurfaceMaterial(const TextureParams &mp);

}  // namespace pbrt

#endif  // PBRT_MATERIALS_KDSUBSURFACE_H
