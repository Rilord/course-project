


// textures/scale.cpp*
#include "textures/scale.h"

namespace pbrt {

// ScaleTexture Method Definitions
ScaleTexture<Float, Float> *CreateScaleFloatTexture(const Transform &tex2world,
                                                    const TextureParams &tp) {
    return new ScaleTexture<Float, Float>(tp.GetFloatTexture("tex1", 1.f),
                                          tp.GetFloatTexture("tex2", 1.f));
}

ScaleTexture<Spectrum, Spectrum> *CreateScaleSpectrumTexture(
    const Transform &tex2world, const TextureParams &tp) {
    return new ScaleTexture<Spectrum, Spectrum>(
        tp.GetSpectrumTexture("tex1", Spectrum(1.f)),
        tp.GetSpectrumTexture("tex2", Spectrum(1.f)));
}

}  // namespace pbrt
