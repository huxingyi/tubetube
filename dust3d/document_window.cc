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
#include <hu/gles/indie_game_engine.h>
#include <hu/widget/button.h>
#include <hu/widget/widget.h>
#include <dust3d/document_window.h>
#include <dust3d/data/dust3d_vertical_png.h>

Settings *DocumentWindow::settings()
{
    static Settings *s_settings = new Settings("tubetube.ini");
    return s_settings;
}

DocumentWindow::DocumentWindow():
    Window(String::toInt(settings()->value("window.width", 640)), String::toInt(settings()->value("window.height", 360)))
{
    setTitle("Tubetube");

    engine()->setBackgroundColor(Color("#00252525"));
    
    auto toolBoxWidget = new Widget;
    toolBoxWidget->setName("toolBoxWidget");
    toolBoxWidget->setLayoutDirection(Widget::LayoutDirection::LeftToRight);
    toolBoxWidget->addSpacing(5.0);
    auto openReferenceSheetButton = new Button;
    openReferenceSheetButton->setName("openReferenceSheetButton");
    openReferenceSheetButton->setBackgroundColor(Color("#fc6621"));
    openReferenceSheetButton->setColor(Color("#000000")); //f7d9c8
    openReferenceSheetButton->setText("Open Image..");
    openReferenceSheetButton->setIcon("toolbar_pointer.svg");
    toolBoxWidget->addWidget(openReferenceSheetButton);
    toolBoxWidget->addExpanding(0.5);
    toolBoxWidget->addSpacing(5.0);
    toolBoxWidget->addWidget(new Button);
    toolBoxWidget->addSpacing(5.0);
    toolBoxWidget->addWidget(new Button);
    toolBoxWidget->addSpacing(5.0);
    toolBoxWidget->addExpanding(1.5);
    
    auto backgroundImageWidget = new Widget("Turnaround");
    backgroundImageWidget->setName("backgroundImageWidget");
    backgroundImageWidget->setHeightPolicy(Widget::SizePolicy::RelativeSize);
    backgroundImageWidget->setHeight(1.0);
    backgroundImageWidget->setWidthPolicy(Widget::SizePolicy::FlexibleSize);
    backgroundImageWidget->setExpandingWeight(1.0);
    //backgroundImageWidget->setBackgroundImageResourceName("reference-image.jpg");
    backgroundImageWidget->setBackgroundImageOpacity(0.25);
    
    auto logoWidget = new Widget;
    logoWidget->setName("logoWidget");
    logoWidget->setSizePolicy(Widget::SizePolicy::FixedSize);
    logoWidget->setWidth(25.0);
    logoWidget->setHeight(71.0);
    logoWidget->setBackgroundImageResourceName("dust3d/data/dust3d_vertical.png");
    
    auto leftBarLayout = new Widget;
    leftBarLayout->setName("leftBarLayout");
    leftBarLayout->setLayoutDirection(Widget::LayoutDirection::TopToBottom);
    leftBarLayout->setHeightPolicy(Widget::SizePolicy::RelativeSize);
    leftBarLayout->setHeight(1.0);
    leftBarLayout->addExpanding();
    leftBarLayout->addSpacing(5.0);
    //leftBarLayout->addWidget(toolBoxWidget);
    leftBarLayout->addWidget(logoWidget);
    leftBarLayout->addSpacing(5.0);
    
    auto mainLayout = new Widget;
    mainLayout->setName("mainLayout");
    mainLayout->setLayoutDirection(Widget::LayoutDirection::LeftToRight);
    mainLayout->setHeightPolicy(Widget::SizePolicy::RelativeSize);
    mainLayout->setHeight(1.0);
    mainLayout->addWidget(leftBarLayout);
    mainLayout->addWidget(backgroundImageWidget);
    
    //IndieGameEngine::indie()->rootWidget()->addSpacing(5.0);
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
    
    engine()->run([=]() {
            Image *image = new Image;
            image->load("reference-image.jpg");
            return (void *)image;
        }, [=](void *result) {
            Image *image = (Image *)result;
            this->engine()->setImageResource("reference-image.jpg", image->width(), image->height(), image->data());
            delete image;
            Widget::get("Turnaround")->setBackgroundImageResourceName("reference-image.jpg");
        }
    );
    
    engine()->windowSizeChanged.connect([=]() {
        Widget *turnaroundWidget = Widget::get("Turnaround");
        size_t targetWidth = turnaroundWidget->layoutWidth();
        size_t targetHeight = turnaroundWidget->layoutHeight();
        this->engine()->run([=]() {
                Image *image = new Image;
                image->load("reference-image.jpg");
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
                Image *image = (Image *)result;
                this->engine()->setImageResource("reference-image.jpg", image->width(), image->height(), image->data());
                delete image;
                Widget::get("Turnaround")->setBackgroundImageResourceName("reference-image.jpg");
            }
        );
    });
    
    setVisible(true);
}

