#ifndef TRELLIS_H
#define TRELLIS_H

#include <vector>
#include <random>
using namespace std;

using mask = unsigned;

class trellis{
    int height, mlen, slen;
    vector<int> first, last, col;
public:
    // Generate trellis of a given height and sizes, from a seed.
    trellis(int h, int ml, int sl, int seed);

    // Get trellis height.
    int h();

    // Get message length.
    int message_len();

    // Get stegotext length.
    int stego_len();
    
    // For stegotext position i, gets first position
    // affected by i. By convention, fst(-1) = 0,
    // fst(stego_len()) = message_len()
    int fst(int);

    // For stegotext position i, gets first position
    // not affected by i. By convention, fst(-1) = 0,
    // fst(stego_len()) = message_len()
    int lst(int);

    // dFst(x) = fst(x) - fst(x - 1)
    int dFst(int);

    // dLst(x) = lst(x) - lst(x - 1)
    int dLst(int);

    // len(x) = lst(x) - fst(x)
    int len(int);

    // Effect of stegosymbol x on message, as a bitmask,
    // from left to right, in big-endian order.
    int effect(int);

    // Apply trellis matrix to stegosequence.
    vector<unsigned> recover(vector<unsigned>);
};

#endif
