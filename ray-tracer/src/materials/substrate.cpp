


// materials/substrate.cpp*
#include "materials/substrate.h"
#include "spectrum.h"
#include "reflection.h"
#include "paramset.h"
#include "texture.h"
#include "interaction.h"

namespace pbrt {

// SubstrateMaterial Method Definitions
void SubstrateMaterial::ComputeScatteringFunctions(
    SurfaceInteraction *si, MemoryArena &arena, TransportMode mode,
    bool allowMultipleLobes) const {
    // Perform bump mapping with _bumpMap_, if present
    if (bumpMap) Bump(bumpMap, si);
    si->bsdf = ARENA_ALLOC(arena, BSDF)(*si);
    Spectrum d = Kd->Evaluate(*si).Clamp();
    Spectrum s = Ks->Evaluate(*si).Clamp();
    Float roughu = nu->Evaluate(*si);
    Float roughv = nv->Evaluate(*si);

    if (!d.IsBlack() || !s.IsBlack()) {
        if (remapRoughness) {
            roughu = TrowbridgeReitzDistribution::RoughnessToAlpha(roughu);
            roughv = TrowbridgeReitzDistribution::RoughnessToAlpha(roughv);
        }
        MicrofacetDistribution *distrib =
            ARENA_ALLOC(arena, TrowbridgeReitzDistribution)(roughu, roughv);
        si->bsdf->Add(ARENA_ALLOC(arena, FresnelBlend)(d, s, distrib));
    }
}

SubstrateMaterial *CreateSubstrateMaterial(const TextureParams &mp) {
    std::shared_ptr<Texture<Spectrum>> Kd =
        mp.GetSpectrumTexture("Kd", Spectrum(.5f));
    std::shared_ptr<Texture<Spectrum>> Ks =
        mp.GetSpectrumTexture("Ks", Spectrum(.5f));
    std::shared_ptr<Texture<Float>> uroughness =
        mp.GetFloatTexture("uroughness", .1f);
    std::shared_ptr<Texture<Float>> vroughness =
        mp.GetFloatTexture("vroughness", .1f);
    std::shared_ptr<Texture<Float>> bumpMap =
        mp.GetFloatTextureOrNull("bumpmap");
    bool remapRoughness = mp.FindBool("remaproughness", true);
    return new SubstrateMaterial(Kd, Ks, uroughness, vroughness, bumpMap,
                                 remapRoughness);
}

}  // namespace pbrt
