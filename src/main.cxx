#include "model.hxx"
#include "sampler.hxx"
#include "trellis.hxx"
#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>
using namespace std;

std::mutex mtx;

static constexpr int stego_len = 20;

using message = vector<unsigned>;
using stego = vector<unsigned>;

static constexpr int nr_tests = 1e6;

template <typename T> double total_var(map<T, double> m1, map<T, double> m2) {
    double ret = 0;
    for (auto x : m1) {
        if (ret < abs(x.second - m2[x.first])) {
            ret = max(ret, abs(x.second - m2[x.first]));
        }
    }
    return ret;
}

template <typename T> double chi_sq(map<T, double> e, map<T, double> o) {
    double ret = 0;
    for (auto x : e) {
        ret += (o[x.first] * nr_tests - e[x.first] * nr_tests) *
               (o[x.first] * nr_tests - e[x.first] * nr_tests) /
               (e[x.first] * nr_tests);
    }
    return ret;
}

template <typename T> double kl_div(map<T, double> m1, map<T, double> m2) {
    double ret = 0;
    for (auto x : m1) {
        ret += m1[x.first] * log(m1[x.first] / m2[x.first]);
    }
    return ret;
}

int main() {

    cout << "message len & total_var(cond, uncond) & kl-div(cond, uncond) & "
            "kl-div(uncond, cond) & chi-sq & df"
         << endl;
    mutex mtx;

    model m = model::model_from_file("model");
    auto computation = [&](int message_len) {
        map<unsigned, double> uncond, full_cond;
        map<message, map<unsigned, double>> cond;
        trellis t(2, message_len, stego_len, 15342);

        for (int ss = 0; (ss >> stego_len) == 0; ++ss) {
            vector<unsigned> stego;
            for (int i = 0; i < stego_len; ++i)
                stego.push_back(((ss >> i) & 1) + 1);

            auto tmp = t.recover(m.encode_sequence(stego));

            uncond[ss] = m.probability(stego);
            cond[t.recover(m.encode_sequence(stego))][ss] =
                m.probability(stego);
        }

        for (auto &x : cond) {
            double sum = 0;
            for (auto &y : x.second)
                sum += y.second;
            for (auto &y : x.second)
                y.second /= sum;
        }

        for (auto &x : cond)
            for (auto &y : x.second)
                full_cond[y.first] += y.second / (1 << message_len);

        double chisq = chi_sq(full_cond, uncond);
        mtx.lock();
        cout << message_len << " & " << total_var(full_cond, uncond) << " & "
             << kl_div(full_cond, uncond) << " & " << kl_div(uncond, full_cond)
             << " & " << chisq << " & " << (1 << stego_len) - 1 << endl;
        mtx.unlock();
    };
    vector<thread> ths;
    for (int i = 1; i <= 13; ++i)
        ths.emplace_back(computation, i);

    for (auto &x : ths)
        x.join();
}
