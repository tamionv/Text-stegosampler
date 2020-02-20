#include <fstream>
#include <iostream>
#include "trellis.hxx"
#include "model.hxx"
#include "sampler.hxx"
using namespace std;

static constexpr int message_len = 200;
static constexpr int stego_len = 1000;
static constexpr int trellis_height = 7;

int main(){
    int seed = time(nullptr);

    // I happened to use 
    ifstream model_input("illiad.txt");

    string model_contents{
        istreambuf_iterator<char>(model_input),
        istreambuf_iterator<char>()};

    model m = model::model_from_text(5, model_contents);
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

    return 0;
}
