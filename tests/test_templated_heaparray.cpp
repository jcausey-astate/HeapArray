#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <string>
#include <sstream>
#include "../heaparray.h"

template <typename DType>
void print_array(DType* a, int size);
template <typename DType>
void print_heaparray(const HeapArray<DType>& ha);
template <typename DType>
void print_levels(const HeapArray<DType>& ha);
std::string randstr(size_t length=3);

int main() {
    //srand(time(0));
    srand(8283);
    {
        const int vsize = 35;
        int       test_values[vsize];
        for(int i = 0; i < vsize; ++i){
            test_values[i] = rand() % 100;
        }

        print_array(test_values, vsize);
        std::cout << "\n";

        HeapArray<int> ha;

        for(int i = 0; i < vsize; ++i){
            ha.insert(test_values[i]);
            print_levels(ha);
            std::cout << "\n";
        }

        print_heaparray(ha);
        std::cout << "\n";
        print_levels(ha);

        std::cout << "Making from an array:\n";

        HeapArray<int> ha2(test_values, test_values+vsize, test_values+vsize, false);
        print_levels(ha2);

        std::cout << "Searching...\n";

        for(auto v : test_values){
            if(ha2.contains(v) != true){
                std::cout << "Failed to find " << v << "\n";
                break;
            }
        }

        std::cout << "Removing values...\n";

        for(auto v : test_values){
            std::cout << "Remove " << v << " -- ";
            if(!ha2.remove(v)){
                std::cout << "Failed (didn't find value).\n";
                break;
            }
            else{
                std::cout << "OK\n";
            }
            print_levels(ha2);
            std::cout << "\n";
        }

        std::cout << "Remove values that aren't there...\n";
        bool ok = true;
        for(int i = 0; i < 100; ++i){
            int v = rand() % 100;
            if(std::find(test_values, test_values+vsize, v) == test_values + vsize){
                if(ha.remove(v)){
                    std::cout << "Failed.  Removed " << v << ", which wasn't in the array.\n";
                    ok = false;
                    break;
                }
            }
        }
        if(ok){
            std::cout << "OK\n";
        }

        print_heaparray(ha);
    }
    {
        const int   vsize = 35;
        std::string test_values[vsize];
        for(int i = 0; i < vsize; ++i){
            test_values[i] = randstr();
        }

        print_array(test_values, vsize);
        std::cout << "\n";

        HeapArray<std::string> ha;

        for(int i = 0; i < vsize; ++i){
            ha.insert(test_values[i]);
            print_levels(ha);
            std::cout << "\n";
        }

        print_heaparray(ha);
        std::cout << "\n";
        print_levels(ha);

        std::cout << "Making from an array:\n";

        HeapArray<std::string> ha2(test_values, test_values+vsize, test_values+vsize, false);
        print_levels(ha2);

        std::cout << "Searching...\n";

        for(auto v : test_values){
            if(ha2.contains(v) != true){
                std::cout << "Failed to find " << v << "\n";
                break;
            }
        }

        std::cout << "Removing values...\n";

        for(auto v : test_values){
            std::cout << "Remove " << v << " -- ";
            if(!ha2.remove(v)){
                std::cout << "Failed (didn't find value).\n";
                break;
            }
            else{
                std::cout << "OK\n";
            }
            print_levels(ha2);
            std::cout << "\n";
        }

        std::cout << "Remove values that aren't there...\n";
        bool ok = true;
        for(int i = 0; i < 100; ++i){
            std::string v = randstr(4);
            if(std::find(test_values, test_values+vsize, v) == test_values + vsize){
                if(ha.remove(v)){
                    std::cout << "Failed.  Removed " << v << ", which wasn't in the array.\n";
                    ok = false;
                    break;
                }
            }
        }
        if(ok){
            std::cout << "OK\n";
        }

        print_heaparray(ha);
    }

    return 0;
}

template <typename DType>
void print_array(DType* a, int size){
    for(int i = 0; i < size; ++i){
        std::cout << std::setw(3) << a[i] << (i+1 < size ? ", " : "\n");
    }
}

template <typename DType>
void print_heaparray(const HeapArray<DType>& ha){
    for(size_t i = 0; i < ha.size(); ++i){
        std::cout << std::setw(4) << ha[i] << (i+1 < ha.size() ? ", " : "\n");
    }
}

template <typename DType>
void print_levels(const HeapArray<DType>& ha){
    size_t p = 0;
    for(size_t i = 0; i < ha.size(); ++i){
        std::cout << std::setw(4) << ha[i] << (i+1 < ha.size() ? ", " : "\n");
        if(static_cast<size_t>(sqrt(i+1)) == p+1){
            std::cout << "\n";
            ++p;
        }
    }
}

std::string randstr(size_t length){
    static std::string alphabet{"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"};
    std::ostringstream ss;
    for(size_t i = 0; i < length; ++i){
        ss << alphabet[rand() % alphabet.size()];
    }
    return ss.str();
}
