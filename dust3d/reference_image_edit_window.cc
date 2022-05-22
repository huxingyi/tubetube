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

#define NOMINMAX
#include <functional>
#include <hu/gles/indie_game_engine.h>
#include <hu/widget/radio_button.h>
#include <hu/widget/text.h>
#include <hu/widget/canvas.h>
#include <dust3d/reference_image_edit_window.h>
#include <dust3d/document_window.h>
#include <dust3d/style_constants.h>

ReferenceImageEditWindow::ReferenceImageEditWindow():
    Window(String::toInt(DocumentWindow::settings()->value("referenceImageEditWindow.width", 800)), String::toInt(DocumentWindow::settings()->value("referenceImageEditWindow.height", 360)))
{
    setTitle("Reference image edit");
    engine()->setBackgroundColor(Color("#00000000"));
    
    auto sourceImageWidget = new Widget("referenceImageEditWindow.sourceImage");
    sourceImageWidget->setName("sourceImageWidget");
    sourceImageWidget->setWidth(1.0, Widget::SizePolicy::FlexibleSize);
    sourceImageWidget->setHeight(1.0, Widget::SizePolicy::RelativeSize);
    sourceImageWidget->setBackgroundColor(Color(Style::BackgroundColor));
    
    auto canvas = new Canvas;
    m_canvas = canvas;
    canvas->setWidth(1.0, Widget::SizePolicy::RelativeSize);
    canvas->setHeight(1.0, Widget::SizePolicy::RelativeSize);
    sourceImageWidget->addWidget(canvas);
    
    auto rightLayout = new Widget;
    rightLayout->setName("rightLayout");
    rightLayout->setLayoutDirection(Widget::LayoutDirection::TopToBottom);
    rightLayout->setWidth(Style::SidebarWidth, Widget::SizePolicy::FixedSize);
    rightLayout->setHeight(1.0, Widget::SizePolicy::RelativeSize);
    rightLayout->setBackgroundColor(Color(Style::FrameBackgroundColor));
    
    auto referenceImagePreviewWidget = new Widget("referenceImageEditWindow.referenceImagePreview");
    referenceImagePreviewWidget->setName("referenceImagePreviewWidget");
    double referenceImageWidth = Style::SidebarWidth - Style::BorderSize * 2.0;
    referenceImagePreviewWidget->setWidth(referenceImageWidth, Widget::SizePolicy::FixedSize);
    referenceImagePreviewWidget->setHeight(referenceImageWidth * 0.5, Widget::SizePolicy::FixedSize);
    referenceImagePreviewWidget->setBackgroundColor(Color(Style::BackgroundColor));
    
    auto referenceImagePreviewLayout = new Widget;
    referenceImagePreviewLayout->addSpacing(Style::BorderSize);
    referenceImagePreviewLayout->addWidget(referenceImagePreviewWidget);

    auto loadImageButton = new PushButton;
    loadImageButton->setHeight(Style::NormalFontLineHeight + loadImageButton->paddingHeight(), Widget::SizePolicy::FixedSize);
    loadImageButton->setText("Load source image");
    loadImageButton->setBackgroundColor(Color(Style::ButtonColor));
    loadImageButton->setColor(Color(Style::ButtonFontColor));
    engine()->windowSizeChanged.connect([=]() {
        loadImageButton->setWidth(engine()->measureFontWidth(loadImageButton->text(), loadImageButton->layoutHeight() - loadImageButton->paddingHeight()) + loadImageButton->paddingWidth(), Widget::SizePolicy::FixedSize);
    });
    loadImageButton->mouseEntered.connect([=]() {
        loadImageButton->setBackgroundColor(Color(Style::ButtonColor).lighted());
    });
    loadImageButton->mouseLeaved.connect([=]() {
        loadImageButton->setBackgroundColor(Color(Style::ButtonColor));
    });
    loadImageButton->mousePressed.connect([=]() {
        loadImageButton->setBackgroundColor(Color(Style::ButtonColor).darked());
    });
    loadImageButton->mouseReleased.connect([=]() {
        loadImageButton->setBackgroundColor(Color(Style::ButtonColor));
        auto selectedFile = this->selectSingleFileByUser({"jpg", "jpeg", "png"});
        if (selectedFile.empty())
            return;
        setImage(selectedFile);
    });
    
    auto loadImageButtonLayout = new Widget;
    loadImageButtonLayout->setName("loadImageButtonLayout");
    loadImageButtonLayout->setLayoutDirection(Widget::LayoutDirection::LeftToRight);
    loadImageButtonLayout->addExpanding();
    loadImageButtonLayout->addWidget(loadImageButton);
    loadImageButtonLayout->addExpanding();
    
    auto copyToFrontButton = new PushButton;
    copyToFrontButton->setHeight(Style::NormalFontLineHeight + copyToFrontButton->paddingHeight(), Widget::SizePolicy::FixedSize);
    copyToFrontButton->setText("Copy to front");
    copyToFrontButton->setBackgroundColor(Color(Style::ButtonColor));
    copyToFrontButton->setColor(Color(Style::ButtonFontColor));
    engine()->windowSizeChanged.connect([=]() {
        copyToFrontButton->setWidth(engine()->measureFontWidth(copyToFrontButton->text(), copyToFrontButton->layoutHeight() - copyToFrontButton->paddingHeight()) + copyToFrontButton->paddingWidth(), Widget::SizePolicy::FixedSize);
    });
    copyToFrontButton->mouseEntered.connect([=]() {
        copyToFrontButton->setBackgroundColor(Color(Style::ButtonColor).lighted());
    });
    copyToFrontButton->mouseLeaved.connect([=]() {
        copyToFrontButton->setBackgroundColor(Color(Style::ButtonColor));
    });
    copyToFrontButton->mousePressed.connect([=]() {
        copyToFrontButton->setBackgroundColor(Color(Style::ButtonColor).darked());
    });
    copyToFrontButton->mouseReleased.connect([=]() {
        copyToFrontButton->setBackgroundColor(Color(Style::ButtonColor));
        this->copyClipToFront();
    });
    
    auto copyToSideButton = new PushButton;
    copyToSideButton->setHeight(Style::NormalFontLineHeight + copyToSideButton->paddingHeight(), Widget::SizePolicy::FixedSize);
    copyToSideButton->setText("Copy to side");
    copyToSideButton->setBackgroundColor(Color(Style::ButtonColor));
    copyToSideButton->setColor(Color(Style::ButtonFontColor));
    engine()->windowSizeChanged.connect([=]() {
        copyToSideButton->setWidth(engine()->measureFontWidth(copyToSideButton->text(), copyToSideButton->layoutHeight() - copyToSideButton->paddingHeight()) + copyToSideButton->paddingWidth(), Widget::SizePolicy::FixedSize);
    });
    copyToSideButton->mouseEntered.connect([=]() {
        copyToSideButton->setBackgroundColor(Color(Style::ButtonColor).lighted());
    });
    copyToSideButton->mouseLeaved.connect([=]() {
        copyToSideButton->setBackgroundColor(Color(Style::ButtonColor));
    });
    copyToSideButton->mousePressed.connect([=]() {
        copyToSideButton->setBackgroundColor(Color(Style::ButtonColor).darked());
    });
    copyToSideButton->mouseReleased.connect([=]() {
        copyToSideButton->setBackgroundColor(Color(Style::ButtonColor));
        this->copyClipToSide();
    });
    
    auto copyButtonsLayout = new Widget;
    copyButtonsLayout->setName("copyButtonsLayout");
    copyButtonsLayout->setLayoutDirection(Widget::LayoutDirection::LeftToRight);
    copyButtonsLayout->addSpacing(Style::SidebarHorizontalSpacing);
    copyButtonsLayout->addWidget(copyToFrontButton);
    copyButtonsLayout->addExpanding();
    copyButtonsLayout->addWidget(copyToSideButton);
    copyButtonsLayout->addSpacing(Style::SidebarHorizontalSpacing);
    
    auto saveButton = new PushButton;
    saveButton->setHeight(Style::NormalFontLineHeight + saveButton->paddingHeight(), Widget::SizePolicy::FixedSize);
    saveButton->setText("Save");
    saveButton->setBackgroundColor(Color(Style::HighlightButtonColor));
    saveButton->setColor(Color(Style::HighlightButtonFontColor));
    engine()->windowSizeChanged.connect([=]() {
        saveButton->setWidth(engine()->measureFontWidth(saveButton->text(), saveButton->layoutHeight() - saveButton->paddingHeight()) + saveButton->paddingWidth(), Widget::SizePolicy::FixedSize);
    });
    saveButton->mouseEntered.connect([=]() {
        saveButton->setBackgroundColor(Color(Style::HighlightButtonColor).lighted());
    });
    saveButton->mouseLeaved.connect([=]() {
        saveButton->setBackgroundColor(Color(Style::HighlightButtonColor));
    });
    saveButton->mousePressed.connect([=]() {
        saveButton->setBackgroundColor(Color(Style::HighlightButtonColor).darked());
    });
    saveButton->mouseReleased.connect([=]() {
        saveButton->setBackgroundColor(Color(Style::HighlightButtonColor));
    });
    
    auto saveButtonLayout = new Widget;
    saveButtonLayout->setName("saveButtonLayout");
    saveButtonLayout->setLayoutDirection(Widget::LayoutDirection::LeftToRight);
    saveButtonLayout->addExpanding();
    saveButtonLayout->addWidget(saveButton);
    saveButtonLayout->addSpacing(Style::SidebarHorizontalSpacing);

    rightLayout->addSpacing(Style::BorderSize);
    rightLayout->addWidget(referenceImagePreviewLayout);
    rightLayout->addSpacing(Style::SidebarVerticalInternalSpacing);
    rightLayout->addWidget(copyButtonsLayout);
    rightLayout->addSpacing(Style::SidebarVerticalSpacing * 2.0);
    rightLayout->addWidget(loadImageButtonLayout);
    rightLayout->addSpacing(Style::SidebarVerticalSpacing);
    rightLayout->addExpanding();
    rightLayout->addWidget(saveButtonLayout);
    rightLayout->addSpacing(Style::SidebarVerticalSpacing);
    
    auto mainLayout = new Widget;
    mainLayout->setName("mainLayout");
    mainLayout->setHeight(1.0, Widget::SizePolicy::RelativeSize);
    mainLayout->setWidth(1.0, Widget::SizePolicy::RelativeSize);
    mainLayout->addWidget(sourceImageWidget);
    mainLayout->addWidget(rightLayout);
    
    engine()->rootWidget()->setName("rootWidget");
    engine()->rootWidget()->addWidget(mainLayout);
    
    engine()->windowSizeChanged.connect(std::bind(&ReferenceImageEditWindow::updatePreviewImage, this));
    engine()->windowSizeChanged.connect(std::bind(&ReferenceImageEditWindow::updateClip, this));
    
    m_canvas->mouseMoved.connect(std::bind(&ReferenceImageEditWindow::handleCanvasMouseMove, this, std::placeholders::_1, std::placeholders::_2));
    m_canvas->mousePressed.connect(std::bind(&ReferenceImageEditWindow::handleCanvasMousePressed, this));
    m_canvas->mouseReleased.connect(std::bind(&ReferenceImageEditWindow::handleCanvasMouseReleased, this));

    setVisible(true);
}

void ReferenceImageEditWindow::setLeftTopHandleMouseHovering(bool hovering)
{
    if (m_leftTopHandleMouseHovering == hovering)
        return;
    m_leftTopHandleMouseHovering = hovering;
    updateClip();
}

void ReferenceImageEditWindow::setRightTopHandleMouseHovering(bool hovering)
{
    if (m_rightTopHandleMouseHovering == hovering)
        return;
    m_rightTopHandleMouseHovering = hovering;
    updateClip();
}

void ReferenceImageEditWindow::setRightBottomHandleMouseHovering(bool hovering)
{
    if (m_rightBottomHandleMouseHovering == hovering)
        return;
    m_rightBottomHandleMouseHovering = hovering;
    updateClip();
}

void ReferenceImageEditWindow::setLeftBottomHandleMouseHovering(bool hovering)
{
    if (m_leftBottomHandleMouseHovering == hovering)
        return;
    m_leftBottomHandleMouseHovering = hovering;
    updateClip();
}

void ReferenceImageEditWindow::handleCanvasMousePressed()
{
    m_mousePressing = true;
}

void ReferenceImageEditWindow::handleCanvasMouseReleased()
{
    m_mousePressing = false;
    if (m_mouseMoveStarted) {
        m_mouseMoveStarted = false;
        m_leftTopHandleMouseMoving = false;
        m_rightTopHandleMouseMoving = false;
        m_rightBottomHandleMouseMoving = false;
        m_leftBottomHandleMouseMoving = false;
    }
}

