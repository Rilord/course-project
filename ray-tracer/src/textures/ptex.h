

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_TEXTURES_PTEX_H
#define PBRT_TEXTURES_PTEX_H

// textures/ptex.h*
#include "pbrt.h"
#include "texture.h"

#include <string>

namespace pbrt {

// PtexTexture Declarations
template <typename T>
class PtexTexture : public Texture<T> {
  public:
    // PtexTexture Public Methods
    PtexTexture(const std::string &filename, Float gamma);
    ~PtexTexture();
    T Evaluate(const SurfaceInteraction &) const;

  private:
    bool valid;
    const std::string filename;
    const Float gamma;
};

PtexTexture<Float> *CreatePtexFloatTexture(const Transform &tex2world,
                                           const TextureParams &tp);
PtexTexture<Spectrum> *CreatePtexSpectrumTexture(const Transform &tex2world,
                                                 const TextureParams &tp);

}  // namespace pbrt

#endif  // PBRT_TEXTURES_PTEX_H
