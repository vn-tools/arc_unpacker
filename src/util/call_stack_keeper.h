#pragma once

namespace au {
namespace util {

    struct CallStackKeeper final
    {
        CallStackKeeper();
        ~CallStackKeeper();
        int current_call_depth() const;
    };

} }
