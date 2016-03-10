#pragma once

#include <vector>
#include "algo/any.h"

namespace au {
namespace dec {
namespace unity {

    struct Node final
    {
        algo::any data;
        Node *parent;
        std::vector<Node*> children;
    };

} } }
