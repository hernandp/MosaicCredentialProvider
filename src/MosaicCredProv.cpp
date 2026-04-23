// ----------------------------------------------------------------------------------------------------------------
// Copyright 2026 Hernan Di Pietro
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
// documentation files (the "Software"), to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and
// to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of
// the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// ----------------------------------------------------------------------------------------------------------------
// MosaicCredProv.cpp : Implementation of DLL Exports.


#include "pch.h"
#include "framework.h"
#include "resource.h"
#include "MosaicCredProv_i.h"
#include "dllmain.h"
#include "dprintf.h"
#include "ProviderRegistration.h"

using namespace ATL;

// Used to determine whether the DLL can be unloaded by OLE.
_Use_decl_annotations_
STDAPI DllCanUnloadNow(void)
{
	return _AtlModule.DllCanUnloadNow();
}

// Returns a class factory to create an object of the requested type.
_Use_decl_annotations_
STDAPI DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID* ppv)
{
	return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}

// DllRegisterServer - Adds entries to the system registry.
_Use_decl_annotations_
STDAPI DllRegisterServer(void)
{
    dprintfW(L"Registering DLL\n");
	// registers object, typelib and all interfaces in typelib
	HRESULT hr = _AtlModule.DllRegisterServer();
	return hr;
}

// DllUnregisterServer - Removes entries from the system registry.
_Use_decl_annotations_
STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _AtlModule.DllUnregisterServer();
	return hr;
}


// Installs the Credential Provider registry entries for the machine,
// registers COM objects first to be sure.  (you can skip this with /SKIPREG option)
// Uninstalling will remove all registry entries including COM objects, 
// except when /SKIPREG is used, where it will only remove Credential Provider registry entries.

STDAPI DllInstall(BOOL bInstall, _In_opt_  LPCWSTR pszCmdLine)
{
	HRESULT hr = E_FAIL;
	
	if (bInstall) {
		if (pszCmdLine != nullptr && (_wcsicmp(pszCmdLine, L"/SKIPREG") != 0))
		{
			dprintfW(L"Registering COM classes\n");
			if (FAILED(hr = DllRegisterServer())) {
                dprintfW(L"DllInstall: DllRegisterServer failed HR = 0x%08x\n", hr);
				return hr;
			}
		}

        dprintfW(L"Registering Credential Provider\n");
		if (FAILED(hr = RegisterCredentialProvider())) {
			return hr;
        }

        dprintfW(L"Registering Credential Provider Filter\n");
		if (FAILED(hr = RegisterCredentialProviderFilter())) {
			return hr;
        }

        dprintfW(L"Registering product configuration\n");
		if (FAILED(hr = RegisterProductConfiguration())) {
			return hr;
        }
	}
	else {
        dprintfW(L"Unregister product configuration\n");
        if (FAILED(hr = UnregisterProductConfiguration())) {
			return hr;
        }

		dprintfW(L"Unregistering Credential Provider Filter\n");
		if (FAILED(hr = UnregisterCredentialProviderFilter())) {
			return hr;
		}

		dprintfW(L"Unregistering Credential Provider\n");
		if (FAILED(hr = UnregisterCredentialProvider())) {
			return hr;
		}

		if (pszCmdLine != nullptr && (_wcsicmp(pszCmdLine, L"/SKIPREG") != 0)) {
			dprintfW(L"Unregistering COM classes\n");
			hr = DllUnregisterServer();
			if (FAILED(hr)) {
				dprintfW(L"DllInstall: DllUnregisterServer failed HR = 0x%08x\n", hr);
				return hr;
			}
        }
    }
	return S_OK;
}



