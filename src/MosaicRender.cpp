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
#include "pch.h"
#include "MosaicRender.h"

const int GLYPH_MARGIN = 4;

namespace
{
    COLORREF GetGlyphColor(MosaicCellGlyph glyphType)
    {
        switch (glyphType) {
        case MosaicCellGlyph::Cross:
            return RGB(255, 0, 0);
        case MosaicCellGlyph::Square:
            return RGB(0, 160, 0);
        case MosaicCellGlyph::Circle:
            return RGB(0, 0, 255);
        default:
            return RGB(0, 0, 0);
        }
    }
}

HBITMAP CreateGlyphBitmap(MosaicCellGlyph glyphType, int buttonSize)
{
    const int width = buttonSize - 4;
    const int height = buttonSize - 4;

    HDC hdcScreen = GetDC(nullptr);
    if (hdcScreen == nullptr) {
        return nullptr;
    }

    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    ReleaseDC(nullptr, hdcScreen);
    if (hBitmap == nullptr || hdcMem == nullptr) {
        if (hBitmap != nullptr) {
            DeleteObject(hBitmap);
        }
        if (hdcMem != nullptr) {
            DeleteDC(hdcMem);
        }
        return nullptr;
    }

    HGDIOBJ oldBmp = SelectObject(hdcMem, hBitmap);
    RECT rc = { 0, 0, width, height };
    HBRUSH white = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(hdcMem, &rc, white);
    DeleteObject(white);

    HPEN pen = CreatePen(PS_SOLID, 2, GetGlyphColor(glyphType));
    HGDIOBJ oldPen = SelectObject(hdcMem, pen);
    HGDIOBJ oldBrush = SelectObject(hdcMem, GetStockObject(NULL_BRUSH));

    switch (glyphType) {
    case MosaicCellGlyph::Blank:
        break;

    case MosaicCellGlyph::Cross:
        MoveToEx(hdcMem, GLYPH_MARGIN, GLYPH_MARGIN, nullptr);
        LineTo(hdcMem, width - GLYPH_MARGIN, height - GLYPH_MARGIN);
        MoveToEx(hdcMem, width - GLYPH_MARGIN, GLYPH_MARGIN, nullptr);
        LineTo(hdcMem, GLYPH_MARGIN, height - GLYPH_MARGIN);
        break;

    case MosaicCellGlyph::Square:
        Rectangle(hdcMem, GLYPH_MARGIN, GLYPH_MARGIN, width - GLYPH_MARGIN, height - GLYPH_MARGIN);
        break;

    case MosaicCellGlyph::Circle:
        Ellipse(hdcMem, GLYPH_MARGIN, GLYPH_MARGIN, width - GLYPH_MARGIN, height - GLYPH_MARGIN);
        break;

    default:
        break;
    }

    SelectObject(hdcMem, oldBrush);
    SelectObject(hdcMem, oldPen);
    DeleteObject(pen);
    SelectObject(hdcMem, oldBmp);
    DeleteDC(hdcMem);

    return hBitmap;
}
