


// textures/uv.cpp*
#include "textures/uv.h"

namespace pbrt {

// UVTexture Method Definitions
Texture<Float> *CreateUVFloatTexture(const Transform &tex2world,
                                     const TextureParams &tp) {
    return nullptr;
}

UVTexture *CreateUVSpectrumTexture(const Transform &tex2world,
                                   const TextureParams &tp) {
    // Initialize 2D texture mapping _map_ from _tp_
    std::unique_ptr<TextureMapping2D> map;
    std::string type = tp.FindString("mapping", "uv");
    if (type == "uv") {
        Float su = tp.FindFloat("uscale", 1.);
        Float sv = tp.FindFloat("vscale", 1.);
        Float du = tp.FindFloat("udelta", 0.);
        Float dv = tp.FindFloat("vdelta", 0.);
        map.reset(new UVMapping2D(su, sv, du, dv));
    } else if (type == "spherical")
        map.reset(new SphericalMapping2D(Inverse(tex2world)));
    else if (type == "cylindrical")
        map.reset(new CylindricalMapping2D(Inverse(tex2world)));
    else if (type == "planar")
        map.reset(new PlanarMapping2D(tp.FindVector3f("v1", Vector3f(1, 0, 0)),
                                      tp.FindVector3f("v2", Vector3f(0, 1, 0)),
                                      tp.FindFloat("udelta", 0.f),
                                      tp.FindFloat("vdelta", 0.f)));
    else {
        Error("2D texture mapping \"%s\" unknown", type.c_str());
        map.reset(new UVMapping2D);
    }
    return new UVTexture(std::move(map));
}

}  // namespace pbrt
