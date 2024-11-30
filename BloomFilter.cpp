#include <iostream>
#include <vector>
#include <random>
#include <ctime>
#include <cmath>
#include "sciplot/sciplot/sciplot.hpp"

namespace plot = sciplot;
using namespace std;

mt19937 gen(static_cast<unsigned int>(time(0)));
uint64_t prime = 2305843009213693951;
uint64_t universe = 99999999999999999;

vector<int> globalSeeds;

void ensureSeeds(int hashNum) {
    uniform_int_distribution<int> dist(0, INT_MAX);
    while (globalSeeds.size() < hashNum) {
        globalSeeds.push_back(dist(gen));
    }
}

vector<pair<uint64_t, uint64_t> > generateCoefficients(int k, uint64_t prime){
    uniform_int_distribution<uint64_t> distA(1, prime - 1);
    uniform_int_distribution<uint64_t> distB(0, prime - 1);
    vector<pair<uint64_t, uint64_t> > coefficients;
    for(int i = 0; i < k; i++){
        uint64_t a = distA(gen);
        uint64_t b = distB(gen);
        coefficients.push_back(make_pair(a,b));
    }
    return coefficients;
    
}

vector<int> hashOne(uint64_t prime, int x, int size, int hashNum, vector<pair<uint64_t, uint64_t> > coefficients){
    vector<int> hashes;
    for(auto e : coefficients){
        hashes.push_back(((e.first*x +e.second)%prime)%size);
    }
    return hashes;
}

vector<int> hashTwo(int hashNum, int x, vector<int> seeds, int size){
    vector<int> hashes;
    for(int i = 0; i < hashNum; i++){
        mt19937 rng(seeds[i] + x);
        uniform_int_distribution<int> dist(0, size - 1);
        hashes.push_back(dist(rng));
    }
    return hashes;

}

class BloomFilterOne{
public:
    BloomFilterOne(int size, int numOfHashes);
    void insert(int x);
    bool contains(int x);

private:
    int size;
    vector<int> table;
    int numOfHashes;
    vector<pair<uint64_t, uint64_t> > coefficients;
};

BloomFilterOne::BloomFilterOne(int size, int numOfHashes){
    table.resize(size, 0);
    this->numOfHashes = numOfHashes;
    coefficients = generateCoefficients(numOfHashes, prime);
    this->size = size;
}

void BloomFilterOne::insert(int x){
    vector<int> hashes = hashOne(prime, x, table.size(), numOfHashes, coefficients);
    for(auto e : hashes){
        if(table[e] == 0) table[e] = 1;
    }
}

bool BloomFilterOne::contains(int x){
    vector<int> hashes = hashOne(prime, x, table.size(), numOfHashes, coefficients);
    for(auto e : hashes){
        if(table[e] == 0) return false;
    }
    return true;

}






class BloomFilterTwo{
public:
    BloomFilterTwo(int size, int numOfHashes);
    void insert(int x);
    bool contains(int x);

private:
    int size;
    vector<int> table;
    int numOfHashes;
};

BloomFilterTwo::BloomFilterTwo(int size, int numOfHashes){
    this->size = size;
    table.resize(size, 0);
    this->numOfHashes = numOfHashes;
    ensureSeeds(numOfHashes);
}

void BloomFilterTwo::insert(int x){
    vector<int> hashes = hashTwo(numOfHashes, x, globalSeeds, size);
    for(auto e : hashes){
        if(table[e] == 0) table[e] = 1;
    }
}

bool BloomFilterTwo::contains(int x){
    vector<int> hashes = hashTwo(numOfHashes, x, globalSeeds, size);
    for(auto e : hashes){
        if(table[e] == 0) return false;
    }
    return true;

}

double filterOne(int hashNum, int items, int c, int tableSize){
    vector<int> seeds;
    uniform_int_distribution<uint64_t> dist(items, universe);
    BloomFilterOne b(tableSize, hashNum);
    for(int i = 0; i < items; i++){
        b.insert(i);
    }
    int count = 0;
    int falsePositives = 0;
    for(int i = items; i < items*10; i++){
        count++;
        int r = dist(gen);
        if(b.contains(r)) falsePositives++;
    }
    return static_cast<double>(falsePositives) / count;
}

