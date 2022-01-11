#ifndef PtexSeparableFilter_h
#define PtexSeparableFilter_h



#include "Ptexture.h"

PTEX_NAMESPACE_BEGIN

class PtexSeparableKernel;

class PtexSeparableFilter : public PtexFilter
{
 public:
    virtual void release() { delete this; }
    virtual void eval(float* result, int firstchan, int nchannels,
                      int faceid, float u, float v,
                      float uw1, float vw1, float uw2, float vw2,
                      float width, float blur);

 protected:
    PtexSeparableFilter(PtexTexture* tx, const PtexFilter::Options& opts ) :
        _tx(tx), _options(opts), _result(0), _weight(0),
        _firstChanOffset(0), _nchan(0), _ntxchan(_tx->numChannels()),
        _dt(tx->dataType()), _uMode(tx->uBorderMode()), _vMode(tx->vBorderMode()),
        _efm(tx->edgeFilterMode())
    {
        // if caller was compiled with older version of struct, set default for new opts
        if (_options.__structSize < (char*)&_options.noedgeblend - (char*)&_options) {
            _options.noedgeblend = 0;
        }
    }
    virtual ~PtexSeparableFilter() {}

    virtual void buildKernel(PtexSeparableKernel& k, float u, float v, float uw, float vw,
                             Res faceRes) = 0;

    void splitAndApply(PtexSeparableKernel& k, int faceid, const Ptex::FaceInfo& f);
    void applyAcrossEdge(PtexSeparableKernel& k, int faceid, const Ptex::FaceInfo& f, int eid);
    void applyToCorner(PtexSeparableKernel& k, int faceid, const Ptex::FaceInfo& f, int eid);
    void applyToCornerFace(PtexSeparableKernel& k, const Ptex::FaceInfo& f, int eid,
                           int cfaceid, const Ptex::FaceInfo& cf, int ceid);
    void apply(PtexSeparableKernel& k, int faceid, const Ptex::FaceInfo& f);

    PtexTexture* _tx;           // texture being evaluated
    Options _options;           // options
    float* _result;             // temp result
    float _weight;              // accumulated weight of data in _result
    int _firstChanOffset;       // byte offset of first channel to eval
    int _nchan;                 // number of channels to eval
    int _ntxchan;               // number of channels in texture
    DataType _dt;               // data type of texture
    BorderMode _uMode, _vMode;  // border modes (clamp,black,periodic)
    EdgeFilterMode _efm; // edge filter mode (rotate when kernel is rotated or not)
};

PTEX_NAMESPACE_END

#endif
