//
// Created by Taras Martynyuk on 6/24/2018.
//
#ifndef SPY_THE_SPIES_STL_OVERLOADS_H
#define SPY_THE_SPIES_STL_OVERLOADS_H
#include <algorithm>

template<class TInputContainer, class OIter>
OIter copy(TInputContainer container, OIter out_it) {
    return std::copy(container.begin(), container.end(), out_it);
};

#endif //SPY_THE_SPIES_STL_OVERLOADS_H
