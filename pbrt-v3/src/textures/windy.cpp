


// textures/windy.cpp*
#include "textures/windy.h"

namespace pbrt {

// WindyTexture Method Definitions
WindyTexture<Float> *CreateWindyFloatTexture(const Transform &tex2world,
                                             const TextureParams &tp) {
    // Initialize 3D texture mapping _map_ from _tp_
    std::unique_ptr<TextureMapping3D> map(new IdentityMapping3D(tex2world));
    return new WindyTexture<Float>(std::move(map));
}

WindyTexture<Spectrum> *CreateWindySpectrumTexture(const Transform &tex2world,
                                                   const TextureParams &tp) {
    // Initialize 3D texture mapping _map_ from _tp_
    std::unique_ptr<TextureMapping3D> map(new IdentityMapping3D(tex2world));
    return new WindyTexture<Spectrum>(std::move(map));
}

}  // namespace pbrt
