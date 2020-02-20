#include "sampler.hxx"
#include <iostream>
#include <cassert>
using namespace std;

// Node in trellis tree. Rather than explicitly maintain all back edges
// in the graph, I will choose a back edge randomly as I go along, thus
// needing constant space per node. Interestingly this reduces the
// trellis graph to a tree.
struct node{
    node *father = nullptr;
    float d = 0;
    symbol father_sym = bottom_symbol;
    int position;

    node(int pos):
        position(pos){}
};

vector<symbol> conditional_sample(const model& c, const trellis& h, vector<unsigned> m, unsigned seed){
    cerr << "Doing conditional sample" << endl;
    minstd_rand mt(seed);

    const unsigned hh = h.h(), hmask = (1 << hh) - 1;

    // Firstly, this is useful for checking if a state is good:
    vector<unsigned> good_check_vec(h.stego_len());
    for(int i = 0; i < h.stego_len(); ++i)
        for(int j = h.fst(i); j < h.fst(i + 1); ++j)
            good_check_vec[i] = 2 * good_check_vec[i] + m[j];

    // The root of the trellis graph.
    node *root = new node { 0 };

    // The current layer is indexed by (mask, context) pairs. But holding
    // these as pairs in the inner loop is innefficient. Thus I will index
    // (mask, context) as mask | context << hh
    //
    // The root ought properly to be only at key 0, but due to the fact that
    // these entries are accessed only if mentioned in visit_now or visit_next,
    // and entries at wrong positions in the text are counted as being invalid,
    // I can use root as a dummy value, to remove a nullptr check later.
    vector<node*> current_layer{c.context_count() << h.h(), root },
        prev_layer{c.context_count() << h.h(), root };

    // Nodes that we visit on next layer.
    vector<unsigned> visit_now, visit_next;

    // We visit the trellis root, which is properly at key 0.
    visit_next.push_back(0);

    // Advance h.stego_len() the current layer.
    for(int i = 0; i < h.stego_len(); ++i){
        const auto mask_lim = (1 << h.len(i)) - 1;
        const auto my_dLst = h.dLst(i);
        const auto my_effect = h.effect(i);

        if(i > 0)
            cerr << '\r';
        cerr << "At conditional sample step " << i;

        swap(current_layer, prev_layer);
        swap(visit_now, visit_next);
        visit_next.clear();

        for(auto current_idx : visit_now){
            const auto current_node = prev_layer[current_idx];
            const auto msk = current_idx & hmask;
            const auto ctx = current_idx >> hh;
            const auto d = current_node->d;

            for(const auto& t : c.cand_and_p(ctx)){
                const auto b = c.encode(ctx, t.first);
                const auto msk_ = ((msk << my_dLst) ^ (b * my_effect)) & mask_lim;

                if(good_check_vec[i] ^ (msk_ >> (h.lst(i) - h.fst(i + 1)))) continue;

                const auto ctx_ = t.second.second;
                const auto p = t.second.first;
                const auto k = msk_ | ctx_ << hh;

                // No nullptr check since current_layer never contains a null pointer.
                if(current_layer[k]->position != i + 1){
                    current_layer[k] = new node { i + 1 };
                    visit_next.push_back(k);
                }
                const auto next_node = current_layer[k];

                if(!bernoulli_distribution(next_node->d / (next_node->d + d * p))(mt)){
                    next_node->father = current_node;
                    next_node->father_sym = t.first;
                }
                next_node->d += d * p;
            }
        }
    }
    cerr << endl;

    // Select a final node from the current (i.e. last) layer, using
    // a similar strategy to before.
    float d = 0;
    node* me = nullptr;

    for(auto x : visit_next){
        auto other = current_layer[x];
        if(!bernoulli_distribution(d / other->d)(mt))
            me = other;
        d += other->d;
    }
    
    // Now backwalk through the graph to reconstitute the result.
    vector<symbol> ret(h.stego_len());

    for(int i = 0; i < h.stego_len(); ++i){
        ret.rbegin()[i] =  me->father_sym;
        me = me->father;
    }

    return ret;
}
