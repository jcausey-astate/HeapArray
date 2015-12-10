/**
 * Profiles Setup (static and dynamic), search,
 * addition, and removal on a linear data structure
 * (std::vector), the new HeapArray, and a logarithmic
 * but pointer-based data structure (std::multiset)
 */

#include <iostream>
#include <algorithm>
#include <vector>
#include <set>
#include <chrono>
#include <iomanip>
    using std::setw;
#include "../heaparray.h"

int main() {
    std::chrono::high_resolution_clock::time_point   begin, end;
    std::chrono::duration<double> ha_duration, sv_duration, ms_duration;
    double    ha_seconds, sv_seconds, ms_seconds;
    const int TSIZE = 100600;
    const int INCR  = 500;
    unsigned long SSIZE = TSIZE * 2;
    int       a0[TSIZE], a1[TSIZE], a2[TSIZE];
    int       n0[TSIZE];
    for(int i = 0; i < TSIZE; i++){
        a0[i] = a1[i] = a2[i] = rand() % TSIZE + (TSIZE / 2);
    }
    {   // Create tight scope for `s` (used only to initialize the negative
        // search array).
        std::multiset<int> s{a0, a0+TSIZE};
        for(int i = 0; i < TSIZE;){
            int v = rand() % (TSIZE*2);
            if(s.find(v) == s.end()){
                n0[i++] = v;
            }
        }
    }
    std::cout << std::setprecision(10);
    std::cout << "Setup Timing (build from static array):\n";
    std::cout << setw(15) << "Data-Size" << ", " << setw(15) << "#-Searches" << ", "<< setw(15) << "Vector"<< ", " << setw(15) << "HeapArray" << ", " << setw(15) << "Multiset\n";
    for(size_t incremental = 100; incremental <= TSIZE; incremental += INCR){   
        begin = std::chrono::high_resolution_clock::now();
        std::vector<int> v{a1, a1+incremental};
        end   = std::chrono::high_resolution_clock::now();
        sv_duration = end-begin;
        sv_seconds  = sv_duration.count();

        begin = std::chrono::high_resolution_clock::now();
        HeapArray h{a2, a2+incremental};
        end   = std::chrono::high_resolution_clock::now();
        ha_duration = end-begin;
        ha_seconds  = ha_duration.count();

        begin = std::chrono::high_resolution_clock::now();
        std::multiset<int> s{a1, a1+incremental};
        end   = std::chrono::high_resolution_clock::now();
        ms_duration = end-begin;
        ms_seconds  = ms_duration.count();

        std::cout << setw(15) << incremental << ", " << setw(15) << " " << ", " << setw(15) << sv_seconds << ", " << setw(15) << ha_seconds << ", " << setw(15) << ms_seconds << "\n";
        std::cout << std::flush;
    }

    std::cout << "\nSearch timing after static setup:\n";
    for(size_t incremental = 100; incremental <= TSIZE; incremental += INCR){   
        SSIZE = 2 * TSIZE;

        std::vector<int>    v{a1, a1+incremental};
        HeapArray           h{a2, a2+incremental};
        std::multiset<int>  s{a1, a1+incremental};
        
        begin = std::chrono::high_resolution_clock::now();
        // Search for values that are in there:
        for(unsigned long i = 0; i < SSIZE; ++i){
            std::find(v.begin(), v.end(), a0[rand() % incremental]); // result doesn't matter
        }
        // Search for values that are not in there:
        for(unsigned long i = 0; i < SSIZE; ++i){
            std::find(v.begin(), v.end(), n0[rand() % incremental]); // result doesn't matter
        }
        end   = std::chrono::high_resolution_clock::now();
        sv_duration = end-begin;
        sv_seconds  = sv_duration.count();

        begin = std::chrono::high_resolution_clock::now();
        // Search for values that are in there:
        for(unsigned long i = 0; i < SSIZE; ++i){
            h.find(a0[rand() % incremental]); // result doesn't matter
        }
        // Search for values that are not in there:
        for(unsigned long i = 0; i < SSIZE; ++i){
            h.find(n0[rand() % incremental]); // result doesn't matter
        }
        end   = std::chrono::high_resolution_clock::now();
        ha_duration = end-begin;
        ha_seconds  = ha_duration.count();

        begin = std::chrono::high_resolution_clock::now();
        // Search for values that are in there:
        for(unsigned long i = 0; i < SSIZE; ++i){
            s.find(a0[rand() % incremental]); // result doesn't matter
        }
        // Search for values that are not in there:
        for(unsigned long i = 0; i < SSIZE; ++i){
            s.find(n0[rand() % incremental]); // result doesn't matter
        }
        end   = std::chrono::high_resolution_clock::now();
        ms_duration = end-begin;
        ms_seconds  = ms_duration.count();

        std::cout << setw(15) << incremental << ", " << setw(15) << 2*SSIZE << ", " << setw(15) << sv_seconds << ", " << setw(15) << ha_seconds << ", " << setw(15) << ms_seconds << "\n";
        std::cout << std::flush;
    }

    std::cout << "\nSetup Timing (build one value at a time, dynamically):\n";
    
    for(size_t incremental = 100; incremental <= TSIZE; incremental += INCR){   
        begin = std::chrono::high_resolution_clock::now();
        std::vector<int> v; // Build vector dynamically
        for(unsigned long j = 0; j < incremental; ++j){
            v.push_back(a0[j]);
        }
        end   = std::chrono::high_resolution_clock::now();
        sv_duration = end-begin;
        sv_seconds  = sv_duration.count();

        begin = std::chrono::high_resolution_clock::now();
        HeapArray h;        // Build heaparray dynamically
        for(unsigned long j = 0; j < incremental; ++j){
            h.insert(a0[j]);
        }
        end   = std::chrono::high_resolution_clock::now();
        ha_duration = end-begin;
        ha_seconds  = ha_duration.count();

        begin = std::chrono::high_resolution_clock::now();
        std::multiset<int> s; // Build multiset dynamically
        for(unsigned long j = 0; j < incremental; ++j){
            s.insert(a0[j]);
        }
        end   = std::chrono::high_resolution_clock::now();
        ms_duration = end-begin;
        ms_seconds  = ms_duration.count();

        std::cout << setw(15) << incremental << ", " << setw(15) << " " << ", " << setw(15) << sv_seconds << ", " << setw(15) << ha_seconds << ", " << setw(15) << ms_seconds << "\n";
        std::cout << std::flush;
    }

    std::cout << "\nSearch timing, after dynamic setup:\n";
    
    for(size_t incremental = 100; incremental <= TSIZE; incremental += INCR){  
        SSIZE = 2 * TSIZE; 
        std::vector<int> v; // Build vector dynamically
        for(unsigned long j = 0; j < incremental; ++j){
            v.push_back(a0[j]);
        }
        HeapArray h;        // Build heaparray dynamically
        for(unsigned long j = 0; j < incremental; ++j){
            h.insert(a0[j]);
        }
        std::multiset<int> s; // Build multiset dynamically
        for(unsigned long j = 0; j < incremental; ++j){
            s.insert(a0[j]);
        }
        
        begin = std::chrono::high_resolution_clock::now();
        // Search for values that are in there:
        for(unsigned long i = 0; i < SSIZE; ++i){
            std::find(v.begin(), v.end(), a0[rand() % incremental]); // result doesn't matter
        }
        // Search for values that are not in there:
        for(unsigned long i = 0; i < SSIZE; ++i){
            std::find(v.begin(), v.end(), n0[rand() % incremental]); // result doesn't matter
        }
        end   = std::chrono::high_resolution_clock::now();
        sv_duration = end-begin;
        sv_seconds  = sv_duration.count();

        begin = std::chrono::high_resolution_clock::now();
        // Search for values that are in there:
        for(unsigned long i = 0; i < SSIZE; ++i){
            h.find(a0[rand() % incremental]); // result doesn't matter
        }
        // Search for values that are not in there:
        for(unsigned long i = 0; i < SSIZE; ++i){
            h.find(n0[rand() % incremental]); // result doesn't matter
        }
        end   = std::chrono::high_resolution_clock::now();
        ha_duration = end-begin;
        ha_seconds  = ha_duration.count();

        begin = std::chrono::high_resolution_clock::now();
        // Search for values that are in there:
        for(unsigned long i = 0; i < SSIZE; ++i){
            s.find(a0[rand() % incremental]); // result doesn't matter
        }
        // Search for values that are not in there:
        for(unsigned long i = 0; i < SSIZE; ++i){
            s.find(n0[rand() % incremental]); // result doesn't matter
        }
        end   = std::chrono::high_resolution_clock::now();
        ms_duration = end-begin;
        ms_seconds  = ms_duration.count();

        std::cout << setw(15) << incremental << ", " << setw(15) << 2*SSIZE << ", " << setw(15) << sv_seconds << ", " << setw(15) << ha_seconds << ", " << setw(15) << ms_seconds << "\n";
        std::cout << std::flush;
    }

    std::cout << "\nScenario timing (create unique N values):\n";
    
    for(size_t incremental = 100; incremental <= TSIZE; incremental += INCR){  
        SSIZE = incremental;
        std::vector<int> v;   // Build vector dynamically
        HeapArray h;          // Build heaparray dynamically
        std::multiset<int> s; // Build multiset dynamically
                
        begin = std::chrono::high_resolution_clock::now();
        for(unsigned long i = 0; i < SSIZE; ++i){
            int item;
            do{    
                item = rand() % SSIZE * 2;                          // generate values
            }while(std::find(v.begin(), v.end(), item) != v.end()); // until one isn't already there
            v.push_back(item);                                      // then add it
        }
        end   = std::chrono::high_resolution_clock::now();
        sv_duration = end-begin;
        sv_seconds  = sv_duration.count();

        begin = std::chrono::high_resolution_clock::now();
        for(unsigned long i = 0; i < SSIZE; ++i){
            int item;
            do{    
                item = rand() % SSIZE * 2;                          // generate values
            }while(h.contains(item));                               // until one isn't already there
            h.insert(item);                                            // then add it
        }
        end   = std::chrono::high_resolution_clock::now();
        ha_duration = end-begin;
        ha_seconds  = ha_duration.count();

        begin = std::chrono::high_resolution_clock::now();
        for(unsigned long i = 0; i < SSIZE; ++i){
            int item;
            do{    
                item = rand() % SSIZE * 2;                          // generate values
            }while(s.find(item) != s.end());                        // until one isn't already there
            s.insert(item);                                         // then add it
        }
        end   = std::chrono::high_resolution_clock::now();
        ms_duration = end-begin;
        ms_seconds  = ms_duration.count();

        std::cout << setw(15) << incremental << ", " << setw(15) << " " << ", " << setw(15) << sv_seconds << ", " << setw(15) << ha_seconds << ", " << setw(15) << ms_seconds << "\n";
        std::cout << std::flush;
    }

    return 0;
}
