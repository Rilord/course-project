

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_TEXTURES_MARBLE_H
#define PBRT_TEXTURES_MARBLE_H

// textures/marble.h*
#include "pbrt.h"
#include "texture.h"
#include "paramset.h"

namespace pbrt {

// MarbleTexture Declarations
class MarbleTexture : public Texture<Spectrum> {
  public:
    // MarbleTexture Public Methods
    MarbleTexture(std::unique_ptr<TextureMapping3D> mapping, int octaves,
                  Float omega, Float scale, Float variation)
        : mapping(std::move(mapping)),
          octaves(octaves),
          omega(omega),
          scale(scale),
          variation(variation) {}
    Spectrum Evaluate(const SurfaceInteraction &si) const {
        Vector3f dpdx, dpdy;
        Point3f p = mapping->Map(si, &dpdx, &dpdy);
        p *= scale;
        Float marble =
            p.y +
            variation * FBm(p, scale * dpdx, scale * dpdy, omega, octaves);
        Float t = .5f + .5f * std::sin(marble);
        // Evaluate marble spline at _t_
        static Float c[][3] = {
            {.58f, .58f, .6f}, {.58f, .58f, .6f}, {.58f, .58f, .6f},
            {.5f, .5f, .5f},   {.6f, .59f, .58f}, {.58f, .58f, .6f},
            {.58f, .58f, .6f}, {.2f, .2f, .33f},  {.58f, .58f, .6f},
        };
#define NC sizeof(c) / sizeof(c[0])
#define NSEG (NC - 3)
        int first = std::min(1, int(std::floor(t * NSEG)));
        t = (t * NSEG - first);
        Spectrum c0 = Spectrum::FromRGB(c[first]);
        Spectrum c1 = Spectrum::FromRGB(c[first + 1]);
        Spectrum c2 = Spectrum::FromRGB(c[first + 2]);
        Spectrum c3 = Spectrum::FromRGB(c[first + 3]);
        // Bezier spline evaluated with de Castilejau's algorithm
        Spectrum s0 = (1.f - t) * c0 + t * c1;
        Spectrum s1 = (1.f - t) * c1 + t * c2;
        Spectrum s2 = (1.f - t) * c2 + t * c3;
        s0 = (1.f - t) * s0 + t * s1;
        s1 = (1.f - t) * s1 + t * s2;
        // Extra scale of 1.5 to increase variation among colors
        return 1.5f * ((1.f - t) * s0 + t * s1);
    }

  private:
    // MarbleTexture Private Data
    std::unique_ptr<TextureMapping3D> mapping;
    const int octaves;
    const Float omega, scale, variation;
};

Texture<Float> *CreateMarbleFloatTexture(const Transform &tex2world,
                                         const TextureParams &tp);
MarbleTexture *CreateMarbleSpectrumTexture(const Transform &tex2world,
                                           const TextureParams &tp);

}  // namespace pbrt

#endif  // PBRT_TEXTURES_MARBLE_H
