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
#pragma once

#include <credentialprovider.h>
#include <cguid.h>

// {088fa508-94a6-4430-a4cb-6fc6e3c0b9e2}
DEFINE_GUID(CPFG_STYLE_LINK_AS_BUTTON,      0x088fa508, 0x94a6, 0x4430, 0xa4, 0xcb, 0x6f, 0xc6, 0xe3, 0xc0, 0xb9, 0xe2);
DEFINE_GUID(CPFG_CREDENTIAL_PROVIDER_LOGO,  0x2d837775, 0xf6cd, 0x464e, 0xa7, 0x45, 0x48, 0x2f, 0xd0, 0xb4, 0x74, 0x93);
DEFINE_GUID(CPFG_CREDENTIAL_PROVIDER_LABEL, 0x286BBFF3, 0xBAD4, 0x438F, 0xB0, 0x07, 0x79, 0xB7, 0x26, 0x7C, 0x3D, 0x48);

constexpr DWORD CP_FIELD_TILE_IMAGE = 0;
constexpr DWORD CP_FIELD_HEADER = 1;
constexpr DWORD CP_FIELD_EXPLAIN = 2;
constexpr DWORD CP_FIELD_VERIFY_CMD = 3;
constexpr DWORD CP_FIELD_SETUP_CMD = 4;
constexpr DWORD CP_FIELD_ENROLL_PASSWORD = 5;
constexpr DWORD CP_FIELD_ENROLL_SUBMIT = 6;
constexpr DWORD CP_FIELD_FORGOT_MOSAIC = 7;
constexpr DWORD CP_FIELD_CANCEL_CMD = 8;

const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR g_fieldDescriptors[] = {
    { CP_FIELD_TILE_IMAGE,     CPFT_TILE_IMAGE,    (LPWSTR)L"MosaicCP",GUID_NULL},
    { CP_FIELD_HEADER,         CPFT_LARGE_TEXT,    (LPWSTR)L"Label 1", GUID_NULL },
    { CP_FIELD_EXPLAIN,        CPFT_SMALL_TEXT,    (LPWSTR)L"Label 2", GUID_NULL },
    { CP_FIELD_VERIFY_CMD,     CPFT_SUBMIT_BUTTON, (LPWSTR)L"verify_cmd", GUID_NULL },
    { CP_FIELD_SETUP_CMD,      CPFT_COMMAND_LINK,  (LPWSTR)L"setup_mosaic_cmd", CPFG_STYLE_LINK_AS_BUTTON },
    { CP_FIELD_ENROLL_PASSWORD,    CPFT_PASSWORD_TEXT,  (LPWSTR)L"Password", GUID_NULL },
    { CP_FIELD_ENROLL_SUBMIT,      CPFT_COMMAND_LINK,  (LPWSTR)L"Submit", CPFG_STYLE_LINK_AS_BUTTON },
    { CP_FIELD_FORGOT_MOSAIC,      CPFT_COMMAND_LINK,  (LPWSTR)L"forgot_mosaic_cmd", CPFG_STYLE_LINK_AS_BUTTON },
    { CP_FIELD_CANCEL_CMD,         CPFT_COMMAND_LINK,  (LPWSTR)L"cancel_cmd", CPFG_STYLE_LINK_AS_BUTTON }
};

constexpr size_t CP_NUM_FIELDS = sizeof(g_fieldDescriptors) / sizeof(g_fieldDescriptors[0]);

