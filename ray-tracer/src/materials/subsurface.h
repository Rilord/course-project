

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_MATERIALS_SUBSURFACE_H
#define PBRT_MATERIALS_SUBSURFACE_H

// materials/subsurface.h*
#include "pbrt.h"
#include "material.h"
#include "reflection.h"
#include "bssrdf.h"

namespace pbrt {

// SubsurfaceMaterial Declarations
class SubsurfaceMaterial : public Material {
  public:
    // SubsurfaceMaterial Public Methods
    SubsurfaceMaterial(Float scale,
                       const std::shared_ptr<Texture<Spectrum>> &Kr,
                       const std::shared_ptr<Texture<Spectrum>> &Kt,
                       const std::shared_ptr<Texture<Spectrum>> &sigma_a,
                       const std::shared_ptr<Texture<Spectrum>> &sigma_s,
                       Float g, Float eta,
                       const std::shared_ptr<Texture<Float>> &uRoughness,
                       const std::shared_ptr<Texture<Float>> &vRoughness,
                       const std::shared_ptr<Texture<Float>> &bumpMap,
                       bool remapRoughness)
        : scale(scale),
          Kr(Kr),
          Kt(Kt),
          sigma_a(sigma_a),
          sigma_s(sigma_s),
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
    // SubsurfaceMaterial Private Data
    const Float scale;
    std::shared_ptr<Texture<Spectrum>> Kr, Kt, sigma_a, sigma_s;
    std::shared_ptr<Texture<Float>> uRoughness, vRoughness;
    std::shared_ptr<Texture<Float>> bumpMap;
    const Float eta;
    const bool remapRoughness;
    BSSRDFTable table;
};

SubsurfaceMaterial *CreateSubsurfaceMaterial(const TextureParams &mp);

}  // namespace pbrt

#endif  // PBRT_MATERIALS_SUBSURFACE_H
