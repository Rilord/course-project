


// textures/wrinkled.cpp*
#include "textures/wrinkled.h"

namespace pbrt {

// WrinkledTexture Method Definitions
WrinkledTexture<Float> *CreateWrinkledFloatTexture(const Transform &tex2world,
                                                   const TextureParams &tp) {
    // Initialize 3D texture mapping _map_ from _tp_
    std::unique_ptr<TextureMapping3D> map(new IdentityMapping3D(tex2world));
    return new WrinkledTexture<Float>(std::move(map), tp.FindInt("octaves", 8),
                                      tp.FindFloat("roughness", .5f));
}

WrinkledTexture<Spectrum> *CreateWrinkledSpectrumTexture(
    const Transform &tex2world, const TextureParams &tp) {
    // Initialize 3D texture mapping _map_ from _tp_
    std::unique_ptr<TextureMapping3D> map(new IdentityMapping3D(tex2world));
    return new WrinkledTexture<Spectrum>(std::move(map),
                                         tp.FindInt("octaves", 8),
                                         tp.FindFloat("roughness", .5f));
}

}  // namespace pbrt
