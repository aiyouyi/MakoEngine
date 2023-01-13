#include "CC3DRandom.h"

std::mt19937 CC3DRandom::s_RandomEngine;
std::uniform_int_distribution<std::mt19937::result_type> CC3DRandom::s_Distribution;