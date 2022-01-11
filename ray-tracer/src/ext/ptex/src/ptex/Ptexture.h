#ifndef Ptexture_h
#define Ptexture_h





#if defined(_WIN32) || defined(_WINDOWS) || defined(_MSC_VER)
# ifndef PTEXAPI
#  ifndef PTEX_STATIC
#    ifdef PTEX_EXPORTS
#       define PTEXAPI __declspec(dllexport)
#    else
#       define PTEXAPI __declspec(dllimport)
#    endif
#  else
#    define PTEXAPI
#  endif
# endif
#else
#  ifndef PTEXAPI
#    define PTEXAPI
#  endif
#  ifndef DOXYGEN
#    define PTEX_USE_STDSTRING
#  endif
#endif

#include "PtexInt.h"
#include <ostream>

#include "PtexVersion.h"
#ifdef DOXYGEN

namespace Ptex {
#else
PTEX_NAMESPACE_BEGIN
#endif


enum MeshType {
     mt_triangle,		///< Mesh is triangle-based.
     mt_quad			///< Mesh is quad-based.
 };


enum DataType {
    dt_uint8,		///< Unsigned, 8-bit integer.
    dt_uint16,		///< Unsigned, 16-bit integer.
    dt_half,		///< Half-precision (16-bit) floating point.
    dt_float		///< Single-precision (32-bit) floating point.
};


enum EdgeFilterMode {
    efm_none,		///< Don't do anything with the values.
    efm_tanvec		///< Values are vectors in tangent space; rotate values.
};


enum BorderMode {
    m_clamp,		///< texel access is clamped to border
    m_black,		///< texel beyond border are assumed to be black
    m_periodic		///< texel access wraps to other side of face
};


enum EdgeId {
    e_bottom,		///< Bottom edge, from UV (0,0) to (1,0)
    e_right,		///< Right edge, from UV (1,0) to (1,1)
    e_top,			///< Top edge, from UV (1,1) to (0,1)
    e_left			///< Left edge, from UV (0,1) to (0,0)
};


enum MetaDataType {
    mdt_string,		///< Null-terminated string.
    mdt_int8,		///< Signed 8-bit integer.
    mdt_int16,		///< Signed 16-bit integer.
    mdt_int32,		///< Signed 32-bit integer.
    mdt_float,		///< Single-precision (32-bit) floating point.
    mdt_double		///< Double-precision (32-bit) floating point.
};


PTEXAPI const char* MeshTypeName(MeshType mt);


PTEXAPI const char* DataTypeName(DataType dt);


PTEXAPI const char* BorderModeName(BorderMode m);


PTEXAPI const char* EdgeFilterModeName(EdgeFilterMode m);


PTEXAPI const char* EdgeIdName(EdgeId eid);


PTEXAPI const char* MetaDataTypeName(MetaDataType mdt);


inline int DataSize(DataType dt) {
    static const int sizes[] = { 1,2,2,4 };
    return sizes[dt];
}


inline float OneValue(DataType dt) {
    static const float one[] = { 255.f, 65535.f, 1.f, 1.f };
    return one[dt];
}


inline float OneValueInv(DataType dt) {
    static const float one[] = { 1.f/255.f, 1.f/65535.f, 1.f, 1.f };
    return one[dt];
}


PTEXAPI void ConvertToFloat(float* dst, const void* src,
                            Ptex::DataType dt, int numChannels);


PTEXAPI void ConvertFromFloat(void* dst, const float* src,
                              Ptex::DataType dt, int numChannels);


struct Res {
    int8_t ulog2;		///< log base 2 of u resolution, in texels
    int8_t vlog2;		///< log base 2 of v resolution, in texels

    /// Default constructor, sets res to 0 (1x1 texel).
    Res() : ulog2(0), vlog2(0) {}

    /// Constructor.
    Res(int8_t ulog2_, int8_t vlog2_) : ulog2(ulog2_), vlog2(vlog2_) {}

    /// Constructor.
    Res(uint16_t value) : ulog2(int8_t(value&0xff)), vlog2(int8_t((value>>8)&0xff)) {}

