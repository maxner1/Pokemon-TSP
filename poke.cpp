// Project Identifier: 5949F553E20B650AB0FB2266D3C0822B13D248B0
//
//  poke.cpp
//  p4
//
//  Created by Matthew Axner on 11/25/19.
//  Copyright © 2019 Matthew Axner. All rights reserved.
//

#include <stdio.h>
#include <iomanip>
#include <algorithm>
#include <iostream>
#include <getopt.h>
#include <vector>
#include <cmath>

#include "xcode_redirect.hpp"
#include "Graph.h"

using namespace std;

/* ------------------------- POKEMON STRUCTS ------------------------- */

struct pokeA {
    pair<int, int> loc;
    int id;
    char type;
    bool visited;
    double distance;
    int parent;
};

struct pokeB {
    pair<int, int> loc;
};

struct pokeC {
    pair<int, int> loc;
};

/* ------------------------- FUNCTION DECLARATIONS ------------------------- */

void mode_MST(vector<pokeA> &pokemon);
void mode_FASTTSP(vector<pokeB> &pokemon);
void mode_OPTTSP(Graph &g, vector<pokeC> &pokemon);

int closestUnvisited(vector<pokeA> &pokemon);

double euclideanDistanceA(vector<pokeA>::iterator &p, int other,
                          vector<pokeA> &pokemon);
double euclideanDistanceB(int a, int b, vector<pokeB> &pokemon);
double euclideanDistanceC(int a, int b, vector<pokeC> &pokemon);

double getArbTSP(vector<int> &tour, vector<pokeC> &pokemon);
void genPerms(vector<int> &tour, int permLength, double &bestDist, Graph &g,
              vector<int> &finalTour, double currDist, vector<pokeC> &pokemon);
bool promising(int permLength, double &bestDist, double currDist,
               vector<pokeC> &pokemon);

void printMST(vector<int> &tree, vector<pokeA> &pokemon);
void printFASTTSP(vector<int> &tour, vector<pokeB> &pokemon);
void printOPTTSP(vector<int> &tour, Graph &g);
void print_help();

/* ------------------------- MAIN FUNCTION ------------------------- */

int num_pokemon;

int main(int argc, char **argv) {
    
    cout << std::setprecision(2);
    cout << std::fixed;
    xcode_redirect(argc, argv);
    
    /*
     Handling options...
     --mode <string> OR -m <string>
     --help OR -h
     */
    
    int option_index = 0, option = 0;
    
    char mode = ' ';
    
    opterr = false;
    
    struct option longOpts[] = {
        { "mode", required_argument, nullptr, 'm' },
        { "help", no_argument, nullptr, 'h' },
        { nullptr, 0, nullptr, '\0' }
    };
    
    while ((option = getopt_long(argc, argv, "m:h", longOpts,
                                 &option_index)) != -1) {
        switch (option) {
            case 'm':
                if (strcasecmp(optarg, "MST") == 0)
                    mode = 'A';
                else if (strcasecmp(optarg, "FASTTSP") == 0)
                    mode = 'B';
                else if (strcasecmp(optarg, "OPTTSP") == 0)
                    mode = 'C';
                else
                    cerr << "ERROR: Invalid mode entered. Try again!";
                break;
            case 'h':
                print_help();
                exit(0);
                break;
        }
    }
    
    /*
     Reading input from file...
        -int number of pokemon
        -pairs of ints representing pokemon locations
    */
    
    bool has_coastline = false, has_sea = false, has_land = false;
    vector<pokeA> pokemonA;
    vector<pokeB> pokemonB;
    vector<pokeC> pokemonC;
    Graph g;
    
    cin >> num_pokemon;
    
    if (mode == 'C')
        g.resize(num_pokemon);
    
    for (int i = 0; i < num_pokemon; ++i) {
        if (mode == 'A') {
            //assign location to this pokemon
            pokeA pok;
            cin >> pok.loc.first >> pok.loc.second;
            
            //assign correct type (land, sea, or coastline) to this pokemon
            if ((pok.loc.first == 0 && pok.loc.second <= 0) ||
                (pok.loc.first <= 0 && pok.loc.second == 0)) {
                has_coastline = true;
                pok.type = 'c';
            }
            else if (pok.loc.first > 0 || pok.loc.second > 0) {
                has_land = true;
                pok.type = 'l';
            }
            else {
                has_sea = true;
                pok.type = 's';
            }
            
            pok.visited = false;
            pok.distance = INFINITY;
            pok.id = i;
            
            pokemonA.push_back(pok);
        }
        else if (mode == 'B' || mode == 'C') {
            pokeB pok;
            cin >> pok.loc.first >> pok.loc.second;
            
            pokemonB.push_back(pok);
        }
        else {
            //assign location to this pokemon
            pokeC pok;
            cin >> pok.loc.first >> pok.loc.second;
            
            for (int j = 0; j < (signed)pokemonC.size(); ++j) {
                //cout << pokemonC[(unsigned)j].loc.first - (pok.loc.first) << "\n"
                // << pokemonC[(unsigned)j].loc.second - (pok.loc.second) << "\n";
                double weight = ((static_cast<double>(pokemonC[(unsigned)j].loc.first) -
                                  static_cast<double>(pok.loc.first)) *
                                 (static_cast<double>(pokemonC[(unsigned)j].loc.first) -
                                  static_cast<double>(pok.loc.first)) +
                                 (static_cast<double>(pokemonC[(unsigned)j].loc.second) -
                                  static_cast<double>(pok.loc.second)) *
                                 (static_cast<double>(pokemonC[(unsigned)j].loc.second) -
                                  static_cast<double>(pok.loc.second)));
                g.addEdge(j, (int)pokemonC.size(), weight);
            }
            
            pokemonC.push_back(pok);
        }
    }
    
    /*
     Algorithm time.
         -A: MST Mode
            finds MST of points whilst considering region
         -B: FASTTSP Mode
            uses TSP heuristics to determine fastest route between pokemon
         -C: OPTTSP Mode
            finds the actual fastest route between pokemon. may be a lil slow
    */
    
    if (mode == 'A') {
        if (!has_coastline && has_sea && has_land) {
            cerr << "Cannot construct MST\n";
            exit(1);
        }
        mode_MST(pokemonA);
    }
    else if (mode == 'B') {
        mode_FASTTSP(pokemonB);
    }
    else {
        mode_FASTTSP(pokemonB);
    }
    
    return 0;
} // main()
    
/* ------------------------- HELPER FUNCTIONS ------------------------- */

void mode_MST(vector<pokeA> &pokemon) {
    vector<int> tree(pokemon.size());
    auto p = pokemon.begin();
    
    p->visited = true;
    p->distance = 0;
    p->parent = 0;

    int count = 0;
    for (int i = 0; i < (signed)pokemon.size(); ++i) {
        int closest = closestUnvisited(pokemon);
        p = pokemon.begin() + closest;
        pokemon[(unsigned)closest].visited = true;
        tree[(unsigned)count++] = closest;
        
        for (int i = 0; i < num_pokemon; ++i) {
            if (!pokemon[(unsigned)i].visited) {
                double dist = euclideanDistanceA(p, i, pokemon);
                if (dist < pokemon[(unsigned)i].distance) {
                    pokemon[(unsigned)i].distance = euclideanDistanceA(p, i, pokemon);
                    pokemon[(unsigned)i].parent = p->id;
                }
            }
        }
    }
    
    printMST(tree, pokemon);
} //mode_MST()

void mode_FASTTSP(vector<pokeB> &pokemon) {
    vector<int> tour;
    tour.reserve((unsigned)num_pokemon);
    tour.push_back(0);
    tour.push_back(1);
    tour.push_back(0);
    
    int count = 2;
    double minDist = INFINITY;
    
    while (count < num_pokemon) {
        int loc = 0;
        for (int i = 0; i < (signed)tour.size() - 1; ++i) {
            int first = tour[(unsigned)i], second = tour[(unsigned)i + 1];
            
            double d1 = euclideanDistanceB(first, second, pokemon);
            double d2 = euclideanDistanceB(second, count, pokemon);
            double d3 = euclideanDistanceB(count, first, pokemon);
            
            if ((d2 + d3 - d1) < minDist) {
                minDist = d2 + d3 - d1;
                loc = i + 1;
            }
        }
        tour.insert(tour.begin() + loc, count);
        ++count;
        minDist = INFINITY;
    }
    
    printFASTTSP(tour, pokemon);
} //mode_FASTTSP()

void mode_OPTTSP(Graph &g, vector<pokeC> &pokemon) {
    vector<int> tour;
    double arbTSP = getArbTSP(tour, pokemon);
    tour.pop_back();
    vector<int> finalTour = tour;
    
    genPerms(tour, 1, arbTSP, g, finalTour, 0, pokemon);

    
    finalTour.push_back(0);
    printOPTTSP(finalTour, g);
} //mode_OPTTSP()


void genPerms(vector<int> &tour, int permLength, double &bestDist, Graph &g,
              vector<int> &finalTour, double currDist, vector<pokeC> &pokemon) {
    //END OF PERMUTATION:
    //if this perm has lower cost than best, then this is your new best
    if ((signed)tour.size() == permLength) {
        currDist += g.getWeight(tour[tour.size() - 1], 0);
        if (currDist < bestDist) {
            bestDist = currDist;
            finalTour = tour;
        }
        return;
    }
    //AT ANY POINT:
    //IF current cost > bestDist, don't continue checking
    if (!promising(permLength, bestDist, currDist, pokemon))
        return;
    //ELSE try every permutation remaining
    for (int i = permLength; i < (signed)tour.size(); ++i) {
        swap(tour[(unsigned)permLength], tour[(unsigned)i]);
        currDist += g.getWeight(tour[(unsigned)permLength],
                                tour[(unsigned)permLength - 1]);
        genPerms(tour, permLength + 1, bestDist, g, finalTour, currDist, pokemon);
        currDist -= g.getWeight(tour[(unsigned)permLength],
                                tour[(unsigned)permLength - 1]);
        swap(tour[(unsigned)permLength], tour[(unsigned)i]);
    }
} // genPerms()

bool promising(int permLength, double &bestDist, double currDist, vector<pokeC> &pokemon) {
    if (pokemon.size() - (unsigned)permLength < 3)
        return true;
    vector<pokeC> temp_pokemon(pokemon.begin() + permLength, pokemon.end());
    vector<int> temp_tour;
    double restOfDist = getArbTSP(temp_tour, temp_pokemon);
    
    if (currDist + restOfDist > bestDist)
        return false;
    return true;
} //promising()

int closestUnvisited(vector<pokeA> &pokemon) {
    double min = INFINITY;
    int index = 0;
    
    for (int i = 0; i < num_pokemon; ++i) {
        if (!pokemon[(unsigned)i].visited && pokemon[(unsigned)i].distance < min) {
            min = pokemon[(unsigned)i].distance;
            index = i;
        }
    }
    return index;
} //closestUnvisited()

double getArbTSP(vector<int> &tour, vector<pokeC> &pokemon) {
    tour.reserve(pokemon.size());
    tour.push_back(0);
    tour.push_back(1);
    tour.push_back(0);
    
    int count = 2;
    double minDist = INFINITY;
    
    while (count < (signed)pokemon.size()) {
        int loc = 0;
        for (int i = 0; i < (signed)tour.size() - 1; ++i) {
            int first = tour[(unsigned)i], second = tour[(unsigned)i + 1];
            
            double d1 = euclideanDistanceC(first, second, pokemon);
            double d2 = euclideanDistanceC(second, count, pokemon);
            double d3 = euclideanDistanceC(count, first, pokemon);
            
            if ((d2 + d3 - d1) < minDist) {
                minDist = d2 + d3 - d1;
                loc = i + 1;
            }
        }
        tour.insert(tour.begin() + loc, count);
        ++count;
        minDist = INFINITY;
    }
    
    double total_weight = 0;
    for (int i = 0; i < (signed)tour.size() - 1; ++i) {
        total_weight += euclideanDistanceC(tour[(unsigned)i],
                                                tour[(unsigned)i + 1], pokemon);
    }
    total_weight += euclideanDistanceC(tour[tour.size() - 1], tour[0], pokemon);
    
    vector<int> tour2 = tour;
    return total_weight;
} //getArbTSP()

