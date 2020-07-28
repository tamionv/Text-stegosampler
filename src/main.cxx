#include "model.hxx"
#include "sampler.hxx"
#include "trellis.hxx"
#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <thread>
using namespace std;

static constexpr int message_len = MESSAGELEN;
static constexpr int stego_len = STEGOLEN;
static constexpr int trellis_height = TRELLISH;

using message = vector<unsigned>;
using stego = vector<unsigned>;

map<message, map<stego, double>> theoretical_dist, experimental_dist;

template <typename T>
double total_var(vector<unsigned> v, map<T, double> m1, map<T, double> m2) {
    double ret = 0;
    for (auto x : m1) {
        ret = max(ret, abs(x.second - m2[x.first]));
    }
    return ret;
}

template <typename T>
double kl_div(vector<unsigned> v, map<T, double> m1, map<T, double> m2) {
    double ret = 0;
    for (auto x : m1) {
        ret += m1[x.first] * log(m1[x.first] / m2[x.first]);
    }
    return ret;
}

int main() {
    model m = model::model_from_file("model");
    trellis t("trellis");

    vector<unsigned> tmp = {2, 2, 1, 1, 1, 2, 2, 2, 2, 2,
                            2, 1, 2, 2, 2, 2, 2, 2, 2, 2};
    auto ss = t.recover(m.encode_sequence(tmp));
    for (auto x : ss)
        cerr << x << ' ';
    cerr << endl;

    for (int ss = 0; (ss >> stego_len) == 0; ++ss) {
        vector<unsigned> stego;
        for (int i = 0; i < stego_len; ++i)
            stego.push_back(((ss >> i) & 1) + 1);

        auto tmp = t.recover(m.encode_sequence(stego));

        theoretical_dist[t.recover(m.encode_sequence(stego))][stego] +=
            m.probability(stego);
        experimental_dist[t.recover(m.encode_sequence(stego))][stego] = 0.0;
    }
    cerr << endl;

    cerr << theoretical_dist[ss][tmp] << endl;

    for (auto &x : theoretical_dist) {
        double sum = 0;
        for (auto &y : x.second)
            sum += y.second;
        for (auto &y : x.second)
            y.second /= sum;
    }

    cerr << theoretical_dist[ss][tmp] << endl;

    static constexpr int nr_tests = 1e6;

    auto one_thread_work = [m, t](int msg) {
        mt19937 mt(143232 + msg);

        vector<unsigned> message;
        for (int i = 0; i < message_len; ++i)
            message.push_back((msg >> i) & 1);

        for (int i = 0; i < nr_tests; ++i) {
            auto stego = conditional_sample(m, t, message, mt);

            experimental_dist[message][stego] += 1.0 / nr_tests;
        }
    };

    vector<thread> ths;

    for (int i = 0; i < 15; ++i)
        ths.emplace_back(one_thread_work, i);

    for (auto &x : ths)
        x.join();

    cerr << experimental_dist[ss][tmp] << endl;

    cerr << "KL DIV FROM THEORETICAL TO EXPERIMENTAL" << endl;
    for (auto &x : theoretical_dist) {
        // for (auto y : x.first)
        // cout << y << ' ';
        // cout << ",";
        cout << total_var(x.first, x.second, experimental_dist[x.first]) << ' '
             << kl_div(x.first, x.second, experimental_dist[x.first]) << endl;
    }

    return 0;
}