double filterTwo(int hashNum, int items, int c, int tableSize){
    vector<int> seeds;
    uniform_int_distribution<uint64_t> dist(items, universe);
    BloomFilterTwo meow(tableSize, hashNum);
    for(int i = 0; i < items; i++){
        meow.insert(i);
    }
    int count = 0;
    int falsePositives = 0;
    for(int i = items; i < items*10; i++){
        count++;
        int r = dist(gen);
        if(meow.contains(r)) falsePositives++;
    }
    return static_cast<double>(falsePositives) / count;
}

double median(vector<double> v){
    sort(v.begin(), v.end());
    if(v.size() % 2 == 1){
        return v[v.size()/2];
    }
    else{
        return (v[v.size()/2-1]+v[v.size()/2])/2;
    }
}



int main(){
    int items = 10000;
    int c = 10;
    int tableSize = items*c;
    int optimalHashNum = static_cast<int>(round(c * log(2.0)));
    vector<int> hashNumVec;
    vector<double> falsePositivesOne;
    vector<double> graphOne;
    vector<int> hashNumVecTwo;
    vector<double> graphTwo;
    vector<double> graphThree;
    vector<int> hashNumTheoretical;
    vector<int> xCordinate(items);
    vector<int> testOne(items, 0);
    vector<int> testTwo(items, 0);
    vector<pair<uint64_t, uint64_t> > coefficients = generateCoefficients(1, prime);
    ensureSeeds(1);

    for(int i = 0; i < items; i++){
        xCordinate[i] = i;
    }
    

    for(int i = 0; i < tableSize; i++){
        vector<int> v1 = hashOne(prime, i, items, 1, coefficients);
        testOne[v1[0]]++;
        vector<int> v2 = hashTwo(1, i, globalSeeds, items);
        testTwo[v2[0]]++;
    }
    sort(testOne.begin(), testOne.end());
    sort(testTwo.begin(), testTwo.end());
    cout << testOne[0] << " " << testTwo[0] << endl;
    cout << testOne[testOne.size()-1] << " " << testTwo[testTwo.size()-1] << endl;

    plot::Plot2D scatter;
    scatter.xlabel("Index, Inserts: " + to_string(tableSize));
    scatter.ylabel("Collisions");
    scatter.drawPoints(xCordinate, testOne).label("Hash One");
    scatter.drawPoints(xCordinate, testTwo).label("Hash Two");
    scatter.legend().atOutsideBottom().displayHorizontal().displayExpandWidthBy(2);
    plot::Figure figHash = {{scatter}};
    plot::Canvas canvasHash = {{figHash}};
    canvasHash.show();
    
    
    for(int j = optimalHashNum - 5; j < optimalHashNum + 5; j++){
        if(j <= 0) continue;
        falsePositivesOne.clear();
        for(int i = 0; i < 10; i ++){
            falsePositivesOne.push_back(filterOne(j, items, c, tableSize));
        }
        hashNumVec.push_back(j);
        graphOne.push_back(median(falsePositivesOne));
    }
    
    for(int j = optimalHashNum - 5; j < optimalHashNum + 5; j++){
        if(j <= 0) continue;    
        falsePositivesOne.clear();
        for(int i = 0; i < 10; i ++){
            falsePositivesOne.push_back(filterTwo(j, items, c, tableSize));
        }
        hashNumVecTwo.push_back(j);
        graphTwo.push_back(median(falsePositivesOne));
    }

    for(int j = optimalHashNum - 5; j < optimalHashNum + 5; j++){
        if(j <= 0) continue;
        hashNumTheoretical.push_back(j);
        graphThree.push_back(pow(1.0-exp(-static_cast<double>(j)/c), j));
    }



    plot::Plot2D plot;
    plot.xlabel("Hash Functions, c=" + to_string(c) + ", optimal k=" + to_string(optimalHashNum));
    plot.ylabel("False Positive Rate");
    plot.yrange(0.0075, 0.037);
    plot.drawCurve(hashNumVec, graphOne).label("Hash One");
    plot.drawCurve(hashNumVecTwo, graphTwo).label("Hash Two");
    plot.drawCurve(hashNumTheoretical, graphThree).label("Theoertical");
    plot.legend().atOutsideBottom().displayHorizontal().displayExpandWidthBy(2);
    plot::Figure fig = {{plot}};
    plot::Canvas canvas = {{fig}};
    canvas.show();

    return 0;
    
    
}
