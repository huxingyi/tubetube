/*
 *  Copyright (c) 2016-2022 Jeremy HU <jeremy-at-dust3d dot org>. All rights reserved. 
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

#ifndef DUST3D_DOCUMENT_SNAPSHOT_H_
#define DUST3D_DOCUMENT_SNAPSHOT_H_

#include <map>
#include <hu/base/string.h>

namespace Dust3d
{
    
class Snapshot
{
public:
    std::map<std::string, std::string> canvas;
    std::map<std::string, std::map<std::string, std::string>> nodes;
    std::map<std::string, std::map<std::string, std::string>> edges;
    std::map<std::string, std::map<std::string, std::string>> parts;
    std::map<std::string, std::map<std::string, std::string>> components;
    std::map<std::string, std::string> rootComponent;
    std::vector<std::pair<std::map<std::string, std::string>, std::vector<std::pair<std::map<std::string, std::string>, std::vector<std::map<std::string, std::string>>>>>> materials; // std::pair<Material attributes, layers>  layer: std::pair<Layer attributes, maps>
};
    
}

#endif
