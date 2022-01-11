

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_TEXTURES_WRINKLED_H
#define PBRT_TEXTURES_WRINKLED_H

// textures/wrinkled.h*
#include "pbrt.h"
#include "texture.h"
#include "paramset.h"

namespace pbrt {

// WrinkledTexture Declarations
template <typename T>
class WrinkledTexture : public Texture<T> {
  public:
    // WrinkledTexture Public Methods
    WrinkledTexture(std::unique_ptr<TextureMapping3D> mapping, int octaves,
                    Float omega)
        : mapping(std::move(mapping)), octaves(octaves), omega(omega) {}
    T Evaluate(const SurfaceInteraction &si) const {
        Vector3f dpdx, dpdy;
        Point3f p = mapping->Map(si, &dpdx, &dpdy);
        return Turbulence(p, dpdx, dpdy, omega, octaves);
    }

  private:
    // WrinkledTexture Private Data
    std::unique_ptr<TextureMapping3D> mapping;
    int octaves;
    Float omega;
};

WrinkledTexture<Float> *CreateWrinkledFloatTexture(const Transform &tex2world,
                                                   const TextureParams &tp);
WrinkledTexture<Spectrum> *CreateWrinkledSpectrumTexture(
    const Transform &tex2world, const TextureParams &tp);

}  // namespace pbrt

#endif  // PBRT_TEXTURES_WRINKLED_H