void ReferenceImageEditWindow::handleCanvasMouseMove(double x, double y)
{
    Widget *sourceImageWidget = Widget::get("referenceImageEditWindow.sourceImage");
    double realX = (x - sourceImageWidget->layoutLeft()) / sourceImageWidget->layoutWidth();
    double realY = (y - sourceImageWidget->layoutTop()) / sourceImageWidget->layoutHeight();
    double handleHalfWidth = 0.75 * m_handleSize / sourceImageWidget->layoutWidth();
    double handleHalfHeight = 0.75 * m_handleSize / sourceImageWidget->layoutHeight();
    setLeftTopHandleMouseHovering(realX >= m_clipLeft - handleHalfWidth && realX <= m_clipLeft + handleHalfWidth && 
        realY >= m_clipTop - handleHalfHeight && realY <= m_clipTop + handleHalfHeight);
    setRightTopHandleMouseHovering(realX >= m_clipRight - handleHalfWidth && realX <= m_clipRight + handleHalfWidth && 
        realY >= m_clipTop - handleHalfHeight && realY <= m_clipTop + handleHalfHeight);
    setRightBottomHandleMouseHovering(realX >= m_clipRight - handleHalfWidth && realX <= m_clipRight + handleHalfWidth && 
        realY >= m_clipBottom - handleHalfHeight && realY <= m_clipBottom + handleHalfHeight);
    setLeftBottomHandleMouseHovering(realX >= m_clipLeft - handleHalfWidth && realX <= m_clipLeft + handleHalfWidth && 
        realY >= m_clipBottom - handleHalfHeight && realY <= m_clipBottom + handleHalfHeight);
    if (m_mousePressing && !m_mouseMoveStarted) {
        if (m_leftTopHandleMouseHovering) {
            m_leftTopHandleMouseMoving = true;
            m_mouseMoveStarted = true;
        } else if (m_rightTopHandleMouseHovering) {
            m_rightTopHandleMouseMoving = true;
            m_mouseMoveStarted = true;
        } else if (m_rightBottomHandleMouseHovering) {
            m_rightBottomHandleMouseMoving = true;
            m_mouseMoveStarted = true;
        } else if (m_leftBottomHandleMouseHovering) {
            m_leftBottomHandleMouseMoving = true;
            m_mouseMoveStarted = true;
        } else {
            m_mouseMoveStarted = true;
        }
        if (m_mouseMoveStarted) {
            m_mouseMoveFromX = realX;
            m_mouseMoveFromY = realY;
        }
    }
    if (m_mouseMoveStarted) {
        double moveX = realX - m_mouseMoveFromX;
        double moveY = realY - m_mouseMoveFromY;
        if (m_leftTopHandleMouseMoving) {
            m_clipLeft += moveX;
            m_clipTop += moveY;
            updateClip();
        } else if (m_rightTopHandleMouseMoving) {
            m_clipRight += moveX;
            m_clipTop += moveY;
            updateClip();
        } else if (m_rightBottomHandleMouseMoving) {
            m_clipRight += moveX;
            m_clipBottom += moveY;
            updateClip();
        } else if (m_leftBottomHandleMouseMoving) {
            m_clipLeft += moveX;
            m_clipBottom += moveY;
            updateClip();
        } else {
            m_clipLeft += moveX;
            m_clipTop += moveY;
            m_clipRight += moveX;
            m_clipBottom += moveY;
            updateClip();
        }
        m_mouseMoveFromX = realX;
        m_mouseMoveFromY = realY;
    }
}

