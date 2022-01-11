

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_TEXTURES_UV_H
#define PBRT_TEXTURES_UV_H

// textures/uv.h*
#include "pbrt.h"
#include "texture.h"
#include "paramset.h"

namespace pbrt {

// UVTexture Declarations
class UVTexture : public Texture<Spectrum> {
  public:
    // UVTexture Public Methods
    UVTexture(std::unique_ptr<TextureMapping2D> mapping)
        : mapping(std::move(mapping)) {}
    Spectrum Evaluate(const SurfaceInteraction &si) const {
        Vector2f dstdx, dstdy;
        Point2f st = mapping->Map(si, &dstdx, &dstdy);
        Float rgb[3] = {st[0] - std::floor(st[0]), st[1] - std::floor(st[1]),
                        0};
        return Spectrum::FromRGB(rgb);
    }

  private:
    std::unique_ptr<TextureMapping2D> mapping;
};

Texture<Float> *CreateUVFloatTexture(const Transform &tex2world,
                                     const TextureParams &tp);
UVTexture *CreateUVSpectrumTexture(const Transform &tex2world,
                                   const TextureParams &tp);

}  // namespace pbrt

#endif  // PBRT_TEXTURES_UV_H
