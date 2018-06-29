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
struct HeuristicPtrComparator;

using CameFromMap = unordered_map<const State*, pair<const State*, Command>>;

void constructMovePathAndPrintAnswer(const State* winning_state,
                                     const CameFromMap& came_from,
                                     const vector<string>& feature_names);

void aStar(CameFromMap& came_from, const State* start_state, const State*& winning_state);

const int kSuspectCount = 15;
const int kInnocentsCount = 9;
const int kSpiesCount = 6;

//endregion

//region struct
// used as a unordered set for spies and innocents
class SuspectSet {
public:
    static const int kSuspectsCount = kSuspectCount;

    SuspectSet() : suspect_marks_(kSuspectsCount, false), marks_count(0) {}

    SuspectSet(const SuspectSet& other) :
        suspect_marks_(other.suspect_marks_),
        marks_count(other.marks_count) {}

    SuspectSet(SuspectSet&& other) :
        suspect_marks_(move(other.suspect_marks_)),
        marks_count(other.marks_count) {}

    int size()const {
        return marks_count;
    }
    bool empty()const {
        return marks_count == 0;
    }
    bool contains(int suspect_id)const {
        assert(suspect_id >= 0 && suspect_id < kSuspectsCount);
        return suspect_marks_.at(suspect_id);
    }
    void addSuspect(int suspect_id) {
        assert(suspect_id >= 0 && suspect_id < kSuspectsCount);
        suspect_marks_.at(suspect_id) = true;
        marks_count++;
    }

private:

    int marks_count;
    vector<bool> suspect_marks_;
};

class Feature  {
public:
    const int id;
    Feature(const int id) : id(id) {}

    Feature(const int id, SuspectSet&& spies, unordered_set<int>&& innocents)
        : id(id), spies_(spies),
        innocents_(innocents) {
    }

    const SuspectSet& spies()const { return  spies_; }
    int spiesCount()const {
        return spies_.size();
    }
    bool noSpies()const {
        return spies_.empty();
    }
    bool hasSpy(int spy)const {
        return spies_.contains(spy);
    }
    void addSpy(int spy) {
        spies_.addSuspect(spy);
        assert(spies_.size() <= kSpiesCount);
    }

    const unordered_set<int> innocents() const { return innocents_; }


    void addInnocent(int innocent) {
        innocents_.emplace(innocent);
    }

private:
    SuspectSet spies_;
    unordered_set<int> innocents_;

//    bool spiesAndInnocentsAreDisjoint() {
//        return areDisjoint(spies_, innocents());
//    }
};
struct Command {
    // what feature to mark
    int feature_id;
    // if false - indicates innocent
    bool indicates_spies;
};

struct HeuristicPtrComparator {
    const int max_spies = 6;
    const int max_innocents = 9;

    // sorts descending
    bool operator()(const State* left, const State* right) {
        return heuristic(*left) >= heuristic(*right);
    }
    // heuristic distance - the lower, the closer we are to goal, hence the better
    double heuristic(const State& state);
};

class State {
public:
    const int spies_left;
    const int innocents_left;
#ifndef NDEBUG
    double heuristic;
#endif
    const int start_features_count;

    State(vector<Feature>&& features,
          int innocents_left,
          int spies_left, const
          int start_features_count)
        : features_(std::move(features)), innocents_left(innocents_left), spies_left(spies_left),
        start_features_count(start_features_count) {
#ifndef NDEBUG
        heuristic = HeuristicPtrComparator().heuristic(*this);
#endif
    }

    bool onlySpiesLeft()const {
        bool res = innocents_left == 0;
        assert(res ? spies_left != 0 : true);
        assert(res ? featuresHaveNoInnocents() : true);
        return res;
    }

