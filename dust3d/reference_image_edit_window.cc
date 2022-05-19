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
    
    auto previewLayout = new Widget("PreviewImage");
    previewLayout->setName("previewLayout");
    previewLayout->setWidth(1.0, Widget::SizePolicy::FlexibleSize);
    previewLayout->setHeight(1.0, Widget::SizePolicy::RelativeSize);
    previewLayout->setBackgroundColor(Color(Style::BackgroundColor));
    
    auto canvas = new Canvas;
    canvas->setWidth(1.0, Widget::SizePolicy::RelativeSize);
    canvas->setHeight(1.0, Widget::SizePolicy::RelativeSize);
    canvas->addLine(0.1, 0.5, 0.9, 0.5, Color("#00ff00"));
    canvas->addRectangle(0.05, 0.45, 0.15, 0.55, Color("#00ff00"));
    previewLayout->addWidget(canvas);
    
    auto rightLayout = new Widget;
    rightLayout->setName("rightLayout");
    rightLayout->setLayoutDirection(Widget::LayoutDirection::TopToBottom);
    rightLayout->setWidth(250.0, Widget::SizePolicy::FixedSize);
    rightLayout->setHeight(1.0, Widget::SizePolicy::RelativeSize);
    rightLayout->setBackgroundColor(Color(Style::FrameBackgroundColor));

    auto loadImageButton = new PushButton;
    loadImageButton->setHeight(Style::NormalFontLineHeight + loadImageButton->paddingHeight(), Widget::SizePolicy::FixedSize);
    loadImageButton->setText("Load image");
    loadImageButton->setBackgroundColor(Color(Style::MainColor));
    loadImageButton->setColor(Color(Style::MainInversedFontColor));
    engine()->windowSizeChanged.connect([=]() {
        loadImageButton->setWidth(engine()->measureFontWidth(loadImageButton->text(), loadImageButton->layoutHeight() - loadImageButton->paddingHeight()) + loadImageButton->paddingWidth(), Widget::SizePolicy::FixedSize);
    });
    loadImageButton->mouseEntered.connect([=]() {
        loadImageButton->setBackgroundColor(Color(Style::MainColor).lighted());
    });
    loadImageButton->mouseLeaved.connect([=]() {
        loadImageButton->setBackgroundColor(Color(Style::MainColor));
    });
    loadImageButton->mousePressed.connect([=]() {
        loadImageButton->setBackgroundColor(Color(Style::MainColor).darked());
    });
    loadImageButton->mouseReleased.connect([=]() {
        loadImageButton->setBackgroundColor(Color(Style::MainColor));
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
    
    auto profileText = new Text;
    profileText->setText("Target area:");
    profileText->setHeight(Style::NormalFontLineHeight, Widget::SizePolicy::FixedSize);
    profileText->setColor(Color(Style::MainFontColor));
    engine()->windowSizeChanged.connect([=]() {
        profileText->setWidth(engine()->measureFontWidth(profileText->text(), profileText->layoutHeight() - profileText->paddingHeight()), Widget::SizePolicy::FixedSize);
    });
    
    auto frontProfileRadioButton = new RadioButton;
    m_frontProfileRadioButton = frontProfileRadioButton;
    frontProfileRadioButton->setText("Front");
    frontProfileRadioButton->setHeight(Style::NormalFontLineHeight, Widget::SizePolicy::FixedSize);
    frontProfileRadioButton->setBackgroundColor(Color(Style::MainColor));
    frontProfileRadioButton->setColor(Color(Style::MainFontColor));
    frontProfileRadioButton->setChecked(true);
    engine()->windowSizeChanged.connect([=]() {
        frontProfileRadioButton->setWidth(engine()->measureFontWidth(frontProfileRadioButton->text(), frontProfileRadioButton->layoutHeight() - frontProfileRadioButton->paddingHeight()) + frontProfileRadioButton->layoutHeight() * 1.5 + frontProfileRadioButton->paddingWidth(), Widget::SizePolicy::FixedSize);
    });
    frontProfileRadioButton->mouseEntered.connect([=]() {
        frontProfileRadioButton->setBackgroundColor(Color(Style::MainColor).lighted());
    });
    frontProfileRadioButton->mouseLeaved.connect([=]() {
        frontProfileRadioButton->setBackgroundColor(Color(Style::MainColor));
    });
    frontProfileRadioButton->mousePressed.connect([=]() {
        setTargetArea(TargetArea::Front);
    });
    
    auto sideProfileRadioButton = new RadioButton;
    m_sideProfileRadioButton = sideProfileRadioButton;
    sideProfileRadioButton->setText("Side");
    sideProfileRadioButton->setHeight(Style::NormalFontLineHeight, Widget::SizePolicy::FixedSize);
    sideProfileRadioButton->setBackgroundColor(Color(Style::MainColor));
    sideProfileRadioButton->setColor(Color(Style::MainFontColor));
    engine()->windowSizeChanged.connect([=]() {
        sideProfileRadioButton->setWidth(engine()->measureFontWidth(sideProfileRadioButton->text(), sideProfileRadioButton->layoutHeight() - sideProfileRadioButton->paddingHeight()) + sideProfileRadioButton->layoutHeight() * 1.5 + sideProfileRadioButton->paddingWidth(), Widget::SizePolicy::FixedSize);
    });
    sideProfileRadioButton->mouseEntered.connect([=]() {
        sideProfileRadioButton->setBackgroundColor(Color(Style::MainColor).lighted());
    });
    sideProfileRadioButton->mouseLeaved.connect([=]() {
        sideProfileRadioButton->setBackgroundColor(Color(Style::MainColor));
    });
    sideProfileRadioButton->mousePressed.connect([=]() {
        setTargetArea(TargetArea::Side);
    });
    
    setTargetArea(m_targetArea, true);
    
    auto profileRadiosLayout = new Widget;
    profileRadiosLayout->setName("profileRadiosLayout");
    profileRadiosLayout->setLayoutDirection(Widget::LayoutDirection::LeftToRight);
    profileRadiosLayout->addSpacing(15.0);
    profileRadiosLayout->addWidget(profileText);
    profileRadiosLayout->addExpanding();
    profileRadiosLayout->addSpacing(10.0);
    profileRadiosLayout->addWidget(frontProfileRadioButton);
    profileRadiosLayout->addSpacing(10.0);
    profileRadiosLayout->addWidget(sideProfileRadioButton);
    profileRadiosLayout->addSpacing(15.0);
    
    rightLayout->addSpacing(30.0);
    rightLayout->addWidget(loadImageButtonLayout);
    rightLayout->addSpacing(35.0);
    rightLayout->addWidget(profileRadiosLayout);
    rightLayout->addExpanding();
    
    auto mainLayout = new Widget;
    mainLayout->setName("mainLayout");
    mainLayout->setHeight(1.0, Widget::SizePolicy::RelativeSize);
    mainLayout->setWidth(1.0, Widget::SizePolicy::RelativeSize);
    mainLayout->addWidget(previewLayout);
    mainLayout->addWidget(rightLayout);
    
    engine()->rootWidget()->setName("rootWidget");
    engine()->rootWidget()->addWidget(mainLayout);
    
    engine()->windowSizeChanged.connect(std::bind(&ReferenceImageEditWindow::updatePreviewImage, this));
    
    setVisible(true);
}

void ReferenceImageEditWindow::setTargetArea(TargetArea targetArea, bool forceUpdate)
{
    if (!forceUpdate && m_targetArea == targetArea)
        return;
    m_targetArea = targetArea;
    if (nullptr != m_frontProfileRadioButton)
        m_frontProfileRadioButton->setChecked(TargetArea::Front == m_targetArea);
    if (nullptr != m_sideProfileRadioButton)
        m_sideProfileRadioButton->setChecked(TargetArea::Side == m_targetArea);
}

void ReferenceImageEditWindow::updatePreviewImage()
{
    if (nullptr == m_image)
        return;
    
    Widget *previewImageWidget = Widget::get("PreviewImage");
    size_t targetWidth = previewImageWidget->layoutWidth();
    size_t targetHeight = previewImageWidget->layoutHeight();
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
            this->engine()->setImageResource("preview-image", resizedImage->width(), resizedImage->height(), resizedImage->data());
            delete resizedImage;
            Widget::get("PreviewImage")->setBackgroundImageResourceName("preview-image");
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
