


// textures/fbm.cpp*
#include "textures/fbm.h"

namespace pbrt {

// FBmTexture Method Definitions
FBmTexture<Float> *CreateFBmFloatTexture(const Transform &tex2world,
                                         const TextureParams &tp) {
    // Initialize 3D texture mapping _map_ from _tp_
    std::unique_ptr<TextureMapping3D> map(new IdentityMapping3D(tex2world));
    return new FBmTexture<Float>(std::move(map), tp.FindInt("octaves", 8),
                                 tp.FindFloat("roughness", .5f));
}

FBmTexture<Spectrum> *CreateFBmSpectrumTexture(const Transform &tex2world,
                                               const TextureParams &tp) {
    // Initialize 3D texture mapping _map_ from _tp_
    std::unique_ptr<TextureMapping3D> map(new IdentityMapping3D(tex2world));
    return new FBmTexture<Spectrum>(std::move(map), tp.FindInt("octaves", 8),
                                    tp.FindFloat("roughness", .5f));
}

}  // namespace pbrt
