


#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_MATERIALS_DISNEY_H
#define PBRT_MATERIALS_DISNEY_H

// materials/disney.h*
#include "material.h"
#include "pbrt.h"

namespace pbrt {

// DisneyMaterial Declarations
class DisneyMaterial : public Material {
  public:
    // DisneyMaterial Public Methods
    DisneyMaterial(const std::shared_ptr<Texture<Spectrum>> &color,
                   const std::shared_ptr<Texture<Float>> &metallic,
                   const std::shared_ptr<Texture<Float>> &eta,
                   const std::shared_ptr<Texture<Float>> &roughness,
                   const std::shared_ptr<Texture<Float>> &specularTint,
                   const std::shared_ptr<Texture<Float>> &anisotropic,
                   const std::shared_ptr<Texture<Float>> &sheen,
                   const std::shared_ptr<Texture<Float>> &sheenTint,
                   const std::shared_ptr<Texture<Float>> &clearcoat,
                   const std::shared_ptr<Texture<Float>> &clearcoatGloss,
                   const std::shared_ptr<Texture<Float>> &specTrans,
                   const std::shared_ptr<Texture<Spectrum>> &scatterDistance,
                   bool thin,
                   const std::shared_ptr<Texture<Float>> &flatness,
                   const std::shared_ptr<Texture<Float>> &diffTrans,
                   const std::shared_ptr<Texture<Float>> &bumpMap)
        : color(color),
          metallic(metallic),
          eta(eta),
          roughness(roughness),
          specularTint(specularTint),
          anisotropic(anisotropic),
          sheen(sheen),
          sheenTint(sheenTint),
          clearcoat(clearcoat),
          clearcoatGloss(clearcoatGloss),
          specTrans(specTrans),
          scatterDistance(scatterDistance),
          thin(thin),
          flatness(flatness),
          diffTrans(diffTrans),
          bumpMap(bumpMap) {}
    void ComputeScatteringFunctions(SurfaceInteraction *si, MemoryArena &arena,
                                    TransportMode mode,
                                    bool allowMultipleLobes) const;

  private:
    // DisneyMaterial Private Data
    std::shared_ptr<Texture<Spectrum>> color;
    std::shared_ptr<Texture<Float>> metallic, eta;
    std::shared_ptr<Texture<Float>> roughness, specularTint, anisotropic, sheen;
    std::shared_ptr<Texture<Float>> sheenTint, clearcoat, clearcoatGloss;
    std::shared_ptr<Texture<Float>> specTrans;
    std::shared_ptr<Texture<Spectrum>> scatterDistance;
    bool thin;
    std::shared_ptr<Texture<Float>> flatness, diffTrans, bumpMap;
};

DisneyMaterial *CreateDisneyMaterial(const TextureParams &mp);

}  // namespace pbrt

#endif  // PBRT_MATERIALS_DISNEY_H
