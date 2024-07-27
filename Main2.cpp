#include <iostream>
#include <vector>
#include <unordered_map>
#include <utility>
#include <algorithm>
#include <random>
#include <climits>
#include <tuple>
#include <queue>

using namespace std;

//Job structure
struct Job {
    int id;
    int executionTime;
    vector<int> dependencies;
};

// hash function for pairs
struct pair_hash {
    template <class T1, class T2>
    size_t operator()(const pair<T1, T2>& p) const {
        auto hash1 = hash<T1>{}(p.first);
        auto hash2 = hash<T2>{}(p.second);
        return hash1 ^ hash2;
    }
};

//topological sort
vector<int> topologicalSort(const vector<Job>& jobs) {
    int numJobs = jobs.size();
    vector<int> inDegree(numJobs, 0);
    vector<vector<int>> adjList(numJobs);

    // build adjacency list and in-degree count
    for (const auto& job : jobs) {
        for (int dep : job.dependencies) {
            adjList[dep].push_back(job.id);
            inDegree[job.id]++;
        }
    }

    // topological sort using Kahn's algorithm
    queue<int> q;
    for (int i = 0; i < numJobs; ++i) {
        if (inDegree[i] == 0) {
            q.push(i);
        }
    }

    vector<int> topoOrder;
    while (!q.empty()) {
        int node = q.front();
        q.pop();
        topoOrder.push_back(node);
        for (int neighbor : adjList[node]) {
            if (--inDegree[neighbor] == 0) {
                q.push(neighbor);
            }
        }
    }

    return topoOrder;
}

// function to calculate fitness of a schedule
 int calculateFitness(const vector<pair<int, int>>& schedule, const vector<Job>& jobs, const unordered_map<pair<int, int>, int, pair_hash>& communicationTime, int K) {
   int numJobs = jobs.size();
   vector <int> machineEndTime(K, 0); // end time of each machine
    vector <int> jobEndTime(numJobs, 0); // end time of each job

    // calculate end times
for (const auto& entry : schedule) {
        int jobId = entry.first;
        int machineId = entry.second;
        int startTime = machineEndTime[machineId];

        for (int dep : jobs[jobId].dependencies) {
            int depMachineId = -1;
            for (const auto& e : schedule) {
                if (e.first == dep) {
                    depMachineId = e.second;
                    break;
                }
            }
            if (depMachineId != machineId) {
                startTime = max(startTime, jobEndTime[dep] + communicationTime.at({dep, jobId}));
            } else {
                startTime = max(startTime, jobEndTime[dep]);
            }
        }

        jobEndTime[jobId] = startTime + jobs[jobId].executionTime;
        machineEndTime[machineId] = jobEndTime[jobId];
    }

    return *max_element(machineEndTime.begin(), machineEndTime.end());
}

// genetic algorithm
void geneticAlgorithm(const vector<Job>& jobs, const unordered_map<pair<int, int>, int, pair_hash>& communicationTime, int K, int populationSize, int generations) {
    int numJobs = jobs.size();
    vector<vector<pair<int, int>>> population(populationSize, vector<pair<int, int>>(numJobs));
    random_device rd;
    mt19937 g(rd());

    // topological order
    vector<int> topoOrder = topologicalSort(jobs);

    // initialize population
    for (auto& schedule : population) {
        for (int i = 0; i < numJobs; ++i) {
            schedule[i] = {topoOrder[i], rand() % K};
        }
    }

    vector<tuple<int, vector<pair<int, int>>>> fitnessPopulation;
    for (int generation = 0; generation < generations; ++generation) {
        // calculate fitness for each schedule
        fitnessPopulation.clear();
        for (const auto& schedule : population) {
            int fitness = calculateFitness(schedule, jobs, communicationTime, K);
            fitnessPopulation.push_back({fitness, schedule});
        }

        // sort them
        sort(fitnessPopulation.begin(), fitnessPopulation.end());

        cout << "In Generation " << generation+1 << ", The Best Completion Time is " << get<0>(fitnessPopulation[0]) << endl;

        // selection part
        population.clear();
        for (int i = 0; i < populationSize / 2; ++i) {
            population.push_back(get<1>(fitnessPopulation[i]));
        }

 // crossover and mutation part
while (static_cast<int>(population.size()) < populationSize) {
            int parent1Idx = rand() % (populationSize / 2);
            int parent2Idx = rand() % (populationSize / 2);
            vector<pair<int, int>> child = population[parent1Idx];

            // Crossover
            int crossoverPoint = rand() % numJobs;
            for (int j = crossoverPoint; j < numJobs; ++j) {
                child[j] = population[parent2Idx][j];
            }

            // Mutation
            if (rand() % 100 < 10) { // 10% mutation rate
                int pos1 = rand() % numJobs;
                int newMachine = rand() % K;
                child[pos1].second = newMachine;
            }

            population.push_back(child);
        }
    }

    // output the best schedule
    vector<pair<int, int>> bestSchedule = get<1>(fitnessPopulation[0]);
    cout << "Final Best Completion Time is " << get<0>(fitnessPopulation[0]) << endl;
    cout << "Best Schedule That We Have Found:" << endl;
    for (int machineId = 0; machineId < K; ++machineId) {
        cout << "Machine " << machineId+1 << ": ";
        for (const auto& entry : bestSchedule) {
            if (entry.second == machineId) {
                cout << "Job" << entry.first << " ";
            }
        }
        cout << endl;
    }
}

