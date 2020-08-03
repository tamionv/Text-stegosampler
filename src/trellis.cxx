#include "trellis.hxx"
#include <cassert>
#include <fstream>
#include <iostream>
#include <mutex>
#include <set>
using namespace std;

trellis::trellis(int h, int ml, int sl, int seed)
    : height(h), mlen(ml), slen(sl), first(sl), last(sl), col(sl) {
    mt19937 mt(seed);
    static mutex mtx;

    // Width of a trellis block.
    int width = sl / ml;

    do {
        for (int i = 0; i < slen; ++i) {
            first[i] = min(ml - 1, i / width);
            last[i] = min(ml, first[i] + height);
            col[i] =
                uniform_int_distribution<int>(1, (1 << (last[i] - first[i])) - 1)(mt);
            mtx.lock();
            mtx.unlock();
            // col[i] = (1 << (last[i] - first[i])) - 1;
        }
    } while (!is_good());

};

bool trellis::is_good() {
    set<vector<unsigned>> possible_ms;

    for (unsigned ss = 0; (ss >> slen) == 0; ++ss) {
        vector<unsigned> stego(slen);
        for (int i = 0; i < slen; ++i)
            stego[i] = (ss >> i) & 1;

        possible_ms.insert(recover(stego));

        if (possible_ms.size() >> mlen)
            return true;
    }
    return false;
}

trellis::trellis(string s) {
    ifstream f(s);
    f >> height >> mlen >> slen;
    first.resize(slen);
    last.resize(slen);
    col.resize(slen);
    for (int i = 0; i < slen; ++i)
        f >> first[i] >> last[i] >> col[i];
}

int trellis::h() const { return height; }

int trellis::message_len() const { return mlen; }

int trellis::stego_len() const { return slen; }

int trellis::fst(int i) const {
    return i < 0 ? 0 : i >= slen ? mlen : first[i];
}

int trellis::lst(int i) const { return i < 0 ? 0 : i >= slen ? mlen : last[i]; }

int trellis::dFst(int i) const { return fst(i) - fst(i - 1); }

int trellis::dLst(int i) const { return lst(i) - lst(i - 1); }

int trellis::effect(int x) const { return col[x]; }

int trellis::len(int x) const { return lst(x) - fst(x); }

vector<unsigned> trellis::recover(vector<unsigned> s) const {
    vector<unsigned> m(message_len());
    for (int i = 0; i < stego_len(); ++i) {
        if (s[i] == 0)
            continue;
        for (int j = fst(i); j < lst(i); ++j)
            m[j] ^= (effect(i) >> (lst(i) - j - 1)) & 1;
    }
    return m;
}
