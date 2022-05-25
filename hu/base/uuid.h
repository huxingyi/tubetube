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

#ifndef HU_BASE_UUID_H_
#define HU_BASE_UUID_H_

#include <random>
#include <sstream>
#include <chrono>
#include <ctime>

namespace Hu
{

class Uuid
{
public:
    class RandomGenerator
    {
    public:
        RandomGenerator():
            m_randomGenerator(m_randomDevice() + std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now()).time_since_epoch().count()),
            m_randomDistribution(0, 15)
        {
        }
        
        int generate()
        {
            return m_randomDistribution(m_randomGenerator);
        }
    
    private:
        std::random_device m_randomDevice;
        std::mt19937 m_randomGenerator;
        std::uniform_int_distribution<int> m_randomDistribution;
    };

    static inline RandomGenerator *m_generator = new RandomGenerator;
    
    Uuid(const std::string &string)
    {
        if (sizeof("{hhhhhhhh-hhhh-hhhh-hhhh-hhhhhhhhhhhh}") - 1 == string.length() &&
                '{' == string[0] &&
                validate(&string[1], string.length() - 2)) {
            m_uuid = string.substr(1, string.length() - 2);
            return;
        }
        if (sizeof("hhhhhhhh-hhhh-hhhh-hhhh-hhhhhhhhhhhh") - 1 == string.length() &&
                validate(&string[0], string.length())) {
            m_uuid = string;
            return;
        }
    }
    
    static bool validate(const char *string, size_t length)
    {
        return '-' == string[8] && 
            '-' == string[13] && 
            '-' == string[18] && 
            '-' == string[23];
    }
    
    static Uuid createUuid()
    {
        std::ostringstream ss;
        ss << std::hex;
        ss << m_generator->generate();
        ss << m_generator->generate();
        ss << m_generator->generate();
        ss << m_generator->generate();
        ss << m_generator->generate();
        ss << m_generator->generate();
        ss << m_generator->generate();
        ss << m_generator->generate();
        ss << "-";
        ss << m_generator->generate();
        ss << m_generator->generate();
        ss << m_generator->generate();
        ss << m_generator->generate();
        ss << "-";
        ss << m_generator->generate();
        ss << m_generator->generate();
        ss << m_generator->generate();
        ss << m_generator->generate();
        ss << "-";
        ss << m_generator->generate();
        ss << m_generator->generate();
        ss << m_generator->generate();
        ss << m_generator->generate();
        ss << "-";
        ss << m_generator->generate();
        ss << m_generator->generate();
        ss << m_generator->generate();
        ss << m_generator->generate();
        ss << m_generator->generate();
        ss << m_generator->generate();
        ss << m_generator->generate();
        ss << m_generator->generate();
        ss << m_generator->generate();
        ss << m_generator->generate();
        ss << m_generator->generate();
        ss << m_generator->generate();
        return Uuid(ss.str());
    }
    
    const std::string &toString() const
    {
        return m_uuid;
    }
    
private:
    friend struct std::hash<Uuid>;
    friend bool operator==(const Uuid &left, const Uuid &right);
    friend bool operator!=(const Uuid &left, const Uuid &right);
    friend bool operator<(const Uuid &left, const Uuid &right);
    
    std::string m_uuid;
};

inline bool operator==(const Uuid &left, const Uuid &right)
{
    return left.m_uuid == right.m_uuid;
}

inline bool operator!=(const Uuid &left, const Uuid &right)
{
    return left.m_uuid != right.m_uuid;
}

inline bool operator<(const Uuid &left, const Uuid &right)
{
    return left.m_uuid < right.m_uuid;
}

}

namespace std
{
    
template<>
struct hash<Hu::Uuid>
{
    size_t operator()(const Hu::Uuid &uuid) const
    {
        return std::hash<std::string>()(uuid.m_uuid);
    }
};

}

#endif
