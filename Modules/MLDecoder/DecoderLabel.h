//
// MATLAB Compiler: 6.3 (R2016b)
// Date: Sat Dec 02 22:10:16 2017
// Arguments: "-B" "macro_default" "-W" "cpplib:DecoderLabel" "-T" "link:lib"
// "-d" "C:\Users\Haoan
// Feng\Documents\MATLAB\Examples\actionAutoEncoder\DecoderLabel\for_testing"
// "-v" "C:\Users\Haoan
// Feng\Documents\MATLAB\Examples\actionAutoEncoder\DecoderLabel.m"
// "C:\Users\Haoan Feng\Documents\MATLAB\Examples\actionAutoEncoder\loadData.m"
// "C:\Users\Haoan
// Feng\Documents\MATLAB\Examples\actionAutoEncoder\loadDeepnet.m" 
//

#ifndef __DecoderLabel_h
#define __DecoderLabel_h 1

#if defined(__cplusplus) && !defined(mclmcrrt_h) && defined(__linux__)
#  pragma implementation "mclmcrrt.h"
#endif
#include "mclmcrrt.h"
#include "mclcppclass.h"
#ifdef __cplusplus
extern "C" {
#endif

#if defined(__SUNPRO_CC)
/* Solaris shared libraries use __global, rather than mapfiles
 * to define the API exported from a shared library. __global is
 * only necessary when building the library -- files including
 * this header file to use the library do not need the __global
 * declaration; hence the EXPORTING_<library> logic.
 */

#ifdef EXPORTING_DecoderLabel
#define PUBLIC_DecoderLabel_C_API __global
#else
#define PUBLIC_DecoderLabel_C_API /* No import statement needed. */
#endif

#define LIB_DecoderLabel_C_API PUBLIC_DecoderLabel_C_API

#elif defined(_HPUX_SOURCE)

#ifdef EXPORTING_DecoderLabel
#define PUBLIC_DecoderLabel_C_API __declspec(dllexport)
#else
#define PUBLIC_DecoderLabel_C_API __declspec(dllimport)
#endif

#define LIB_DecoderLabel_C_API PUBLIC_DecoderLabel_C_API


#else

#define LIB_DecoderLabel_C_API

#endif

/* This symbol is defined in shared libraries. Define it here
 * (to nothing) in case this isn't a shared library. 
 */
#ifndef LIB_DecoderLabel_C_API 
#define LIB_DecoderLabel_C_API /* No special import/export declaration */
#endif

extern LIB_DecoderLabel_C_API 
bool MW_CALL_CONV DecoderLabelInitializeWithHandlers(
       mclOutputHandlerFcn error_handler, 
       mclOutputHandlerFcn print_handler);

extern LIB_DecoderLabel_C_API 
bool MW_CALL_CONV DecoderLabelInitialize(void);

extern LIB_DecoderLabel_C_API 
void MW_CALL_CONV DecoderLabelTerminate(void);



extern LIB_DecoderLabel_C_API 
void MW_CALL_CONV DecoderLabelPrintStackTrace(void);

extern LIB_DecoderLabel_C_API 
bool MW_CALL_CONV mlxDecoderLabel(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);

extern LIB_DecoderLabel_C_API 
bool MW_CALL_CONV mlxLoadData(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);

extern LIB_DecoderLabel_C_API 
bool MW_CALL_CONV mlxLoadDeepnet(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);


#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

/* On Windows, use __declspec to control the exported API */
#if defined(_MSC_VER) || defined(__BORLANDC__)

#ifdef EXPORTING_DecoderLabel
#define PUBLIC_DecoderLabel_CPP_API __declspec(dllexport)
#else
#define PUBLIC_DecoderLabel_CPP_API __declspec(dllimport)
#endif

#define LIB_DecoderLabel_CPP_API PUBLIC_DecoderLabel_CPP_API

#else

#if !defined(LIB_DecoderLabel_CPP_API)
#if defined(LIB_DecoderLabel_C_API)
#define LIB_DecoderLabel_CPP_API LIB_DecoderLabel_C_API
#else
#define LIB_DecoderLabel_CPP_API /* empty! */ 
#endif
#endif

#endif

extern LIB_DecoderLabel_CPP_API void MW_CALL_CONV DecoderLabel(int nargout, mwArray& label, const mwArray& xTrain);

extern LIB_DecoderLabel_CPP_API void MW_CALL_CONV loadData(int nargout, mwArray& xTrain);

extern LIB_DecoderLabel_CPP_API void MW_CALL_CONV loadDeepnet();

#endif
#endif