void ReferenceImageEditWindow::updateClip()
{
    m_canvas->clear();
    
    m_canvas->addLine(m_clipLeft, m_clipTop, m_clipRight, m_clipTop, Color(Style::HighlightColor));
    m_canvas->addLine(m_clipRight, m_clipTop, m_clipRight, m_clipBottom, Color(Style::HighlightColor));
    m_canvas->addLine(m_clipRight, m_clipBottom, m_clipLeft, m_clipBottom, Color(Style::HighlightColor));
    m_canvas->addLine(m_clipLeft, m_clipBottom, m_clipLeft, m_clipTop, Color(Style::HighlightColor));
    
    Color maskColor = Color(Style::HighlightColor);
    maskColor.alpha() = 0.2;
    m_canvas->addRectangle(0.0, 0.0, m_clipLeft, 1.0, maskColor);
    m_canvas->addRectangle(m_clipRight, 0.0, 1.0, 1.0, maskColor);
    m_canvas->addRectangle(m_clipLeft, 0.0, m_clipRight, m_clipTop, maskColor);
    m_canvas->addRectangle(m_clipLeft, m_clipBottom, m_clipRight, 1.0, maskColor);
    
    Widget *sourceImageWidget = Widget::get("referenceImageEditWindow.sourceImage");
    
    double handleHalfWidth = 0.5 * m_handleSize / sourceImageWidget->layoutWidth();
    double handleHalfHeight = 0.5 * m_handleSize / sourceImageWidget->layoutHeight();
    m_canvas->addRectangle(m_clipLeft - handleHalfWidth, m_clipTop - handleHalfHeight, m_clipLeft + handleHalfWidth, m_clipTop + handleHalfHeight, m_leftTopHandleMouseHovering ? Color(Style::HighlightColor).lighted() : Color(Style::HighlightColor));
    m_canvas->addRectangle(m_clipRight - handleHalfWidth, m_clipTop - handleHalfHeight, m_clipRight + handleHalfWidth, m_clipTop + handleHalfHeight, m_rightTopHandleMouseHovering ? Color(Style::HighlightColor).lighted() : Color(Style::HighlightColor));
    m_canvas->addRectangle(m_clipRight - handleHalfWidth, m_clipBottom - handleHalfHeight, m_clipRight + handleHalfWidth, m_clipBottom + handleHalfHeight, m_rightBottomHandleMouseHovering ? Color(Style::HighlightColor).lighted() : Color(Style::HighlightColor));
    m_canvas->addRectangle(m_clipLeft - handleHalfWidth, m_clipBottom - handleHalfHeight, m_clipLeft + handleHalfWidth, m_clipBottom + handleHalfHeight, m_leftBottomHandleMouseHovering ? Color(Style::HighlightColor).lighted() : Color(Style::HighlightColor));
}