    /// U resolution in texels.
    int u() const { return 1<<(unsigned)ulog2; }

    /// V resolution in texels.
    int v() const { return 1<<(unsigned)vlog2; }

    /// Resolution as a single 16-bit integer value.
    uint16_t val() const { return uint16_t(ulog2 | (vlog2<<8)); }

    /// Total size of specified texture in texels (u * v).
    int size() const { return u() * v(); }

    /// Comparison operator.
    bool operator==(const Res& r) const { return r.ulog2 == ulog2 && r.vlog2 == vlog2; }

    /// Comparison operator.
    bool operator!=(const Res& r) const { return !(r==*this); }

    /// True if res is >= given res in both u and v directions.
    bool operator>=(const Res& r) const { return ulog2 >= r.ulog2 && vlog2 >= r.vlog2; }

    /// Get value of resolution with u and v swapped.
    Res swappeduv() const { return Res(vlog2, ulog2); }

    /// Swap the u and v resolution values in place.
    void swapuv() { *this = swappeduv(); }

    /// Clamp the resolution value against the given value.
    void clamp(const Res& r) {
        if (ulog2 > r.ulog2) ulog2 = r.ulog2;
        if (vlog2 > r.vlog2) vlog2 = r.vlog2;
    }

    /// Determine the number of tiles in the u direction for the given tile res.
    int ntilesu(Res tileres) const { return 1<<(ulog2-tileres.ulog2); }

    /// Determine the number of tiles in the v direction for the given tile res.
    int ntilesv(Res tileres) const { return 1<<(vlog2-tileres.vlog2); }

    /// Determine the total number of tiles for the given tile res.
    int ntiles(Res tileres) const { return ntilesu(tileres) * ntilesv(tileres); }
};


struct FaceInfo {
    Res res;		///< Resolution of face.
    uint8_t adjedges;       ///< Adjacent edges, 2 bits per edge.
    uint8_t flags;		///< Flags.
    int32_t adjfaces[4];	///< Adjacent faces (-1 == no adjacent face).

    /// Default constructor
    FaceInfo() : res(), adjedges(0), flags(0)
    {
        adjfaces[0] = adjfaces[1] = adjfaces[2] = adjfaces[3] = -1;
    }

    /// Constructor.
    FaceInfo(Res res_) : res(res_), adjedges(0), flags(0)
    {
        adjfaces[0] = adjfaces[1] = adjfaces[2] = adjfaces[3] = -1;
    }

    /// Constructor.
    FaceInfo(Res res_, int adjfaces_[4], int adjedges_[4], bool isSubface_=false)
        : res(res_), flags(isSubface_ ? flag_subface : 0)
    {
        setadjfaces(adjfaces_[0], adjfaces_[1], adjfaces_[2], adjfaces_[3]);
        setadjedges(adjedges_[0], adjedges_[1], adjedges_[2], adjedges_[3]);
    }

    /// Access an adjacent edge id.  The eid value must be 0..3.
    EdgeId adjedge(int eid) const { return EdgeId((adjedges >> (2*eid)) & 3); }

    /// Access an adjacent face id.  The eid value must be 0..3.
    int adjface(int eid) const { return adjfaces[eid]; }

    /// Determine if face is constant (by checking a flag).
    bool isConstant() const { return (flags & flag_constant) != 0; }

    /// Determine if neighborhood of face is constant (by checking a flag).
    bool isNeighborhoodConstant() const { return (flags & flag_nbconstant) != 0; }

    /// Determine if face has edits in the file (by checking a flag).
    bool hasEdits() const { return (flags & flag_hasedits) != 0; }

    /// Determine if face is a subface (by checking a flag).
    bool isSubface() const { return (flags & flag_subface) != 0; }

    /// Set the adjfaces data.
    void setadjfaces(int f0, int f1, int f2, int f3)
    { adjfaces[0] = f0, adjfaces[1] = f1, adjfaces[2] = f2; adjfaces[3] = f3; }

