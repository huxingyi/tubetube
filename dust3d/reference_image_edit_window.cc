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
#include <dust3d/reference_image_edit_window.h>
#include <dust3d/document_window.h>

ReferenceImageEditWindow::ReferenceImageEditWindow():
    Window(String::toInt(DocumentWindow::settings()->value("referenceImageEditWindow.width", 640)), String::toInt(DocumentWindow::settings()->value("referenceImageEditWindow.height", 360)))
{
    setTitle("Reference image edit");
    engine()->setBackgroundColor(Color("#00000000"));
    
    auto previewLayout = new Widget("PreviewImage");
    previewLayout->setName("previewLayout");
    previewLayout->setWidth(1.0, Widget::SizePolicy::FlexibleSize);
    previewLayout->setHeight(1.0, Widget::SizePolicy::RelativeSize);
    previewLayout->setBackgroundColor(Color("#000000"));
    
    auto rightLayout = new Widget;
    rightLayout->setName("rightLayout");
    rightLayout->setLayoutDirection(Widget::LayoutDirection::TopToBottom);
    rightLayout->setWidth(250.0, Widget::SizePolicy::FixedSize);
    rightLayout->setHeight(1.0, Widget::SizePolicy::RelativeSize);
    rightLayout->setBackgroundColor(Color("#252525"));

    auto loadImageButton = new Button;
    loadImageButton->setHeight(20.0 + loadImageButton->paddingHeight(), Widget::SizePolicy::FixedSize);
    loadImageButton->setText("Select image");
    loadImageButton->setBackgroundColor(Color("#fc6621"));
    loadImageButton->setColor(Color("#ffffff"));
    engine()->windowSizeChanged.connect([=]() {
        loadImageButton->setWidth(engine()->measureFontWidth(loadImageButton->text(), loadImageButton->layoutHeight() - loadImageButton->paddingHeight()) + loadImageButton->paddingWidth(), Widget::SizePolicy::FixedSize);
    });
    loadImageButton->mouseEntered.connect([=]() {
        loadImageButton->setBackgroundColor(Color("#fc6621").lighted());
    });
    loadImageButton->mouseLeaved.connect([=]() {
        loadImageButton->setBackgroundColor(Color("#fc6621"));
    });
    loadImageButton->mousePressed.connect([=]() {
        loadImageButton->setBackgroundColor(Color("#fc6621").darked());
    });
    loadImageButton->mouseReleased.connect([=]() {
        loadImageButton->setBackgroundColor(Color("#fc6621"));
        auto selectedFile = this->selectSingleFileByUser({"jpg", "jpeg", "png"});
        std::cout << "selectedFile:" << selectedFile << std::endl;
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
    
    rightLayout->addSpacing(30.0);
    rightLayout->addWidget(loadImageButtonLayout);
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
            Image *resizedImage = new Image(toWidth, toHeight);
            resizedImage->clear(0, 0, 0, 0);
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