std::unique_ptr<Image> &ReferenceImageEditWindow::resizedImage()
{
    return m_resizedImage;
}

std::unique_ptr<Image> &ReferenceImageEditWindow::frontImage()
{
    return m_frontImage;
}

std::unique_ptr<Image> &ReferenceImageEditWindow::sideImage()
{
    return m_sideImage;
}

std::unique_ptr<Image> &ReferenceImageEditWindow::referenceImage()
{
    return m_referenceImage;
}

void ReferenceImageEditWindow::updateReferenceImage()
{
    if (m_referenceImageFlags.processing) {
        m_referenceImageFlags.dirty = true;
        return;
    }
    
    m_referenceImageFlags.dirty = false;
    m_referenceImageFlags.processing = true;
    
    Image *frontImage = nullptr != m_frontImage ? new Image(*m_frontImage) : nullptr;
    Image *sideImage = nullptr != m_sideImage ? new Image(*m_sideImage) : nullptr;
    engine()->run([=]() {
            Image *frontScaledImage = nullptr != frontImage ? frontImage->scaledToHeight(ReferenceImageEditWindow::m_targetReferenceHeight) : nullptr;
            Image *sideScaledImage = nullptr != sideImage ? sideImage->scaledToHeight(ReferenceImageEditWindow::m_targetReferenceHeight) : nullptr;
            size_t frontScaledImageWidth = nullptr != frontScaledImage ? frontScaledImage->width() : 0;
            size_t sideScaledImageWidth = nullptr != sideScaledImage ? sideScaledImage->width() : 0;
            size_t targetWidth = std::max(frontScaledImageWidth + sideScaledImageWidth, ReferenceImageEditWindow::m_targetReferenceWidth);
            Image *referenceImage = new Image(targetWidth, ReferenceImageEditWindow::m_targetReferenceHeight);
            referenceImage->clear(255, 255, 255, 255);
            size_t left = (targetWidth - (frontScaledImageWidth + sideScaledImageWidth)) / 2;
            if (nullptr != frontScaledImage)
                referenceImage->copy(*frontScaledImage, 0, 0, left, 0, frontScaledImage->width(), frontScaledImage->height());
            if (nullptr != sideScaledImage)
                referenceImage->copy(*sideScaledImage, 0, 0, left + frontScaledImageWidth, 0, sideScaledImage->width(), sideScaledImage->height());
            delete frontImage;
            delete sideImage;
            delete frontScaledImage;
            delete sideScaledImage;
            return (void *)referenceImage;
        }, [=](void *result) {
            Image *referenceImage = (Image *)result;
            this->engine()->setImageResource("referenceImageEditWindow.referenceImagePreview", referenceImage->width(), referenceImage->height(), referenceImage->data());
            this->referenceImage().reset(referenceImage);
            Widget::get("referenceImageEditWindow.referenceImagePreview")->setBackgroundImageResourceName("referenceImageEditWindow.referenceImagePreview");
            m_referenceImageFlags.processing = false;
            if (m_referenceImageFlags.dirty)
                updateReferenceImage();
        }
    );
}

