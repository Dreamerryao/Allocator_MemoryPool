#pragma GCC optimize(3,"Ofast","inline")
#include <iostream>
#include <random>
#include <vector>
#include <chrono>
#include "my_allocator.h"
#include "new_allocator.h"
using namespace std;


using Point2D = std::pair<int, int>;

template<template<class> class AnyAllocator>
class test {
private:
    int TestSize = 10000;
    int PickSize = 10000;
public:
    void run(unsigned seed);
};

template<template<class> class AnyAllocator>
void test<AnyAllocator>::run(unsigned seed) {
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> dis(1, TestSize);

    // vector creation
    using IntVec = std::vector<int, AnyAllocator<int>>;
    std::vector<IntVec, AnyAllocator<IntVec>> vecints(TestSize);
    for (int i = 0; i < TestSize; i++)
        vecints[i].resize(dis(gen));

    using PointVec = std::vector<Point2D, AnyAllocator<Point2D>>;
    std::vector<PointVec, AnyAllocator<PointVec>> vecpts(TestSize);
    for (int i = 0; i < TestSize; i++)
        vecpts[i].resize(dis(gen));

    // vector resize
    for (int i = 0; i < PickSize; i++) {
        int idx = dis(gen) - 1;
        int size = dis(gen);
        vecints[idx].resize(size);
        vecpts[idx].resize(size);
    }

    // vector element assignment
    {
        int val = 10;
        int idx1 = dis(gen) - 1;
        int idx2 = vecints[idx1].size() / 2;
        vecints[idx1][idx2] = val;
        if (vecints[idx1][idx2] == val)
            std::cout << "correct assignment in vecints: " << idx1 << std::endl;
        else
            std::cout << "incorrect assignment in vecints: " << idx1 << std::endl;
    }
    {
        Point2D val(11, 15);
        int idx1 = dis(gen) - 1;
        int idx2 = vecpts[idx1].size() / 2;
        vecpts[idx1][idx2] = val;
        if (vecpts[idx1][idx2] == val)
            std::cout << "correct assignment in vecpts: " << idx1 << std::endl;
        else
            std::cout << "incorrect assignment in vecpts: " << idx1 << std::endl;
    }

}

int main() {
    test<MyAllocator> test1;
    test<Malloc_Allocator> test2;
    test<std::allocator> test3;
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    auto start = chrono::system_clock::now();
    test1.run(seed);
    auto end = chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "memory pool allocator cost " << duration.count() << " ms" << std::endl;
    start = chrono::system_clock::now();
    test2.run(seed);
    end = chrono::system_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "default allocator cost " << duration.count() << " ms" << std::endl;
    start = chrono::system_clock::now();
    test3.run(seed);
    end = chrono::system_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "std allocator cost " << duration.count() << " ms" << std::endl;
    return 0;
}