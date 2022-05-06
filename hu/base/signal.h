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
 
#ifndef HU_BASE_SIGNAL_H_
#define HU_BASE_SIGNAL_H_

#include <map>
#include <functional>

namespace Hu
{

template <typename... Args>
class Signal
{
public:
    int64_t connect(const std::function<void (Args...)> &slot)
    {
        int64_t slotId = m_nextSlotId++;
        m_slots.insert({slotId, slot});
        return slotId;
    }
    
    void emit(Args... parameters) 
    {
        for (const auto &it: m_slots) {
            it.second(parameters...);
        }
    }
    
private:
    std::map<int64_t, std::function<void (Args...)>> m_slots;
    int64_t m_nextSlotId = 1;
};
    
}

#endif
