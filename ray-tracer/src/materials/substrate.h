

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_MATERIALS_SUBSTRATE_H
#define PBRT_MATERIALS_SUBSTRATE_H

// materials/substrate.h*
#include "pbrt.h"
#include "material.h"

namespace pbrt {

// SubstrateMaterial Declarations
class SubstrateMaterial : public Material {
  public:
    // SubstrateMaterial Public Methods
    SubstrateMaterial(const std::shared_ptr<Texture<Spectrum>> &Kd,
                      const std::shared_ptr<Texture<Spectrum>> &Ks,
                      const std::shared_ptr<Texture<Float>> &nu,
                      const std::shared_ptr<Texture<Float>> &nv,
                      const std::shared_ptr<Texture<Float>> &bumpMap,
                      bool remapRoughness)
        : Kd(Kd),
          Ks(Ks),
          nu(nu),
          nv(nv),
          bumpMap(bumpMap),
          remapRoughness(remapRoughness) {}
    void ComputeScatteringFunctions(SurfaceInteraction *si, MemoryArena &arena,
                                    TransportMode mode,
                                    bool allowMultipleLobes) const;

  private:
    // SubstrateMaterial Private Data
    std::shared_ptr<Texture<Spectrum>> Kd, Ks;
    std::shared_ptr<Texture<Float>> nu, nv;
    std::shared_ptr<Texture<Float>> bumpMap;
    bool remapRoughness;
};

SubstrateMaterial *CreateSubstrateMaterial(const TextureParams &mp);

}  // namespace pbrt

#endif  // PBRT_MATERIALS_SUBSTRATE_H
