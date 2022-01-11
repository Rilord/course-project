#ifndef PtexVersion_h
#define PtexVersion_h



#define PtexAPIVersion 4
#define PtexFileMajorVersion 1
#define PtexFileMinorVersion 4
#define PtexLibraryMajorVersion 2
#define PtexLibraryMinorVersion 2

#define PTEX_NAMESPACE Ptex
#ifdef PTEX_VENDOR
#  define make_version_ns_(major,minor,vendor) v##major##_##minor##_##vendor
#  define make_version_ns(major,minor,vendor) make_version_ns_(major,minor,vendor)
#  define PTEX_VERSION_NAMESPACE make_version_ns(PtexLibraryMajorVersion,PtexLibraryMinorVersion,PTEX_VENDOR)
#else
#  define make_version_ns_(major,minor) v##major##_##minor
#  define make_version_ns(major,minor) make_version_ns_(major,minor)
#  define PTEX_VERSION_NAMESPACE make_version_ns(PtexLibraryMajorVersion,PtexLibraryMinorVersion)
#endif

#define PTEX_NAMESPACE_BEGIN \
namespace PTEX_NAMESPACE { \
namespace PTEX_VERSION_NAMESPACE {} \
using namespace PTEX_VERSION_NAMESPACE; \
namespace PTEX_VERSION_NAMESPACE {

#define PTEX_NAMESPACE_END }}
#endif
