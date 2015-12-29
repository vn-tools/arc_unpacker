#pragma once

#include <memory>
#include <mutex>

namespace au {
namespace flow {

    class ITask
    {
    public:
        virtual ~ITask() { }
        virtual bool work() const = 0;
    };

    class TaskScheduler final
    {
    public:
        TaskScheduler();
        ~TaskScheduler();
        bool run(const size_t number_of_threads = 0);
        void push_front(std::shared_ptr<ITask> task);
        void push_back(std::shared_ptr<ITask> task);
        void join();
        std::mutex mutex;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }
