#define NDEBUG
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
#include <chrono>
using namespace std;
using namespace std::chrono;

//region defs

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
class Timer {
public:
    void start() {
        time_started = high_resolution_clock::now();
    }
    void stopAndPrintResult(const string& description) {
        auto finish = high_resolution_clock::now();
        auto seconds = duration_cast<microseconds>(finish - time_started);
        cerr << description << ": " << (seconds.count() / 1000.0) << endl;
    }

    high_resolution_clock::time_point time_started;
};

class State;
class Command;

void constructMovePathAndPrintAnswer(State* winning_state,
                                     const unordered_map<State*, pair<State*, Command>>& came_from,
                                     const vector<string>& feature_names);
//endregion

//region struct

class Feature  {
public:
    const int id;
    Feature(const int id) : id(id) {}

    Feature(const int id, unordered_set<int>&& spies, unordered_set<int>&& innocents)
        : id(id), spies_(spies), innocents_(innocents) {}


    const unordered_set<int> spies()const { return spies_; }
    const unordered_set<int> innocents()const { return innocents_; }

    void addSpy(int spy) {
        spies_.emplace(spy);
//        assert(spiesAndInnocentsAreDisjoint());
    }
    void addInnocent(int innocent) {
        innocents_.emplace(innocent);
//        assert(spiesAndInnocentsAreDisjoint());
    }

    string to_string() {
        stringstream ss;
        ss << "{ id : " << id << ",\n" << "\tspies: ";
        ostream_iterator<int> ostream_it(ss, ", ");
        copy(spies_, ostream_it);
        ss << "\n\tinnocents: ";
        copy(innocents_, ostream_it);
        ss << "\n}";

        return ss.str();
    }
private:
    unordered_set<int> spies_;
    unordered_set<int> innocents_;

    bool spiesAndInnocentsAreDisjoint() {
        return areDisjoint(spies(), innocents());
    }
};
struct Command {
    // what feature to mark
    int feature_id;
    // if false - indicates innocent
    bool indicates_spies;
};
// mark as innocent : 9 * hash-const ()
class State {

public:
    const int spies_left;
    const int innocents_left;

    State(vector<Feature>&& features, int innocents_left, int spies_left)
        : features_(std::move(features)), innocents_left(innocents_left), spies_left(spies_left) {}

    bool onlySpiesLeft() {
        return all_of(features().begin(), features().end(),
                     [](const Feature& feature) {
                         return feature.spies().size() == 0;
        });
    }

    bool onlyInnocentsLeft() {
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
                new_features.emplace_back(feature.id,
                                          getSpiesIgnoringIndicated(feature, indicated_feature),
                                          unordered_set<int>(feature.innocents()));
            }
        }
        assert(all_of(new_features.begin(), new_features.end(),
                     [&indicated_feature](Feature f) {
                         return areDisjoint(f.spies(), indicated_feature.spies()) &&
                                areDisjoint(f.innocents(), indicated_feature.spies());
            }));

        return new State(move(new_features), 0, 0);
    }
    State* indicateInnocentsHaveFeature(const Feature& indicated_feature) const {
        vector<Feature> new_features;
        new_features.reserve(features_.size() - 1);

        for (auto& feature : features_) {
            if (feature.id != indicated_feature.id) {
                new_features.emplace_back(feature.id,
                                          unordered_set<int>(feature.spies()),
                                          getInnocentsIgnoringIndicated(feature, indicated_feature));
            }
        }
        assert(all_of(new_features.begin(), new_features.end(),
                      [&indicated_feature](Feature f) {
                          return areDisjoint(f.spies(), indicated_feature.innocents()) &&
                                 areDisjoint(f.innocents(), indicated_feature.innocents());
                      }));
        return new State(move(new_features), 0, 0);
    }

private:
    vector<Feature> features_;

    unordered_set<int> getInnocentsIgnoringIndicated(
        const Feature& feature_to_modify,
        const Feature& indicated_feature) const
    {
        unordered_set<int> new_innocents;
        new_innocents.reserve(feature_to_modify.innocents().size());

        for (int innocent : feature_to_modify.innocents()) {
            if (indicated_feature.innocents().count(innocent) == 0) {
                new_innocents.emplace(innocent);
            }
        }
        return new_innocents;
    }

    unordered_set<int> getSpiesIgnoringIndicated(
                                                const Feature& feature_to_modify,
                                                const Feature& indicated_feature)const {
        unordered_set<int> new_spies;
        new_spies.reserve(feature_to_modify.spies().size());

        for (int spy : feature_to_modify.spies()) {
            if (indicated_feature.spies().count(spy) == 0) {
                new_spies.emplace(spy);
            }
        }
        return new_spies;
    }
};

// heuristic distance - the lower, the closer we are to goal, hence the better

struct HeuristicComparator {
    const int max_spies = 6;
    const int max_innocents = 9;

    // sorts descending
    bool operator()(const State& left, const State& right) {
        return heuristic(left) > heuristic(right);
    }

    int heuristic(const State& state) {
        return 3 * state.spies_left / max_spies +
               2 * state.innocents_left / max_innocents;
    }
};

// the less percentage of spies or innocents left - the better



// prefer when both spies and innocents left are low or
// when only one is low?

//endregion

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
    //region mock
