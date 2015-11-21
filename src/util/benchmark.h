#pragma once

#include <ctime>
#include "log.h"

namespace au {
namespace util {

    class Benchmark final
    {
    public:
        inline Benchmark()
        {
            start = clock();
        }

        inline void measure()
        {
            const clock_t diff = clock() - start;
            const int msec = diff * 1000 / CLOCKS_PER_SEC;
            Log.debug("Took %d.%04ds\n", msec / 1000, msec % 1000);
            start = clock();
        }

        inline void reset()
        {
            start = clock();
        }
    private:
        clock_t start;
    };

} }
