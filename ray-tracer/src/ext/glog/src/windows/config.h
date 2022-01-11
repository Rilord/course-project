


#define GOOGLE_NAMESPACE google


#define _END_GOOGLE_NAMESPACE_ }


#define _START_GOOGLE_NAMESPACE_ namespace google {


#ifndef GOOGLE_GLOG_DLL_DECL
# define GOOGLE_GLOG_IS_A_DLL  1
# define GOOGLE_GLOG_DLL_DECL  __declspec(dllexport)
# define GOOGLE_GLOG_DLL_DECL_FOR_UNITTESTS  __declspec(dllimport)
#endif
