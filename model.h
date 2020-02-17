#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <map>
using namespace std;


// Each symbol will be represented by an integer code.
using symbol = int;

// A priori I fix that _|_ is represented by 0.
constexpr int bottom_symbol = 0;

// Each possible context will be represented by an integer code.
using context = int;

// Likewise, I fix that the context containing only _|_ is represented by 0.
constexpr int bottom_context = 0;

class model {
    map<string, symbol> symbol_to_code;
    vector<string> code_to_symbol;

    map<vector<symbol>, context> sequence_to_context;
    vector<vector<symbol>> context_to_sequence;

    // Tokenises a text, adding in symbol meanings to model.
    vector<symbol> tokenise(string);
public:
    // Create a model from an input text.
    static model model_from_text(string);

    // Translate a symbol code back into its meaning.
    string symbol_meaning(symbol);
    // Translate a concrete symbol into its code.
    symbol symbol_name(string);

    // Given the context code for c1 ... ck, and a new
    // symbol code c', find context code for c2 ... ck c'. 
    context advance_context(context, symbol);

    // Given a context code, return possible following symbols,
    // together with relevant probabilities.
    vector<pair<double, symbol>> cand_and_p(context);

    // Given a context code, and a symbol, encode the symbol.
    int encode(context, symbol);
};

#endif
