

#include "PtexPlatform.h"
#include "PtexUtils.h"
#include "PtexHalf.h"
#include "PtexTriangleKernel.h"

PTEX_NAMESPACE_BEGIN

namespace {
    inline float gaussian(float x_squared)
    {
        static const float scale = -0.5f * (PtexTriangleKernelWidth * PtexTriangleKernelWidth);
        return (float)exp(scale * x_squared);
    }

    // apply to 1..4 channels (unrolled channel loop) of packed data (nTxChan==nChan)
    // the ellipse equation, Q, is calculated via finite differences (Heckbert '89 pg 57)
    template<class T, int nChan>
    void Apply(PtexTriangleKernelIter& k, float* result, void* data, int , int )
    {
        int nTxChan = nChan;
        float DDQ = 2.0f*k.A;
        for (int vi = k.v1; vi != k.v2; vi++) {
            int xw = k.rowlen - vi;
            int x1 = PtexUtils::max(k.u1, xw-k.w2);
            int x2 = PtexUtils::min(k.u2, xw-k.w1);
            float U = (float)x1 - k.u;
            float V = (float)vi - k.v;
            float DQ = k.A*(2.0f*U+1.0f)+k.B*V;
            float Q = k.A*U*U + (k.B*U + k.C*V)*V;
            T* p = static_cast<T*>(data) + (vi * k.rowlen + x1) * nTxChan;
            T* pEnd = p + (x2-x1)*nTxChan;
            for (; p < pEnd; p += nTxChan) {
                if (Q < 1.0f) {
                    float weight = gaussian(Q)*k.wscale;
                    k.weight += weight;
                    PtexUtils::VecAccum<T,nChan>()(result, p, weight);
                }
                Q += DQ;
                DQ += DDQ;
            }
        }
    }

    // apply to 1..4 channels (unrolled channel loop) w/ pixel stride
    template<class T, int nChan>
    void ApplyS(PtexTriangleKernelIter& k, float* result, void* data, int , int nTxChan)
    {
        float DDQ = 2.0f*k.A;
        for (int vi = k.v1; vi != k.v2; vi++) {
            int xw = k.rowlen - vi;
            int x1 = PtexUtils::max(k.u1, xw-k.w2);
            int x2 = PtexUtils::min(k.u2, xw-k.w1);
            float U = (float)x1 - k.u;
            float V = (float)vi - k.v;
            float DQ = k.A*(2.0f*U+1.0f)+k.B*V;
            float Q = k.A*U*U + (k.B*U + k.C*V)*V;
            T* p = static_cast<T*>(data) + (vi * k.rowlen + x1) * nTxChan;
            T* pEnd = p + (x2-x1)*nTxChan;
            for (; p < pEnd; p += nTxChan) {
                if (Q < 1.0f) {
                    float weight = gaussian(Q)*k.wscale;
                    k.weight += weight;
                    PtexUtils::VecAccum<T,nChan>()(result, p, weight);
                }
                Q += DQ;
                DQ += DDQ;
            }
        }
    }

    // apply to N channels (general case)
    template<class T>
    void ApplyN(PtexTriangleKernelIter& k, float* result, void* data, int nChan, int nTxChan)
    {
        float DDQ = 2.0f*k.A;
        for (int vi = k.v1; vi != k.v2; vi++) {
            int xw = k.rowlen - vi;
            int x1 = PtexUtils::max(k.u1, xw-k.w2);
            int x2 = PtexUtils::min(k.u2, xw-k.w1);
            float U = (float)x1 - k.u;
            float V = (float)vi - k.v;
            float DQ = k.A*(2.0f*U+1.0f)+k.B*V;
            float Q = k.A*U*U + (k.B*U + k.C*V)*V;
            T* p = static_cast<T*>(data) + (vi * k.rowlen + x1) * nTxChan;
            T* pEnd = p + (x2-x1)*nTxChan;
            for (; p < pEnd; p += nTxChan) {
                if (Q < 1.0f) {
                    float weight = gaussian(Q)*k.wscale;
                    k.weight += weight;
                    PtexUtils::VecAccumN<T>()(result, p, nChan, weight);
                }
                Q += DQ;
                DQ += DDQ;
            }
        }
    }
}


PtexTriangleKernelIter::ApplyFn
PtexTriangleKernelIter::applyFunctions[] = {
    // nChan == nTxChan
    ApplyN<uint8_t>,  ApplyN<uint16_t>,  ApplyN<PtexHalf>,  ApplyN<float>,
    Apply<uint8_t,1>, Apply<uint16_t,1>, Apply<PtexHalf,1>, Apply<float,1>,
    Apply<uint8_t,2>, Apply<uint16_t,2>, Apply<PtexHalf,2>, Apply<float,2>,
    Apply<uint8_t,3>, Apply<uint16_t,3>, Apply<PtexHalf,3>, Apply<float,3>,
    Apply<uint8_t,4>, Apply<uint16_t,4>, Apply<PtexHalf,4>, Apply<float,4>,

    // nChan != nTxChan (need pixel stride)
    ApplyN<uint8_t>,   ApplyN<uint16_t>,   ApplyN<PtexHalf>,   ApplyN<float>,
    ApplyS<uint8_t,1>, ApplyS<uint16_t,1>, ApplyS<PtexHalf,1>, ApplyS<float,1>,
    ApplyS<uint8_t,2>, ApplyS<uint16_t,2>, ApplyS<PtexHalf,2>, ApplyS<float,2>,
    ApplyS<uint8_t,3>, ApplyS<uint16_t,3>, ApplyS<PtexHalf,3>, ApplyS<float,3>,
    ApplyS<uint8_t,4>, ApplyS<uint16_t,4>, ApplyS<PtexHalf,4>, ApplyS<float,4>,
};


void PtexTriangleKernelIter::applyConst(float* dst, void* data, DataType dt, int nChan)
{
    // iterate over texel locations and calculate weight as if texture weren't const
    float DDQ = 2.0f*A;
    for (int vi = v1; vi != v2; vi++) {
        int xw = rowlen - vi;
        int x1 = PtexUtils::max(u1, xw-w2);
        int x2 = PtexUtils::min(u2, xw-w1);
        float U = (float)x1 - u;
        float V = (float)vi - v;
        float DQ = A*(2.0f*U+1.0f)+B*V;
        float Q = A*U*U + (B*U + C*V)*V;
        for (int x = x1; x < x2; x++) {
            if (Q < 1.0f) {
                weight += gaussian(Q)*wscale;
            }
            Q += DQ;
            DQ += DDQ;
        }
    }

    // apply weight to single texel value
    PtexUtils::applyConst(weight, dst, data, dt, nChan);
}

PTEX_NAMESPACE_END
