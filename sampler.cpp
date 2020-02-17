#include "sampler.h"

#define MK_KEY(x, y) ((unsigned long long)(x) | ((unsigned long long)(y) << 32))
#define GET_MSK(k) ((k) & 0xffffffffll)
#define GET_CTX(k) ((k) >> 32)

// Node in trellis tree. Rather than explicitly maintain all back edges
// in the graph, I will choose a back edge randomly as I go along, thus
// needing constant space per node. Interestingly this reduces the
// trellis graph to a tree.
struct node{
    mask msk;
    context ctx;
    double d;

    node *father;
    symbol father_sym;

    node(mask m, context c):
        msk(m),
        ctx(c),
        d(0),
        father(nullptr),
        father_sym(bottom_symbol){}
};

vector<symbol> conditional_sample(model& c, trellis& h, vector<unsigned> m, unsigned seed){
    mt19937 mt(seed);
    using ull = unsigned long long;
    // The current layer is indexed by (mask, context) pairs. But holding
    // these as pairs in the inner loop is innefficient. Thus I will index
    // (mask, context) as mask | context << 32 i.e. MK_KEY(mask, context).
    // Note that mask = GET_MSK(MK_KEY(mask, context)), and that 
    // context = GET_CTX(MK_KEY(mask, context))
    map<ull, node*> current_layer;

    // Conditionally get a node in the next layer, creating it if it doesn't exist.
    auto get_node = [&](unsigned msk, context ctx){
        auto k = MK_KEY(msk, ctx);
        auto it = current_layer.find(k);
        if(it == end(current_layer))
            return current_layer[k] = new node { msk, ctx };
        return it->second;
    };

    // Add trellis root.
    current_layer[MK_KEY(0, bottom_context)] = new node { 0, bottom_context };

    // Advance h.stego_len() the current layer.
    for(int i = 0; i < h.stego_len(); ++i){
        map<ull, node*> prev_layer = move(current_layer);
        for(auto current_node : prev_layer){
            auto msk = current_node.second->msk;
            auto ctx = current_node.second->ctx;
            auto d = current_node.second->d;

            for(auto t : c.cand_and_p(ctx)){
                auto b = c.encode(ctx, t.first);
                auto msk_ = ((msk << h.dLst(i)) ^ (b *  h.effect(i)))% (1 << h.len(i));
                auto ctx_ = t.second.second;
                auto p = t.second.first;
                    
                auto next_node = get_node(msk_, ctx_);
                if(!bernoulli_distribution(next_node->d / (next_node->d + d * p))(mt)){
                    next_node->father = current_node.second;
                    next_node->father_sym = t.first;
                }
                next_node->d += d * p;
            }
        }
    }

    // Select a final node from the current (i.e. last) layer, using
    // a similar strategy to before.
    double d = 0;
    node* me = nullptr;

    for(auto x : current_layer){
        if(!bernoulli_distribution(d / x.second->d)(mt))
            me = x.second;
        d += x.second->d;
    }
    
    // Now backwalk through the graph to reconstitute the result.
    vector<symbol> ret(h.stego_len());

    for(int i = 0; i < h.stego_len(); ++i){
        ret.rbegin()[i] =  me->father_sym;
        me = me->father;
    }

    return ret;
}
