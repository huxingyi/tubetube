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
#include <hu/base/string.h>
#include <hu/base/image.h>
#include <hu/base/debug.h>
#include <hu/base/uuid.h>
#include <hu/gles/indie_game_engine.h>
#include <hu/widget/push_button.h>
#include <hu/widget/widget.h>
#include <dust3d/desktop/document_window.h>
#include <dust3d/desktop/reference_image_edit_window.h>
#include <dust3d/desktop/style_constants.h>
#include <dust3d/document/ds3_file.h>
#include <dust3d/document/document.h>
#include <dust3d/document/snapshot_xml.h>
#include <dust3d/data/dust3d_vertical_png.h>

namespace Dust3d
{

Hu::Settings *DocumentWindow::settings()
{
    static Hu::Settings *s_settings = new Hu::Settings("tubetube.ini");
    return s_settings;
}

DocumentWindow::DocumentWindow():
    Window(Hu::String::toInt(settings()->value("mainWindow.width", 640)), Hu::String::toInt(settings()->value("mainWindow.height", 360))),
    m_document(std::make_unique<Document>())
{
    setTitle("Tubetube");
    
    auto selectButton = new Hu::PushButton(this);
    selectButton->setName("selectButton");
    selectButton->setPadding(Style::ToolbarIconPadding, Style::ToolbarIconPadding, Style::ToolbarIconPadding, Style::ToolbarIconPadding);
    selectButton->setWidth(Style::ToolbarIconSize, Hu::Widget::SizePolicy::FixedSize);
    selectButton->setHeight(Style::ToolbarIconSize, Hu::Widget::SizePolicy::FixedSize);
    selectButton->setBackgroundColor(Hu::Color(Style::ButtonColor));
    selectButton->setColor(Hu::Color(Style::WhiteColor));
    selectButton->setIcon("toolbar_pointer.svg");
    
    auto addButton = new Hu::PushButton(this);
    addButton->setName("addButton");
    addButton->setPadding(Style::ToolbarIconPadding, Style::ToolbarIconPadding, Style::ToolbarIconPadding, Style::ToolbarIconPadding);
    addButton->setWidth(Style::ToolbarIconSize, Hu::Widget::SizePolicy::FixedSize);
    addButton->setHeight(Style::ToolbarIconSize, Hu::Widget::SizePolicy::FixedSize);
    //addButton->setBackgroundColor(Hu::Color(Style::ButtonColor));
    addButton->setColor(Hu::Color(Style::WhiteColor));
    addButton->setIcon("toolbar_add.svg");
    
    auto xButton = new Hu::PushButton(this);
    xButton->setName("xButton");
    xButton->setPadding(Style::ToolbarIconPadding, Style::ToolbarIconPadding, Style::ToolbarIconPadding, Style::ToolbarIconPadding);
    xButton->setWidth(Style::ToolbarIconSize, Hu::Widget::SizePolicy::FixedSize);
    xButton->setHeight(Style::ToolbarIconSize, Hu::Widget::SizePolicy::FixedSize);
    xButton->setBackgroundColor(Hu::Color(Style::ButtonColor));
    xButton->setColor(Hu::Color(Style::RedColor));
    xButton->setIcon("toolbar_x.svg");
    
    auto yButton = new Hu::PushButton(this);
    yButton->setName("yButton");
    yButton->setPadding(Style::ToolbarIconPadding, Style::ToolbarIconPadding, Style::ToolbarIconPadding, Style::ToolbarIconPadding);
    yButton->setWidth(Style::ToolbarIconSize, Hu::Widget::SizePolicy::FixedSize);
    yButton->setHeight(Style::ToolbarIconSize, Hu::Widget::SizePolicy::FixedSize);
    yButton->setBackgroundColor(Hu::Color(Style::ButtonColor));
    yButton->setColor(Hu::Color(Style::GreenColor));
    yButton->setIcon("toolbar_y.svg");
    
    auto zButton = new Hu::PushButton(this);
    zButton->setName("zButton");
    zButton->setPadding(Style::ToolbarIconPadding, Style::ToolbarIconPadding, Style::ToolbarIconPadding, Style::ToolbarIconPadding);
    zButton->setWidth(Style::ToolbarIconSize, Hu::Widget::SizePolicy::FixedSize);
    zButton->setHeight(Style::ToolbarIconSize, Hu::Widget::SizePolicy::FixedSize);
    zButton->setBackgroundColor(Hu::Color(Style::ButtonColor));
    zButton->setColor(Hu::Color(Style::BlueColor));
    zButton->setIcon("toolbar_z.svg");
    
    auto radiusButton = new Hu::PushButton(this);
    radiusButton->setName("radiusButton");
    radiusButton->setPadding(Style::ToolbarIconPadding, Style::ToolbarIconPadding, Style::ToolbarIconPadding, Style::ToolbarIconPadding);
    radiusButton->setWidth(Style::ToolbarIconSize, Hu::Widget::SizePolicy::FixedSize);
    radiusButton->setHeight(Style::ToolbarIconSize, Hu::Widget::SizePolicy::FixedSize);
    radiusButton->setBackgroundColor(Hu::Color(Style::ButtonColor));
    radiusButton->setColor(Hu::Color(Style::WhiteColor));
    radiusButton->setIcon("toolbar_radius.svg");
    
    auto imageButton = new Hu::PushButton(this);
    imageButton->setName("imageButton");
    imageButton->setPadding(Style::ToolbarIconPadding, Style::ToolbarIconPadding, Style::ToolbarIconPadding, Style::ToolbarIconPadding);
    imageButton->setWidth(Style::ToolbarIconSize, Hu::Widget::SizePolicy::FixedSize);
    imageButton->setHeight(Style::ToolbarIconSize, Hu::Widget::SizePolicy::FixedSize);
    //imageButton->setBackgroundColor(Hu::Color(Style::ButtonColor));
    imageButton->setColor(Hu::Color(Style::WhiteColor));
    imageButton->setIcon("toolbar_image.svg");
    imageButton->mouseEntered.connect([=]() {
        imageButton->setBackgroundColor(Hu::Color(Style::ButtonColor).lighted());
    });
    imageButton->mouseLeaved.connect([=]() {
        imageButton->setBackgroundColor(Hu::Color(0.0, 0.0, 0.0, 0.0));
    });
    imageButton->mousePressed.connect([=]() {
        imageButton->setBackgroundColor(Hu::Color(Style::ButtonColor).darked());
    });
    imageButton->mouseReleased.connect([=]() {
        imageButton->setBackgroundColor(Hu::Color(Style::ButtonColor));
        openReferenceImageEditWindow();
    });
    
    auto selectButtonLayout = new Hu::Widget(this);
    selectButtonLayout->setName("selectButtonLayout");
    selectButtonLayout->setLayoutDirection(Hu::Widget::LayoutDirection::LeftToRight);
    selectButtonLayout->setWidth(Style::ToolbarIconSize + Style::ToolbarSpacing * 2, Hu::Widget::SizePolicy::FixedSize);
    selectButtonLayout->addSpacing(Style::ToolbarSpacing);
    selectButtonLayout->addWidget(selectButton);
    selectButtonLayout->addSpacing(Style::ToolbarSpacing);
    
    auto addButtonLayout = new Hu::Widget(this);
    addButtonLayout->setName("addButtonLayout");
    addButtonLayout->setLayoutDirection(Hu::Widget::LayoutDirection::LeftToRight);
    addButtonLayout->setWidth(Style::ToolbarIconSize + Style::ToolbarSpacing * 2, Hu::Widget::SizePolicy::FixedSize);
    addButtonLayout->addSpacing(Style::ToolbarSpacing);
    addButtonLayout->addWidget(addButton);
    addButtonLayout->addSpacing(Style::ToolbarSpacing);
    
    auto xButtonLayout = new Hu::Widget(this);
    xButtonLayout->setName("xButtonLayout");
    xButtonLayout->setLayoutDirection(Hu::Widget::LayoutDirection::LeftToRight);
    xButtonLayout->setWidth(Style::ToolbarIconSize + Style::ToolbarSpacing * 2, Hu::Widget::SizePolicy::FixedSize);
    xButtonLayout->addSpacing(Style::ToolbarSpacing);
    xButtonLayout->addWidget(xButton);
    xButtonLayout->addSpacing(Style::ToolbarSpacing);
    
    auto yButtonLayout = new Hu::Widget(this);
    yButtonLayout->setName("yButtonLayout");
    yButtonLayout->setLayoutDirection(Hu::Widget::LayoutDirection::LeftToRight);
    yButtonLayout->setWidth(Style::ToolbarIconSize + Style::ToolbarSpacing * 2, Hu::Widget::SizePolicy::FixedSize);
    yButtonLayout->addSpacing(Style::ToolbarSpacing);
    yButtonLayout->addWidget(yButton);
    yButtonLayout->addSpacing(Style::ToolbarSpacing);
    
    auto zButtonLayout = new Hu::Widget(this);
    zButtonLayout->setName("zButtonLayout");
    zButtonLayout->setLayoutDirection(Hu::Widget::LayoutDirection::LeftToRight);
    zButtonLayout->setWidth(Style::ToolbarIconSize + Style::ToolbarSpacing * 2, Hu::Widget::SizePolicy::FixedSize);
    zButtonLayout->addSpacing(Style::ToolbarSpacing);
    zButtonLayout->addWidget(zButton);
    zButtonLayout->addSpacing(Style::ToolbarSpacing);
    
    auto radiusButtonLayout = new Hu::Widget(this);
    radiusButtonLayout->setName("radiusButtonLayout");
    radiusButtonLayout->setLayoutDirection(Hu::Widget::LayoutDirection::LeftToRight);
    radiusButtonLayout->setWidth(Style::ToolbarIconSize + Style::ToolbarSpacing * 2, Hu::Widget::SizePolicy::FixedSize);
    radiusButtonLayout->addSpacing(Style::ToolbarSpacing);
    radiusButtonLayout->addWidget(radiusButton);
    radiusButtonLayout->addSpacing(Style::ToolbarSpacing);
    
    auto imageButtonLayout = new Hu::Widget(this);
    imageButtonLayout->setName("imageButtonLayout");
    imageButtonLayout->setLayoutDirection(Hu::Widget::LayoutDirection::LeftToRight);
    imageButtonLayout->setWidth(Style::ToolbarIconSize + Style::ToolbarSpacing * 2, Hu::Widget::SizePolicy::FixedSize);
    imageButtonLayout->addSpacing(Style::ToolbarSpacing);
    imageButtonLayout->addWidget(imageButton);
    imageButtonLayout->addSpacing(Style::ToolbarSpacing);
    
    auto backgroundImageWidget = new Hu::Widget(this, "documentWindow.turnaround");
    backgroundImageWidget->setName("backgroundImageWidget");
    backgroundImageWidget->setLayoutDirection(Hu::Widget::LayoutDirection::LeftToRight);
    backgroundImageWidget->setHeight(1.0, Hu::Widget::SizePolicy::RelativeSize);
    backgroundImageWidget->setWidth(1.0, Hu::Widget::SizePolicy::FlexibleSize);
    backgroundImageWidget->setBackgroundColor(Hu::Color(Style::BackgroundColor));
    backgroundImageWidget->setBackgroundImageOpacity(0.25);
    
    auto logoWidget = new Hu::Widget(this);
    logoWidget->setName("logoWidget");
    logoWidget->setWidth(25.0, Hu::Widget::SizePolicy::FixedSize);
    logoWidget->setHeight(71.0, Hu::Widget::SizePolicy::FixedSize);
    logoWidget->setBackgroundImageResourceName("dust3d/data/dust3d_vertical.png");
    
    auto leftBarLayout = new Hu::Widget(this);
    leftBarLayout->setName("leftBarLayout");
    leftBarLayout->setLayoutDirection(Hu::Widget::LayoutDirection::TopToBottom);
    leftBarLayout->setWidth(1.0, Hu::Widget::SizePolicy::MinimalSize);
    leftBarLayout->setHeight(1.0, Hu::Widget::SizePolicy::RelativeSize);
    leftBarLayout->addSpacing(Style::ToolbarSpacing * 2.0);
    leftBarLayout->addWidget(selectButtonLayout);
    leftBarLayout->addSpacing(Style::ToolbarSpacing * 0.5);
    leftBarLayout->addWidget(addButtonLayout);
    leftBarLayout->addSpacing(Style::ToolbarSpacing * 3.0);
    leftBarLayout->addWidget(xButtonLayout);
    leftBarLayout->addWidget(yButtonLayout);
    leftBarLayout->addWidget(zButtonLayout);
    leftBarLayout->addWidget(radiusButtonLayout);
    leftBarLayout->addSpacing(Style::ToolbarSpacing * 3.0);
    leftBarLayout->addWidget(imageButtonLayout);
    leftBarLayout->addExpanding();
    leftBarLayout->addWidget(logoWidget);
    leftBarLayout->addSpacing(Style::ToolbarSpacing);
    
    auto mainLayout = new Hu::Widget(this);
    mainLayout->setName("mainLayout");
    mainLayout->setLayoutDirection(Hu::Widget::LayoutDirection::LeftToRight);
    mainLayout->setHeight(1.0, Hu::Widget::SizePolicy::RelativeSize);
    mainLayout->addWidget(leftBarLayout);
    mainLayout->addWidget(backgroundImageWidget);
    
    engine()->rootWidget()->setBackgroundColor(Hu::Color(Style::FrameBackgroundColor));
    engine()->rootWidget()->setName("rootWidget");
    engine()->rootWidget()->addWidget(mainLayout);
    
    engine()->run([=]() {
            Hu::Image *image = new Hu::Image;
            image->load(Data::dust3d_vertical_png, sizeof(Data::dust3d_vertical_png));
            return (void *)image;
        }, [=](void *result) {
            Hu::Image *image = (Hu::Image *)result;
            this->engine()->setImageResource("dust3d/data/dust3d_vertical.png", image->width(), image->height(), image->data());
            delete image;
        }
    );
    
    engine()->windowSizeChanged.connect(std::bind(&DocumentWindow::updateReferenceImageView, this));
    engine()->shouldPopupMenu.connect(std::bind(&DocumentWindow::popupMenu, this));
    
    m_document->referenceImageChanged.connect(std::bind(&DocumentWindow::updateReferenceImageView, this));
    m_document->open("model-bicycle.ds3");
    
    setVisible(true);
}

void DocumentWindow::popupMenu()
{
    //Window *menu = openPopupWindow();
    //menu->engine()->setBackgroundColor(Hu::Color("#efefef"));
    //menu->setVisible(true);
    //ReferenceImageEditWindow *editWindow = new ReferenceImageEditWindow();
    //editWindow->setVisible(true);
}

void DocumentWindow::openReferenceImageEditWindow()
{
    if (nullptr == m_referenceImageEditWindow) {
        m_referenceImageEditWindow = std::make_unique<ReferenceImageEditWindow>();
        m_referenceImageEditWindow->confirmed.connect([=] {
            if (nullptr != this->m_referenceImageEditWindow->referenceImage()) {
                document()->setReferenceImage(std::move(this->m_referenceImageEditWindow->referenceImage()));
            }
            this->m_referenceImageEditWindow.reset();
        });
        m_referenceImageEditWindow->closed.connect([=] {
            this->m_referenceImageEditWindow.reset();
        });
    }
    m_referenceImageEditWindow->setVisible(true);
    m_referenceImageEditWindow->bringToForeground();
}

DirtyFlags &DocumentWindow::referenceImageFlags()
{
    return m_referenceImageFlags;
}

void DocumentWindow::updateReferenceImageView()
{
    if (m_referenceImageFlags.processing) {
        m_referenceImageFlags.dirty = true;
        return;
    }
    
    m_referenceImageFlags.dirty = false;
    
    if (nullptr == document()->referenceImage())
        return;

    Hu::Widget *turnaroundWidget = getWidget("documentWindow.turnaround");
    size_t targetWidth = turnaroundWidget->layoutWidth();
    size_t targetHeight = turnaroundWidget->layoutHeight();
    
    if (0 == targetWidth || 0 == targetHeight)
        return;
    
    m_referenceImageFlags.processing = true;
    Hu::Image *image = new Hu::Image(*document()->referenceImage());
    engine()->run([=]() {
            size_t toWidth = image->width();
            size_t toHeight = toWidth * targetHeight / targetWidth;
            if (toHeight < image->height()) {
                toHeight = image->height();
                toWidth = toHeight * targetWidth / targetHeight;
            }
            Hu::Image *resizedImage = new Hu::Image(toWidth, toHeight);
            resizedImage->clear(255, 255, 255, 0);
            resizedImage->copy(*image, 0, 0, (resizedImage->width() - image->width()) / 2, (resizedImage->height() - image->height()) / 2, image->width(), image->height());
            delete image;
            return (void *)resizedImage;
        }, [=](void *result) {
            Hu::Image *resizedImage = (Hu::Image *)result;
            this->engine()->setImageResource("documentWindow.turnaround", resizedImage->width(), resizedImage->height(), resizedImage->data());
            
            //{
            //    auto testImage = std::make_unique<Hu::Image>(*resizedImage);
            //    Document document;
            //    document.setReferenceImage(std::move(testImage));
            //    document.save("C:\\Users\\Jeremy\\Repositories\\tubetube\\bin\\test.ds3");
            //}
            
            delete resizedImage;
            this->getWidget("documentWindow.turnaround")->setBackgroundImageResourceName("documentWindow.turnaround");
            this->referenceImageFlags().processing = false;
            if (this->referenceImageFlags().dirty)
                this->updateReferenceImageView();
        }
    );
}

Document *DocumentWindow::document()
{
    return m_document.get();
}

}