int main() {
    srand(time(NULL));
    int K = 3;    // number of machines
    int populationSize = 80000; //total poplulation
    int generations = 15;// number of generations
//jobs vector which repersent={jobs id/ execution time /their dependencies}
    vector<Job> jobs = {
    {0, 8, {}},
    {1, 5, {0}},
    {2, 7, {0, 1}},
    {3, 6, {1}},
    {4, 9, {2, 3}},
    {5, 4, {2, 4}},
    {6, 3, {4, 5}},
    {7, 8, {3, 6}},
    {8, 5, {6, 7}},
    {9, 6, {7, 8}},
    {10, 4, {8}},
    {11, 7, {9}},
    {12, 8, {10}},
    {13, 5, {11, 12}},
    {14, 6, {13}},
    {15, 3, {14}},
    {16, 9, {15}},
    {17, 4, {16}},
    {18, 8, {17}},
    {19, 5, {18}},
};

// communication times between machines for jobs which dependent to each other
unordered_map<pair<int, int>, int, pair_hash> communicationTime = {
    {make_pair(0, 1), 2}, {make_pair(0, 2), 3}, {make_pair(0, 3), 4},
    {make_pair(0, 4), 5}, {make_pair(0, 5), 6}, {make_pair(0, 6), 7},
    {make_pair(0, 7), 8}, {make_pair(0, 8), 9}, {make_pair(0, 9), 10},
    {make_pair(0, 10), 11}, {make_pair(0, 11), 12}, {make_pair(0, 12), 13},
    {make_pair(0, 13), 14}, {make_pair(0, 14), 15}, {make_pair(0, 15), 16},
    {make_pair(0, 16), 17}, {make_pair(0, 17), 18}, {make_pair(0, 18), 19},
    {make_pair(0, 19), 20}, {make_pair(1, 0), 2}, {make_pair(1, 2), 3},
    {make_pair(1, 3), 4}, {make_pair(1, 4), 5}, {make_pair(1, 5), 6},
    {make_pair(1, 6), 7}, {make_pair(1, 7), 8}, {make_pair(1, 8), 9},
    {make_pair(1, 9), 10}, {make_pair(1, 10), 11}, {make_pair(1, 11), 12},
    {make_pair(1, 12), 13}, {make_pair(1, 13), 14}, {make_pair(1, 14), 15},
    {make_pair(1, 15), 16}, {make_pair(1, 16), 17}, {make_pair(1, 17), 18},
    {make_pair(1, 18), 19}, {make_pair(1, 19), 20}, {make_pair(2, 0), 3},
    {make_pair(2, 1), 3}, {make_pair(2, 3), 4}, {make_pair(2, 4), 5},
    {make_pair(2, 5), 6}, {make_pair(2, 6), 7}, {make_pair(2, 7), 8},
    {make_pair(2, 8), 9}, {make_pair(2, 9), 10}, {make_pair(2, 10), 11},
    {make_pair(2, 11), 12}, {make_pair(2, 12), 13}, {make_pair(2, 13), 14},
    {make_pair(2, 14), 15}, {make_pair(2, 15), 16}, {make_pair(2, 16), 17},
    {make_pair(2, 17), 18}, {make_pair(2, 18), 19}, {make_pair(2, 19), 20},
    {make_pair(3, 0), 4}, {make_pair(3, 1), 4}, {make_pair(3, 2), 4},
    {make_pair(3, 4), 5}, {make_pair(3, 5), 6}, {make_pair(3, 6), 7},
    {make_pair(3, 7), 8}, {make_pair(3, 8), 9}, {make_pair(3, 9), 10},
    {make_pair(3, 10), 11}, {make_pair(3, 11), 12}, {make_pair(3, 12), 13},
    {make_pair(3, 13), 14}, {make_pair(3, 14), 15}, {make_pair(3, 15), 16},
    {make_pair(3, 16), 17}, {make_pair(3, 17), 18}, {make_pair(3, 18), 19},
    {make_pair(3, 19), 20}, {make_pair(4, 0), 5}, {make_pair(4, 1), 5},
    {make_pair(4, 2), 5}, {make_pair(4, 3), 5}, {make_pair(4, 5), 6},
    {make_pair(4, 6), 7}, {make_pair(4, 7), 8}, {make_pair(4, 8), 9},
    {make_pair(4, 9), 10}, {make_pair(4, 10), 11}, {make_pair(4, 11), 12},
    {make_pair(4, 12), 13}, {make_pair(4, 13), 14}, {make_pair(4, 14), 15},
    {make_pair(4, 15), 16}, {make_pair(4, 16), 17}, {make_pair(4, 17), 18},
    {make_pair(4, 18), 19}, {make_pair(4, 19), 20}, {make_pair(5, 0), 6},
    {make_pair(5, 1), 6}, {make_pair(5, 2), 6}, {make_pair(5, 3), 6},
    {make_pair(5, 4), 6}, {make_pair(5, 6), 7}, {make_pair(5, 7), 8},
    {make_pair(5, 8), 9}, {make_pair(5, 9), 10}, {make_pair(5, 10), 11},
    {make_pair(5, 11), 12}, {make_pair(5, 12), 13}, {make_pair(5, 13), 14},
    {make_pair(5, 14), 15}, {make_pair(5, 15), 16}, {make_pair(5, 16), 17},
    {make_pair(5, 17), 18}, {make_pair(5, 18), 19}, {make_pair(5, 19), 20},
    {make_pair(6, 0), 7}, {make_pair(6, 1), 7}, {make_pair(6, 2), 7},
    {make_pair(6, 3), 7}, {make_pair(6, 4), 7}, {make_pair(6, 5), 7},
    {make_pair(6, 7), 8}, {make_pair(6, 8), 9}, {make_pair(6, 9), 10},
    {make_pair(6, 10), 11}, {make_pair(6, 11), 12}, {make_pair(6, 12), 13},
    {make_pair(6, 13), 14}, {make_pair(6, 14), 15}, {make_pair(6, 15), 16},
    {make_pair(6, 16), 17}, {make_pair(6, 17), 18}, {make_pair(6, 18), 19},
    {make_pair(6, 19), 20}, {make_pair(7, 0), 8}, {make_pair(7, 1), 8},
    {make_pair(7, 2), 8}, {make_pair(7, 3), 8}, {make_pair(7, 4), 8},
    {make_pair(7, 5), 8}, {make_pair(7, 6), 8}, {make_pair(7, 8), 9},
    {make_pair(7, 9), 10}, {make_pair(7, 10), 11}, {make_pair(7, 11), 12},
    {make_pair(7, 12), 13}, {make_pair(7, 13), 14}, {make_pair(7, 14), 15},
    {make_pair(7, 15), 16}, {make_pair(7, 16), 17}, {make_pair(7, 17), 18},
    {make_pair(7, 18), 19}, {make_pair(7, 19), 20}, {make_pair(8, 0), 9},
    {make_pair(8, 1), 9}, {make_pair(8, 2), 9}, {make_pair(8, 3), 9},
    {make_pair(8, 4), 9}, {make_pair(8, 5), 9}, {make_pair(8, 6), 9},
    {make_pair(8, 7), 9}, {make_pair(8, 9), 10}, {make_pair(8, 10), 11},
    {make_pair(8, 11), 12}, {make_pair(8, 12), 13}, {make_pair(8, 13), 14},
    {make_pair(8, 14), 15}, {make_pair(8, 15), 16}, {make_pair(8, 16), 17},
    {make_pair(8, 17), 18}, {make_pair(8, 18), 19}, {make_pair(8, 19), 20},
    {make_pair(9, 0), 10}, {make_pair(9, 1), 10}, {make_pair(9, 2), 10},
    {make_pair(9, 3), 10}, {make_pair(9, 4), 10}, {make_pair(9, 5), 10},
    {make_pair(9, 6), 10}, {make_pair(9, 7), 10}, {make_pair(9, 8), 10},
    {make_pair(9, 10), 11}, {make_pair(9, 11), 12}, {make_pair(9, 12), 13},
    {make_pair(9, 13), 14}, {make_pair(9, 14), 15}, {make_pair(9, 15), 16},
    {make_pair(9, 16), 17}, {make_pair(9, 17), 18}, {make_pair(9, 18), 19},
    {make_pair(9, 19), 20}, {make_pair(10, 0), 11}, {make_pair(10, 1), 11},
    {make_pair(10, 2), 11}, {make_pair(10, 3), 11}, {make_pair(10, 4), 11},
    {make_pair(10, 5), 11}, {make_pair(10, 6), 11}, {make_pair(10, 7), 11},
    {make_pair(10, 8), 11}, {make_pair(10, 9), 11}, {make_pair(10, 11), 12},
    {make_pair(10, 12), 13}, {make_pair(10, 13), 14}, {make_pair(10, 14), 15},
    {make_pair(10, 15), 16}, {make_pair(10, 16), 17}, {make_pair(10, 17), 18},
    {make_pair(10, 18), 19}, {make_pair(10, 19), 20}, {make_pair(11, 0), 12},
    {make_pair(11, 1), 12}, {make_pair(11, 2), 12}, {make_pair(11, 3), 12},
    {make_pair(11, 4), 12}, {make_pair(11, 5), 12}, {make_pair(11, 6), 12},
    {make_pair(11, 7), 12}, {make_pair(11, 8), 12}, {make_pair(11, 9), 12},
    {make_pair(11, 10), 12}, {make_pair(11, 12), 13}, {make_pair(11, 13), 14},
    {make_pair(11, 14), 15}, {make_pair(11, 15), 16}, {make_pair(11, 16), 17},
    {make_pair(11, 17), 18}, {make_pair(11, 18), 19}, {make_pair(11, 19), 20},
    {make_pair(12, 0), 13}, {make_pair(12, 1), 13}, {make_pair(12, 2), 13},
    {make_pair(12, 3), 13}, {make_pair(12, 4), 13}, {make_pair(12, 5), 13},
    {make_pair(12, 6), 13}, {make_pair(12, 7), 13}, {make_pair(12, 8), 13},
    {make_pair(12, 9), 13}, {make_pair(12, 10), 13}, {make_pair(12, 11), 13},
    {make_pair(12, 13), 14}, {make_pair(12, 14), 15}, {make_pair(12, 15), 16},
    {make_pair(12, 16), 17}, {make_pair(12, 17), 18}, {make_pair(12, 18), 19},
    {make_pair(12, 19), 20}, {make_pair(13, 0), 14}, {make_pair(13, 1), 14},
    {make_pair(13, 2), 14}, {make_pair(13, 3), 14}, {make_pair(13, 4), 14}, 
    {make_pair(13, 5), 14}, {make_pair(13, 6), 14},
    {make_pair(13, 7), 14}, {make_pair(13, 8), 14}, {make_pair(13, 9), 14},
    {make_pair(13, 10), 14}, {make_pair(13, 11), 14}, {make_pair(13, 12), 14},
    {make_pair(13, 14), 15}, {make_pair(13, 15), 16}, {make_pair(13, 16), 17},
    {make_pair(13, 17), 18}, {make_pair(13, 18), 19}, {make_pair(13, 19), 20},
    {make_pair(14, 0), 15}, {make_pair(14, 1), 15}, {make_pair(14, 2), 15},
    {make_pair(14, 3), 15}, {make_pair(14, 4), 15}, {make_pair(14, 5), 15},
    {make_pair(14, 6), 15}, {make_pair(14, 7), 15}, {make_pair(14, 8), 15},
    {make_pair(14, 9), 15}, {make_pair(14, 10), 15}, {make_pair(14, 11), 15},
    {make_pair(14, 12), 15}, {make_pair(14, 13), 15}, {make_pair(14, 15), 16},
    {make_pair(14, 16), 17}, {make_pair(14, 17), 18}, {make_pair(14, 18), 19},
    {make_pair(14, 19), 20}, {make_pair(15, 0), 16}, {make_pair(15, 1), 16},
    {make_pair(15, 2), 16}, {make_pair(15, 3), 16}, {make_pair(15, 4), 16},
    {make_pair(15, 5), 16}, {make_pair(15, 6), 16}, {make_pair(15, 7), 16},
    {make_pair(15, 8), 16}, {make_pair(15, 9), 16}, {make_pair(15, 10), 16},
    {make_pair(15, 11), 16}, {make_pair(15, 12), 16}, {make_pair(15, 13), 16},
    {make_pair(15, 14), 16}, {make_pair(15, 16), 17}, {make_pair(15, 17), 18},
    {make_pair(15, 18), 19}, {make_pair(15, 19), 20}, {make_pair(16, 0), 17},
    {make_pair(16, 1), 17}, {make_pair(16, 2), 17}, {make_pair(16, 3), 17},
    {make_pair(16, 4), 17}, {make_pair(16, 5), 17}, {make_pair(16, 6), 17},
    {make_pair(16, 7), 17}, {make_pair(16, 8), 17}, {make_pair(16, 9), 17},
    {make_pair(16, 10), 17}, {make_pair(16, 11), 17}, {make_pair(16, 12), 17},
    {make_pair(16, 13), 17}, {make_pair(16, 14), 17}, {make_pair(16, 15), 17},
    {make_pair(16, 17), 18}, {make_pair(16, 18), 19}, {make_pair(16, 19), 20},
    {make_pair(17, 0), 18}, {make_pair(17, 1), 18}, {make_pair(17, 2), 18},
    {make_pair(17, 3), 18}, {make_pair(17, 4), 18}, {make_pair(17, 5), 18},
    {make_pair(17, 6), 18}, {make_pair(17, 7), 18}, {make_pair(17, 8), 18},
    {make_pair(17, 9), 18}, {make_pair(17, 10), 18}, {make_pair(17, 11), 18},
    {make_pair(17, 12), 18}, {make_pair(17, 13), 18}, {make_pair(17, 14), 18},
    {make_pair(17, 15), 18}, {make_pair(17, 16), 18}, {make_pair(17, 18), 19},
    {make_pair(17, 19), 20}, {make_pair(18, 0), 19}, {make_pair(18, 1), 19},
    {make_pair(18, 2), 19}, {make_pair(18, 3), 19}, {make_pair(18, 4), 19},
    {make_pair(18, 5), 19}, {make_pair(18, 6), 19}, {make_pair(18, 7), 19},
    {make_pair(18, 8), 19}, {make_pair(18, 9), 19}, {make_pair(18, 10), 19},
    {make_pair(18, 11), 19}, {make_pair(18, 12), 19}, {make_pair(18, 13), 19},
    {make_pair(18, 14), 19}, {make_pair(18, 15), 19}, {make_pair(18, 16), 19},
    {make_pair(18, 17), 19}, {make_pair(18, 19), 20}, {make_pair(19, 0), 20},
    {make_pair(19, 1), 20}, {make_pair(19, 2), 20}, {make_pair(19, 3), 20},
    {make_pair(19, 4), 20}, {make_pair(19, 5), 20}, {make_pair(19, 6), 20},
    {make_pair(19, 7), 20}, {make_pair(19, 8), 20}, {make_pair(19, 9), 20},
    {make_pair(19, 10), 20}, {make_pair(19, 11), 20}, {make_pair(19, 12), 20},
    {make_pair(19, 13), 20}, {make_pair(19, 14), 20}, {make_pair(19, 15), 20},
    {make_pair(19, 16), 20}, {make_pair(19, 17), 20}, {make_pair(19, 18), 20}
};



    geneticAlgorithm(jobs, communicationTime, K, populationSize, generations);

    return 0;
}
