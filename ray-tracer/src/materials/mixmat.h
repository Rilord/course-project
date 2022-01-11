

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_MATERIALS_MIXMAT_H
#define PBRT_MATERIALS_MIXMAT_H

// materials/mixmat.h*
#include "pbrt.h"
#include "material.h"

namespace pbrt {

// MixMaterial Declarations
class MixMaterial : public Material {
  public:
    // MixMaterial Public Methods
    MixMaterial(const std::shared_ptr<Material> &m1,
                const std::shared_ptr<Material> &m2,
                const std::shared_ptr<Texture<Spectrum>> &scale)
        : m1(m1), m2(m2), scale(scale) {}
    void ComputeScatteringFunctions(SurfaceInteraction *si, MemoryArena &arena,
                                    TransportMode mode,
                                    bool allowMultipleLobes) const;

  private:
    // MixMaterial Private Data
    std::shared_ptr<Material> m1, m2;
    std::shared_ptr<Texture<Spectrum>> scale;
};

MixMaterial *CreateMixMaterial(const TextureParams &mp,
                               const std::shared_ptr<Material> &m1,
                               const std::shared_ptr<Material> &m2);

}  // namespace pbrt

#endif  // PBRT_MATERIALS_MIXMAT_H
