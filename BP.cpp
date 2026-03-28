#include <iostream>
#include <vector>
#include <utility>
#include <fstream>
#include <algorithm>
#include <numeric>  
#include <random>
#include <chrono>



using namespace std;
random_device rd;
mt19937 gen(rd());

void branchAndBound(int , int , int ,vector<int>& , vector<int>& ,vector<int>& , int , int , int& ,
                    vector<bool>& , vector<bool>& , vector<double>& );

pair<int, vector<bool>> geneticAlgorithm(vector<int>& values, vector<int>& weights, int W, int N);
int f( vector<bool>& chromosoms, vector<int>& values, vector<int>& weights, int W);
vector<bool> rand_hrom(int N);                                       
vector<vector<bool>> make_population(int N, int popSize);
vector<bool> tournamentSelection(vector<vector<bool>>& p, vector<int>& fV);
pair<vector<bool>, vector<bool>> cross(vector<bool>& p_1, vector<bool>& p_2);
void mutation(vector<bool>& chromosoms);

int main() {
    ifstream file("ks_4_0.txt");
    
    if (!file.is_open()) {
        cerr << "File isn't open!" << endl;
        return 1;
    }
    
    int N, W;  // количество предметов и вместимость рюкзака
    file >> N >> W;
    
    vector<pair<int, int>> v;  // (стоимость, вес)
    for (int i = 0; i < N; i++) {
        int pi, wi;
        file >> pi >> wi;
        v.push_back(make_pair(pi, wi));
    }
    file.close();
    
    vector<int> values(N), weights(N);
    for (int i = 0; i < N; ++i) {
        values[i] = v[i].first;
        weights[i] = v[i].second;
    }
    vector<int> order(N);
    iota(order.begin(), order.end(), 0);
    vector<double> rat(N);
    for (int i = 0; i < N; ++i) {
        rat[i] = static_cast<double>(values[i]) / weights[i];
    }
    sort(order.begin(), order.end(), [&](int a, int b) {
        return rat[a] > rat[b];
    });
    
    int bv = 0;
    vector<bool> best(N, false);
    vector<bool> current(N, false);
    
    branchAndBound(0, 0, 0, values, weights, order, W, N, bv, best, current, rat);
    
    cout << "Best value branch and bounds = " << bv << endl;
    
    auto [gaValue, gaSolution] = geneticAlgorithm(values, weights, W, N);
    cout << "Best value genetic =" << gaValue << endl;
    return 0;
}
void branchAndBound(int idx, int currentWeight, int currentValue,vector<int>& values,vector<int>& weights,
                    vector<int>& order, int W, int N, int& bestValue,vector<bool>& best, vector<bool>& current,
                    vector<double>& rat) {
    if (idx == N) {
        if (currentValue > bestValue) {
            bestValue = currentValue;
            best = current;
        }
        return;
    }

    int itemIdx = order[idx];
    int w = weights[itemIdx];
    int v = values[itemIdx];

  
    double bound = currentValue;
    int remainingWeight = W - currentWeight;
    int j = idx;
    while (j < N && weights[order[j]] <= remainingWeight) {
        bound += values[order[j]];
        remainingWeight -= weights[order[j]];
        j++;
    }
    if (j < N){
        bound += rat[order[j]] * remainingWeight;
    }
    if (bound <= bestValue) return;
    if (currentWeight + w <= W) {
        current[itemIdx] = true;
        branchAndBound(idx + 1, currentWeight + w, currentValue + v,
                       values, weights, order, W, N,
                       bestValue, best, current, rat);
        current[itemIdx] = false;
    }
    branchAndBound(idx + 1, currentWeight, currentValue,values, weights, order, W, N, bestValue, best, current, rat);
}








// оценка популяции
int f(vector<bool>& chromosoms, vector<int>& values,  vector<int>& weights, int W) {
    int totalWeight = 0;
    int totalValue = 0;
    
    for (size_t i = 0; i < chromosoms.size(); ++i) {
        if (chromosoms[i]) {
            totalWeight += weights[i];
            totalValue += values[i];
        }
    }
    
    // если вес больше возможного
    if (totalWeight > W) {
        return 0;
    }
    return totalValue;
}


vector<bool> rand_hrom(int N) {
    vector<bool> chrom(N);
    uniform_int_distribution<> dist(0, 1);
    for (int i = 0; i < N; ++i) {
        chrom[i] = dist(gen);
    }
    return chrom;
}

// Создание начальной популяции
vector<vector<bool>> make_population(int N, int popSize) {
    vector<vector<bool>> p(popSize);
    for (int i = 0; i < popSize; ++i) {
        p[i] = rand_hrom(N);
    }
    return p;
}

// выбирается лучший
vector<bool> tournamentSelection(vector<vector<bool>>& p, vector<int>& fV) {
    uniform_int_distribution<> dist(0, p.size() - 1);
    
    int bestIdx = dist(gen);
    for (int i = 1; i < 3; ++i) {
        int idx = dist(gen);
        if (fV[idx] > fV[bestIdx]) {
            bestIdx = idx;
        }
    }
    return p[bestIdx];
}

