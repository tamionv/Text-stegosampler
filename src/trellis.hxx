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

    // Get trellis from filename
    trellis(string);

    // Get trellis height.
    int h() const;
 
    // Get message length.
    int message_len() const;

    // Get stegotext length.
    int stego_len() const;
    
    // For stegotext position i, gets first position
    // affected by i. By convention, fst(-1) = 0,
    // fst(stego_len()) = message_len()
    int fst(int) const;

    // For stegotext position i, gets first position
    // not affected by i. By convention, fst(-1) = 0,
    // fst(stego_len()) = message_len()
    int lst(int) const;

    // dFst(x) = fst(x) - fst(x - 1)
    int dFst(int) const;

    // dLst(x) = lst(x) - lst(x - 1)
    int dLst(int) const;

    // len(x) = lst(x) - fst(x)
    int len(int) const;

    // Effect of stegosymbol x on message, as a bitmask,
    // from left to right, in big-endian order.
    int effect(int) const;

    // Apply trellis matrix to stegosequence.
    vector<unsigned> recover(vector<unsigned>) const;
};

#endif