//
//    stringstream cin;
//    cin << "Fred Mark Kim Anita Dwayne Nick\n"
//           "Daniel 1 chinese\n"
//           "Clem 1 german\n"
//           "Dwayne 1 french\n"
//           "Anita 1 french\n"
//           "Spruce 1 german\n"
//           "Fred 1 french\n"
//           "Adan 1 chinese\n"
//           "Sven 1 irish\n"
//           "Nick 1 french\n"
//           "Tim 1 irish\n"
//           "Harley 1 english\n"
//           "Mary 1 russian\n"
//           "Kim 1 french\n"
//           "Rashad 1 chinese\n"
//           "Mark 1 french\n";

    //    stringstream cin;
    //    cin << "Tabitha  Rolf Derick Ronaldo Tempest Jeanne\n"
//           "Tabitha 1 scottish \n"
//           "Rolf 1 hebrew \n"
//           "Mohammad 1 arabic \n"
//           "Jacob 1 arabic \n"
//           "Derick 1 hebrew \n"
//           "Meta 1 arabic \n"
//           "Ronaldo 1 scottish \n"
//           "Melville 1 arabic \n"
//           "Hermon 1 arabic \n"
//           "Tempest 1 swedish \n"
//           "Jeanne 1 persian \n"
//           "Kourtney 1 arabic \n"
//           "Dallas 1 arabic \n"
//           "Vena 1 arabic \n"
//           "Eros 1 arabic\n";
     //endregion
    Timer timer;
    timer.start();
    //region input
    assert(false);
    const int kSuspectCount = 15;

    string enemy1, enemy2, enemy3, enemy4, enemy5, enemy6;
    cin >> enemy1 >> enemy2 >> enemy3 >> enemy4 >> enemy5 >> enemy6; cin.ignore();
    unordered_set<string> spies ({ enemy1, enemy2, enemy3, enemy4, enemy5, enemy6 });    // temp for input

    // map { id : suspect_name }
    vector<string> suspect_names(kSuspectCount, "#");

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

        int feat_count; cin >> feat_count; cin.ignore();

        for (int i = 0; i < feat_count; ++i) {
            string feature_name; cin >> feature_name; cin.ignore();

            auto id_it = nameIdMap.find(feature_name);

            if(id_it == nameIdMap.end()) {
                id_it = nameIdMap.insert(make_pair(
                    feature_name, curr_id)).first;
                feature_names.push_back(feature_name);

                assert(feature_names.size() == curr_id + 1);
                assert(feature_names.at(curr_id) == feature_name);

                features.emplace_back(curr_id++);
            }

            assert(id_it->second < curr_id);
            if(is_spy) {
                features.at(id_it->second).addSpy(suspect_id);
            } else {
                features.at(id_it->second).addInnocent(suspect_id);
            }
        }
        cerr << "\n";
    }

    for (int i = 0; i < features.size(); ++i)
        { assert(features.at(i).id == i); }
//    cerr << "Suspects: \n";
//    for (int i = 0; i < suspect_names.size(); ++i) {
//        cerr << "\t" << suspect_names.at(i);
//        cerr << (are_spies.at(i) ?
//            " : spy,\n" : " : innocent,\n");
//    }
//
//    cerr << "features:\n" ;
//    for (auto& feature : features) {
//        cerr <<  feature_names.at(feature.id) << " : " << feature.to_string() << "\n";
//    }
    //endregion
    timer.stopAndPrintResult("parsed input");
    timer.start();
    auto* start_state = new State(std::move(features), 9, 6);

    // TODO: use doubly-linked tree of Moves instead?
    // and delete states on the fly, after we've added adjacent
    // { state : { , prev state } }
    unordered_map<State*, pair<State *, Command>> came_from;
    came_from.insert(make_pair(start_state,
                               make_pair(nullptr, Command{ -1, false })));

    queue<State*> states ({ start_state});
//    states.push();

    PossibleMovesCalculator movesCalculator;
    State* winning_state;

    int states_additions = 1;
    int states_poppings = 0;

    while (! states.empty())  {
        auto* curr_state = states.front();
        states.pop();
        states_poppings++;

        if(curr_state->onlySpiesLeft() || curr_state->onlyInnocentsLeft()) {
            winning_state = curr_state;
            break;
        }

        movesCalculator.calculatePossibleMoves(*curr_state);
        for (auto& move : movesCalculator.possibleMoves()) {
            came_from.insert(make_pair(move.first,
                                       make_pair(curr_state, move.second)));
            states.push(move.first);
            states_additions++;
        }
    }

    timer.stopAndPrintResult("\nalgorithm");
    cerr << "states additions: " << states_additions << endl;
    cerr << "states poppings: " << states_poppings << endl;
    timer.start();

    constructMovePathAndPrintAnswer(winning_state, came_from, feature_names);
    timer.stopAndPrintResult("answer");
    //TODO: delete states - iter over came_from;
}

void constructMovePathAndPrintAnswer(
    State* winning_state,
    const unordered_map<State*, pair<State*, Command>>& came_from,
    const vector<string>& feature_names) {
    // get moves sequence
    stack<Command> commands;
    State* curr_state = winning_state;
    while (true) {
        auto move = came_from.at(curr_state);
        if (move.first == nullptr) {
            break;
        }
        commands.emplace(move.second);
        curr_state = move.first;
    }

    cerr << "\n";
    while (!commands.empty()) {
        auto move = commands.top();
        commands.pop();

        if (!move.indicates_spies) {
            cout << "NOT ";
        }
        cout << feature_names.at(move.feature_id) << endl;
    }
}