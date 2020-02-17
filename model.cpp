#include "model.h"

vector<symbol> model::tokenise(string s){
    // Return value.
    vector<symbol> ret;

    // The current word in the following iteration.
    string current;

    // Add in the current to the result, if appropriate, and empty it.
    auto add_and_empty = [&](){
        if(!current.empty()){
            ret.push_back(symbol_name(current));
            current.clear();
        }
    };

    for(auto c : s){
        if(isalnum(c))
            current.push_back(c);
        else
            add_and_empty();
        if(ispunct(c))
            ret.push_back(symbol_name(string(1, c)));
    }
    return ret;
}

// Initialise the model with a-priori information about bottom symbol
// and bottom context.
model::model(unsigned context_len):
    clen(context_len),
    symbol_to_code({ { string{""}, bottom_symbol } }),
    code_to_symbol({ string{""} }),
    sequence_to_context({ { vector<symbol>(context_len, bottom_symbol), bottom_context } }),
    following_symbols({ { { bottom_symbol, { 0.0, bottom_context } } } }),
    total_count({ 1.0 }){}

model model::model_from_text(unsigned context_len, string s){
    // Return value.
    model ret(context_len);

    // First tokenise model, adding in all relevant symbols:
    auto toks = ret.tokenise(s);

    // Now for all context/symbol pairs, just increment model.

    // This variable will hold the context throughout.
    vector<symbol> ctx(context_len, bottom_symbol);

    for(unsigned i = 0; i < toks.size(); ++i){
        // First construct the context.
        // Add _|_ symbols.
        fill(begin(ctx), begin(ctx) + max(context_len, i) - i, 0);

        // Add in real symbols.
        copy(begin(toks) - min(context_len, i), begin(toks) + i, begin(ctx) + max(context_len, i) - i);

        // Increment model
        ret.increment_model(ctx, toks[i]);
    }

    return ret;
}

string model::symbol_meaning(symbol s){
    return code_to_symbol[s];
}

symbol model::symbol_name(string s){
    auto it = symbol_to_code.find(s);
    if(it == end(symbol_to_code)){
        code_to_symbol.push_back(s);
        return symbol_to_code[s] = symbol_to_code.size();
    }
    return it->second;
}

context model::context_name(const vector<symbol>& v){
    auto it = sequence_to_context.find(v);
    if(it == end(sequence_to_context)){
        following_symbols.emplace_back();
        total_count.push_back(0);
        return sequence_to_context[v] = sequence_to_context.size();
    }
    return it->second;
}

void model::increment_model(const vector<symbol>& v, symbol s){
    auto c1 = context_name(v);

    // Get outgoing edges from map.
    auto& out = following_symbols[c1];

    auto it = out.find(s);

    // If this is a new transition.
    if(it == end(out)){
        // Construct next context.
        vector<symbol> target(begin(v) + 1, end(v));
        target.push_back(s);

        // Add transition
        out[s] = make_pair(1.0, context_name(target));
    }

    // Otherwise just increment model directly.
    else 
        it->second.first += 1.0;

    // And increment total count.
    total_count[c1] += 1.0;
}

map<symbol, pair<double, context>> model::cand_and_p(context c){
    return following_symbols[c];
}

unsigned model::encode(context c, symbol s){
    return __builtin_popcount(c + s) % 2;
}

vector<unsigned> model::encode_sequence(vector<symbol> v){
    context c = bottom_context;
    vector<unsigned> ret;

    for(auto x : v){
        ret.push_back(encode(c, x));
        c = following_symbols[c][x].second;
    }
    return ret;
}