    /// Set the adjedges data.
    void setadjedges(int e0, int e1, int e2, int e3)
    { adjedges = (uint8_t)((e0&3) | ((e1&3)<<2) | ((e2&3)<<4) | ((e3&3)<<6)); }

    /// Flag bit values (for internal use).
    enum { flag_constant = 1, flag_hasedits = 2, flag_nbconstant = 4, flag_subface = 8 };
};



#ifdef PTEX_USE_STDSTRING
typedef std::string String;
#else
class String
{
public:
    String() : _str(0) {}
    String(const String& str) : _str(0) { *this = str; }
    PTEXAPI ~String();
    PTEXAPI String& operator=(const char* str);
    String& operator=(const String& str) { *this = str._str; return *this; }
    String& operator=(const std::string& str) { *this = str.c_str(); return *this; }
    const char* c_str() const { return _str ? _str : ""; }
    bool empty() const { return _str == 0 || _str[0] == '\0'; }

private:
    char* _str;
};
#endif

/// std::stream output operator.  \relates Ptex::String
#ifndef PTEX_USE_STDSTRING
std::ostream& operator << (std::ostream& stream, const Ptex::String& str);
#endif


#ifdef DOXYGEN
} // end namespace Ptex
#endif


class PtexMetaData {
 protected:
    /// Destructor not for public use.  Use release() instead.
    virtual ~PtexMetaData() {}

 public:
    /// Release resources held by this pointer (pointer becomes invalid).
    virtual void release() = 0;

    /// Query number of meta data entries stored in file.
    virtual int numKeys() = 0;

    /// Query the name and type of a meta data entry.
    virtual void getKey(int index, const char*& key, Ptex::MetaDataType& type) = 0;

    /// Query the index and type of a meta data entry by name.
    virtual bool findKey(const char* key, int& index, Ptex::MetaDataType& type) = 0;


    virtual void getValue(const char* key, const char*& value) = 0;


    virtual void getValue(int index, const char*& value) = 0;


    virtual void getValue(const char* key, const int8_t*& value, int& count) = 0;


    virtual void getValue(int index, const int8_t*& value, int& count) = 0;


    virtual void getValue(const char* key, const int16_t*& value, int& count) = 0;


    virtual void getValue(int index, const int16_t*& value, int& count) = 0;


    virtual void getValue(const char* key, const int32_t*& value, int& count) = 0;


    virtual void getValue(int index, const int32_t*& value, int& count) = 0;


    virtual void getValue(const char* key, const float*& value, int& count) = 0;


    virtual void getValue(int index, const float*& value, int& count) = 0;


    virtual void getValue(const char* key, const double*& value, int& count) = 0;


    virtual void getValue(int index, const double*& value, int& count) = 0;
};



class PtexFaceData {
 protected:
    /// Destructor not for public use.  Use release() instead.
    virtual ~PtexFaceData() {}

 public:
    /// Release resources held by this pointer (pointer becomes invalid).
    virtual void release() = 0;


    virtual bool isConstant() = 0;


    virtual Ptex::Res res() = 0;


    virtual void getPixel(int u, int v, void* result) = 0;


    virtual void* getData() = 0;


    virtual bool isTiled() = 0;


    virtual Ptex::Res tileRes() = 0;


    virtual PtexFaceData* getTile(int tile) = 0;
};



class PtexTexture {
 protected:
    /// Destructor not for public use.  Use release() instead.
    virtual ~PtexTexture() {}

 public:

    PTEXAPI static PtexTexture* open(const char* path, Ptex::String& error, bool premultiply=0);


    /// Release resources held by this pointer (pointer becomes invalid).
    virtual void release() = 0;


    virtual const char* path() = 0;


    struct Info {
        MeshType meshType;
        DataType dataType;
        BorderMode uBorderMode;
        BorderMode vBorderMode;
        EdgeFilterMode edgeFilterMode;
        int alphaChannel;
        int numChannels;
        int numFaces;
    };
    virtual Info getInfo() = 0;


    virtual Ptex::MeshType meshType() = 0;


    virtual Ptex::DataType dataType() = 0;


    virtual Ptex::BorderMode uBorderMode() = 0;


