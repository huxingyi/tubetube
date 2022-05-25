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
    
    Uuid(const std::string &string):
        m_uuid(string)
    {
    }
    
    static Uuid createUuid()
    {
        std::stringstream ss;
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
    std::string m_uuid;
};

}

#endif
