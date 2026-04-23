

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 8.01.0628 */
/* at Tue Jan 19 00:14:07 2038
 */
/* Compiler settings for MosaicCredProv.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.01.0628 
    protocol : all , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */



#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        EXTERN_C __declspec(selectany) const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif // !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, LIBID_MosaicCredProvLib,0x5183C3D9,0x9A6A,0x4A8C,0x90,0x34,0xFB,0x77,0x63,0x4C,0x68,0xB5);


MIDL_DEFINE_GUID(CLSID, CLSID_MosaicCredentialProvider,0x30106E01,0xB65F,0x480E,0x99,0x3E,0x92,0xD5,0xD7,0x31,0x0C,0x5E);


MIDL_DEFINE_GUID(CLSID, CLSID_MosaicCredentialProviderCredential,0x6eddc324,0x1233,0x4597,0xb1,0x63,0x2c,0x98,0x92,0x10,0xac,0xeb);


MIDL_DEFINE_GUID(CLSID, CLSID_MosaicCredentialProviderFilter,0x5daab89b,0x38ac,0x437e,0x94,0xf9,0x23,0x79,0x12,0x7f,0x85,0x64);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



