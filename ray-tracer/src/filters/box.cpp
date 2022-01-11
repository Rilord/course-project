


// filters/box.cpp*
#include "filters/box.h"
#include "paramset.h"

namespace pbrt {

// Box Filter Method Definitions
Float BoxFilter::Evaluate(const Point2f &p) const { return 1.; }

BoxFilter *CreateBoxFilter(const ParamSet &ps) {
    Float xw = ps.FindOneFloat("xwidth", 0.5f);
    Float yw = ps.FindOneFloat("ywidth", 0.5f);
    return new BoxFilter(Vector2f(xw, yw));
}

}  // namespace pbrt
