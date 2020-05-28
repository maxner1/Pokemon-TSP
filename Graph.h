// Project Identifier: 5949F553E20B650AB0FB2266D3C0822B13D248B0
//
//  Graph.h
//  p4
//
//  Created by Matthew Axner on 12/3/19.
//  Copyright © 2019 Matthew Axner. All rights reserved.
//

#ifndef Graph_h
#define Graph_h


#endif /* Graph_h */

#include <list>
#include <queue>

using namespace std;

class Graph {
private:
    vector<vector<double> > edges;

public:
    Graph() {
        edges = vector<vector<double> >();
    }
    
    void resize(int num);
    void addEdge(int u, int v, double w);
    double getWeight(int u, int v);
};

void Graph::resize(int num) {
    edges = vector<vector<double> >((unsigned)num,
                                    vector<double>((unsigned)num, INFINITY));
}

void Graph::addEdge(int u, int v, double w) {
    edges.at((unsigned)u).at((unsigned)v) = w;
    edges.at((unsigned)v).at((unsigned)u) = w;
}

double Graph::getWeight(int u, int v) {
    return edges.at((unsigned)u).at((unsigned)v);
}
