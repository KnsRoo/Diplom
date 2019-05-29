#include <iostream>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <time.h>
#include <math.h>
#include <algorithm>
#include <functional>
#include <array>
#include <iterator>
#include <utility>

using namespace std;

int irand(int a, int b) {
	return rand() % (b - a + 1) + a;
}

bool exist(vector<int> a, int var){
    #pragma parallel for
    for (int i = 0; i < a.size(); i++){
        if (a[i] == var) return true;
    }
    return false;
}

bool exist(vector<pair<int,int>> a, pair<int,int> var){
    #pragma parallel for
    for (int i = 0; i < a.size(); i++){
        if (a[i] == var) return true;
    }
    int temp = var.first;
    var.first = var.second;
    var.second = temp;
    #pragma parallel for
    for (int i = 0; i < a.size(); i++){
        if (a[i] == var) return true;
    }
    return false;
}

class Identity{
    int x, y; float ab = 0;
    vector<int> matrix;

public:

Identity(int x, int y){
    this->x = x;
    this->y = y;
    #pragma parallel for
    for (int i = 0; i < x*y; i++){
        this->matrix.push_back(0);
    }
}

void init(){
    int j = 0;
    #pragma parallel for
    for (int i = 0; i < this->y; i++){
        this->matrix[this->x*i+j] = 1;
        j++;
    }
}

void shuffle(){
    vector<int> shuffled;
    vector<int> poses; int temp;
    while (poses.size() != this->y){
        temp = irand(0,this->y-1);
        if (!exist(poses, temp)) poses.push_back(temp);
    }
    #pragma parallel for
    for (int i = 0; i < poses.size(); i++){
        for (int j = 0; j < this->x; j++){
            shuffled.push_back(this->matrix[this->x*poses[i]+j]);
        }
    }
    #pragma parallel for
    for (int i = 0; i < this->x*this->y; i++){
        this->matrix[i] = shuffled[i];
    }
}

void ability(vector<int> target){
    this->ab = 0;
    #pragma parallel for reduction(:+this->ab)
    for (int i = 0; i < target.size(); i++){
        this->ab+=this->matrix[i]*target[i];
    }
}

bool wrong(){
    int isum = 0, jsum = 0;
    #pragma parallel for
    for (int i = 0; i < this->x; i++){
        #pragma parallel for reduction(:+isum)
        for (int j = 0; j < this->y; j++){
            isum+=this->matrix[this->x*i+j];
        }
        if (isum > 1) return true;
        isum = 0;
    }
    #pragma parallel for
    for (int i = 0; i < this->y; i++){
        #pragma parallel for reduction(:+jsum)
        for (int j = 0; j < this->x; j++){
            jsum+=this->matrix[this->y*j+i];
        }
        if (jsum > 1) return true;
        jsum = 0;
    }
    return false;
}

void mutation(){
    while (wrong()){
        int pos, null;
        #pragma parallel for
        for (int i = 0; i < this->y; i++){
            int jsum = 0;
            #pragma parallel for
            for (int j = 0; j < this->x; j++){
                jsum+=this->matrix[this->y*j+i];
            }
            if (jsum > 1) pos = i;
            if (jsum == 0) null = i;
        }
        int i = whereT(pos);
        set((i / this->y)*this->y+null, 1); set(i, 0);
    }
}

void set(int pos, int value){
    this->matrix[pos] = value;
}

float get_ability(){
    return this->ab;
}

int getx(){ return this->x; }
int gety(){ return this->y; }

void print(){
    #pragma parallel for
    for (int i = 0; i < this->x; i++){
        #pragma parallel for
        for (int j = 0; j < this->y; j++){
            cout<<this->matrix[this->x*i+j]<<" ";  //
        }
        cout<<endl;
    }
}

int whereT(int i){
    int pos;
    for (int j = 0; j < this->y; j++){
        if (this->matrix[this->y*j+i] == 1) pos = this->y*j+i;
    }
    return pos;
}

int where(int i){
    int pos;
    for (int j = 0; j < this->x; j++){
        if (this->matrix[this->x*i+j] == 1) pos = this->x*i+j;
    }
    return pos;
}

};

