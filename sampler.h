#ifndef SAMPLER_H
#define SAMPLER_H

#include <vector>
#include <random>
#include "trellis.h"
#include "model.h"

using namespace std;

// Sample according to c, conditioning that h.recover(c.encode(return value)) is m.
vector<symbol> conditional_sample(model& c, trellis& h, vector<unsigned> m, unsigned seed);

#endif
