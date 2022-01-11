


// textures/marble.cpp*
#include "textures/marble.h"

namespace pbrt {

// MarbleTexture Method Definitions
Texture<Float> *CreateMarbleFloatTexture(const Transform &tex2world,
                                         const TextureParams &tp) {
    return nullptr;
}

MarbleTexture *CreateMarbleSpectrumTexture(const Transform &tex2world,
                                           const TextureParams &tp) {
    // Initialize 3D texture mapping _map_ from _tp_
    std::unique_ptr<TextureMapping3D> map(new IdentityMapping3D(tex2world));
    return new MarbleTexture(std::move(map), tp.FindInt("octaves", 8),
                             tp.FindFloat("roughness", .5f),
                             tp.FindFloat("scale", 1.f),
                             tp.FindFloat("variation", .2f));
}

}  // namespace pbrt
