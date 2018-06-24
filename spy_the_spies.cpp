#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <regex>
#include <cassert>

using namespace std;

template <class TInputContainer, class OIter>
OIter copy(TInputContainer container, OIter out_it) {
    return copy(container.begin(), container.end(), out_it);
};

template<class TContainer, class TPred>
bool all_of(TContainer container, TPred pred) {
    return copy(container.begin(), container.end(), pred);
};


struct Feature  {
public:
    const int id;
    Feature(const int id) : id(id) {}

    int spiesCount()
        { return spies_.size(); }
    int innocentsCount()
        { return innocents_.size(); }

    void addSpy(int spy)
        {
            spies_.insert(spy);
            assert(spiesAndInnocentsAreDiscjoint());
        }
    void addInnocent(int innocent)
        {
            innocents_.insert(innocent);
            assert(spiesAndInnocentsAreDiscjoint());
        }

    string to_string() {
        stringstream ss;
        ss << "{ id : " << id << ",\n" << "\tspies:";
        ostream_iterator<int> ostream_it(ss, ", ");
        copy(spies_, ostream_it);
        ss << "\n\tinnocents:";
        copy(innocents_, ostream_it);
        ss << "\n}";

        return ss.str();
    }
private:
    unordered_set<int> spies_;
    unordered_set<int> innocents_;

    bool spiesAndInnocentsAreDiscjoint() {
        vector<int> intercetion;
        set_intersection(spies_.begin(), spies_.end(),
                         innocents_.begin(), innocents_.end() ,
                         intercetion.begin());
        return intercetion.empty();
    }
};

struct Move {
    // what feature to mark
    int feature_id;
    // if false - indicates innocent
    bool indicates_spies;
};
// choosing next states:
// can't mark feature as spy  if any innocent has it
// can't  mark feature as innocent if any spy has it

// mark as spy : for all spies with feature - delete entries for spy in all other features



// feature having hashmaps for spies and innocents:
// anySpyHasFeature - hash-const
// anyInnocentHasFeature - hash-const


// select markable features: F

// mark as spy:
//  get all spies with feature : hash-const
// delete all spies from other features: 5 * hash-const

// mark as innocent : 9 * hash-const ()
struct State {

public:
    explicit State(vector<Feature>&& features)
        : features_(std::move(features)) {}

    void getPossibleNextStates(vector<State*>& next_states) {

    }

    bool onlySpiesLeft() {

    }

private:
    vector<Feature> features_;

    State* indicateSpiesHaveFeature(int feature_id) {

    }
    State* indicateInnocentsHaveFeature(int feature_id) {

    }
    void getSpyOnlyFeatures(vector<int>& spy_only_features) {

    }
    void getInnocentOnlyFeatures(vector<int>& innocent_only_features) {

    }
    // if false, some innocent has the feature
    bool anySpyHasFeature(int feature_id) {

    }
    // if false, some spy has the feature
    bool anyInnocentHasFeature(int feature_id) {

    }
};

int main()
{
    //region input
    const int kSuspectCount = 15;

    string enemy1, enemy2, enemy3, enemy4, enemy5, enemy6;
    cin >> enemy1 >> enemy2 >> enemy3 >> enemy4 >> enemy5 >> enemy6; cin.ignore();
    unordered_set<string> spies ({ enemy1, enemy2, enemy3, enemy4, enemy5, enemy6 });    // temp for input

    // map { id : suspect_name }
    vector<string> suspect_names(kSuspectCount, "#");
    // map { id : bool }, true if spy
    vector<bool> are_spies(kSuspectCount, false);

    // map {id : feature name}
    vector<string> feature_names;
    feature_names.reserve(kSuspectCount * 3);
    // {name : id obj}
    unordered_map<string, int> nameIdMap;
    vector<Feature> features;   // temp for input
    features.reserve(kSuspectCount * 3);

    int curr_id = 0;

    for (int suspect_id = 0; suspect_id < kSuspectCount; suspect_id++) {
        string suspect_name; cin >> suspect_name; cin.ignore();
        suspect_names.at(suspect_id) = suspect_name;

        bool is_spy = spies.count(suspect_name) != 0;
        are_spies[suspect_id] = is_spy;

        int feat_count; cin >> feat_count; cin.ignore();

        for (int i = 0; i < feat_count; ++i) {
            string feature_name; cin >> feature_name; cin.ignore();
            feature_names.push_back(feature_name);

            auto id_it = nameIdMap.find(feature_name);
            if(id_it == nameIdMap.end()) {
                id_it = nameIdMap.insert(make_pair(
                    feature_name, curr_id)).first;
                features.push_back(Feature(curr_id++));
            }
            assert(id_it->second < curr_id);
            if(is_spy) {
                features.at(id_it->second).addSpy(suspect_id);
            } else {
                features.at(id_it->second).addInnocent(suspect_id);
            }

        }
    }

    for (int i = 0; i < features.size(); ++i)
        { assert(features.at(i).id == i); }

    cerr << "Suspects: \n";
    for (int i = 0; i < suspect_names.size(); ++i) {
        cerr << "\t" << suspect_names.at(i);
        cerr << (are_spies.at(i) ?
            " : spy,\n" : " : innocent,\n");
    }

    cerr << "features:\n" ;//<< curr_id + 1;
    for (auto& feature : features) {
        cerr << feature.to_string() << "\n";
    }

    State start_state(std::move(features));


    //endregion

//    State* start_state = new State();
    // Write an action using cout. DON'T FORGET THE "<< endl"
    // To debug: cerr << "Debug messages..." << endl;

    cout << "answer" << endl;
}