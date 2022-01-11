

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_TEXTURES_MIX_H
#define PBRT_TEXTURES_MIX_H

// textures/mix.h*
#include "pbrt.h"
#include "texture.h"
#include "paramset.h"

namespace pbrt {

// MixTexture Declarations
template <typename T>
class MixTexture : public Texture<T> {
  public:
    // MixTexture Public Methods
    MixTexture(const std::shared_ptr<Texture<T>> &tex1,
               const std::shared_ptr<Texture<T>> &tex2,
               const std::shared_ptr<Texture<Float>> &amount)
        : tex1(tex1), tex2(tex2), amount(amount) {}
    T Evaluate(const SurfaceInteraction &si) const {
        T t1 = tex1->Evaluate(si), t2 = tex2->Evaluate(si);
        Float amt = amount->Evaluate(si);
        return (1 - amt) * t1 + amt * t2;
    }

  private:
    std::shared_ptr<Texture<T>> tex1, tex2;
    std::shared_ptr<Texture<Float>> amount;
};

MixTexture<Float> *CreateMixFloatTexture(const Transform &tex2world,
                                         const TextureParams &tp);
MixTexture<Spectrum> *CreateMixSpectrumTexture(const Transform &tex2world,
                                               const TextureParams &tp);

}  // namespace pbrt

#endif  // PBRT_TEXTURES_MIX_H
