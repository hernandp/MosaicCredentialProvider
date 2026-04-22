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
#include "pch.h"
#include "framework.h"
#include "PatternCredProv_i.h"
#include "ProductConfig.h"
#include "dllmain.h"
#include "dprintf.h"
#include "ProviderRegistration.h"

HRESULT RegisterCredentialProviderFilter() {
	ATL::CRegKey hklmCredProvFilter;
	LSTATUS ls = hklmCredProvFilter.Open(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\Credential Provider Filters", KEY_READ | KEY_WRITE | KEY_SET_VALUE);
	if (ls != ERROR_SUCCESS) {
		dprintfW(L"Failed to open Credential Provider Filters registry key, LSTATUS = 0x%08x\n", ls);
		return HRESULT_FROM_WIN32(ls);
	}

	LPOLESTR szClsId = nullptr;
	HRESULT hr = StringFromCLSID(CLSID_PatternCredentialProviderFilter, &szClsId);
	if (FAILED(hr)) {
		dprintfW(L"Failed to convert CLSID to string, HR = 0x%08x\n", hr);
		return hr;
	}
	ls = hklmCredProvFilter.Create(hklmCredProvFilter, szClsId);
	if (ls != ERROR_SUCCESS) {
		dprintfW(L"Failed to create Credential Provider Filter registry key, LSTATUS = 0x%08x\n", ls);
		CoTaskMemFree(szClsId);
		return HRESULT_FROM_WIN32(ls);
	}
	CoTaskMemFree(szClsId);
	ls = hklmCredProvFilter.SetStringValue(nullptr, L"Pattern Credential Provider Filter");
	if (ls != ERROR_SUCCESS) {
		dprintfW(L"Failed to create Credential Provider Filter default value, LSTATUS = 0x%08x\n", ls);
		return HRESULT_FROM_WIN32(ls);
	}
	dprintfW(L"Credential Provider Filter registry entries created successfully\n");
	return S_OK;
}

HRESULT RegisterCredentialProvider()
{
	ATL::CRegKey hklmCredProv;
	LSTATUS ls = hklmCredProv.Open(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\Credential Providers", KEY_READ | KEY_WRITE | KEY_SET_VALUE);
	if (ls != ERROR_SUCCESS) {
		dprintfW(L"Failed to open Credential Providers registry key, LSTATUS = 0x%08x\n", ls);
		return HRESULT_FROM_WIN32(ls);
	}

	LPOLESTR szClsId = nullptr;
	HRESULT hr = StringFromCLSID(CLSID_PatternCredentialProvider, &szClsId);
	if (FAILED(hr)) {
		dprintfW(L"Failed to convert CLSID to string, HR = 0x%08x\n", hr);
		return hr;
	}
	ls = hklmCredProv.Create(hklmCredProv, szClsId);
	if (ls != ERROR_SUCCESS) {
		dprintfW(L"Failed to create Credential Provider registry key, LSTATUS = 0x%08x\n", ls);
		CoTaskMemFree(szClsId);
		return HRESULT_FROM_WIN32(ls);
	}
	CoTaskMemFree(szClsId);

	ls = hklmCredProv.SetStringValue(nullptr, L"Pattern Credential Provider");
	if (ls != ERROR_SUCCESS) {
		dprintfW(L"Failed to create Credential Provider default value, LSTATUS = 0x%08x\n", ls);
		return HRESULT_FROM_WIN32(ls);
	}
	dprintfW(L"Credential Provider registry entries created successfully\n");

	return S_OK;
}

HRESULT UnregisterCredentialProviderFilter()
{
	ATL::CRegKey hklmCredProvFilter;

	LSTATUS ls = hklmCredProvFilter.Open(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\Credential Provider Filters", KEY_READ | KEY_WRITE);
	if (ls != ERROR_SUCCESS) {
		dprintfW(L"Failed to open Credential Provider Filters registry key, LSTATUS = 0x%08x\n", ls);
		return HRESULT_FROM_WIN32(ls);
	}

	LPOLESTR szClsId = nullptr;
	HRESULT hr = StringFromCLSID(CLSID_PatternCredentialProviderFilter, &szClsId);
	if (FAILED(hr)) {
		dprintfW(L"Failed to convert CLSID to string, HR = 0x%08x\n", hr);
		return hr;
	}
	ls = hklmCredProvFilter.DeleteSubKey(szClsId);
	if (ls != ERROR_SUCCESS) {
		dprintfW(L"Failed to delete Credential Provider Filter registry key, LSTATUS = 0x%08x\n", ls);
		CoTaskMemFree(szClsId);
		return HRESULT_FROM_WIN32(ls);
	}
	CoTaskMemFree(szClsId);
	dprintfW(L"Credential Provider Filter registry entries removed successfully\n");
	return S_OK;
}

HRESULT UnregisterCredentialProvider()
{
	ATL::CRegKey hklmCredProv;
	LSTATUS ls = hklmCredProv.Open(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\Credential Providers", KEY_READ | KEY_WRITE);
	if (ls != ERROR_SUCCESS) {
		dprintfW(L"Failed to open Credential Providers registry key, LSTATUS = 0x%08x\n", ls);
		return HRESULT_FROM_WIN32(ls);
	}

	LPOLESTR szClsId = nullptr;
	HRESULT hr = StringFromCLSID(CLSID_PatternCredentialProvider, &szClsId);
	if (FAILED(hr)) {
		dprintfW(L"Failed to convert CLSID to string, HR = 0x%08x\n", hr);
		return hr;
	}
	ls = hklmCredProv.DeleteSubKey(szClsId);
	if (ls != ERROR_SUCCESS) {
		dprintfW(L"Failed to delete Credential Provider registry key, LSTATUS = 0x%08x\n", ls);
		CoTaskMemFree(szClsId);
		return HRESULT_FROM_WIN32(ls);
	}
	CoTaskMemFree(szClsId);
	dprintfW(L"Credential Provider registry entries removed successfully\n");
	return S_OK;
}

HRESULT RegisterProductConfiguration()
{
	ATL::CRegKey cpConfig;
	LSTATUS ls = cpConfig.Create(HKEY_LOCAL_MACHINE, g_wszProductKeyPath);
	if (ls != ERROR_SUCCESS) {
		dprintfW(L"Failed to open configuration registry key, LSTATUS = 0x%08x\n", ls);
		return HRESULT_FROM_WIN32(ls);
	}
	ls = cpConfig.SetDWORDValue(L"Enabled", 1);
	if (ls != ERROR_SUCCESS) {
		dprintfW(L"Failed to set configuration registry value, LSTATUS = 0x%08x\n", ls);
		return HRESULT_FROM_WIN32(ls);
	}
	dprintfW(L"Product configuration registry entries created successfully\n");
	return S_OK;
}

HRESULT UnregisterProductConfiguration()
{
	ATL::CRegKey cpConfig;
	LSTATUS ls = cpConfig.Open(HKEY_LOCAL_MACHINE, g_wszProductKeyPath, KEY_READ | KEY_WRITE);
	if (ls != ERROR_SUCCESS) {
		dprintfW(L"Failed to open configuration registry key, LSTATUS = 0x%08x\n", ls);
		return HRESULT_FROM_WIN32(ls);
	}
	ls = cpConfig.DeleteValue(L"Enabled");
	if (ls != ERROR_SUCCESS) {
		dprintfW(L"Failed to delete configuration registry value, LSTATUS = 0x%08x\n", ls);
		return HRESULT_FROM_WIN32(ls);
	}
	dprintfW(L"Product configuration registry entries removed successfully\n");
	return S_OK;
}
