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

#pragma once

#include <memory>
#include <mutex>

namespace au {
namespace flow {

    class ITask
    {
    public:
        virtual ~ITask() {}
        virtual bool work() const = 0;
    };

    struct TaskSchedulerResult final
    {
        int success_count;
        int error_count;
    };

    class TaskScheduler final
    {
    public:
        TaskScheduler();
        ~TaskScheduler();
        TaskSchedulerResult run(const size_t number_of_threads = 0);
        void push_front(std::shared_ptr<ITask> task);
        void push_back(std::shared_ptr<ITask> task);
        void join();
        std::mutex mutex;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }
