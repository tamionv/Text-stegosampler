#ifndef SAMPLER_H
#define SAMPLER_H

#include "model.hxx"
#include "trellis.hxx"
#include <random>
#include <cassert>
#include <vector>
using namespace std;

// Node in trellis tree. Rather than explicitly maintain all back edges
// in the graph, I will choose a back edge randomly as I go along, thus
// needing constant space per node. Interestingly this reduces the
// trellis graph to a tree.
struct node {
    double d = 0;
    vector<tuple<double, symbol, node *>> father;

    int position = 0;

    node(int pos) : position(pos) {}
};

template <typename T>
void conditional_sample(int nr_tests, const model &c, const trellis &h,
                        vector<unsigned> m, mt19937 &mt, T &&t) {
    // cerr << "Doing conditional sample" << endl;
    const unsigned hh = h.h(), hmask = (1 << hh) - 1;

    vector<node *> elems;

    // Firstly, this is useful for checking if a state is good:
    vector<unsigned> good_check_vec(h.stego_len());
    for (int i = 0; i < h.stego_len(); ++i)
        for (int j = h.fst(i); j < h.fst(i + 1); ++j)
            good_check_vec[i] = 2 * good_check_vec[i] + m[j];

    // The root of the trellis graph.
    node *root = new node{0};
    elems.push_back(root);

    root->d = 1;

    // The current layer is indexed by (mask, context) pairs. But holding
    // these as pairs in the inner loop is innefficient. Thus I will index
    // (mask, context) as mask | context << hh
    //
    // The root ought properly to be only at key 0, but due to the fact that
    // these entries are accessed only if mentioned in visit_now or visit_next,
    // and entries at wrong positions in the text are counted as being invalid,
    // I can use root as a dummy value, to remove a nullptr check later.
    vector<node *> current_layer{c.context_count() << h.h(), root},
        prev_layer{c.context_count() << h.h(), root};

    // Nodes that we visit on next layer.
    vector<unsigned> visit_now, visit_next;

    // We visit the trellis root, which is properly at key 0.
    visit_next.push_back(0);

    // Advance h.stego_len() the current layer.
    for (int i = 0; i < h.stego_len(); ++i) {
        const auto mask_lim = (1 << h.len(i)) - 1;
        const auto my_dLst = h.dLst(i);
        const auto my_effect = h.effect(i);

        // if (i > 0)
        //            cerr << '\r';
        //       cerr << "At conditional sample step " << i;

        swap(current_layer, prev_layer);
        swap(visit_now, visit_next);
        visit_next.clear();

        for (auto current_idx : visit_now) {
            const auto current_node = prev_layer[current_idx];
            const auto msk = current_idx & hmask;
            const auto ctx = current_idx >> hh;
            const auto d = current_node->d;

            assert(c.cand_and_p(ctx).size() == 2);
            for (const auto &t : c.cand_and_p(ctx)) {
                const auto b = c.encode(ctx, t.first);
                const auto msk_ =
                    ((msk << my_dLst) ^ (b * my_effect)) & mask_lim;

                if (good_check_vec[i] ^ (msk_ >> (h.lst(i) - h.fst(i + 1)))) {
                    continue;
                }

                const auto ctx_ = t.second.second;
                const auto p = t.second.first;
                const auto k = msk_ | ctx_ << hh;

                // No nullptr check since current_layer never contains a null
                // pointer.
                if (current_layer[k]->position != i + 1) {
                    current_layer[k] = new node{i + 1};
                    elems.push_back(current_layer[k]);
                    visit_next.push_back(k);
                }
                const auto next_node = current_layer[k];

                next_node->father.emplace_back(d * p, t.first, current_node);
                next_node->d += d * p;
            }
        }
    }

    for (int i = 1; i <= nr_tests; ++i) {
        // Select a final node from the current (i.e. last) layer, using
        // a similar strategy to before.
        double d = 0;
        node *me = nullptr;

        for (auto x : visit_next) {
            auto other = current_layer[x];
            if (!bernoulli_distribution(d / (d + other->d))(mt))
                me = other;
            d += other->d;
        }

        // Now backwalk through the graph to reconstitute the result.
        vector<symbol> ret(h.stego_len());

        for (int i = 0; i < h.stego_len(); ++i) {
            double d = 0;
            node *father = nullptr;
            symbol father_sym;

            for (auto x : me->father) {
                if (!bernoulli_distribution(d / (d + get<0>(x)))(mt)) {
                    father = get<2>(x);
                    father_sym = get<1>(x);
                }
                d += get<0>(x);
            }

            ret.rbegin()[i] = father_sym;
            me = father;
        }

        t(i, ret);
    }

    for (auto x : elems)
        delete x;
}

#endif
