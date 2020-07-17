#include <fstream>
#include <iostream>
#include <cassert>
#include <algorithm>
#include "trellis.hxx"
#include "model.hxx"
#include "sampler.hxx"
using namespace std;

static constexpr int message_len = MESSAGELEN;
static constexpr int stego_len = STEGOLEN;
static constexpr int trellis_height = TRELLISH;

int main(){
    int seed = time(nullptr);
    ifstream model_input(MODELNAME);
    string model_contents{
        istreambuf_iterator<char>(model_input),
        istreambuf_iterator<char>()};

    model m = model::model_from_text(CONTEXTLEN, model_contents);
    trellis t(trellis_height, message_len, stego_len, seed);
    vector<unsigned> message(message_len);
    random_device rd;
    for(auto& x : message) x = uniform_int_distribution<int>(0, 1)(rd);

    auto ret = conditional_sample(m, t, message, seed);

    auto verif = t.recover(m.encode_sequence(ret));


    for(auto x : ret)
        cout << m.symbol_meaning(x) << ' ';
    cout << endl;

    for(auto x : message)
        cerr << x << ' ';
    cerr << endl;

    for(auto x : verif)
        cerr << x << ' ';
    cerr << endl;

    assert(equal(begin(message), end(message), begin(verif)));

    return 0;
}
