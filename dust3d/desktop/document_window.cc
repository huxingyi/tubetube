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

Settings *DocumentWindow::settings()
{
    static Settings *s_settings = new Settings("tubetube.ini");
    return s_settings;
}

DocumentWindow::DocumentWindow():
    Window(String::toInt(settings()->value("mainWindow.width", 640)), String::toInt(settings()->value("mainWindow.height", 360)))
{
    setTitle("Tubetube");
    
    auto selectButton = new PushButton(this);
    selectButton->setName("selectButton");
    selectButton->setPadding(Style::ToolbarIconPadding, Style::ToolbarIconPadding, Style::ToolbarIconPadding, Style::ToolbarIconPadding);
    selectButton->setWidth(Style::ToolbarIconSize, Widget::SizePolicy::FixedSize);
    selectButton->setHeight(Style::ToolbarIconSize, Widget::SizePolicy::FixedSize);
    selectButton->setBackgroundColor(Color(Style::ButtonColor));
    selectButton->setColor(Color(Style::WhiteColor));
    selectButton->setIcon("toolbar_pointer.svg");
    
    auto addButton = new PushButton(this);
    addButton->setName("addButton");
    addButton->setPadding(Style::ToolbarIconPadding, Style::ToolbarIconPadding, Style::ToolbarIconPadding, Style::ToolbarIconPadding);
    addButton->setWidth(Style::ToolbarIconSize, Widget::SizePolicy::FixedSize);
    addButton->setHeight(Style::ToolbarIconSize, Widget::SizePolicy::FixedSize);
    //addButton->setBackgroundColor(Color(Style::ButtonColor));
    addButton->setColor(Color(Style::WhiteColor));
    addButton->setIcon("toolbar_add.svg");
    
    auto xButton = new PushButton(this);
    xButton->setName("xButton");
    xButton->setPadding(Style::ToolbarIconPadding, Style::ToolbarIconPadding, Style::ToolbarIconPadding, Style::ToolbarIconPadding);
    xButton->setWidth(Style::ToolbarIconSize, Widget::SizePolicy::FixedSize);
    xButton->setHeight(Style::ToolbarIconSize, Widget::SizePolicy::FixedSize);
    xButton->setBackgroundColor(Color(Style::ButtonColor));
    xButton->setColor(Color(Style::RedColor));
    xButton->setIcon("toolbar_x.svg");
    
    auto yButton = new PushButton(this);
    yButton->setName("yButton");
    yButton->setPadding(Style::ToolbarIconPadding, Style::ToolbarIconPadding, Style::ToolbarIconPadding, Style::ToolbarIconPadding);
    yButton->setWidth(Style::ToolbarIconSize, Widget::SizePolicy::FixedSize);
    yButton->setHeight(Style::ToolbarIconSize, Widget::SizePolicy::FixedSize);
    yButton->setBackgroundColor(Color(Style::ButtonColor));
    yButton->setColor(Color(Style::GreenColor));
    yButton->setIcon("toolbar_y.svg");
    
    auto zButton = new PushButton(this);
    zButton->setName("zButton");
    zButton->setPadding(Style::ToolbarIconPadding, Style::ToolbarIconPadding, Style::ToolbarIconPadding, Style::ToolbarIconPadding);
    zButton->setWidth(Style::ToolbarIconSize, Widget::SizePolicy::FixedSize);
    zButton->setHeight(Style::ToolbarIconSize, Widget::SizePolicy::FixedSize);
    zButton->setBackgroundColor(Color(Style::ButtonColor));
    zButton->setColor(Color(Style::BlueColor));
    zButton->setIcon("toolbar_z.svg");
    
    auto radiusButton = new PushButton(this);
    radiusButton->setName("radiusButton");
    radiusButton->setPadding(Style::ToolbarIconPadding, Style::ToolbarIconPadding, Style::ToolbarIconPadding, Style::ToolbarIconPadding);
    radiusButton->setWidth(Style::ToolbarIconSize, Widget::SizePolicy::FixedSize);
    radiusButton->setHeight(Style::ToolbarIconSize, Widget::SizePolicy::FixedSize);
    radiusButton->setBackgroundColor(Color(Style::ButtonColor));
    radiusButton->setColor(Color(Style::WhiteColor));
    radiusButton->setIcon("toolbar_radius.svg");
    
    auto imageButton = new PushButton(this);
    imageButton->setName("imageButton");
    imageButton->setPadding(Style::ToolbarIconPadding, Style::ToolbarIconPadding, Style::ToolbarIconPadding, Style::ToolbarIconPadding);
    imageButton->setWidth(Style::ToolbarIconSize, Widget::SizePolicy::FixedSize);
    imageButton->setHeight(Style::ToolbarIconSize, Widget::SizePolicy::FixedSize);
    //imageButton->setBackgroundColor(Color(Style::ButtonColor));
    imageButton->setColor(Color(Style::WhiteColor));
    imageButton->setIcon("toolbar_image.svg");
    imageButton->mouseEntered.connect([=]() {
        imageButton->setBackgroundColor(Color(Style::ButtonColor).lighted());
    });
    imageButton->mouseLeaved.connect([=]() {
        imageButton->setBackgroundColor(Color(0.0, 0.0, 0.0, 0.0));
    });
    imageButton->mousePressed.connect([=]() {
        imageButton->setBackgroundColor(Color(Style::ButtonColor).darked());
    });
    imageButton->mouseReleased.connect([=]() {
        imageButton->setBackgroundColor(Color(Style::ButtonColor));
        openReferenceImageEditWindow();
    });
    
    auto selectButtonLayout = new Widget(this);
    selectButtonLayout->setName("selectButtonLayout");
    selectButtonLayout->setLayoutDirection(Widget::LayoutDirection::LeftToRight);
    selectButtonLayout->setWidth(Style::ToolbarIconSize + Style::ToolbarSpacing * 2, Widget::SizePolicy::FixedSize);
    selectButtonLayout->addSpacing(Style::ToolbarSpacing);
    selectButtonLayout->addWidget(selectButton);
    selectButtonLayout->addSpacing(Style::ToolbarSpacing);
    
    auto addButtonLayout = new Widget(this);
    addButtonLayout->setName("addButtonLayout");
    addButtonLayout->setLayoutDirection(Widget::LayoutDirection::LeftToRight);
    addButtonLayout->setWidth(Style::ToolbarIconSize + Style::ToolbarSpacing * 2, Widget::SizePolicy::FixedSize);
    addButtonLayout->addSpacing(Style::ToolbarSpacing);
    addButtonLayout->addWidget(addButton);
    addButtonLayout->addSpacing(Style::ToolbarSpacing);
    
    auto xButtonLayout = new Widget(this);
    xButtonLayout->setName("xButtonLayout");
    xButtonLayout->setLayoutDirection(Widget::LayoutDirection::LeftToRight);
    xButtonLayout->setWidth(Style::ToolbarIconSize + Style::ToolbarSpacing * 2, Widget::SizePolicy::FixedSize);
    xButtonLayout->addSpacing(Style::ToolbarSpacing);
    xButtonLayout->addWidget(xButton);
    xButtonLayout->addSpacing(Style::ToolbarSpacing);
    
    auto yButtonLayout = new Widget(this);
    yButtonLayout->setName("yButtonLayout");
    yButtonLayout->setLayoutDirection(Widget::LayoutDirection::LeftToRight);
    yButtonLayout->setWidth(Style::ToolbarIconSize + Style::ToolbarSpacing * 2, Widget::SizePolicy::FixedSize);
    yButtonLayout->addSpacing(Style::ToolbarSpacing);
    yButtonLayout->addWidget(yButton);
    yButtonLayout->addSpacing(Style::ToolbarSpacing);
    
    auto zButtonLayout = new Widget(this);
    zButtonLayout->setName("zButtonLayout");
    zButtonLayout->setLayoutDirection(Widget::LayoutDirection::LeftToRight);
    zButtonLayout->setWidth(Style::ToolbarIconSize + Style::ToolbarSpacing * 2, Widget::SizePolicy::FixedSize);
    zButtonLayout->addSpacing(Style::ToolbarSpacing);
    zButtonLayout->addWidget(zButton);
    zButtonLayout->addSpacing(Style::ToolbarSpacing);
    
    auto radiusButtonLayout = new Widget(this);
    radiusButtonLayout->setName("radiusButtonLayout");
    radiusButtonLayout->setLayoutDirection(Widget::LayoutDirection::LeftToRight);
    radiusButtonLayout->setWidth(Style::ToolbarIconSize + Style::ToolbarSpacing * 2, Widget::SizePolicy::FixedSize);
    radiusButtonLayout->addSpacing(Style::ToolbarSpacing);
    radiusButtonLayout->addWidget(radiusButton);
    radiusButtonLayout->addSpacing(Style::ToolbarSpacing);
    
    auto imageButtonLayout = new Widget(this);
    imageButtonLayout->setName("imageButtonLayout");
    imageButtonLayout->setLayoutDirection(Widget::LayoutDirection::LeftToRight);
    imageButtonLayout->setWidth(Style::ToolbarIconSize + Style::ToolbarSpacing * 2, Widget::SizePolicy::FixedSize);
    imageButtonLayout->addSpacing(Style::ToolbarSpacing);
    imageButtonLayout->addWidget(imageButton);
    imageButtonLayout->addSpacing(Style::ToolbarSpacing);
    
    auto backgroundImageWidget = new Widget(this, "documentWindow.turnaround");
    backgroundImageWidget->setName("backgroundImageWidget");
    backgroundImageWidget->setLayoutDirection(Widget::LayoutDirection::LeftToRight);
    backgroundImageWidget->setHeight(1.0, Widget::SizePolicy::RelativeSize);
    backgroundImageWidget->setWidth(1.0, Widget::SizePolicy::FlexibleSize);
    backgroundImageWidget->setBackgroundColor(Color(Style::BackgroundColor));
    backgroundImageWidget->setBackgroundImageOpacity(0.25);
    
    auto logoWidget = new Widget(this);
    logoWidget->setName("logoWidget");
    logoWidget->setWidth(25.0, Widget::SizePolicy::FixedSize);
    logoWidget->setHeight(71.0, Widget::SizePolicy::FixedSize);
    logoWidget->setBackgroundImageResourceName("dust3d/data/dust3d_vertical.png");
    
    auto leftBarLayout = new Widget(this);
    leftBarLayout->setName("leftBarLayout");
    leftBarLayout->setLayoutDirection(Widget::LayoutDirection::TopToBottom);
    leftBarLayout->setWidth(1.0, Widget::SizePolicy::MinimalSize);
    leftBarLayout->setHeight(1.0, Widget::SizePolicy::RelativeSize);
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
    
    auto mainLayout = new Widget(this);
    mainLayout->setName("mainLayout");
    mainLayout->setLayoutDirection(Widget::LayoutDirection::LeftToRight);
    mainLayout->setHeight(1.0, Widget::SizePolicy::RelativeSize);
    mainLayout->addWidget(leftBarLayout);
    mainLayout->addWidget(backgroundImageWidget);
    
    engine()->rootWidget()->setBackgroundColor(Color(Style::FrameBackgroundColor));
    engine()->rootWidget()->setName("rootWidget");
    engine()->rootWidget()->addWidget(mainLayout);
    
    engine()->run([=]() {
            Image *image = new Image;
            image->load(Data::dust3d_vertical_png, sizeof(Data::dust3d_vertical_png));
            return (void *)image;
        }, [=](void *result) {
            Image *image = (Image *)result;
            this->engine()->setImageResource("dust3d/data/dust3d_vertical.png", image->width(), image->height(), image->data());
            delete image;
        }
    );
    
    engine()->windowSizeChanged.connect(std::bind(&DocumentWindow::updateReferenceImage, this));
    engine()->shouldPopupMenu.connect(std::bind(&DocumentWindow::popupMenu, this));
    
    setVisible(true);
    
    //setReferenceImage("reference-image.jpg");
}

