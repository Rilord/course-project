

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_TEXTURES_SCALE_H
#define PBRT_TEXTURES_SCALE_H

// textures/scale.h*
#include "pbrt.h"
#include "texture.h"
#include "paramset.h"

namespace pbrt {

// ScaleTexture Declarations
template <typename T1, typename T2>
class ScaleTexture : public Texture<T2> {
  public:
    // ScaleTexture Public Methods
    ScaleTexture(const std::shared_ptr<Texture<T1>> &tex1,
                 const std::shared_ptr<Texture<T2>> &tex2)
        : tex1(tex1), tex2(tex2) {}
    T2 Evaluate(const SurfaceInteraction &si) const {
        return tex1->Evaluate(si) * tex2->Evaluate(si);
    }

  private:
    // ScaleTexture Private Data
    std::shared_ptr<Texture<T1>> tex1;
    std::shared_ptr<Texture<T2>> tex2;
};

ScaleTexture<Float, Float> *CreateScaleFloatTexture(const Transform &tex2world,
                                                    const TextureParams &tp);
ScaleTexture<Spectrum, Spectrum> *CreateScaleSpectrumTexture(
    const Transform &tex2world, const TextureParams &tp);

}  // namespace pbrt

#endif  // PBRT_TEXTURES_SCALE_H
