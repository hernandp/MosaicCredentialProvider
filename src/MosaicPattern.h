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

#include <cstddef>
#include <string>

constexpr int kMosaicButtonRows = 4;
constexpr int kMosaicButtonCols = 4;
constexpr int kMosaicButtonCount = kMosaicButtonRows * kMosaicButtonCols;

enum MosaicButtonFaceState : int
{
    MOSAIC_STATE_EMPTY = 0,
    MOSAIC_BITMAP_CROSS = 1,
    MOSAIC_BITMAP_SQUARE = 2,
    MOSAIC_BITMAP_CIRCLE = 3,
    MOSAIC_STATE_COUNT = 4
};

std::wstring BuildNormalizedPattern(const int* faceStates, size_t count);
