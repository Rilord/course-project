#ifndef PtexTriangleFilter_h
#define PtexTriangleFilter_h



#include "Ptexture.h"

PTEX_NAMESPACE_BEGIN

class PtexTriangleKernel;
class PtexTriangleKernelIter;

class PtexTriangleFilter : public PtexFilter
{
 public:
    PtexTriangleFilter(PtexTexture* tx, const PtexFilter::Options& opts ) :
        _tx(tx), _options(opts), _result(0), _weight(0),
        _firstChanOffset(0), _nchan(0), _ntxchan(0),
        _dt((DataType)0) {}
    virtual void release() { delete this; }
    virtual void eval(float* result, int firstchan, int nchannels,
                      int faceid, float u, float v,
                      float uw1, float vw1, float uw2, float vw2,
                      float width, float blur);

 protected:
    void buildKernel(PtexTriangleKernel& k, float u, float v,
                     float uw1, float vw1, float uw2, float vw2,
                     float width, float blur, Res faceRes);

    void splitAndApply(PtexTriangleKernel& k, int faceid, const Ptex::FaceInfo& f);
    void applyAcrossEdge(PtexTriangleKernel& k, const Ptex::FaceInfo& f, int eid);
    void apply(PtexTriangleKernel& k, int faceid, const Ptex::FaceInfo& f);
    void applyIter(PtexTriangleKernelIter& k, PtexFaceData* dh);

    virtual ~PtexTriangleFilter() {}

    PtexTexture* _tx;           // texture being evaluated
    Options _options;           // options
    float* _result;             // temp result
    float _weight;              // accumulated weight of data in _result
    int _firstChanOffset;       // byte offset of first channel to eval
    int _nchan;                 // number of channels to eval
    int _ntxchan;               // number of channels in texture
    DataType _dt;               // data type of texture
};

PTEX_NAMESPACE_END

#endif
