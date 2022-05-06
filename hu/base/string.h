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

#ifndef HU_BASE_STRING_H_
#define HU_BASE_STRING_H_

#include <string>
#include <sstream>
#include <vector>

namespace Hu
{
namespace String
{

inline std::vector<std::string> split(const std::string &string, char seperator)
{
    std::vector<std::string> tokens;
    std::stringstream stream(string);
    std::string token;
    while (std::getline(stream, token, seperator))
        tokens.push_back(token);
    return std::move(tokens);
}

inline std::string trimmed(std::string string)
{
    string.erase(string.begin(), std::find_if(string.begin(), string.end(), [](int ch) {
        return !std::isspace(ch);
    }));
    string.erase(std::find_if(string.rbegin(), string.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), string.end());
    return std::move(string);
}

int toInt(const std::string &string)
{
    return std::strtol(string.c_str(), nullptr, 10);
}

}
}

#endif

