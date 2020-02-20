#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <map>
#include <string>
using namespace std;

// Each symbol will be represented by an integer code.
using symbol = unsigned;

// A priori I fix that _|_ is represented by 0.
constexpr unsigned bottom_symbol = 0;

// Each possible context will be represented by an integer code.
using context = unsigned;

// Likewise, I fix that the context containing only _|_ is represented by 0.
constexpr unsigned bottom_context = 0;

class model {
    unsigned clen;
    map<string, symbol> symbol_to_code;
    vector<string> code_to_symbol;

    map<vector<symbol>, context> sequence_to_context;
    vector<map<symbol, pair<float, context>>> following_symbols;
    vector<float> total_count;

    // Tokenises a string, splitting it into words and punctuation,
    // and adding in symbol meanings to model.
    vector<symbol> tokenise(string);
public:
    // Empty model constructor.
    model(unsigned context_length);

    // Create a model from an input text.
    static model model_from_text(unsigned context_len, string);

    // Translate a symbol code back into its meaning.
    string symbol_meaning(symbol);
    // Translate a concrete symbol into its code. Adds symbol
    // to model if not yet encountered.
    symbol symbol_name(string);

    // Translate a sequence of symbols into its context code.
    // Adds context to model if not yet encountered.
    context context_name(const vector<symbol>&);

    // Adds one to the count of a certain context/symbol pair.
    void increment_model(const vector<symbol>&, symbol);

    // Given a context code, return possible following symbols,
    // together with relevant probabilities. Also include the
    // contexts that would result.
    const map<symbol, pair<float, context>>& cand_and_p(context) const;

    // Given a context code, and a symbol, encode the symbol.
    unsigned encode(context, symbol) const;

    // Given a string of symbols, encode the symbols.
    vector<unsigned> encode_sequence(vector<symbol>);

    // Find number of contexts.
    unsigned context_count() const;
};

#endif