double euclideanDistanceA(vector<pokeA>::iterator &p, int other,
                          vector<pokeA> &pokemon) {
    if ((p->type == 'l' && pokemon[(unsigned)other].type == 's') ||
        (p->type == 's' && pokemon[(unsigned)other].type == 'l'))
        return INFINITY;
    else
        return (abs(static_cast<double>(p->loc.first) -
                    static_cast<double>(pokemon[(unsigned)other].loc.first)) *
                abs(static_cast<double>(p->loc.first) -
                    static_cast<double>(pokemon[(unsigned)other].loc.first)) +
                (abs(static_cast<double>(p->loc.second) -
                     static_cast<double>(pokemon[(unsigned)other].loc.second)) *
                 abs(static_cast<double>(p->loc.second) -
                     static_cast<double>(pokemon[(unsigned)other].loc.second))));
} //euclideanDistanceA()

double euclideanDistanceB(int a, int b, vector<pokeB> &pokemon) {
    return ((static_cast<double>(pokemon[(unsigned)a].loc.first) -
            static_cast<double>(pokemon[(unsigned)b].loc.first)) *
            (static_cast<double>(pokemon[(unsigned)a].loc.first) -
            static_cast<double>(pokemon[(unsigned)b].loc.first)) +
            (static_cast<double>(pokemon[(unsigned)a].loc.second) -
             static_cast<double>(pokemon[(unsigned)b].loc.second)) *
            (static_cast<double>(pokemon[(unsigned)a].loc.second) -
             static_cast<double>(pokemon[(unsigned)b].loc.second)));
} //euclideanDistanceB()

double euclideanDistanceC(int a, int b, vector<pokeC> &pokemon) {
    return ((static_cast<double>(pokemon[(unsigned)a].loc.first) -
             static_cast<double>(pokemon[(unsigned)b].loc.first)) *
            (static_cast<double>(pokemon[(unsigned)a].loc.first) -
             static_cast<double>(pokemon[(unsigned)b].loc.first)) +
            (static_cast<double>(pokemon[(unsigned)a].loc.second) -
             static_cast<double>(pokemon[(unsigned)b].loc.second)) *
            (static_cast<double>(pokemon[(unsigned)a].loc.second) -
             static_cast<double>(pokemon[(unsigned)b].loc.second)));
} //euclideanDistanceC()

void printMST(vector<int> &tree, vector<pokeA> &pokemon) {
    double total_weight = 0;
    for (int i = 0; i < (signed)pokemon.size(); ++i) {
        total_weight += sqrt(pokemon[(unsigned)i].distance);
    }
    cout << total_weight << "\n";
    
    for (int i = 1; i < (signed)tree.size(); ++i) {
        if (pokemon[(unsigned)i].parent < i)
            cout << pokemon[(unsigned)i].parent<< " " << i << "\n";
        else
            cout << i << " " << pokemon[(unsigned)i].parent << "\n";
    }
} //printMST()

void printFASTTSP(vector<int> &tour, vector<pokeB> &pokemon) {
    double total_weight = 0;
    for (int i = 0; i < (signed)tour.size() - 1; ++i) {
        total_weight += sqrt(euclideanDistanceB(tour[(unsigned)i],
                                                tour[(unsigned)i + 1], pokemon));
    }
    total_weight += sqrt(euclideanDistanceB(tour[tour.size() - 1], tour[0], pokemon));
    
    cout << total_weight << "\n";
    for (int i = 0; i < (signed)tour.size() - 1; ++i) {
        cout << tour[(unsigned)i] << " ";
    }
    cout << "\n";
} //printFASTTSP()

void printOPTTSP(vector<int> &tour, Graph &g) {
    double total_weight = 0;
    for (int i = 1; i < (signed)tour.size(); ++i) {
        total_weight += sqrt(g.getWeight(tour[(unsigned)i - 1], tour[(unsigned)i]));
    }
    
    cout << total_weight << "\n";
    for (int i = 0; i < (signed)tour.size() - 1; ++i) {
        cout << tour[(unsigned)i] << " ";
    }
    cout << "\n";
} //printFASTTSP()
    
void print_help() {
    cout << "You've asked for help! Here are your options:\n\n"
    << "\t--mode <string> OR -m <string>: indicate the mode to be executed.\n"
    << "\tPart A: \"MST\"\n\tPart B: \"FASTTSP\"\n\tPart C: \"OPTTSP\"\n";
} // print_help()
