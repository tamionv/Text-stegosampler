#include "model.hxx"
#include <iostream>
#include <fstream>
using namespace std;

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
            current.push_back(tolower(c));
        else
            add_and_empty();
        if(ispunct(c))
            ret.push_back(symbol_name(string(1, c)));
    }
    ofstream g("token-log.txt");
    for(auto x : ret)
        g << symbol_meaning(x) << '\n';
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

    // Fills ctx with contents ending before symbol j, assuming that
    // symbol i is the first symbol in the text.
    auto fill_context = [&](unsigned i, unsigned j){
        unsigned amount = min(j - i, context_len);
        fill(begin(ctx), begin(ctx) + context_len - amount, bottom_symbol);
        copy(begin(toks) + j - amount, begin(toks) + j, begin(ctx) + context_len - amount);
    };

    cerr << "Making model" << endl;

    for(unsigned i = 0; i < toks.size(); ++i){
        if(i > 0) cerr << "\r";
        cerr << "At model making step " << i;

        fill_context(0, i);
        ret.increment_model(ctx, toks[i]);
    }

    // Now add in special logic for "sentence beginnings". This
    // makes the start of our modelled covers more diverse.
    for(unsigned i = 0; i < toks.size(); ++i){
        cerr << "\rAt model making step " << toks.size() + i;

        if(ret.symbol_meaning(toks[i]) != ".")
            continue;

        for(unsigned j = i + 1; j < i + context_len + 1 && j < toks.size(); ++j){
            fill_context(i + 1, j);
            ret.increment_model(ctx, toks[j]);
        }
    }
    cerr << endl;


    return ret;
}

string model::symbol_meaning(symbol s){
    return code_to_symbol[s];
}

symbol model::symbol_name(string s){
    auto it = symbol_to_code.find(s);
    if(it == end(symbol_to_code)){
        code_to_symbol.push_back(s);
        symbol_to_code.emplace_hint(it, s, symbol_to_code.size());
        return symbol_to_code.size() - 1;
    }
    return it->second;
}

context model::context_name(const vector<symbol>& v){
    auto it = sequence_to_context.find(v);
    if(it == end(sequence_to_context)){
        following_symbols.emplace_back();
        total_count.push_back(0);
        sequence_to_context.emplace_hint(it, v, sequence_to_context.size());
        return sequence_to_context.size() - 1;
    }
    return it->second;
}

void model::increment_model(const vector<symbol>& v, symbol s){
    auto c1 = context_name(v);

    // Get outgoing edges from map.
    auto it = following_symbols[c1].find(s);

    // If this is a new transition.
    if(it == end(following_symbols[c1])){
        // Construct next context.
        vector<symbol> target(begin(v) + 1, end(v));
        target.push_back(s);

        // Add transition
        following_symbols[c1][s] = make_pair(1.0, context_name(target));
    }
    // Otherwise just increment model directly.
    else 
        it->second.first += 1.0;

    // And increment total count.
    total_count[c1] += 1.0;
}

const map<symbol, pair<float, context>>& model::cand_and_p(context c) const{
    return following_symbols[c];
}

unsigned model::encode(context c, symbol s) const{
    return (c ^ (s / 2)) & 1;
}

vector<unsigned> model::encode_sequence(vector<symbol> v){
    vector<unsigned> ret;

    for(auto x : v)
        ret.push_back(encode(x));
    return ret;
}

unsigned model::context_count() const{
    return sequence_to_context.size();
}
