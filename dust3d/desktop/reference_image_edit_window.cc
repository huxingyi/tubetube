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
#include <dust3d/desktop/reference_image_edit_window.h>
#include <dust3d/desktop/document_window.h>
#include <dust3d/desktop/style_constants.h>

namespace Dust3d
{

ReferenceImageEditWindow::ReferenceImageEditWindow():
    Window(Hu::String::toInt(DocumentWindow::settings()->value("referenceImageEditWindow.width", 800)), Hu::String::toInt(DocumentWindow::settings()->value("referenceImageEditWindow.height", 360)))
{
    setTitle("Reference image edit");
    engine()->setBackgroundColor(Hu::Color("#00000000"));
    
    auto sourceImageWidget = new Hu::Widget(this);
    m_sourceImageWidgetId = sourceImageWidget->id();
    sourceImageWidget->setName("sourceImageWidget");
    sourceImageWidget->setWidth(1.0, Hu::Widget::SizePolicy::FlexibleSize);
    sourceImageWidget->setHeight(1.0, Hu::Widget::SizePolicy::RelativeSize);
    sourceImageWidget->setBackgroundColor(Hu::Color(Style::BackgroundColor));
    
    auto canvas = new Hu::Canvas(this);
    m_canvas = canvas;
    canvas->setWidth(1.0, Hu::Widget::SizePolicy::RelativeSize);
    canvas->setHeight(1.0, Hu::Widget::SizePolicy::RelativeSize);
    sourceImageWidget->addWidget(canvas);
    
    auto rightLayout = new Hu::Widget(this);
    rightLayout->setName("rightLayout");
    rightLayout->setLayoutDirection(Hu::Widget::LayoutDirection::TopToBottom);
    rightLayout->setWidth(Style::SidebarWidth, Hu::Widget::SizePolicy::FixedSize);
    rightLayout->setHeight(1.0, Hu::Widget::SizePolicy::RelativeSize);
    rightLayout->setBackgroundColor(Hu::Color(Style::FrameBackgroundColor));
    
    auto referenceImagePreviewWidget = new Hu::Widget(this);
    m_referenceImagePreviewWidgetId = referenceImagePreviewWidget->id();
    referenceImagePreviewWidget->setName("referenceImagePreviewWidget");
    double referenceImageWidth = Style::SidebarWidth - Style::BorderSize * 2.0;
    referenceImagePreviewWidget->setWidth(referenceImageWidth, Hu::Widget::SizePolicy::FixedSize);
    referenceImagePreviewWidget->setHeight(referenceImageWidth * 0.5, Hu::Widget::SizePolicy::FixedSize);
    referenceImagePreviewWidget->setBackgroundColor(Hu::Color(Style::BackgroundColor));
    
    auto referenceImagePreviewLayout = new Hu::Widget(this);
    referenceImagePreviewLayout->addSpacing(Style::BorderSize);
    referenceImagePreviewLayout->addWidget(referenceImagePreviewWidget);

    auto loadImageButton = new Hu::PushButton(this);
    loadImageButton->setHeight(Style::NormalFontLineHeight + loadImageButton->paddingHeight(), Hu::Widget::SizePolicy::FixedSize);
    loadImageButton->setText("Load source image");
    loadImageButton->setBackgroundColor(Hu::Color(Style::ButtonColor));
    loadImageButton->setColor(Hu::Color(Style::ButtonFontColor));
    engine()->windowSizeChanged.connect([=]() {
        loadImageButton->setWidth(engine()->measureFontWidth(loadImageButton->text(), loadImageButton->layoutHeight() - loadImageButton->paddingHeight()) + loadImageButton->paddingWidth(), Hu::Widget::SizePolicy::FixedSize);
    });
    loadImageButton->mouseEntered.connect([=]() {
        loadImageButton->setBackgroundColor(Hu::Color(Style::ButtonColor).lighted());
    });
    loadImageButton->mouseLeaved.connect([=]() {
        loadImageButton->setBackgroundColor(Hu::Color(Style::ButtonColor));
    });
    loadImageButton->mousePressed.connect([=]() {
        loadImageButton->setBackgroundColor(Hu::Color(Style::ButtonColor).darked());
    });
    loadImageButton->mouseReleased.connect([=]() {
        loadImageButton->setBackgroundColor(Hu::Color(Style::ButtonColor));
        this->requestToSelectSingleFile({"jpg", "jpeg", "png"});
        //auto selectedFile = this->selectSingleFileByUser({"jpg", "jpeg", "png"});
        //if (selectedFile.empty())
        //    return;
        //setImage(selectedFile);
    });
    
    fileSelected.connect(std::bind(&ReferenceImageEditWindow::setImage, this, std::placeholders::_1));
    
    auto loadImageButtonLayout = new Hu::Widget(this);
    loadImageButtonLayout->setName("loadImageButtonLayout");
    loadImageButtonLayout->setLayoutDirection(Hu::Widget::LayoutDirection::LeftToRight);
    loadImageButtonLayout->addExpanding();
    loadImageButtonLayout->addWidget(loadImageButton);
    loadImageButtonLayout->addExpanding();
    
    auto copyToFrontButton = new Hu::PushButton(this);
    copyToFrontButton->setHeight(Style::NormalFontLineHeight + copyToFrontButton->paddingHeight(), Hu::Widget::SizePolicy::FixedSize);
    copyToFrontButton->setText("Copy to front");
    copyToFrontButton->setBackgroundColor(Hu::Color(Style::ButtonColor));
    copyToFrontButton->setColor(Hu::Color(Style::ButtonFontColor));
    engine()->windowSizeChanged.connect([=]() {
        copyToFrontButton->setWidth(engine()->measureFontWidth(copyToFrontButton->text(), copyToFrontButton->layoutHeight() - copyToFrontButton->paddingHeight()) + copyToFrontButton->paddingWidth(), Hu::Widget::SizePolicy::FixedSize);
    });
    copyToFrontButton->mouseEntered.connect([=]() {
        copyToFrontButton->setBackgroundColor(Hu::Color(Style::ButtonColor).lighted());
    });
    copyToFrontButton->mouseLeaved.connect([=]() {
        copyToFrontButton->setBackgroundColor(Hu::Color(Style::ButtonColor));
    });
    copyToFrontButton->mousePressed.connect([=]() {
        copyToFrontButton->setBackgroundColor(Hu::Color(Style::ButtonColor).darked());
    });
    copyToFrontButton->mouseReleased.connect([=]() {
        copyToFrontButton->setBackgroundColor(Hu::Color(Style::ButtonColor));
        this->copyClipToFront();
    });
    
    auto copyToSideButton = new Hu::PushButton(this);
    copyToSideButton->setHeight(Style::NormalFontLineHeight + copyToSideButton->paddingHeight(), Hu::Widget::SizePolicy::FixedSize);
    copyToSideButton->setText("Copy to side");
    copyToSideButton->setBackgroundColor(Hu::Color(Style::ButtonColor));
    copyToSideButton->setColor(Hu::Color(Style::ButtonFontColor));
    engine()->windowSizeChanged.connect([=]() {
        copyToSideButton->setWidth(engine()->measureFontWidth(copyToSideButton->text(), copyToSideButton->layoutHeight() - copyToSideButton->paddingHeight()) + copyToSideButton->paddingWidth(), Hu::Widget::SizePolicy::FixedSize);
    });
    copyToSideButton->mouseEntered.connect([=]() {
        copyToSideButton->setBackgroundColor(Hu::Color(Style::ButtonColor).lighted());
    });
    copyToSideButton->mouseLeaved.connect([=]() {
        copyToSideButton->setBackgroundColor(Hu::Color(Style::ButtonColor));
    });
    copyToSideButton->mousePressed.connect([=]() {
        copyToSideButton->setBackgroundColor(Hu::Color(Style::ButtonColor).darked());
    });
    copyToSideButton->mouseReleased.connect([=]() {
        copyToSideButton->setBackgroundColor(Hu::Color(Style::ButtonColor));
        this->copyClipToSide();
    });
    
    auto copyButtonsLayout = new Hu::Widget(this);
    copyButtonsLayout->setName("copyButtonsLayout");
    copyButtonsLayout->setLayoutDirection(Hu::Widget::LayoutDirection::LeftToRight);
    copyButtonsLayout->addSpacing(Style::SidebarHorizontalSpacing);
    copyButtonsLayout->addWidget(copyToFrontButton);
    copyButtonsLayout->addExpanding();
    copyButtonsLayout->addWidget(copyToSideButton);
    copyButtonsLayout->addSpacing(Style::SidebarHorizontalSpacing);
    
    auto saveButton = new Hu::PushButton(this);
    saveButton->setHeight(Style::NormalFontLineHeight + saveButton->paddingHeight(), Hu::Widget::SizePolicy::FixedSize);
    saveButton->setText("Save");
    saveButton->setBackgroundColor(Hu::Color(Style::HighlightButtonColor));
    saveButton->setColor(Hu::Color(Style::HighlightButtonFontColor));
    engine()->windowSizeChanged.connect([=]() {
        saveButton->setWidth(engine()->measureFontWidth(saveButton->text(), saveButton->layoutHeight() - saveButton->paddingHeight()) + saveButton->paddingWidth(), Hu::Widget::SizePolicy::FixedSize);
    });
    saveButton->mouseEntered.connect([=]() {
        saveButton->setBackgroundColor(Hu::Color(Style::HighlightButtonColor).lighted());
    });
    saveButton->mouseLeaved.connect([=]() {
        saveButton->setBackgroundColor(Hu::Color(Style::HighlightButtonColor));
    });
    saveButton->mousePressed.connect([=]() {
        saveButton->setBackgroundColor(Hu::Color(Style::HighlightButtonColor).darked());
    });
    saveButton->mouseReleased.connect([=]() {
        saveButton->setBackgroundColor(Hu::Color(Style::HighlightButtonColor));
        this->confirmed.emit();
    });
    
    auto saveButtonLayout = new Hu::Widget(this);
    saveButtonLayout->setName("saveButtonLayout");
    saveButtonLayout->setLayoutDirection(Hu::Widget::LayoutDirection::LeftToRight);
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
    
    auto mainLayout = new Hu::Widget(this);
    mainLayout->setName("mainLayout");
    mainLayout->setHeight(1.0, Hu::Widget::SizePolicy::RelativeSize);
    mainLayout->setWidth(1.0, Hu::Widget::SizePolicy::RelativeSize);
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
    Hu::Widget *sourceImageWidget = getWidget(m_sourceImageWidgetId);
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
    
    m_canvas->addLine(m_clipLeft, m_clipTop, m_clipRight, m_clipTop, Hu::Color(Style::HighlightColor));
    m_canvas->addLine(m_clipRight, m_clipTop, m_clipRight, m_clipBottom, Hu::Color(Style::HighlightColor));
    m_canvas->addLine(m_clipRight, m_clipBottom, m_clipLeft, m_clipBottom, Hu::Color(Style::HighlightColor));
    m_canvas->addLine(m_clipLeft, m_clipBottom, m_clipLeft, m_clipTop, Hu::Color(Style::HighlightColor));
    
    Hu::Color maskColor = Hu::Color(Style::HighlightColor);
    maskColor.alpha() = 0.2;
    m_canvas->addRectangle(0.0, 0.0, m_clipLeft, 1.0, maskColor);
    m_canvas->addRectangle(m_clipRight, 0.0, 1.0, 1.0, maskColor);
    m_canvas->addRectangle(m_clipLeft, 0.0, m_clipRight, m_clipTop, maskColor);
    m_canvas->addRectangle(m_clipLeft, m_clipBottom, m_clipRight, 1.0, maskColor);
    
    Hu::Widget *sourceImageWidget = getWidget(m_sourceImageWidgetId);
    
    double handleHalfWidth = 0.5 * m_handleSize / sourceImageWidget->layoutWidth();
    double handleHalfHeight = 0.5 * m_handleSize / sourceImageWidget->layoutHeight();
    m_canvas->addRectangle(m_clipLeft - handleHalfWidth, m_clipTop - handleHalfHeight, m_clipLeft + handleHalfWidth, m_clipTop + handleHalfHeight, m_leftTopHandleMouseHovering ? Hu::Color(Style::HighlightColor).lighted() : Hu::Color(Style::HighlightColor));
    m_canvas->addRectangle(m_clipRight - handleHalfWidth, m_clipTop - handleHalfHeight, m_clipRight + handleHalfWidth, m_clipTop + handleHalfHeight, m_rightTopHandleMouseHovering ? Hu::Color(Style::HighlightColor).lighted() : Hu::Color(Style::HighlightColor));
    m_canvas->addRectangle(m_clipRight - handleHalfWidth, m_clipBottom - handleHalfHeight, m_clipRight + handleHalfWidth, m_clipBottom + handleHalfHeight, m_rightBottomHandleMouseHovering ? Hu::Color(Style::HighlightColor).lighted() : Hu::Color(Style::HighlightColor));
    m_canvas->addRectangle(m_clipLeft - handleHalfWidth, m_clipBottom - handleHalfHeight, m_clipLeft + handleHalfWidth, m_clipBottom + handleHalfHeight, m_leftBottomHandleMouseHovering ? Hu::Color(Style::HighlightColor).lighted() : Hu::Color(Style::HighlightColor));
}

std::unique_ptr<Hu::Image> &ReferenceImageEditWindow::resizedImage()
{
    return m_resizedImage;
}

std::unique_ptr<Hu::Image> &ReferenceImageEditWindow::frontImage()
{
    return m_frontImage;
}

std::unique_ptr<Hu::Image> &ReferenceImageEditWindow::sideImage()
{
    return m_sideImage;
}

std::unique_ptr<Hu::Image> &ReferenceImageEditWindow::referenceImage()
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
    