    bool onlyInnocentsLeft()const {
        bool res = spies_left == 0;
        assert(res ? innocents_left != 0 : true);
        assert(res ? featuresHaveNoSpies() : true);
        return res;
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
//        assert(all_of(new_features.begin(), new_features.end(),
//                     [&indicated_feature](Feature f) {
//                         return areDisjoint(f.spies(), indicated_feature.spies()) &&
//                                areDisjoint(f.innocents(), indicated_feature.spies());
//            }));
        int new_spies_count = spies_left - indicated_feature.spiesCount();

        return new State(move(new_features), innocents_left, new_spies_count, start_features_count);
    }
    State* indicateInnocentsHaveFeature(const Feature& indicated_feature) const {
        vector<Feature> new_features;
        new_features.reserve(features_.size() - 1);

        for (auto& feature : features_) {
            if (feature.id != indicated_feature.id) {
                new_features.emplace_back(feature.id,
                                          SuspectSet(feature.spies()),
                                          getInnocentsIgnoringIndicated(feature, indicated_feature));
            }
        }
//        assert(all_of(new_features.begin(), new_features.end(),
//                      [&indicated_feature](Feature f) {
//                          return areDisjoint(f.spies(), indicated_feature.innocents()) &&
//                                 areDisjoint(f.innocents(), indicated_feature.innocents());
//                      }));
        int new_innocents_count = innocents_left - indicated_feature.innocents().size();
        return new State(move(new_features), new_innocents_count, spies_left, start_features_count);
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
    SuspectSet getSpiesIgnoringIndicated(
                                        const Feature& feature_to_modify,
                                        const Feature& indicated_feature)const {
        SuspectSet new_spies;

        for (int spy = 0; spy < kSuspectCount; ++spy) {
            if (feature_to_modify.hasSpy(spy) &&
                ! indicated_feature.hasSpy(spy)) {
                new_spies.addSuspect(spy);
            }
        }
        assert(new_spies.size() <= kSpiesCount);
        return new_spies;
    }

    bool featuresHaveNoSpies() const {
        return all_of(features().begin(), features().end(),
                      [](const Feature& feature) {
                          return feature.noSpies();
                      });
    }
    bool featuresHaveNoInnocents() const {
        return all_of(features().begin(), features().end(),
               [](const Feature& feature) {
                   return feature.innocents().empty();
               });
    }
};

double HeuristicPtrComparator::heuristic(const State& state) {
    // range [0,  18 + 18] = [0, 36]
    double win_distance = 3 * state.spies_left +
                          2 * state.innocents_left;
    // range [0, #features]
    double start_distance = state.start_features_count - state.features().size();

    const double start_bias = 2.7;
    double scale = 36 / static_cast<double>(state.start_features_count);
    return win_distance + scale * start_bias * start_distance;
//    return start_distance;
}

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
    // assert(false);
    //region mock
//
    // stringstream cin;
    // cin << "Fred Mark Kim Anita Dwayne Nick\n"
    //       "Daniel 1 chinese\n"
    //       "Clem 1 german\n"
    //       "Dwayne 1 french\n"
    //       "Anita 1 french\n"
    //       "Spruce 1 german\n"
    //       "Fred 1 french\n"
    //       "Adan 1 chinese\n"
    //       "Sven 1 irish\n"
    //       "Nick 1 french\n"
    //       "Tim 1 irish\n"
    //       "Harley 1 english\n"
    //       "Mary 1 russian\n"
    //       "Kim 1 french\n"
    //       "Rashad 1 chinese\n"
    //       "Mark 1 french\n";

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

//    stringstream cin;
//    cin << "Tabitha Mohammad Ronaldo Jeanne Vena Eros\n"
//           "Tabitha 2 tall thin\n"
//           "Rolf 2 blue-eyed glasses\n"
//           "Mohammad 2 thin green-eyed\n"
//           "Jacob 2 blue-eyed blond\n"
//           "Derick 2 glasses red-haired\n"
//           "Meta 2 chubby freckled\n"
//           "Ronaldo 2 tall thin\n"
//           "Melville 2 blue-eyed chubby\n"
//           "Hermon 2 tattooed blond\n"
//           "Tempest 2 chubby freckled\n"
//           "Jeanne 2 thin tall\n"
//           "Kourtney 2 blond tattooed\n"
//           "Dallas 2 glasses freckled\n"
//           "Vena 2 green-eyed thin\n"
//           "Eros 2 thin brown-haired\n";
    //endregion
    Timer timer;
    timer.start();
    //region input


    string enemy1, enemy2, enemy3, enemy4, enemy5, enemy6;
    cin >> enemy1 >> enemy2 >> enemy3 >> enemy4 >> enemy5 >> enemy6; cin.ignore();
    unordered_set<string> spies ({ enemy1, enemy2, enemy3, enemy4, enemy5, enemy6 });    // temp for input

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
    //endregion
    timer.stopAndPrintResult("parsed input");

    // TODO: use doubly-linked tree of Moves instead?
    // and delete states on the fly, after we've added adjacent
    // { state : { , prev state } }
    CameFromMap came_from;
    came_from.reserve(features.size() * 8);
    const State* winning_state = nullptr;

    aStar(came_from, new State(std::move(features), 9, 6, features.size()), winning_state);

    timer.start();
    constructMovePathAndPrintAnswer(winning_state, came_from, feature_names);
    timer.stopAndPrintResult("answer");
    //TODO: delete states - iter over came_from;
}

void constructMovePathAndPrintAnswer(
    const State* winning_state,
    const CameFromMap& came_from,
    const vector<string>& feature_names) {
    // get moves sequence
    stack<Command> commands;
    const State* curr_state = winning_state;
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

void aStar(CameFromMap& came_from, const State* start_state, const State*& winning_state) {
    Timer timer;
    timer.start();

    came_from.insert(make_pair(start_state,
                               make_pair(nullptr, Command{ -1, false })));

//    vector<const State*> container;
//    container.reserve(start_state->features().size() * 8);
    priority_queue<const State*, vector<const State*>, HeuristicPtrComparator> states;
    states.push(start_state);

    PossibleMovesCalculator movesCalculator;

    int states_additions = 1;
    int states_poppings = 0;

    while (!states.empty()) {
        auto* curr_state = states.top();
        states.pop();
        states_poppings++;

        if (curr_state->onlySpiesLeft() || curr_state->onlyInnocentsLeft()) {
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
}

