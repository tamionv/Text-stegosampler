#include "model.hxx"
#include <cassert>
#include <fstream>
#include <iostream>
using namespace std;

model::model() : cnt(0) {}

model model::model_from_file(string s) {
    model ret;
    ifstream f(s);

    f >> ret.cnt;

    for (int i = 0; i < ret.cnt; ++i)
        for (int j = 0; j < ret.cnt; ++j)
            f >> ret.buf[i][j];
    return ret;
}

const map<symbol, pair<double, context>> model::cand_and_p(context c) const {
    map<symbol, pair<double, context>> ret;
    for (int i = 0; i < cnt; ++i)
        if (buf[c][i])
            ret[i] = make_pair(buf[c][i], i);

    return ret;
}

unsigned model::encode(context c, symbol s) const { return s - 1; }

vector<unsigned> model::encode_sequence(vector<symbol> v) const {
    for (auto &x : v)
        --x;
    return v;
}

unsigned model::context_count() const { return cnt; }

double model::probability(vector<symbol> v) {
    double d = buf[0][v[0]];
    for (int i = 0, j = 1; j < (int)v.size(); ++i, ++j)
        d *= buf[v[i]][v[j]];
    assert(d > 0);
    return d;
}
