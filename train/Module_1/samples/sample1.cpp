#include <iostream>
#include <unordered_map>
#include <map>
#include <chrono>

void run(){
    std::map<int, int> map;
    auto start = std::chrono::high_resolution_clock::now();
    for ( int i = 0; i < 1000000; ++ i){
        map[i] = 2 * i;
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "Map: " << elapsed << std::endl;

    std::unordered_map<int, int> umap;
    auto ustart = std::chrono::high_resolution_clock::now();
    for ( int i = 0; i < 1000000; ++ i){
        umap[i] = 2 * i;
    }
    auto uend = std::chrono::high_resolution_clock::now();
    auto uelapsed = std::chrono::duration_cast<std::chrono::microseconds>(uend - ustart).count();

    std::cout << "U_Map: " << uelapsed << std::endl;

}

int main(){
    run();
    return 0;
}