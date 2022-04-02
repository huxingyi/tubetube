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

#ifndef DUST3D_BASE_TASK_LIST_H_
#define DUST3D_BASE_TASK_LIST_H_

#include <list>
#include <thread>
#include <atomic>
#include <functional>
#include <dust3d/base/task.h>

namespace dust3d
{

class TaskList
{
public:
    class TaskRunner: public std::thread
    {
    public:
        TaskRunner(std::unique_ptr<Task> task):
            m_task(std::move(task)),
            m_workFunction(std::bind(&Task::work, m_task.get())),
            m_afterFunction(std::bind(&Task::after, m_task.get())),
            std::thread(std::bind(&TaskRunner::run, this))
        {
        }
        
        TaskRunner(std::function<void (void)> work, std::function<void (void)> after=nullptr):
            m_workFunction(work),
            m_afterFunction(after),
            std::thread(std::bind(&TaskRunner::run, this))
        {
        }
        
        bool workDone()
        {
            return m_workDone;
        }
        
        void finish()
        {
            join();
            if (nullptr != m_afterFunction)
                m_afterFunction();
        }
        
    private:
        std::function<void (void)> m_workFunction = nullptr;
        std::function<void (void)> m_afterFunction = nullptr;
        std::unique_ptr<Task> m_task;
        std::atomic<bool> m_workDone = false;
        
        void run()
        {
            m_workFunction();
            m_workDone = true;
        }
    };

    void post(std::unique_ptr<Task> task)
    {
        m_list.push_back(std::make_unique<TaskRunner>(std::move(task)));
    }
    
    void post(std::function<void (void)> work, std::function<void (void)> after=nullptr)
    {
        m_list.push_back(std::make_unique<TaskRunner>(work, after));
    }
    
    bool anyWorkDone()
    {
        for (std::list<std::unique_ptr<TaskRunner>>::iterator it = m_list.begin(); it != m_list.end(); ) {
            if ((*it)->workDone())
                return true;
            ++it;
        }
        return false;
    }
    
    void update()
    {
        for (std::list<std::unique_ptr<TaskRunner>>::iterator it = m_list.begin(); it != m_list.end(); ) {
            if ((*it)->workDone()) {
                (*it)->finish();
                it = m_list.erase(it);
                continue;
            }
            ++it;
        }
    }
    
    size_t size()
    {
        return m_list.size();
    }
    
private:
    std::list<std::unique_ptr<TaskRunner>> m_list;
};

}

#endif
