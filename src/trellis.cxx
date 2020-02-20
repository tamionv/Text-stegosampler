#include "trellis.hxx"
#include <iostream>
using namespace std;

trellis::trellis(int h, int ml, int sl, int seed):
    height(h),
    mlen(ml),
    slen(sl),
    first(sl),
    last(sl),
    col(sl)
{
    mt19937 mt(seed);

    // Width of a trellis block.
    int width = (sl - h + ml - 1) / ml;

    for(int i = 0; i < slen; ++i){
        first[i] = i / width;
        last[i] = min(ml, first[i] + height);
        col[i] = uniform_int_distribution<int>(0, 1 << (last[i] - first[i]))(mt);
    }
};

int trellis::h() const{
    return height;
}

int trellis::message_len() const{
    return mlen;
}

int trellis::stego_len() const{
    return slen;
}

int trellis::fst(int i) const{
    return i < 0 ? 0 : i >= slen ? mlen : first[i];
}

int trellis::lst(int i) const{
    return i < 0 ? 0 : i >= slen ? mlen : last[i];
}

int trellis::dFst(int i) const{
    return fst(i) - fst(i - 1);
}

int trellis::dLst(int i) const{
    return lst(i) - lst(i - 1);
}

int trellis::effect(int x) const{
    return col[x];
}

int trellis::len(int x) const{
    return lst(x) - fst(x);
}

vector<unsigned> trellis::recover(vector<unsigned> s) const{
    vector<unsigned> m(message_len());
    for(int i = 0; i < stego_len(); ++i){
        if(s[i] == 0) continue;
        for(int j = fst(i); j < lst(i); ++j)
            m[j] ^= (effect(i) >> (lst(i) - j - 1)) & 1;
    }
    return m;
}
