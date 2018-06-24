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
#include <queue>
using namespace std;

template <class TInputContainer, class OIter>
OIter copy(TInputContainer container, OIter out_it) {
    return copy(container.begin(), container.end(), out_it);
};

template <class TIter1, class TIter2>
bool areDisjoint(TIter1 container, TIter2 other_container) {
    vector<typename TIter1::value_type> intersection;
    set_intersection(container.begin(), container.end(),
                    other_container.begin(), other_container.end(),
                    intersection.begin());

    return intersection.empty();
};


struct Feature  {
public:
    const int id;
    Feature(const int id) : id(id) {}

    Feature(const int id, unordered_set<int>&& spies, unordered_set<int>&& innocents)
        : id(id), spies_(spies), innocents_(innocents) {}


    const unordered_set<int> spies()const { return spies_; }
    const unordered_set<int> innocents()const { return innocents_; }

    void addSpy(int spy) {
        spies_.emplace(spy);
        assert(spiesAndInnocentsAreDisjoint());
    }
    void addInnocent(int innocent) {
        innocents_.emplace(innocent);
        assert(spiesAndInnocentsAreDisjoint());
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

    bool spiesAndInnocentsAreDisjoint() {
//        vector<int> intercetion;
//        set_intersection(spies_.begin(), spies_.end(),
//                         innocents_.begin(), innocents_.end() ,
//                         intercetion.begin());
//        return intercetion.empty();
        return areDisjoint(spies(), innocents());
    }
};

struct Command {
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

    bool onlySpiesLeft() {
        return all_of(features().begin(), features().end(),
                     [](const Feature& feature) {
                         return feature.innocents().size() == 0;
        });
    }

    const vector<Feature>& features() const { return features_; }

    State* indicateSpiesHaveFeature(const Feature& indicated_feature) const {
        vector<Feature> new_features;
        new_features.reserve(features_.size() - 1);

        for (auto& feature : features_) {
            if(feature.id != indicated_feature.id) {
                unordered_set<int> new_spies;
                new_spies.reserve(feature.spies().size());

                for(int spy : feature.spies()) {
                    if(indicated_feature.spies().count(spy) == 0) {
                        new_spies.emplace(spy);
                    }
                }
                unordered_set<int> innocents_copy(feature.innocents());

                new_features.emplace_back(feature.id,
                                          move(new_spies), move(innocents_copy));
            }
        }
        assert(all_of(new_features.begin(), new_features.end(),
                     [&indicated_feature](Feature f) {
                         return areDisjoint(f.spies(), indicated_feature.spies()) &&
                                areDisjoint(f.innocents(), indicated_feature.spies());
            }));
        return new State(move(new_features));
    }
    State* indicateInnocentsHaveFeature(const Feature& indicated_feature) const {
        vector<Feature> new_features;
        new_features.reserve(features_.size() - 1);

        for (auto& feature : features_) {
            if (feature.id != indicated_feature.id) {
                unordered_set<int> new_innocents;
                new_innocents.reserve(feature.innocents().size());

                for (int innocent : feature.innocents()) {
                    if(indicated_feature.innocents().count(innocent) == 0) {
                        new_innocents.emplace(innocent);
                    }
                }
                unordered_set<int> spies_copy(feature.spies());

                new_features.emplace_back(feature.id,
                                          move(spies_copy), move(new_innocents));
            }
        }
        assert(all_of(new_features.begin(), new_features.end(),
                      [&indicated_feature](Feature f) {
                          return areDisjoint(f.spies(), indicated_feature.innocents()) &&
                                 areDisjoint(f.innocents(), indicated_feature.innocents());
                      }));
        return new State(move(new_features));
    }

private:
    vector<Feature> features_;
};

class PossibleMovesCalculator {
public:
    const vector<pair<State*, Command>>& possibleMoves() {return possible_moves; }

    void calculatePossibleMoves(const State& state) {
        possible_moves.clear();
        for (auto& feature : state.features()) {
            if (feature.innocents().empty()) {
                auto* indicate_spies_state = state.indicateSpiesHaveFeature(feature);
                possible_moves.emplace_back(
                    indicate_spies_state, Command {feature.id, true});
            }
            if(feature.spies().empty()) {
                auto* indicate_innocents_state = state.indicateInnocentsHaveFeature(feature);
                possible_moves.emplace_back(
                    indicate_innocents_state, Command{ feature.id, false });
            }
        }
    }

private:
    vector<pair<State*, Command>> possible_moves;
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
    }//endregion

    auto* start_state = new State(std::move(features));

    // TODO: use doubly-linked tree of Moves instead?
    // and delete states on the fly, after we've added adjacent
    // { state : { , prev state } }
    unordered_map<State*, pair<State *, Command>> came_from;
    came_from.insert(make_pair(start_state,
                               make_pair(nullptr, Command{ -1, false })));

    queue<State*> states;
    states.push(start_state);

    PossibleMovesCalculator movesCalculator;
    State* winning_state;

    while (! states.empty()) {
        auto* curr_state = states.front();
        states.pop();

        if(curr_state->onlySpiesLeft()) {
            winning_state = curr_state;
            break;
        }

        movesCalculator.calculatePossibleMoves(*curr_state);
        for (auto& move : movesCalculator.possibleMoves()) {
            came_from.insert(make_pair(move.first,
                                       make_pair(curr_state, move.second)));
            states.push(move.first);
        }
    }

    // get moves sequence
    stack<Command> commands;
    State* curr_state = winning_state;
    while (curr_state != nullptr) {
        auto move = came_from.at(curr_state);
        commands.emplace(move.second);
        curr_state = move.first;
    }

    cerr << "\n";
    while(! commands.empty()) {
        auto move = commands.top();
        commands.pop();

        if(! move.indicates_spies) {
            cout << "NOT ";
        }
        cout << feature_names.at(move.feature_id);
    }

    cout << "answer" << endl;
}