    virtual Ptex::BorderMode vBorderMode() = 0;


    virtual Ptex::EdgeFilterMode edgeFilterMode() = 0;


    virtual int alphaChannel() = 0;


    virtual int numChannels() = 0;


    virtual int numFaces() = 0;


    virtual bool hasEdits() = 0;


    virtual bool hasMipMaps() = 0;


    virtual PtexMetaData* getMetaData() = 0;


    virtual const Ptex::FaceInfo& getFaceInfo(int faceid) = 0;


    virtual void getData(int faceid, void* buffer, int stride) = 0;


    virtual void getData(int faceid, void* buffer, int stride, Ptex::Res res) = 0;


    virtual PtexFaceData* getData(int faceid) = 0;


    virtual PtexFaceData* getData(int faceid, Ptex::Res res) = 0;


    virtual void getPixel(int faceid, int u, int v,
                          float* result, int firstchan, int nchannels) = 0;


    virtual void getPixel(int faceid, int u, int v,
                          float* result, int firstchan, int nchannels,
                          Ptex::Res res) = 0;
};



class PtexInputHandler {
 protected:
    virtual ~PtexInputHandler() {}

 public:
    typedef void* Handle;


    virtual Handle open(const char* path) = 0;


    virtual void seek(Handle handle, int64_t pos) = 0;


    virtual size_t read(void* buffer, size_t size, Handle handle) = 0;


    virtual bool close(Handle handle) = 0;


    virtual const char* lastError() = 0;
};



class PtexErrorHandler {
 protected:
    virtual ~PtexErrorHandler() {}

 public:
    virtual void reportError(const char* error) = 0;
};




class PtexCache {
 protected:
    /// Destructor not for public use.  Use release() instead.
    virtual ~PtexCache() {}

 public:

    PTEXAPI static PtexCache* create(int maxFiles,
                                     size_t maxMem,
                                     bool premultiply=false,
                                     PtexInputHandler* inputHandler=0,
                                     PtexErrorHandler* errorHandler=0);

    /// Release PtexCache.  Cache will be immediately destroyed and all resources will be released.
    virtual void release() = 0;


    virtual void setSearchPath(const char* path) = 0;


    virtual const char* getSearchPath() = 0;


    virtual PtexTexture* get(const char* path, Ptex::String& error) = 0;


    virtual void purge(PtexTexture* texture) = 0;


    virtual void purge(const char* path) = 0;


    virtual void purgeAll() = 0;

    struct Stats {
        uint64_t memUsed;
        uint64_t peakMemUsed;
        uint64_t filesOpen;
        uint64_t peakFilesOpen;
        uint64_t filesAccessed;
        uint64_t fileReopens;
        uint64_t blockReads;
    };


    virtual void getStats(Stats& stats) = 0;
};




class PtexWriter {
 protected:
    /// Destructor not for public use.  Use release() instead.
    virtual ~PtexWriter() {}

 public:

    PTEXAPI
    static PtexWriter* open(const char* path,
                            Ptex::MeshType mt, Ptex::DataType dt,
                            int nchannels, int alphachan, int nfaces,
                            Ptex::String& error, bool genmipmaps=true);


    PTEXAPI
    static PtexWriter* edit(const char* path, bool incremental,
                            Ptex::MeshType mt, Ptex::DataType dt,
                            int nchannels, int alphachan, int nfaces,
                            Ptex::String& error, bool genmipmaps=true);


    PTEXAPI
    static bool applyEdits(const char* path, Ptex::String& error);


    virtual void release() = 0;


    virtual void setBorderModes(Ptex::BorderMode uBorderMode, Ptex::BorderMode vBorderMode) = 0;


    virtual void setEdgeFilterMode(Ptex::EdgeFilterMode edgeFilterMode) = 0;


    virtual void writeMeta(const char* key, const char* string) = 0;


    virtual void writeMeta(const char* key, const int8_t* value, int count) = 0;


    virtual void writeMeta(const char* key, const int16_t* value, int count) = 0;


    virtual void writeMeta(const char* key, const int32_t* value, int count) = 0;