float mean_ability(vector<Identity> pop){
    float sum = 0;
    for (int i = 0; i < pop.size(); i++){
        sum+=(pop[i].get_ability());
    }
    return sum/pop.size();
}


vector<pair<int,int>> combinations_with_replacement(vector<int> nums){
    vector<pair<int,int>> dekart;
    #pragma parallel for
    for (int i = 0; i < nums.size(); i++){
        #pragma parallel for
        for (int j = 0; j < nums.size(); j++){
            pair<int,int> temp = make_pair(nums[i], nums[j]);
            if (!exist(dekart, temp)) dekart.push_back(temp);
        }
    }
    return dekart;
}

Identity cross(Identity dad, Identity mom){
    Identity *child = new Identity(dad.getx(), dad.gety());
    #pragma parallel for
    for (int i = 0; i < dad.gety(); i++){
        int posd = dad.where(i);
        int posm = mom.where(i);
        int posc = (int)(round((posd+posm)/2));
        child->set(posc, 1);
    }
    child->mutation();
    return *child;
}

vector<Identity> evolution(vector<Identity> pop, float mean, vector<int> target){
    vector<float> interval, interval2; float sum;
    vector<int> nums;
    #pragma parallel for
    for (int i = 0; i < pop.size(); i++){
        sum+=pop[i].get_ability();
    }
    #pragma parallel for
    for (int i = 0; i < pop.size(); i++){
        float cumsum = (i == 0) ? 0 : interval[i-1];
        float temp = pop[i].get_ability()/sum*100+cumsum;
        interval.push_back(temp);
        interval2.push_back(temp);
    }
    #pragma parallel for
    for (int i = 0; i < pop.size(); i++){
        float temp = (float)irand(0, 99);
        interval.push_back(temp);
    }
    sort(interval.begin(), interval.end());
    int k = 0;
    for (int i = 0; i < interval.size(); i++){
        if (interval[i] != interval2[k]) nums.push_back(k);
        else k++;
    }
    vector<pair<int,int>> dekart;
    dekart = combinations_with_replacement(nums);
    int h = dekart.size()-1;
    vector<int> best;
    while (best.size() != pop.size()){
        int temp = irand(0, h);
        best.push_back(temp);
    }
   vector<Identity> newpop;
   #pragma parallel for
   for (int i = 0; i < pop.size(); i++){
        Identity temp = cross(pop[dekart[best[i]].first], pop[dekart[best[i]].second]);
        temp.ability(target);
        newpop.push_back(temp);
    }
   return newpop;
}

int loops(vector<int> maxes){
    int max = 0, index;
    for (int i = 0; i < maxes.size(); i++){
        if (maxes[i] > max){
            max = maxes[i];
            index = i;
        }
    }
    return index;
}

int main()
{
    float y, y_prev = 0;
    const int POP_SIZE = 4;
    const int LOOP_SIZE = 20;
    srand(time(NULL));
    vector<int> target = {100, 150, 90, 200, 200, 100, 70, 150, 250, 80,  70, 100, 190, 100, 120, 200};
    vector<Identity> Population;
    vector<int> maxes;
    vector<Identity> results;
    #pragma parallel for
    for (int i = 0; i < LOOP_SIZE; i++){
        #pragma parallel for
        for (int i = 0; i < POP_SIZE; i++){
            Identity *temp = new Identity(4,4);
            temp->init();
            temp->shuffle();
            temp->ability(target);
            Population.push_back(*temp);
        }
        y = mean_ability(Population);
        while (true){
            Population = evolution(Population, y, target);
            y = mean_ability(Population);
            if (fabs(y - y_prev) == 0) break;
            y_prev = y;
        }
        int max = 0, index;
        #pragma parallel for
        for (int i = 0; i < Population.size(); i++){
            int temp = Population[i].get_ability();
            if (temp > max){
                max = temp;
                index = i;
            }
        }
        maxes.push_back(max);
        results.push_back(Population[index]);
    }
    int max = loops(maxes);
    cout<<maxes[max]<<endl;
    results[max].print();
    return 0;
}
