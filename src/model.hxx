#ifndef MODEL_H
#define MODEL_H

#include <map>
#include <string>
#include <vector>
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
    int cnt;
    double buf[256][256] = {};

  public:
    // Empty model constructor.
    model();

    // Create a model from a file.
    static model model_from_file(string);

    // Given a context code, return possible following symbols,
    // together with relevant probabilities. Also include the
    // contexts that would result.
    const map<symbol, pair<float, context>> cand_and_p(context) const;

    // Given a context code, and a symbol, encode the symbol.
    unsigned encode(context, symbol) const;

    // Given a string of symbols, encode the symbols.
    vector<unsigned> encode_sequence(vector<symbol>) const;

    // Find number of contexts.
    unsigned context_count() const;

    // Probability of a string of symbols;
    double probability(vector<symbol>);
};

#endif
