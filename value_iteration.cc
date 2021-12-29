#include <iostream>
#include <vector>
#include <iomanip>
#include <map>
#include <string>

using namespace std;

// update the index of possible destination in the next move if wall is encountered
void update(int &i, int &j, vector<pair <int, int> > &neighbours) {
    // can't move up
    if (i == 0 || (i == 2 && j == 1)) {
        neighbours[2].first += 1;
    }

    // cant' move down
    if (i == 2 || (i == 0 && j == 1)) {
        neighbours[3].first -= 1;
    }

    // can't move left
    if (j == 0 || (i == 1 && j == 2)) {
        neighbours[0].second += 1;
    }

    // can't move right
    if ((i == 1 && j == 0) || (i == 2 && j == 3)) {
        neighbours[1].second -= 1;
    }
}

// returns the maximum expected future awards from all possible actions at the current state
double get_val(int &i, int &j, vector<vector<double> > &values, double &gamma, double &noise) {
    vector<pair <int, int> > neighbours;
    neighbours.push_back(make_pair(i, j - 1)); //left
    neighbours.push_back(make_pair(i, j + 1)); //right
    neighbours.push_back(make_pair(i - 1, j)); //up
    neighbours.push_back(make_pair(i + 1, j)); //down
    
    update(i, j, neighbours);

    vector<double> val;

    // maps the index in neighbours of a direction to the index of its opposite direction
    // i.e. 0 is the index in neighbours of the action to the left, than it maps to 1, 
    //      which is the index in neighbours of the action to the right.
    map<int, int> opposites;
    opposites[0] = 1;
    opposites[1] = 0;
    opposites[2] = 3;
    opposites[3] = 2;

    double max_val = -10;

    // loop through all possible actions and the find the maximum expected future value
    for (int i = 0; i < 4; i++) {
        double val = (1.0 - noise) * values[neighbours[i].first][neighbours[i].second];
        for (int j = 0; j < 4; j++) {
            if (i != j && j != opposites[i]) {
                val += noise / 2.0 * values[neighbours[j].first][neighbours[j].second];
            }
        }
        val *= gamma;
        if (max_val < val) {
            max_val = val;
        }
    }

    return max_val;                        

}

// return true is each entry of val_copy is within delta of the corresponding entry in values
//      or return false otherwise
bool converged(vector< vector<double> > &val_copy, vector< vector<double> > &values, double &delta) {
    for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 4; j++) {
			if (abs(val_copy[i][j] - values[i][j]) > delta) {
				return false;
			}
		}
	}
	return true;
}

// print the board with the value of each state
void print_grid(vector< vector<double> > &grid) {
    cout << string(53, '-') << endl;
	for (int i = 0; i < 3; i++) {
        cout << "| ";
		for (int j = 0; j < 4; j++) {
		    cout << "  " << fixed << setprecision(6) << grid[i][j] << string(10 - to_string(grid[i][j]).length(), ' ') << "|";	
		}
		cout << endl;
        cout << string(53, '-') << endl;
	}
}

// print the board with policies on each grid
void print_policy(vector<vector<string> > &policy) {
    cout << "Policies: " << endl;
    cout << string(53, '-') << endl;
	for (int i = 0; i < 3; i++) {
        cout << "| ";
		for (int j = 0; j < 4; j++) {
		    cout << "  " << policy[i][j] << string(10 - policy[i][j].length(), ' ') << "|";	
		}
		cout << endl;
        cout << string(53, '-') << endl;
	}
}

// store the optimal policy for each state by getting the arg max of values
void get_policy(vector< vector<double> > &values, vector< vector<string> > &policy) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            vector <pair <int, int> > neighbours;
            neighbours.push_back(make_pair(i, j - 1)); //left
            neighbours.push_back(make_pair(i, j + 1)); //right
            neighbours.push_back(make_pair(i - 1, j)); //up
            neighbours.push_back(make_pair(i + 1, j)); //down

            int max_i = -1; 
            int max_j = -1;
            double max_val = -10;
            update(i, j, neighbours);

            // find the index of the neighbour with the highest expected value
            for (int k = 0; k < 4; k++) {
                int r = neighbours[k].first;
                int c =  neighbours[k].second;
                if ((r != i || c != j) && max_val < values[r][c]) {
                    max_val = values[r][c];
                    max_i = r;
                    max_j = c;
                }
            }

            // extract policy 
            if (max_i == i - 1) {
				policy[i][j] = "Up";
			} else if (max_i == i + 1) {
				policy[i][j] = "Down";
			} else if (max_j == j + 1) {
				policy[i][j] = "Right";
			} else {
				policy[i][j] = "Left";
			}
			if ((i == 0 && j == 3) || (i == 1 && j == 3) || (i == 1 && j == 1)) {
				policy[i][j] = "N/A";
			}
        }
    }
}

// iteratively improve the value of each state 
void improve_state(vector< vector<double> > &values) {
    // define constants
    double gamma = 0.9;
    double noise = 0.2;
    double threshold = 0.00001; // used for convergence

    int counter = 0;
    // loop until convergence
    while (true) {
        vector <vector<double> > val_copy;
        val_copy = values;

        // update the value of each state
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 4; j++) {
                // if i, j is not a terminal state or a state that cannot be entered
                if (!((i == 1 && j == 1) || (i == 0 && j == 3) || (i == 1 && j == 3))) {
                    values[i][j] = get_val(i, j, values, gamma, noise);
                }
            }
        }
        cout << "iteration " << counter << ":" << endl;
		print_grid(values);
        cout << endl;

        if (converged(val_copy, values, threshold)) {
            cout << "Converged" <<endl;
			break;
		}
        counter++;
    }
}

int main() {
    vector< vector<double> > values(3, vector<double>(4, 0));
    vector< vector<string> > policy(3, vector<string>(4, ""));
    // intializes two terminal states
    values[0][3] = 1;
    values[1][3] = -1;

    // iteratively improve the value for each state
    improve_state(values);

    // get the optimal policy for each state
    get_policy(values, policy);

    // print policy
    print_policy(policy);
}