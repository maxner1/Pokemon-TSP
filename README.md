# pokemonTSP
My final project for EECS 281: Data Structures and Algorithms. Split into 3 parts, this project explores various ways of solving the infamous Traveling Salesman Problem.

Input for all: .txt file, containing the number of "Pokemon", and that many pairs of coordinates indicating where that Pokemon can be found on the map.

MST Mode: This part of the project will output the minimum spanning tree connecting all pokemon on the map. In this part of the project, you (as a Pokemon trainer) are unable to fly, and Quadrant III (negative, negative) is treated as the "sea". Therefore you must first catch a Pokemon on the negative X or Y axes (the shoreline) before you move to the sea. Output is in the format of the total weight of the tree followed by the Pokemon by ID paired with their parent Pokemon in the tree, in order of {smaller, larger}.

FASTTSP Mode: This part of the project acts as an approximation algorithm for the TSP. It starts with a tour of just two Pokemon, and quickly finds the most efficient spot to insert each remaining Pokemon. Finally, this mode outputs the total distance of the tour followed by the {okemon in order of the tour, indicated by thier ID.

OPTTSP Mode: This part of the project solves the TSP. It will check every permutation of the tour that is "promising"-- meaning that further calculations will be canceled if the aggregate weight has already exceeded the calculated minimum weight. Output is the same format as FASTTSP mode.
