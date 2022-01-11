


// textures/dots.cpp*
#include "textures/dots.h"

namespace pbrt {

// DotsTexture Method Definitions
DotsTexture<Float> *CreateDotsFloatTexture(const Transform &tex2world,
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
    return new DotsTexture<Float>(std::move(map),
                                  tp.GetFloatTexture("inside", 1.f),
                                  tp.GetFloatTexture("outside", 0.f));
}

DotsTexture<Spectrum> *CreateDotsSpectrumTexture(const Transform &tex2world,
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
    return new DotsTexture<Spectrum>(std::move(map),
                                     tp.GetSpectrumTexture("inside", 1.f),
                                     tp.GetSpectrumTexture("outside", 0.f));
}

}  // namespace pbrt
