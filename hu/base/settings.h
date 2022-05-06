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

#ifndef HU_BASE_SETTINGS_H_
#define HU_BASE_SETTINGS_H_

#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <format>
#include <hu/base/string.h>

namespace Hu
{
    
class Settings
{
public:
    Settings(const std::string &iniPath):
        m_iniPath(iniPath)
    {
        loadFromFile(m_iniPath);
    }
    
    const std::string &value(const std::string &key) const
    {
        auto findValue = m_keyValuePairs.find(key);
        if (findValue == m_keyValuePairs.end())
            return m_notfound;
        return findValue->second;
    }
    
    std::string value(const std::string &key, const std::string &defaultValue) const
    {
        const std::string &string = value(key);
        if (string.empty())
            return defaultValue;
        return string;
    }
    
    std::string value(const std::string &key, int defaultValue) const
    {
        return value(key, std::to_string(defaultValue));
    }
    
    void setValue(const std::string &key, const std::string &toValue)
    {
        if (value(key) == toValue)
            return;
        m_keyValuePairs[key] = toValue;
        m_iniIsDirty = true;
    }
    
    void save()
    {
        if (!m_iniIsDirty)
            return;
        
        saveToFile(m_iniPath);
        
        m_iniIsDirty = false;
    }
    
private:
    std::string m_iniPath;
    bool m_iniIsDirty = false;
    std::map<std::string, std::string> m_keyValuePairs;
    std::string m_notfound;
    
    void loadLine(std::string &line, std::string &section)
    {
        auto trimmedLine = String::trimmed(line);
        if (trimmedLine.empty())
            return;
        if ('#' == trimmedLine[0])
            return;
        if ('[' == trimmedLine.front()) {
            auto sectionNameEnd = trimmedLine.rfind(']');
            if (std::string::npos == sectionNameEnd) {
                huDebug << "Section name end not found:" << line;
                return;
            }
            section = trimmedLine.substr(1, sectionNameEnd - 1) + ".";
            return;
        }
        auto keyEnd = trimmedLine.find('=');
        if (std::string::npos == keyEnd) {
            huDebug << "Key value separator not found:" << line;
            return;
        }
        auto key = trimmedLine.substr(0, keyEnd);
        m_keyValuePairs[section + key] = String::trimmed(trimmedLine.substr(keyEnd + 1));
    }
    
    void loadFromFile(const std::string &iniPath)
    {
        m_keyValuePairs.clear();
        
        if (iniPath.empty())
            return;
        
        std::ifstream file(iniPath);
        if (!file.is_open()) {
            huDebug << "Open file failed:" << iniPath;
            return;
        }
        
        std::string line;
        std::string section;
        while (std::getline(file, line)) {
            loadLine(line, section);
        }
        
        file.close();
    }
    
    void saveToFile(const std::string &iniPath)
    {
        if (m_iniPath.empty())
            return;
        
        std::ofstream file(iniPath);
        std::string lastSection;
        std::string section;
        for (const auto &it: m_keyValuePairs) {
            auto sectionNameEnd = it.first.rfind('.');
            std::string key;
            if (std::string::npos != sectionNameEnd) {
                section = it.first.substr(0, sectionNameEnd);
                key = it.first.substr(sectionNameEnd + 1);
            } else {
                key = it.first;
            }
            if (lastSection != section) {
                lastSection = section;
                file << std::format("[{}]", section) << std::endl;
            }
            file << std::format("{}={}", key, it.second) << std::endl;
        }
        file.close();
    }
};
    
}

#endif
