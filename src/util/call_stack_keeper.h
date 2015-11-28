#pragma once

#include <memory>

namespace au {
namespace util {

    class CallStackKeeper final
    {
    public:
        CallStackKeeper(const size_t limit = 10);
        ~CallStackKeeper();
        void recurse(const std::function<void()> action);
        bool recursion_limit_reached() const;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }
