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
        if (abs(x.second - m2[x.first]) > 0.5) {
            for (auto y : x.first)
                cerr << y << "-";
            cerr << endl;
            cerr << x.second << ' ' << m2[x.first] << endl;
            cerr << endl;
            for (auto y : v)
                cerr << y << '-';
            cerr << endl;
        }
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
        cerr << ss << " OF " << (1 << stego_len) << "\r";
        vector<unsigned> stego;
        for (int i = 0; i < stego_len; ++i)
            stego.push_back(((ss >> i) & 1) + 1);

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

    static constexpr int nr_tests = 1e7;

    auto one_thread_work = [m, t](int msg) {
        mt19937 mt(143232 + msg);

        vector<unsigned> message;
        for (int i = 0; i < message_len; ++i)
            message.push_back((msg >> i) & 1);

        for (int i = 0; i < nr_tests; ++i) {
            auto stego = conditional_sample(
                m, t, message, uniform_int_distribution<int>(0, 1e9)(mt));

            auto tt = t.recover(m.encode_sequence(stego));
            assert(equal(begin(tt), end(tt), begin(message)));

            experimental_dist[message][stego] += 1.0;
        }
    };

    thread t0(one_thread_work, 0);
    thread t1(one_thread_work, 1);
    thread t2(one_thread_work, 2);
    thread t3(one_thread_work, 3);
    thread t4(one_thread_work, 4);
    thread t5(one_thread_work, 5);
    thread t6(one_thread_work, 6);
    thread t7(one_thread_work, 7);
    thread t8(one_thread_work, 8);
    thread t9(one_thread_work, 9);
    thread t10(one_thread_work, 10);
    thread t11(one_thread_work, 11);
    thread t12(one_thread_work, 12);
    thread t13(one_thread_work, 13);
    thread t14(one_thread_work, 14);
    thread t15(one_thread_work, 15);

    t0.join();
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();
    t7.join();
    t8.join();
    t9.join();
    t10.join();
    t11.join();
    t12.join();
    t13.join();
    t14.join();
    t15.join();

    for (auto &x : experimental_dist) {
        double sum = 0;
        for (auto &y : x.second)
            sum += y.second;
        for (auto &y : x.second)
            y.second /= sum;
    }

    for (auto &x : theoretical_dist) {
        // for (auto y : x.first)
        // cout << y << ' ';
        // cout << ",";
        cout << total_var(x.first, experimental_dist[x.first], x.second);
        cout << endl;
        break;
    }

    return 0;
}
