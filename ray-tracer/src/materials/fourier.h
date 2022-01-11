

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_MATERIALS_FOURIER_H
#define PBRT_MATERIALS_FOURIER_H

// materials/fourier.h*
#include "pbrt.h"
#include "material.h"
#include "reflection.h"
#include "interpolation.h"
#include <map>

namespace pbrt {

// FourierMaterial Declarations
class FourierMaterial : public Material {
  public:
    // FourierMaterial Public Methods
    FourierMaterial(const std::string &filename,
                    const std::shared_ptr<Texture<Float>> &bump);
    void ComputeScatteringFunctions(SurfaceInteraction *si, MemoryArena &arena,
                                    TransportMode mode,
                                    bool allowMultipleLobes) const;

  private:
    // FourierMaterial Private Data
    FourierBSDFTable *bsdfTable;
    std::shared_ptr<Texture<Float>> bumpMap;
    static std::map<std::string, std::unique_ptr<FourierBSDFTable>> loadedBSDFs;
};

FourierMaterial *CreateFourierMaterial(const TextureParams &mp);

}  // namespace pbrt

#endif  // PBRT_MATERIALS_FOURIER_H
