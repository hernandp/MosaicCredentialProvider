

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0628 */
/* at Tue Jan 19 00:14:07 2038
 */
/* Compiler settings for PatternCredProv.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.01.0628 
    protocol : all , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */



/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */


#ifndef __PatternCredProv_i_h__
#define __PatternCredProv_i_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef DECLSPEC_XFGVIRT
#if defined(_CONTROL_FLOW_GUARD_XFG)
#define DECLSPEC_XFGVIRT(base, func) __declspec(xfg_virtual(base, func))
#else
#define DECLSPEC_XFGVIRT(base, func)
#endif
#endif

/* Forward Declarations */ 

#ifndef __PatternCredentialProvider_FWD_DEFINED__
#define __PatternCredentialProvider_FWD_DEFINED__

#ifdef __cplusplus
typedef class PatternCredentialProvider PatternCredentialProvider;
#else
typedef struct PatternCredentialProvider PatternCredentialProvider;
#endif /* __cplusplus */

#endif 	/* __PatternCredentialProvider_FWD_DEFINED__ */


#ifndef __PatternCredentialProviderCredential_FWD_DEFINED__
#define __PatternCredentialProviderCredential_FWD_DEFINED__

#ifdef __cplusplus
typedef class PatternCredentialProviderCredential PatternCredentialProviderCredential;
#else
typedef struct PatternCredentialProviderCredential PatternCredentialProviderCredential;
#endif /* __cplusplus */

#endif 	/* __PatternCredentialProviderCredential_FWD_DEFINED__ */


#ifndef __PatternCredentialProviderFilter_FWD_DEFINED__
#define __PatternCredentialProviderFilter_FWD_DEFINED__

#ifdef __cplusplus
typedef class PatternCredentialProviderFilter PatternCredentialProviderFilter;
#else
typedef struct PatternCredentialProviderFilter PatternCredentialProviderFilter;
#endif /* __cplusplus */

#endif 	/* __PatternCredentialProviderFilter_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "credentialprovider.h"
#include "shobjidl.h"

#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __PatternCredProvLib_LIBRARY_DEFINED__
#define __PatternCredProvLib_LIBRARY_DEFINED__

/* library PatternCredProvLib */
/* [uuid] */ 


EXTERN_C const IID LIBID_PatternCredProvLib;

EXTERN_C const CLSID CLSID_PatternCredentialProvider;

#ifdef __cplusplus

class DECLSPEC_UUID("30106E01-B65F-480E-993E-92D5D7310C5E")
PatternCredentialProvider;
#endif

EXTERN_C const CLSID CLSID_PatternCredentialProviderCredential;

#ifdef __cplusplus

class DECLSPEC_UUID("6eddc324-1233-4597-b163-2c989210aceb")
PatternCredentialProviderCredential;
#endif

EXTERN_C const CLSID CLSID_PatternCredentialProviderFilter;

#ifdef __cplusplus

class DECLSPEC_UUID("5daab89b-38ac-437e-94f9-2379127f8564")
PatternCredentialProviderFilter;
#endif
#endif /* __PatternCredProvLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


