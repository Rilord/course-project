


// lights/goniometric.cpp*
#include "lights/goniometric.h"
#include "paramset.h"
#include "sampling.h"
#include "stats.h"

namespace pbrt {

// GonioPhotometricLight Method Definitions
Spectrum GonioPhotometricLight::Sample_Li(const Interaction &ref,
                                          const Point2f &u, Vector3f *wi,
                                          Float *pdf,
                                          VisibilityTester *vis) const {
    ProfilePhase _(Prof::LightSample);
    *wi = Normalize(pLight - ref.p);
    *pdf = 1.f;
    *vis =
        VisibilityTester(ref, Interaction(pLight, ref.time, mediumInterface));
    return I * Scale(-*wi) / DistanceSquared(pLight, ref.p);
}

Spectrum GonioPhotometricLight::Power() const {
    return 4 * Pi * I * Spectrum(mipmap ? mipmap->Lookup(Point2f(.5f, .5f), .5f)
                                        : RGBSpectrum(1.f),
                                 SpectrumType::Illuminant);
}

Float GonioPhotometricLight::Pdf_Li(const Interaction &,
                                    const Vector3f &) const {
    return 0.f;
}

Spectrum GonioPhotometricLight::Sample_Le(const Point2f &u1, const Point2f &u2,
                                          Float time, Ray *ray,
                                          Normal3f *nLight, Float *pdfPos,
                                          Float *pdfDir) const {
    ProfilePhase _(Prof::LightSample);
    *ray = Ray(pLight, UniformSampleSphere(u1), Infinity, time,
               mediumInterface.inside);
    *nLight = (Normal3f)ray->d;
    *pdfPos = 1.f;
    *pdfDir = UniformSpherePdf();
    return I * Scale(ray->d);
}

void GonioPhotometricLight::Pdf_Le(const Ray &, const Normal3f &, Float *pdfPos,
                                   Float *pdfDir) const {
    ProfilePhase _(Prof::LightPdf);
    *pdfPos = 0.f;
    *pdfDir = UniformSpherePdf();
}

std::shared_ptr<GonioPhotometricLight> CreateGoniometricLight(
    const Transform &light2world, const Medium *medium,
    const ParamSet &paramSet) {
    Spectrum I = paramSet.FindOneSpectrum("I", Spectrum(1.0));
    Spectrum sc = paramSet.FindOneSpectrum("scale", Spectrum(1.0));
    std::string texname = paramSet.FindOneFilename("mapname", "");
    return std::make_shared<GonioPhotometricLight>(light2world, medium, I * sc,
                                                   texname);
}

}  // namespace pbrt
