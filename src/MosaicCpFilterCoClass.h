// ----------------------------------------------------------------------------------------------------------------
// Copyright 2026 Hernán Di Pietro
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
// documentation files (the “Software”), to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and
// to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of
// the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// ----------------------------------------------------------------------------------------------------------------
// MosaicCpFilterCoClass.h : Declaration of the CMosaicCredentialProviderFilter

#pragma once
#include "resource.h"       // main symbols
#include "PatternCredProv_i.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

using namespace ATL;

// CMosaicCredentialProviderFilter

class ATL_NO_VTABLE CMosaicCredentialProviderFilter :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMosaicCredentialProviderFilter, &CLSID_PatternCredentialProviderFilter>,
	public ICredentialProviderFilter
{
public:
	CMosaicCredentialProviderFilter()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_MOSAICCPFILTERCOCLASS);

BEGIN_COM_MAP(CMosaicCredentialProviderFilter)
	COM_INTERFACE_ENTRY(ICredentialProviderFilter)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:




// Inherited via ICredentialProviderFilter
  HRESULT __stdcall Filter(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
                           DWORD dwFlags, GUID *rgclsidProviders,
                           BOOL *rgbAllow, DWORD cProviders) override;
  HRESULT __stdcall UpdateRemoteCredential(
      const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION *pcpcsIn,
      CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION *pcpcsOut) override;
};

OBJECT_ENTRY_AUTO(__uuidof(PatternCredentialProviderFilter), CMosaicCredentialProviderFilter)