// скрещивание
pair<vector<bool>, vector<bool>> cross(vector<bool>& p_1,  vector<bool>& p_2) {
    uniform_real_distribution<> probDist(0.0, 1.0);
    uniform_int_distribution<> pointDist(1, p_1.size() - 1);
    
    if (probDist(gen) <= 0.8) {
        int point = pointDist(gen);
        
        vector<bool> child1(p_1.size());
        vector<bool> child2(p_1.size());
        
        for (size_t i = 0; i < p_1.size(); ++i) {
            if (i < point) {
                child1[i] = p_1[i];
                child2[i] = p_2[i];
            } else {
                child1[i] = p_2[i];
                child2[i] = p_1[i];
            }
        }
        return {child1, child2};
    } else {
        return {p_1, p_2};
    }
}

// Мутация
void mutation(vector<bool>& chromosoms) {
    uniform_real_distribution<> probDist(0.0, 1.0);
    for (size_t i = 0; i < chromosoms.size(); ++i) {
        if (probDist(gen) <= 0.01) {
            chromosoms[i] = !chromosoms[i];
        }
    }
}

pair<int, vector<bool>> geneticAlgorithm(vector<int>& values, vector<int>& weights,int W, int N) {

    auto p = make_population(N, 100);
    vector<int> fV(100);
    
    int bestValue = 0;
    vector<bool> bestchromosoms(N, false);
    
    for (int gen = 0; gen < 200; ++gen){
        for (int i = 0; i < 100; ++i){
            fV[i] = f(p[i], values, weights, W);
            if (fV[i] > bestValue) {
                bestValue = fV[i];
                bestchromosoms = p[i];
            }
        }
        
        vector<vector<bool>> newp;

        int bestIdx = max_element(fV.begin(), fV.end()) - fV.begin();
        newp.push_back(p[bestIdx]);

        while (newp.size() < 100) {
            auto p_1 = tournamentSelection(p, fV);
            auto p_2 = tournamentSelection(p, fV);
            
            auto children = cross(p_1, p_2);

            mutation(children.first);
            mutation(children.second);
            

            newp.push_back(children.first);
            if (newp.size() < 100) {
                newp.push_back(children.second);
            }
        }
        
        p = newp;
    }
    
    
    return {bestValue, bestchromosoms};
}

// ==================== ОСНОВНАЯ ФУНКЦИЯ ====================

/*int main() {
    ifstream file("ks_4_0.txt");
    
    if (!file.is_open()) {
        cerr << "File isn't open!" << endl;
        return 1;
    }
    
    int N, W;
    file >> N >> W;
    
    vector<pair<int, int>> v;
    for (int i = 0; i < N; i++) {
        int pi, wi;
        file >> pi >> wi;
        v.push_back(make_pair(pi, wi));
    }
    file.close();
    
    vector<int> values(N), weights(N);
    for (int i = 0; i < N; ++i) {
        values[i] = v[i].first;
        weights[i] = v[i].second;
    }
    
    // ========== МЕТОД ВЕТВЕЙ И ГРАНИЦ ==========
    cout << "========== BRANCH AND BOUND ==========" << endl;
    
    vector<int> order(N);
    iota(order.begin(), order.end(), 0);
    vector<double> rat(N);
    for (int i = 0; i < N; ++i) {
        rat[i] = static_cast<double>(values[i]) / weights[i];
    }
    sort(order.begin(), order.end(), [&](int a, int b) {
        return rat[a] > rat[b];
    });
    
    int bv = 0;
    vector<bool> best(N, false);
    vector<bool> current(N, false);
    
    auto startBB = chrono::high_resolution_clock::now();
    branchAndBound(0, 0, 0,
                   values, weights, order, W, N,
                   bv, best, current, rat);
    auto endBB = chrono::high_resolution_clock::now();
    auto bbTime = chrono::duratn_cast<chrono::microseconds>(endBB - startBB).count();
    
    cout << "Best value: " << bv << endl;
    cout << "Time: " << bbTime << " μs" << endl;
    cout << "Items (index, value, weight):" << endl;
    for (int i = 0; i < N; ++i) {
        if (best[i]) {
            cout << "  " << i << ": value=" << values[i] << " weight=" << weights[i] << endl;
        }
    }
    
    // ========== ГЕНЕТИЧЕСКИЙ АЛГОРИТМ ==========
    cout << "\n========== GENETIC ALGORITHM ==========" << endl;
    
    auto startGA = chrono::high_resolution_clock::now();
    auto [gaValue, gaSolution] = geneticAlgorithm(values, weights, W, N);
    auto endGA = chrono::high_resolution_clock::now();
    auto gaTime = chrono::duratn_cast<chrono::microseconds>(endGA - startGA).count();
    
    cout << "Best value: " << gaValue << endl;
    cout << "Time: " << gaTime << " μs" << endl;
    cout << "Items (index, value, weight):" << endl;
    for (int i = 0; i < N; ++i) {
        if (gaSolution[i]) {
            cout << "  " << i << ": value=" << values[i] << " weight=" << weights[i] << endl;
        }
    }
    
    // Сравнение результатов
    cout << "\n========== COMPARISON ==========" << endl;
    cout << "Branch and Bound: " << bv << endl;
    cout << "Genetic Algorithm: " << gaValue << endl;
    if (bv == gaValue) {
        cout << "✓ Both algorithms found the optimal solution!" << endl;
    } else {
        cout << "✗ Genetic algorithm found suboptimal solution." << endl;
        cout << "  Optimal: " << bv << ", GA found: " << gaValue << endl;
    }
    
    return 0;
}*/