


// materials/mixmat.cpp*
#include "materials/mixmat.h"
#include "materials/matte.h"
#include "spectrum.h"
#include "reflection.h"
#include "paramset.h"
#include "texture.h"
#include "interaction.h"

namespace pbrt {

// MixMaterial Method Definitions
void MixMaterial::ComputeScatteringFunctions(SurfaceInteraction *si,
                                             MemoryArena &arena,
                                             TransportMode mode,
                                             bool allowMultipleLobes) const {
    // Compute weights and original _BxDF_s for mix material
    Spectrum s1 = scale->Evaluate(*si).Clamp();
    Spectrum s2 = (Spectrum(1.f) - s1).Clamp();
    SurfaceInteraction si2 = *si;
    m1->ComputeScatteringFunctions(si, arena, mode, allowMultipleLobes);
    m2->ComputeScatteringFunctions(&si2, arena, mode, allowMultipleLobes);

    // Initialize _si->bsdf_ with weighted mixture of _BxDF_s
    int n1 = si->bsdf->NumComponents(), n2 = si2.bsdf->NumComponents();
    for (int i = 0; i < n1; ++i)
        si->bsdf->bxdfs[i] =
            ARENA_ALLOC(arena, ScaledBxDF)(si->bsdf->bxdfs[i], s1);
    for (int i = 0; i < n2; ++i)
        si->bsdf->Add(ARENA_ALLOC(arena, ScaledBxDF)(si2.bsdf->bxdfs[i], s2));
}

MixMaterial *CreateMixMaterial(const TextureParams &mp,
                               const std::shared_ptr<Material> &m1,
                               const std::shared_ptr<Material> &m2) {
    std::shared_ptr<Texture<Spectrum>> scale =
        mp.GetSpectrumTexture("amount", Spectrum(0.5f));
    return new MixMaterial(m1, m2, scale);
}

}  // namespace pbrt