void DocumentWindow::popupMenu()
{
    //Window *menu = openPopupWindow();
    //menu->engine()->setBackgroundColor(Color("#efefef"));
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
                m_referenceImage = std::move(this->m_referenceImageEditWindow->referenceImage());
                this->updateReferenceImage();
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

void DocumentWindow::updateReferenceImage()
{
    if (m_referenceImageFlags.processing) {
        m_referenceImageFlags.dirty = true;
        return;
    }
    
    m_referenceImageFlags.dirty = false;
    
    if (nullptr == m_referenceImage)
        return;
    
    m_referenceImageFlags.processing = true;
    
    Widget *turnaroundWidget = getWidget("documentWindow.turnaround");
    size_t targetWidth = turnaroundWidget->layoutWidth();
    size_t targetHeight = turnaroundWidget->layoutHeight();
    Image *image = new Image(*m_referenceImage);
    engine()->run([=]() {
            size_t toWidth = image->width();
            size_t toHeight = toWidth * targetHeight / targetWidth;
            if (toHeight < image->height()) {
                toHeight = image->height();
                toWidth = toHeight * targetWidth / targetHeight;
            }
            Image *resizedImage = new Image(toWidth, toHeight);
            resizedImage->clear(255, 255, 255, 0);
            resizedImage->copy(*image, 0, 0, (resizedImage->width() - image->width()) / 2, (resizedImage->height() - image->height()) / 2, image->width(), image->height());
            delete image;
            return (void *)resizedImage;
        }, [=](void *result) {
            Image *resizedImage = (Image *)result;
            this->engine()->setImageResource("documentWindow.turnaround", resizedImage->width(), resizedImage->height(), resizedImage->data());
            
            //{
            //    auto testImage = std::make_unique<Image>(*resizedImage);
            //    Dust3d::Document document;
            //    document.setReferenceImage(std::move(testImage));
            //    document.save("C:\\Users\\Jeremy\\Repositories\\tubetube\\bin\\test.ds3");
            //}
            
            delete resizedImage;
            this->getWidget("documentWindow.turnaround")->setBackgroundImageResourceName("documentWindow.turnaround");
            this->referenceImageFlags().processing = false;
            if (this->referenceImageFlags().dirty)
                this->updateReferenceImage();
        }
    );
}

void DocumentWindow::setReferenceImage(const std::string &path)
{
    m_referenceImage = std::make_unique<Image>();
    m_referenceImage->load(path.c_str());
    engine()->run([=](void *) {
        this->updateReferenceImage();
    });
}
