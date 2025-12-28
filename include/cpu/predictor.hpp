#pragma once
#include "types.hpp"

class predictor {
    private:
        static constexpr uint16_t TABLE_SIZE = 256;
        static constexpr uint8_t INITIAL_STATE = 2;
        uint8_t table[TABLE_SIZE];

    public:
        predictor();
        bool predict(addr_t pc);
        void update(addr_t pc, bool actual_taken);

    private:
        int get_index(addr_t pc);
};