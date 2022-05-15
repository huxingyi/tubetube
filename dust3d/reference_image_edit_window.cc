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
    
    auto previewLayout = new Widget;
    previewLayout->setName("previewLayout");
    previewLayout->setWidth(1.0, Widget::SizePolicy::FlexibleSize);
    
    auto rightLayout = new Widget;
    rightLayout->setName("rightLayout");
    rightLayout->setLayoutDirection(Widget::LayoutDirection::TopToBottom);
    rightLayout->setWidth(250.0, Widget::SizePolicy::FixedSize);
    rightLayout->setHeight(1.0, Widget::SizePolicy::RelativeSize);
    rightLayout->setBackgroundColor(Color("#333333"));

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
    });
    
    auto loadImageButtonLayout = new Widget;
    loadImageButtonLayout->setName("loadImageButtonLayout");
    loadImageButtonLayout->setLayoutDirection(Widget::LayoutDirection::LeftToRight);
    loadImageButtonLayout->addExpanding();
    loadImageButtonLayout->addWidget(loadImageButton);
    loadImageButtonLayout->addExpanding();
    
    rightLayout->addSpacing(10.0);
    rightLayout->addWidget(loadImageButtonLayout);
    rightLayout->addExpanding();
    
    auto mainLayout = new Widget;
    mainLayout->setName("mainLayout");
    mainLayout->setHeight(1.0, Widget::SizePolicy::RelativeSize);
    mainLayout->setWidth(1.0, Widget::SizePolicy::RelativeSize);
    mainLayout->addWidget(previewLayout);
    mainLayout->addWidget(rightLayout);
    mainLayout->setBackgroundColor(Color("#252525"));
    
    engine()->rootWidget()->setName("rootWidget");
    engine()->rootWidget()->addWidget(mainLayout);
    
    setVisible(true);
}
