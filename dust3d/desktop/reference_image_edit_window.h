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

#ifndef DUST3D_DESKTOP_REFERENCE_IMAGE_EDIT_WINDOW_H_
#define DUST3D_DESKTOP_REFERENCE_IMAGE_EDIT_WINDOW_H_

#include <hu/base/image.h>
#include <hu/base/signal.h>
#include <hu/widget/radio_button.h>
#include <hu/widget/canvas.h>
#include <hu/gles/window.h>
#include <dust3d/document/dirty_flags.h>
#include <dust3d/desktop/style_constants.h>

namespace Dust3d
{

class ReferenceImageEditWindow: public Hu::Window
{
public:
    Hu::Signal<> confirmed;
    
    ReferenceImageEditWindow();
    void setImage(const std::string &path);
    void updatePreviewImage();
    void updateClip();
    void handleCanvasMouseMove(double x, double y);
    void handleCanvasMousePressed();
    void handleCanvasMouseReleased();
    void copyClipToFront();
    void copyClipToSide();
    void updateReferenceImage();
    std::unique_ptr<Hu::Image> &resizedImage();
    std::unique_ptr<Hu::Image> &frontImage();
    std::unique_ptr<Hu::Image> &sideImage();
    std::unique_ptr<Hu::Image> &referenceImage();
    const std::string &sourceImageWidgetId() const;
    const std::string &referenceImagePreviewWidgetId() const;
    
    static inline const size_t m_targetReferenceWidth = 1024;
    static inline const size_t m_targetReferenceHeight = 512;
private:
    std::unique_ptr<Hu::Image> m_image;
    std::unique_ptr<Hu::Image> m_resizedImage;
    std::unique_ptr<Hu::Image> m_frontImage;
    std::unique_ptr<Hu::Image> m_sideImage;
    std::unique_ptr<Hu::Image> m_referenceImage;
    std::string m_sourceImageWidgetId;
    std::string m_referenceImagePreviewWidgetId;
    Hu::Canvas *m_canvas = nullptr;
    double m_clipLeft = 0.25;
    double m_clipRight = 0.75;
    double m_clipTop = 0.05;
    double m_clipBottom = 0.95;
    bool m_leftTopHandleMouseHovering = false;
    bool m_leftTopHandleMouseMoving = false;
    bool m_rightTopHandleMouseHovering = false;
    bool m_rightTopHandleMouseMoving = false;
    bool m_rightBottomHandleMouseHovering = false;
    bool m_rightBottomHandleMouseMoving = false;
    bool m_leftBottomHandleMouseHovering = false;
    bool m_leftBottomHandleMouseMoving = false;
    bool m_mousePressing = false;
    bool m_mouseMoveStarted = false;
    double m_mouseMoveFromX = 0.0;
    double m_mouseMoveFromY = 0.0;
    const double m_handleSize = Style::NormalFontLineHeight;
    DirtyFlags m_referenceImageFlags;
    
    void setLeftTopHandleMouseHovering(bool hovering);
    void setRightTopHandleMouseHovering(bool hovering);
    void setRightBottomHandleMouseHovering(bool hovering);
    void setLeftBottomHandleMouseHovering(bool hovering);
};

}

#endif