void ReferenceImageEditWindow::copyClipToFront()
{
    if (nullptr == m_resizedImage)
        return;
    
    double clipLeft = m_clipLeft * m_resizedImage->width();
    double clipRight = m_clipRight * m_resizedImage->width();
    double clipTop = m_clipTop * m_resizedImage->height();
    double clipBottom = m_clipBottom * m_resizedImage->height();
    Image *image = new Image(*m_resizedImage);
    engine()->run([=]() {
            Color clearColor(Style::BackgroundColor);
            Image *clipImage = new Image(clipRight - clipLeft, clipBottom - clipTop);
            clipImage->clear(clearColor.red() * 255.0, clearColor.green() * 255.0, clearColor.blue() * 255.0, clearColor.alpha() * 255.0);
            clipImage->copy(*image, clipLeft, clipTop, 0, 0, clipImage->width(), clipImage->height());
            delete image;
            return (void *)clipImage;
        }, [=](void *result) {
            Image *clipImage = (Image *)result;
            this->frontImage().reset(clipImage);
            this->updateReferenceImage();
        }
    );
}

void ReferenceImageEditWindow::copyClipToSide()
{
    if (nullptr == m_resizedImage)
        return;
    
    double clipLeft = m_clipLeft * m_resizedImage->width();
    double clipRight = m_clipRight * m_resizedImage->width();
    double clipTop = m_clipTop * m_resizedImage->height();
    double clipBottom = m_clipBottom * m_resizedImage->height();
    Image *image = new Image(*m_resizedImage);
    engine()->run([=]() {
            Color clearColor(Style::BackgroundColor);
            Image *clipImage = new Image(clipRight - clipLeft, clipBottom - clipTop);
            clipImage->clear(clearColor.red() * 255.0, clearColor.green() * 255.0, clearColor.blue() * 255.0, clearColor.alpha() * 255.0);
            clipImage->copy(*image, clipLeft, clipTop, 0, 0, clipImage->width(), clipImage->height());
            delete image;
            return (void *)clipImage;
        }, [=](void *result) {
            Image *clipImage = (Image *)result;
            this->sideImage().reset(clipImage);
            this->updateReferenceImage();
        }
    );
}

void ReferenceImageEditWindow::updatePreviewImage()
{
    if (nullptr == m_image)
        return;
    
    Widget *sourceImageWidget = Widget::get("referenceImageEditWindow.sourceImage");
    size_t targetWidth = sourceImageWidget->layoutWidth();
    size_t targetHeight = sourceImageWidget->layoutHeight();
    Image *image = new Image(*m_image);
    engine()->run([=]() {
            size_t toWidth = image->width();
            size_t toHeight = toWidth * targetHeight / targetWidth;
            if (toHeight < image->height()) {
                toHeight = image->height();
                toWidth = toHeight * targetWidth / targetHeight;
            }
            // FIXME: limit the image size to avoid texture error(OpenGL Error : 1281)
            Color clearColor(Style::BackgroundColor);
            Image *resizedImage = new Image(toWidth, toHeight);
            resizedImage->clear(clearColor.red() * 255.0, clearColor.green() * 255.0, clearColor.blue() * 255.0, clearColor.alpha() * 255.0);
            resizedImage->copy(*image, 0, 0, (resizedImage->width() - image->width()) / 2, (resizedImage->height() - image->height()) / 2, image->width(), image->height());
            delete image;
            return (void *)resizedImage;
        }, [=](void *result) {
            Image *resizedImage = (Image *)result;
            this->engine()->setImageResource("referenceImageEditWindow.sourceImage", resizedImage->width(), resizedImage->height(), resizedImage->data());
            this->resizedImage().reset(resizedImage);
            Widget::get("referenceImageEditWindow.sourceImage")->setBackgroundImageResourceName("referenceImageEditWindow.sourceImage");
        }
    );
}

void ReferenceImageEditWindow::setImage(const std::string &path)
{
    m_image = std::make_unique<Image>();
    m_image->load(path.c_str());
    engine()->run([=](void *) {
        this->updatePreviewImage();
    });
}
