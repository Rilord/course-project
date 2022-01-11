

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_TEXTURES_FBM_H
#define PBRT_TEXTURES_FBM_H

// textures/fbm.h*
#include "pbrt.h"
#include "texture.h"
#include "paramset.h"

namespace pbrt {

// FBmTexture Declarations
template <typename T>
class FBmTexture : public Texture<T> {
  public:
    // FBmTexture Public Methods
    FBmTexture(std::unique_ptr<TextureMapping3D> mapping, int octaves,
               Float omega)
        : mapping(std::move(mapping)), omega(omega), octaves(octaves) {}
    T Evaluate(const SurfaceInteraction &si) const {
        Vector3f dpdx, dpdy;
        Point3f P = mapping->Map(si, &dpdx, &dpdy);
        return FBm(P, dpdx, dpdy, omega, octaves);
    }

  private:
    std::unique_ptr<TextureMapping3D> mapping;
    const Float omega;
    const int octaves;
};

FBmTexture<Float> *CreateFBmFloatTexture(const Transform &tex2world,
                                         const TextureParams &tp);
FBmTexture<Spectrum> *CreateFBmSpectrumTexture(const Transform &tex2world,
                                               const TextureParams &tp);

}  // namespace pbrt

#endif  // PBRT_TEXTURES_FBM_H
