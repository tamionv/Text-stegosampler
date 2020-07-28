#include "model.hxx"
#include "sampler.hxx"
#include "trellis.hxx"
#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <thread>
using namespace std;

static constexpr int message_len = 4;
static constexpr int stego_len = 20;

using message = vector<unsigned>;
using stego = vector<unsigned>;

map<message, map<stego, double>> theoretical_dist, experimental_dist;

static constexpr int nr_tests = 1e5;

template <typename T>
double total_var(vector<unsigned> v, map<T, double> m1, map<T, double> m2) {
    double ret = 0;
    for (auto x : m1) {
        ret = max(ret, abs(x.second - m2[x.first]));
    }
    return ret;
}

template <typename T>
double chi_sq(vector<unsigned> v, map<T, double> e, map<T, double> o) {
    double ret = 0;
    for (auto x : e) {
        ret += (o[x.first] * nr_tests - e[x.first] * nr_tests) *
               (o[x.first] * nr_tests - e[x.first] * nr_tests) /
               (e[x.first] * nr_tests);
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

    for (int ss = 0; (ss >> stego_len) == 0; ++ss) {
        vector<unsigned> stego;
        for (int i = 0; i < stego_len; ++i)
            stego.push_back(((ss >> i) & 1) + 1);

        auto tmp = t.recover(m.encode_sequence(stego));

        theoretical_dist[t.recover(m.encode_sequence(stego))][stego] +=
            m.probability(stego);
        experimental_dist[t.recover(m.encode_sequence(stego))][stego] = 0.0;
    }

    for (auto &x : theoretical_dist) {
        double sum = 0;
        for (auto &y : x.second)
            sum += y.second;
        for (auto &y : x.second)
            y.second /= sum;
    }
    auto one_thread_work = [m, t](int msg) {
        mt19937 mt(143232 + msg);

        vector<unsigned> message;
        for (int i = 0; i < message_len; ++i)
            message.push_back((msg >> i) & 1);

        for (int i = 0; i < nr_tests; ++i) {
            auto stego = conditional_sample(m, t, message, mt);

            experimental_dist[message][stego] += 1.0 / nr_tests;
            if (msg == 5 && i % 1000 == 0)
                cerr << i << endl;
        }
    };

    vector<thread> ths;

    for (int i = 0; i < 16; ++i)
        ths.emplace_back(one_thread_work, i);

    for (auto &x : ths)
        x.join();

    for (auto &x : theoretical_dist) {
        // for (auto y : x.first)
        // cout << y << ' ';
        // cout << ",";
        cout << total_var(x.first, x.second, experimental_dist[x.first]) << ' '
             << chi_sq(x.first, x.second, experimental_dist[x.first]) << endl;
    }

    return 0;
}
