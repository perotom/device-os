/**
 ******************************************************************************
  Copyright (c) 2015 Particle Industries, Inc.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */

#if PLATFORM_THREADING

#include <string.h>
#include "active_object.h"
#include "concurrent_hal.h"
#include "timer_hal.h"
#include "spark_wiring_interrupts.h"

void ActiveObjectBase::start_thread()
{
    // prevent the started thread from running until the thread id has been assigned
    // so that calls to isCurrentThread() work correctly
    set_thread(std::thread(run_active_object, this));
    while (!started) {
        os_thread_yield();
    }
}


void ActiveObjectBase::run()
{
    std::lock_guard<std::mutex> lck (_start);
    started = true;

    uint32_t last_background_run = 0;
    for (;;)
    {
    	uint32_t now;
        if (!process())
		{
        	configuration.background_task();
        }
        else if ((now=HAL_Timer_Get_Milli_Seconds())-last_background_run > configuration.take_wait)
        {
        	last_background_run = now;
        	configuration.background_task();
        }
    }
}

bool ActiveObjectBase::process()
{
    bool result = false;
    Item item = nullptr;
    if (take(item) && item)
    {
        Message& msg = *item;
        msg();
        result = true;
    }
    return result;
}

void ActiveObjectBase::run_active_object(ActiveObjectBase* object)
{
    object->run();
}

void ISRTask::operator()()
{
    SPARK_ASSERT(callback && pool);
    callback(data); // Invoke callback
    pool->release(this); // Return this object back to associated pool
}

ISRTaskPool::ISRTaskPool(size_t size) :
        tasks(nullptr),
        availTask(nullptr)
{
    if (size) {
        // Initialize pool of task objects
        tasks = new(std::nothrow) ISRTask[size];
        if (tasks) {
            for (size_t i = 0; i < size; ++i) {
                ISRTask* t = tasks + i;
                if (i != size - 1) {
                    t->next = t + 1;
                } else {
                    t->next = nullptr;
                }
                t->pool = this;
            }
        }
        availTask = tasks;
    }
}

ISRTaskPool::~ISRTaskPool()
{
    delete[] tasks;
}

ISRTask* ISRTaskPool::take(ISRTask::Callback callback, void* data)
{
    ISRTask* task = nullptr;
    ATOMIC_BLOCK() { // Prevent preemption of current ISR
        // Take task object from the pool
        task = availTask;
        if (task) {
            availTask = task->next;
        }
    }
    if (task) {
        // Initialize task object
        task->callback = callback;
        task->data = data;
    }
    return task;
}

void ISRTaskPool::release(ISRTask* task)
{
    SPARK_ASSERT(task);
    ATOMIC_BLOCK() {
        // Return task object to the pool
        task->next = availTask;
        availTask = task;
    }
}

#endif