    Hu::Image *frontImage = nullptr != m_frontImage ? new Hu::Image(*m_frontImage) : nullptr;
    Hu::Image *sideImage = nullptr != m_sideImage ? new Hu::Image(*m_sideImage) : nullptr;
    engine()->run([=]() {
            Hu::Image *frontScaledImage = nullptr != frontImage ? frontImage->scaledToHeight(ReferenceImageEditWindow::m_targetReferenceHeight) : nullptr;
            Hu::Image *sideScaledImage = nullptr != sideImage ? sideImage->scaledToHeight(ReferenceImageEditWindow::m_targetReferenceHeight) : nullptr;
            size_t frontScaledImageWidth = nullptr != frontScaledImage ? frontScaledImage->width() : 0;
            size_t sideScaledImageWidth = nullptr != sideScaledImage ? sideScaledImage->width() : 0;
            size_t targetWidth = std::max(frontScaledImageWidth + sideScaledImageWidth, ReferenceImageEditWindow::m_targetReferenceWidth);
            Hu::Image *referenceImage = new Hu::Image(targetWidth, ReferenceImageEditWindow::m_targetReferenceHeight);
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
            Hu::Image *referenceImage = (Hu::Image *)result;
            this->engine()->setImageResource("referenceImageEditWindow.referenceImagePreview", referenceImage->width(), referenceImage->height(), referenceImage->data());
            this->referenceImage().reset(referenceImage);
            this->getWidget(this->referenceImagePreviewWidgetId())->setBackgroundImageResourceName("referenceImageEditWindow.referenceImagePreview");
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
    Hu::Image *image = new Hu::Image(*m_resizedImage);
    engine()->run([=]() {
            Hu::Color clearColor(Style::BackgroundColor);
            Hu::Image *clipImage = new Hu::Image(clipRight - clipLeft, clipBottom - clipTop);
            clipImage->clear(clearColor.red() * 255.0, clearColor.green() * 255.0, clearColor.blue() * 255.0, clearColor.alpha() * 255.0);
            clipImage->copy(*image, clipLeft, clipTop, 0, 0, clipImage->width(), clipImage->height());
            delete image;
            return (void *)clipImage;
        }, [=](void *result) {
            Hu::Image *clipImage = (Hu::Image *)result;
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
    Hu::Image *image = new Hu::Image(*m_resizedImage);
    engine()->run([=]() {
            Hu::Color clearColor(Style::BackgroundColor);
            Hu::Image *clipImage = new Hu::Image(clipRight - clipLeft, clipBottom - clipTop);
            clipImage->clear(clearColor.red() * 255.0, clearColor.green() * 255.0, clearColor.blue() * 255.0, clearColor.alpha() * 255.0);
            clipImage->copy(*image, clipLeft, clipTop, 0, 0, clipImage->width(), clipImage->height());
            delete image;
            return (void *)clipImage;
        }, [=](void *result) {
            Hu::Image *clipImage = (Hu::Image *)result;
            this->sideImage().reset(clipImage);
            this->updateReferenceImage();
        }
    );
}

void ReferenceImageEditWindow::updatePreviewImage()
{
    if (nullptr == m_image)
        return;
    
    Hu::Widget *sourceImageWidget = getWidget(m_sourceImageWidgetId);
    size_t targetWidth = sourceImageWidget->layoutWidth();
    size_t targetHeight = sourceImageWidget->layoutHeight();
    Hu::Image *image = new Hu::Image(*m_image);
    engine()->run([=]() {
            size_t toWidth = image->width();
            size_t toHeight = toWidth * targetHeight / targetWidth;
            if (toHeight < image->height()) {
                toHeight = image->height();
                toWidth = toHeight * targetWidth / targetHeight;
            }
            // FIXME: limit the image size to avoid texture error(OpenGL Error : 1281)
            Hu::Color clearColor(Style::BackgroundColor);
            Hu::Image *resizedImage = new Hu::Image(toWidth, toHeight);
            resizedImage->clear(clearColor.red() * 255.0, clearColor.green() * 255.0, clearColor.blue() * 255.0, clearColor.alpha() * 255.0);
            resizedImage->copy(*image, 0, 0, (resizedImage->width() - image->width()) / 2, (resizedImage->height() - image->height()) / 2, image->width(), image->height());
            delete image;
            return (void *)resizedImage;
        }, [=](void *result) {
            Hu::Image *resizedImage = (Hu::Image *)result;
            this->engine()->setImageResource("referenceImageEditWindow.sourceImage", resizedImage->width(), resizedImage->height(), resizedImage->data());
            this->resizedImage().reset(resizedImage);
            this->getWidget(this->sourceImageWidgetId())->setBackgroundImageResourceName("referenceImageEditWindow.sourceImage");
        }
    );
}

const std::string &ReferenceImageEditWindow::sourceImageWidgetId() const
{
    return m_sourceImageWidgetId;
}

const std::string &ReferenceImageEditWindow::referenceImagePreviewWidgetId() const
{
    return m_referenceImagePreviewWidgetId;
}

void ReferenceImageEditWindow::setImage(const std::string &path)
{
    if (path.empty())
        return;
    m_image = std::make_unique<Hu::Image>();
    m_image->load(path.c_str());
    engine()->run([=](void *) {
        this->updatePreviewImage();
    });
}

}
