/*
 *  Copyright (c) 2022 Jeremy HU <jeremy-at-dust3d dot org>. All rights reserved. 
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:

 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.

 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

#ifndef DUST3D_STYLE_CONSTANTS_H_
#define DUST3D_STYLE_CONSTANTS_H_

class Style
{
public:
    static inline const char *MainColor = "#f7d9c8";
    static inline const char *MainFontColor = "#f7d9c8";
    static inline const char *HighlightButtonColor = "#fc6621";
    static inline const char *HighlightButtonFontColor = "#353535";
    static inline const char *ButtonColor = "#353535";
    static inline const char *ButtonFontColor = "#f7d9c8";
    static inline const char *BackgroundColor = "#000000";
    static inline const char *FrameBackgroundColor = "#252525";
    static inline const char *HighlightColor = "#fc6621";
    static inline const double NormalFontLineHeight = 17.0;
    static inline const double SidebarWidth = 270.0;
    static inline const double SidebarHorizontalSpacing = 10.0;
    static inline const double SidebarVerticalSpacing = 15.0;
    static inline const double SidebarVerticalInternalSpacing = 7.5;
    static inline const double BorderSize = 1.0;
    static inline const double ToolbarIconPadding = 3.0;
    static inline const double ToolbarIconSize = 17.0 + ToolbarIconPadding * 2.0;
    static inline const double ToolbarSpacing = 5.0;
    static inline const char *RedColor = "#fc6621";
    static inline const char *GreenColor = "#0da9f1";
    static inline const char *BlueColor = "#aaebc4";
    static inline const char *WhiteColor = "#f7d9c8";
};

#endif

