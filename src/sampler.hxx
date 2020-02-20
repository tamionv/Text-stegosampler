#ifndef SAMPLER_H
#define SAMPLER_H

#include <vector>
#include <random>
#include "trellis.hxx"
#include "model.hxx"
using namespace std;

// Sample according to c, conditioning that h.recover(c.encode(return value)) is m.
vector<symbol> conditional_sample(const model& c, const trellis& h, vector<unsigned> m, unsigned seed);

#endif
