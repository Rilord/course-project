

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_TEXTURES_WINDY_H
#define PBRT_TEXTURES_WINDY_H

// textures/windy.h*
#include "pbrt.h"
#include "texture.h"
#include "paramset.h"

namespace pbrt {

// WindyTexture Declarations
template <typename T>
class WindyTexture : public Texture<T> {
  public:
    // WindyTexture Public Methods
    WindyTexture(std::unique_ptr<TextureMapping3D> mapping)
        : mapping(std::move(mapping)) {}
    T Evaluate(const SurfaceInteraction &si) const {
        Vector3f dpdx, dpdy;
        Point3f P = mapping->Map(si, &dpdx, &dpdy);
        Float windStrength = FBm(.1f * P, .1f * dpdx, .1f * dpdy, .5, 3);
        Float waveHeight = FBm(P, dpdx, dpdy, .5, 6);
        return std::abs(windStrength) * waveHeight;
    }

  private:
    std::unique_ptr<TextureMapping3D> mapping;
};

WindyTexture<Float> *CreateWindyFloatTexture(const Transform &tex2world,
                                             const TextureParams &tp);
WindyTexture<Spectrum> *CreateWindySpectrumTexture(const Transform &tex2world,
                                                   const TextureParams &tp);

}  // namespace pbrt

#endif  // PBRT_TEXTURES_WINDY_H
