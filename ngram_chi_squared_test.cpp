#include <bits/stdc++.h>
using namespace std;

vector<vector<string>> tokenise(vector<string>& s){
    // Return value.
    vector<vector<string>> ret;

    // The current word in the following iteration.
    string current;

    // Add in the current to the result, if appropriate, and empty it.
    auto add_and_empty = [&](){
        if(!current.empty()){
            ret.back().push_back(current);
            current.clear();
        }
    };

    for(auto& name : s){
        ifstream f(name);
        ret.emplace_back();
        for(char c; f.get(c); ){
            if(isalnum(c))
                current.push_back(tolower(c));
            else
                add_and_empty();
            if(ispunct(c))
                ret.back().push_back(string(1, c));
        }
    }
    return ret;
}

int main(){
    vector<string> names;
    names.emplace_back("illiad.txt");

    for(int i = 1; i <= 1000; ++i)
        names.emplace_back(string("stegotexts/") + to_string(i));

    auto ss = tokenise(names);

    while(ss.back().empty()) ss.pop_back();
    cerr << ss.size() << endl;

    int clen = 1;

    cout << "CLEN: ";
    cin >> clen;

    map<vector<string>, int> theoretical, experimental;

    auto add_to_map = [&](int idx, map<vector<string>, int>& mp, int start = 0){
        for(int i = start; i + clen <= ss[idx].size(); ++i){
            vector<string> arr(clen);

            copy(begin(ss[idx]) + i, begin(ss[idx]) + i + clen, begin(arr));

            mp[arr]++;
        }
    };

    cout << endl;
    add_to_map(0, theoretical);
    for(int i = 1; i < ss.size(); ++i)
        add_to_map(i, experimental, 250);

    int minimum_frequency = 0;
    int total_count = 0;

    cout << "MINIMUM FREQ: ";
    cin >> minimum_frequency;

    int theory_obs = 0, exp_obs = 0;

    for(auto x : theoretical){
        if(x.second < minimum_frequency) continue;
        theory_obs += x.second;
        exp_obs += experimental[x.first];
    }

    double theory_scale = (double)exp_obs / (double) theory_obs;
    cerr << theory_scale << ' ' << exp_obs << ' ' << theory_obs << endl;

    double chi_sq = 0;

    ofstream g("tmp");

    for(auto x : theoretical){
        if(x.second < minimum_frequency) continue;
        for(auto y : x.first)
            g << y << ", ";
        g << theory_scale * x.second << ", " << experimental[x.first] << endl;
        double tmp = theory_scale * x.second - experimental[x.first];
        chi_sq += tmp * tmp / (theory_scale * x.second);
        ++total_count;
    }

    cout << "CHI_SQ: " << chi_sq << endl;
    cout << "NUMBER OF DF: " << total_count - 1 << endl;
}
