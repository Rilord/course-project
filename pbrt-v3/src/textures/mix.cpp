


// textures/mix.cpp*
#include "textures/mix.h"

namespace pbrt {

// MixTexture Method Definitions
MixTexture<Float> *CreateMixFloatTexture(const Transform &tex2world,
                                         const TextureParams &tp) {
    return new MixTexture<Float>(tp.GetFloatTexture("tex1", 0.f),
                                 tp.GetFloatTexture("tex2", 1.f),
                                 tp.GetFloatTexture("amount", 0.5f));
}

MixTexture<Spectrum> *CreateMixSpectrumTexture(const Transform &tex2world,
                                               const TextureParams &tp) {
    return new MixTexture<Spectrum>(tp.GetSpectrumTexture("tex1", 0.f),
                                    tp.GetSpectrumTexture("tex2", 1.f),
                                    tp.GetFloatTexture("amount", 0.5f));
}

}  // namespace pbrt
