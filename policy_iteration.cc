#include <iostream>
#include <vector>
#include <ctime>
#include <map>
#include <iomanip>

using namespace std;

void print_policy(vector<vector<string> > &policy);
void policy_eval(vector<vector<string> > &policy, vector<vector<double> > &values,
                    double &noise, double &gamma, double &threshold, int &iteration);

// randomly initailize policy
void initialize_policy(vector <vector<string> > &policy) {
    string policies[4] = {"Up", "Down", "Left", "Right"};
    srand(time(0));
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            policy[i][j] = policies[rand() % 4];
            cout << policy[i][j] << string(7 - policy[i][j].length(), ' ');
        }
        cout << endl;
    }
}

// update the index of possible destination in the next move if wall is encountered
void update(int &i, int &j, vector<pair<pair <int, int>, string> > &neighbours) {
    // can't move up
    if (i == 0 || (i == 2 && j == 1)) {
        neighbours[2].first.first += 1;
    }

    // cant' move down
    if (i == 2 || (i == 0 && j == 1)) {
        neighbours[3].first.first -= 1;
    }

    // can't move left
    if (j == 0 || (i == 1 && j == 2)) {
        neighbours[0].first.second += 1;
    }

    // can't move right
    if ((i == 1 && j == 0) || (i == 2 && j == 3)) {
        neighbours[1].first.second -= 1;
    }
}

// check if dealth (change in value) is less than the threshold for all grids
bool converged(vector< vector<double> > &delta, double &threshold) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            if (abs(delta[i][j]) > threshold) {
                return false;
            }
        }
    }

    return true;
}

// get the value of a policy
double get_value(int &i, int &j, string &policy, vector<vector<double> > &values, 
                    double &noise, double &gamma) {
    vector <pair<pair <int, int>, string> > neighbours;
    neighbours.push_back(make_pair(make_pair(i, j - 1), "Left")); //left
    neighbours.push_back(make_pair(make_pair(i, j + 1), "Right")); //right
    neighbours.push_back(make_pair(make_pair(i - 1, j), "Up")); //up
    neighbours.push_back(make_pair(make_pair(i + 1, j), "Down")); //down
    
    update(i, j, neighbours);

    map <string, int> idx;
    idx["Up"] = 2;
    idx["Down"] = 3;
    idx["Left"] = 0;
    idx["Right"] = 1;
    map<int, int> opposites;
    opposites[0] = 1;
    opposites[1] = 0;
    opposites[2] = 3;
    opposites[3] = 2;

    int k = idx[policy];
    double val = (1.0 - noise) * values[neighbours[k].first.first][neighbours[k].first.second];
    for (int j = 0; j < 4; j++) {
        if (k != j && j != opposites[k]) {
            val += noise / 2.0 * values[neighbours[k].first.first][neighbours[j].first.second];
        }
    }

    return val * gamma;
}

// return the max of a, b
double max(double &a, double &b) {
    if (a < b) {
        return b;
    }
    return a;
}

// return the arg max of the max value
string arg_max(int &i, int &j, vector<vector<double> > &values){
    vector <pair<pair <int, int>, string> > neighbours;
    neighbours.push_back(make_pair(make_pair(i, j - 1), "Left")); //left
    neighbours.push_back(make_pair(make_pair(i, j + 1), "Right")); //right
    neighbours.push_back(make_pair(make_pair(i - 1, j), "Up")); //up
    neighbours.push_back(make_pair(make_pair(i + 1, j), "Down")); //down

    int max_i = -1; 
    int max_j = -1;
    double max_val = -10;
    update(i, j, neighbours);

    // find the index of the neighbour with the highest expected value
    string policy = "";
    for (int k = 0; k < 4; k++) {
        int r = neighbours[k].first.first;
        int c =  neighbours[k].first.second;
        if ((r != i || c != j) && max_val < values[r][c]) {
            max_val = values[r][c];
            policy = neighbours[k].second;
        }
    }

    if ((i == 0 && j == 3) || (i == 1 && j == 3) || (i == 1 && j == 1)) {
		policy= "N/A";
	}

    return policy;
}

// check if poclies are stable
bool policy_stable(vector<vector<string> > &policy, vector<vector<string> > &policy_old) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            if (policy[i][j] != policy_old[i][j]) {
                return false;
            }
        }
    }

    return true;
}

// improve the policy
void policy_improve(vector<vector<string> > &policy, vector<vector<double> > &values,
                    double &noise, double &gamma, double &threshold, int &iteration) {
    vector<vector<string> > policy_old;
    policy_old = policy;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            policy[i][j] = arg_max(i, j, values);
        }
    }

    if (policy_stable(policy, policy_old)) {
        print_policy(policy);
    } else {
        policy_eval(policy, values, noise, gamma, threshold, iteration);
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

// print the board with values on each grid
void print_values(vector<vector<double> > &values) {
    cout << string(53, '-') << endl;
	for (int i = 0; i < 3; i++) {
        cout << "| ";
		for (int j = 0; j < 4; j++) {
		    cout << "  " << fixed << setprecision(6) << values[i][j] << string(10 - to_string(values[i][j]).length(), ' ') << "|";	
		}
		cout << endl;
        cout << string(53, '-') << endl;
	}
}

// evaluate policy
void policy_eval(vector<vector<string> > &policy, vector<vector<double> > &values,
                    double &noise, double &gamma, double &threshold, int &iteration) {
    int counter = 0;
    while (true) {
        vector< vector<double> > delta(3, vector<double>(4, 0));
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 4; j++) {
                if (!((i == 1 && j == 1) || (i == 0 && j == 3) || (i == 1 && j == 3))) {
                    double v_old = values[i][j];
                    values[i][j] = get_value(i, j, policy[i][j], values, noise, gamma);
                    delta[i][j] = max(delta[i][j], abs(v_old - values[i][j]));  
                }
            }
        }

        cout << "iteration " << iteration << endl;
        print_values(values);
        cout << endl;
        if (converged(delta, threshold)) {
            cout << "policy eval converged in " << counter << endl;

            break;
        }
        if (counter == 20) {
            break;
        }
        iteration++;
        counter++;
    }

    policy_improve(policy, values, noise, gamma, threshold, iteration);
}

int main() {
    vector< vector<double> > values(5, vector<double>(6, 0));
    vector< vector<vector<double> > > transitions(3, vector<vector<double> >(4, vector<double>(4, 0.25)));
    vector< vector<string> > policy(3, vector<string>(4, ""));

    initialize_policy(policy);

    double gamma = 0.9;
    double noise = 0.2;
    double threshold = 0.0001;

    values[0][3] = 1;
    values[1][3] = -1;
    int iteration = 0;

    // policy evaluation
    policy_eval(policy, values, noise, gamma, threshold, iteration);
}
