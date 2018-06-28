//
// Created by Taras Martynyuk on 6/24/2018.
//
#ifndef SPY_THE_SPIES_STL_OVERLOADS_H
#define SPY_THE_SPIES_STL_OVERLOADS_H
#include <algorithm>
#include <vector>

template<class TInputContainer, class OIter>
OIter copy(TInputContainer container, OIter out_it) {
    return std::copy(container.begin(), container.end(), out_it);
};

template<class TIter1, class TIter2>
bool areDisjoint(TIter1 container, TIter2 other_container) {
    std::vector<typename TIter1::value_type> intersection;
    std::set_intersection(container.begin(), container.end(),
                     other_container.begin(), other_container.end(),
                     intersection.begin());

    return intersection.empty();
};

#endif //SPY_THE_SPIES_STL_OVERLOADS_H
