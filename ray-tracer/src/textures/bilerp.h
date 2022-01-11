

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_TEXTURES_BILERP_H
#define PBRT_TEXTURES_BILERP_H

// textures/bilerp.h*
#include "pbrt.h"
#include "texture.h"
#include "paramset.h"

namespace pbrt {

// BilerpTexture Declarations
template <typename T>
class BilerpTexture : public Texture<T> {
  public:
    // BilerpTexture Public Methods
    BilerpTexture(std::unique_ptr<TextureMapping2D> mapping, const T &v00,
                  const T &v01, const T &v10, const T &v11)
        : mapping(std::move(mapping)), v00(v00), v01(v01), v10(v10), v11(v11) {}
    T Evaluate(const SurfaceInteraction &si) const {
        Vector2f dstdx, dstdy;
        Point2f st = mapping->Map(si, &dstdx, &dstdy);
        return (1 - st[0]) * (1 - st[1]) * v00 + (1 - st[0]) * (st[1]) * v01 +
               (st[0]) * (1 - st[1]) * v10 + (st[0]) * (st[1]) * v11;
    }

  private:
    // BilerpTexture Private Data
    std::unique_ptr<TextureMapping2D> mapping;
    const T v00, v01, v10, v11;
};

BilerpTexture<Float> *CreateBilerpFloatTexture(const Transform &tex2world,
                                               const TextureParams &tp);
BilerpTexture<Spectrum> *CreateBilerpSpectrumTexture(const Transform &tex2world,
                                                     const TextureParams &tp);

}  // namespace pbrt

#endif  // PBRT_TEXTURES_BILERP_H