    virtual void writeMeta(const char* key, const float* value, int count) = 0;


    virtual void writeMeta(const char* key, const double* value, int count) = 0;


    virtual void writeMeta(PtexMetaData* data) = 0;


    virtual bool writeFace(int faceid, const Ptex::FaceInfo& info, const void* data, int stride=0) = 0;


    virtual bool writeConstantFace(int faceid, const Ptex::FaceInfo& info, const void* data) = 0;


    virtual bool close(Ptex::String& error) = 0;

#if NEW_API
    virtual bool writeFaceReduction(int faceid, const Ptex::Res& res, const void* data, int stride=0) = 0;
    virtual bool writeConstantFaceReduction(int faceid, const Ptex::Res& res, const void* data) = 0;
#endif
};



class PtexFilter {
 protected:
    /// Destructor not for public use.  Use release() instead.
    virtual ~PtexFilter() {}

 public:
    /// Filter types
    enum FilterType {
        f_point,                ///< Point-sampled (no filtering)
        f_bilinear,             ///< Bi-linear interpolation
        f_box,                  ///< Box filter
        f_gaussian,             ///< Gaussian filter
        f_bicubic,              ///< General bi-cubic filter (uses sharpness option)
        f_bspline,              ///< BSpline (equivalent to bi-cubic w/ sharpness=0)
        f_catmullrom,           ///< Catmull-Rom (equivalent to bi-cubic w/ sharpness=1)
        f_mitchell              ///< Mitchell (equivalent to bi-cubic w/ sharpness=2/3)
    };

    /// Choose filter options
    struct Options {
        int __structSize;       ///< (for internal use only)
        FilterType filter;      ///< Filter type.
        bool lerp;              ///< Interpolate between mipmap levels.
        float sharpness;        ///< Filter sharpness, 0..1 (for general bi-cubic filter only).
        bool noedgeblend;       ///< Disable cross-face filtering.  Useful for debugging or rendering on polys.

        /// Constructor - sets defaults
        Options(FilterType filter_=f_box, bool lerp_=0, float sharpness_=0, bool noedgeblend_=0) :
            __structSize(sizeof(Options)),
            filter(filter_), lerp(lerp_), sharpness(sharpness_), noedgeblend(noedgeblend_) {}
    };


    PTEXAPI static PtexFilter* getFilter(PtexTexture* tx, const Options& opts);


    virtual void release() = 0;


    virtual void eval(float* result, int firstchan, int nchannels,
                      int faceid, float u, float v, float uw1, float vw1, float uw2, float vw2,
                      float width=1, float blur=0) = 0;
};



template <class T> class PtexPtr {
    T* _ptr;
 public:
    /// Constructor.
    PtexPtr(T* ptr=0) : _ptr(ptr) {}

    /// Destructor, calls ptr->release().
    ~PtexPtr() { if (_ptr) _ptr->release(); }

    /// Use as pointer value.
    operator T* () { return _ptr; }

    /// Access members of pointer.
    T* operator-> () { return _ptr; }

    /// Get pointer value.
    T* get() { return _ptr; }

    /// Swap pointer values.
    void swap(PtexPtr& p)
    {
        T* tmp = p._ptr;
        p._ptr = _ptr;
        _ptr = tmp;
    }

    /// Deallocate object pointed to, and optionally set to new value.
    void reset(T* ptr=0) {
        if (_ptr) _ptr->release();
        _ptr = ptr;
    }

 private:
    /// Copying prohibited
    PtexPtr(const PtexPtr& p);

    /// Assignment prohibited
    void operator= (PtexPtr& p);
};

#ifndef DOXYGEN
namespace PtexUtils {}

PTEX_NAMESPACE_END

using Ptex::PtexMetaData;
using Ptex::PtexFaceData;
using Ptex::PtexTexture;
using Ptex::PtexInputHandler;
using Ptex::PtexErrorHandler;
using Ptex::PtexCache;
using Ptex::PtexWriter;
using Ptex::PtexFilter;
using Ptex::PtexPtr;
namespace PtexUtils = Ptex::PtexUtils;

#endif
#endif
