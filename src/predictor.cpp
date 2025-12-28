#include "cpu/predictor.hpp"
#include <cstring>
#include <algorithm>
#include <iostream>

predictor::predictor() {
    std::memset(table, INITIAL_STATE, TABLE_SIZE); 
}

bool predictor::predict(addr_t pc) {
    int idx = get_index(pc);
    uint8_t state = table[idx];

    return (state >= 2);
}

void predictor::update(addr_t pc, bool actual_taken) {
    int idx = get_index(pc);
    table[idx] = actual_taken ?  std::min(table[idx] + 1, 3) : std::max(table[idx] - 1, 0);
}

int predictor::get_index(addr_t pc) {
    return (pc >> 2) & 0xFF;
}