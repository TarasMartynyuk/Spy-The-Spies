#include <iostream>
#include <string>
#include <utility> #include <vector>
#include <algorithm>
#include <vector>
#include <unordered_map>

using namespace std;

struct Feature {
public:
    int spiesCount();
    int nonSpiesCount();

private:
    vector<int> spies_;
    vector<int> non_spies_;
};

struct Move {
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

    // if false, some innocent feature
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

//    unordered_map<string, Feature*> features;
    // map {int : feature name}
    vector<string> feature_names;
    vector<Feature> features;
    features.reserve(kSuspectCount * 3);

    for (int suspect_id = 0; suspect_id < kSuspectCount; suspect_id++) {
        string suspect;
        getline(cin, suspect);


    }

    //endregion

//    State* start_state = new State();
    // Write an action using cout. DON'T FORGET THE "<< endl"
    // To debug: cerr << "Debug messages..." << endl;

    cout << "answer" << endl;
}