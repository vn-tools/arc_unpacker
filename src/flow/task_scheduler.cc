// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

#include "flow/task_scheduler.h"
#include <chrono>
#include <deque>
#include <thread>
#include <vector>
#include "algo/range.h"

using namespace au;
using namespace au::flow;

struct TaskScheduler::Priv final
{
    std::deque<std::shared_ptr<ITask>> tasks;
    std::vector<std::unique_ptr<std::thread>> threads;
};

TaskScheduler::TaskScheduler() : p(new Priv())
{
}

TaskScheduler::~TaskScheduler()
{
}

void TaskScheduler::push_front(std::shared_ptr<ITask> task)
{
    std::unique_lock<std::mutex> lock(mutex);
    p->tasks.push_front(task);
}

void TaskScheduler::push_back(std::shared_ptr<ITask> task)
{
    std::unique_lock<std::mutex> lock(mutex);
    p->tasks.push_back(task);
}

TaskSchedulerResult TaskScheduler::run(size_t number_of_threads)
{
    if (!number_of_threads)
        number_of_threads = std::thread::hardware_concurrency();
    if (!number_of_threads)
        number_of_threads = 1;

    TaskSchedulerResult result;
    result.success_count = 0;
    result.error_count = 0;
    bool still_running = true;

    for (const auto i : algo::range(number_of_threads))
    {
        p->threads.push_back(std::make_unique<std::thread>([&]()
        {
            while (true)
            {
                std::shared_ptr<ITask> task;

                {
                    std::unique_lock<std::mutex> lock(mutex);
                    if (p->tasks.empty())
                    {
                        if (still_running && number_of_threads > 1)
                        {
                            lock.unlock();
                            std::this_thread::sleep_for(
                                std::chrono::milliseconds(10));
                            continue;
                        }
                        break;
                    }
                    task = p->tasks.front();
                    p->tasks.pop_front();
                }

                const auto local_success = task->work();

                {
                    std::unique_lock<std::mutex> lock(mutex);
                    result.success_count += local_success;
                    result.error_count += !local_success;
                    still_running = !p->tasks.empty();
                }
            }
        }));
    }

    for (auto &t : p->threads)
        t->join();

    return result;
}
