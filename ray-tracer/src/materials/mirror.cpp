


// materials/mirror.cpp*
#include "materials/mirror.h"
#include "spectrum.h"
#include "reflection.h"
#include "paramset.h"
#include "texture.h"
#include "interaction.h"

namespace pbrt {

// MirrorMaterial Method Definitions
void MirrorMaterial::ComputeScatteringFunctions(SurfaceInteraction *si,
                                                MemoryArena &arena,
                                                TransportMode mode,
                                                bool allowMultipleLobes) const {
    // Perform bump mapping with _bumpMap_, if present
    if (bumpMap) Bump(bumpMap, si);
    si->bsdf = ARENA_ALLOC(arena, BSDF)(*si);
    Spectrum R = Kr->Evaluate(*si).Clamp();
    if (!R.IsBlack())
        si->bsdf->Add(ARENA_ALLOC(arena, SpecularReflection)(
            R, ARENA_ALLOC(arena, FresnelNoOp)()));
}

MirrorMaterial *CreateMirrorMaterial(const TextureParams &mp) {
    std::shared_ptr<Texture<Spectrum>> Kr =
        mp.GetSpectrumTexture("Kr", Spectrum(0.9f));
    std::shared_ptr<Texture<Float>> bumpMap =
        mp.GetFloatTextureOrNull("bumpmap");
    return new MirrorMaterial(Kr, bumpMap);
}

}  // namespace pbrt
