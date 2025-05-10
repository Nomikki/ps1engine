#include "utility.hpp"
#include <cmath>

std::array<float, TRIG_TABLE_SIZE> sin_table;
std::array<float, TRIG_TABLE_SIZE> cos_table;

void generate_sincos_lookupTables() {
    for (int i = 0; i < TRIG_TABLE_SIZE; i++) {
        float angle = static_cast<float>(i) * M_PI / 180.0f;
        sin_table[i] = std::sin(angle);
        cos_table[i] = std::cos(angle);
    }
} 