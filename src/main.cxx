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

static constexpr int nr_tests = 1e9;

template <typename T>
double total_var(vector<unsigned> v, map<T, double> m1, map<T, double> m2) {
    double ret = 0;
    for (auto x : m1) {
        if (ret < abs(x.second - m2[x.first])) {
            ret = max(ret, abs(x.second - m2[x.first]));
            if (all_of(begin(v), end(v), [&](int x) { return x == 0; })) {
                for (auto y : x.first)
                    cerr << y << ' ';
                cerr << endl;
                cerr << x.second << ' ' << m2[x.first] << endl;
            }
        }
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
    }

    for (auto &x : theoretical_dist) {
        double sum = 0;
        for (auto &y : x.second)
            sum += y.second;
        for (auto &y : x.second)
            y.second /= sum;

        cerr << x.second.size() - 1 << endl;
    }

    auto one_thread_work = [m, t](int msg) {
        mt19937 mt(143232 + msg);

        vector<unsigned> message;
        for (int i = 0; i < message_len; ++i)
            message.push_back((msg >> i) & 1);

        conditional_sample(
            nr_tests, m, t, message, mt, [&](int i, vector<unsigned> stego) {
                experimental_dist[message][stego] += 1.0 / nr_tests;
                if (msg == 5 && i % 1000 == 0)
                    cerr << i << endl;
            });
    };

    vector<thread> ths;

    for (int i = 0; i < 16; ++i)
        ths.emplace_back(one_thread_work, i);

    for (auto &x : ths)
        x.join();

    cout << "total var & kl_div(theoretical, exp) & kl_div(exp, theoretical) & "
            "chisq & p-value \\\\"
         << endl;
    for (auto &x : theoretical_dist) {
        cout << theoretical_dist[x.first].size() << ' '
             << experimental_dist[x.first].size() << endl;
        // for (auto y : x.first)
        // cout << y << ' ';
        // cout << ",";
        double chisq = chi_sq(x.first, x.second, experimental_dist[x.first]);
        cout << total_var(x.first, x.second, experimental_dist[x.first])
             << " & " << kl_div(x.first, x.second, experimental_dist[x.first])
             << " & " << kl_div(x.first, experimental_dist[x.first], x.second)
             << " & " << chisq << " & "
             << 1.0 - (erfc((65535 - chisq) / (2.0 * sqrt(65535))) / 2.0)
             << endl;
    }

    return 0;
}
