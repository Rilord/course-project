

// materials/fourier.cpp*
#include "materials/fourier.h"
#include "interaction.h"
#include "paramset.h"

namespace pbrt {

std::map<std::string, std::unique_ptr<FourierBSDFTable>>
    FourierMaterial::loadedBSDFs;

// FourierMaterial Method Definitions


inline bool IsBigEndian() {
    uint32_t i = 0x01020304;
    char c[4];
    memcpy(c, &i, 4);
    return (c[0] == 1);
}

static inline uint32_t ByteSwap32(uint32_t x) {
    return ((((x)&0xFF) << 24) | (((x)&0xFF00) << 8) | (((x)&0xFF0000) >> 8) |
            (((x)&0xFF000000) >> 24));
}

bool FourierBSDFTable::Read(const std::string &filename,
                            FourierBSDFTable *bsdfTable) {
    bsdfTable->mu = bsdfTable->cdf = bsdfTable->a = nullptr;
    bsdfTable->aOffset = bsdfTable->m = nullptr;
    bsdfTable->nChannels = 0;

    FILE *f = fopen(filename.c_str(), "rb");

    if (!f) {
        Error("Unable to open tabulated BSDF file \"%s\"", filename.c_str());
        return false;
    }

    auto read32 = [&](void *target, size_t count) -> bool {
        if (fread(target, sizeof(int), count, f) != count) return false;
        if (IsBigEndian()) {
            int32_t *tmp = (int32_t *)target;
            for (size_t i = 0; i < count; ++i) {
                tmp[i] = ByteSwap32(tmp[i]);
            }
        }
        return true;
    };
    auto readfloat = [&](Float *target, size_t count) -> bool {
        if (sizeof(*target) == sizeof(float)) return read32(target, count);

        std::unique_ptr<float[]> buf(new float[count]);
        bool ret = read32(buf.get(), count);
        for (size_t i = 0; i < count; ++i) target[i] = buf[i];
        return ret;
    };

    const char header_exp[8] = {'S', 'C', 'A', 'T', 'F', 'U', 'N', '\x01'};
    char header[8];
    std::unique_ptr<int[]> offsetAndLength;

    if (fread(header, 1, 8, f) != 8 || memcmp(header, header_exp, 8) != 0)
        goto fail;

    int flags, nCoeffs, nBases, unused[4];

    if (!read32(&flags, 1) || !read32(&bsdfTable->nMu, 1) ||
        !read32(&nCoeffs, 1) || !read32(&bsdfTable->mMax, 1) ||
        !read32(&bsdfTable->nChannels, 1) || !read32(&nBases, 1) ||
        !read32(unused, 3) || !readfloat(&bsdfTable->eta, 1) ||
        !read32(&unused, 4))
        goto fail;


    if (flags != 1 ||
        (bsdfTable->nChannels != 1 && bsdfTable->nChannels != 3) || nBases != 1)
        goto fail;

    bsdfTable->mu = new Float[bsdfTable->nMu];
    bsdfTable->cdf = new Float[bsdfTable->nMu * bsdfTable->nMu];
    bsdfTable->a0 = new Float[bsdfTable->nMu * bsdfTable->nMu];
    offsetAndLength.reset(new int[bsdfTable->nMu * bsdfTable->nMu * 2]);
    bsdfTable->aOffset = new int[bsdfTable->nMu * bsdfTable->nMu];
    bsdfTable->m = new int[bsdfTable->nMu * bsdfTable->nMu];
    bsdfTable->a = new Float[nCoeffs];

    if (!readfloat(bsdfTable->mu, bsdfTable->nMu) ||
        !readfloat(bsdfTable->cdf, bsdfTable->nMu * bsdfTable->nMu) ||
        !read32(offsetAndLength.get(), bsdfTable->nMu * bsdfTable->nMu * 2) ||
        !readfloat(bsdfTable->a, nCoeffs))
        goto fail;

    for (int i = 0; i < bsdfTable->nMu * bsdfTable->nMu; ++i) {
        int offset = offsetAndLength[2 * i],
            length = offsetAndLength[2 * i + 1];

        bsdfTable->aOffset[i] = offset;
        bsdfTable->m[i] = length;

        bsdfTable->a0[i] = length > 0 ? bsdfTable->a[offset] : (Float)0;
    }

    bsdfTable->recip = new Float[bsdfTable->mMax];
    for (int i = 0; i < bsdfTable->mMax; ++i)
        bsdfTable->recip[i] = 1 / (Float)i;

    fclose(f);
    return true;
fail:
    bsdfTable->nChannels = 0;
    fclose(f);
    Error(
        "Tabulated BSDF file \"%s\" has an incompatible file format or "
        "version.",
        filename.c_str());
    return false;
}

FourierMaterial::FourierMaterial(const std::string &filename,
                                 const std::shared_ptr<Texture<Float>> &bumpMap)
    : bumpMap(bumpMap) {
    if (loadedBSDFs.find(filename) == loadedBSDFs.end()) {
        std::unique_ptr<FourierBSDFTable> table(new FourierBSDFTable);
        FourierBSDFTable::Read(filename, table.get());
        loadedBSDFs[filename] = std::move(table);
    }
    bsdfTable = loadedBSDFs[filename].get();
}

void FourierMaterial::ComputeScatteringFunctions(
    SurfaceInteraction *si, MemoryArena &arena, TransportMode mode,
    bool allowMultipleLobes) const {
    // Perform bump mapping with _bumpMap_, if present
    if (bumpMap) Bump(bumpMap, si);
    si->bsdf = ARENA_ALLOC(arena, BSDF)(*si);
    // Checking for zero channels works as a proxy for checking whether the
    // table was successfully read from the file.
    if (bsdfTable->nChannels > 0)
        si->bsdf->Add(ARENA_ALLOC(arena, FourierBSDF)(*bsdfTable, mode));
}

FourierMaterial *CreateFourierMaterial(const TextureParams &mp) {
    std::shared_ptr<Texture<Float>> bumpMap =
        mp.GetFloatTextureOrNull("bumpmap");
    return new FourierMaterial(mp.FindFilename("bsdffile"), bumpMap);
}

}  // namespace pbrt
