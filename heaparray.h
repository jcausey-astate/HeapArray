#ifndef HEAPARRAY_H
#define HEAPARRAY_H
/**
 * @file heaparray.h
 * 
 * Defines the HeapArray, an array segmented into sqrt(N) min-max
 * heaps of increasing size (based on odd numbers from 1...2*sqrt(N)).
 * Inspired by the discussion begun by Andrei Alexandrescu here:
 * http://forum.dlang.org/post/n3iakr$q2g$1@digitalmars.com
 * 
 * @author    Jason L Causey
 * @license   Released under the MIT License: http://opensource.org/licenses/MIT
 * @copyright Copyright (c) 2015 Jason L Causey, Arkansas State University 
 * 
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *   THE SOFTWARE.
 */

#include <algorithm>
#include <stdexcept>
#include <tuple>
#include <cmath>
#include "mmheap.h"

const size_t MIN_HEAPARRAY_ALLOCATION = 4;  // TODO: Make this more realistic (based on real cache sizes, etc)

class HeapArray{
public:
    HeapArray() = default;
    HeapArray(const HeapArray& rhs);
    HeapArray(HeapArray&& rhs);
    HeapArray& operator=(const HeapArray& rhs);
    HeapArray& operator=(HeapArray&& rhs);
    HeapArray(int reserve_size, bool allow_resize = true);
    HeapArray(int* begin, int* end, int* physical_end=nullptr, bool allow_resize = true); // take ownership of a pre-existing array
    ~HeapArray();

    void                    insert(int value);
    bool                    remove(int value);
    int                     min()const;
    int                     max()const;
    std::pair<bool, size_t> find(int value)const;
    bool                    contains(int value)const;
    int                     operator[](size_t index)const;
    size_t                  size()const;

protected:
    void                    _init_heaps();
    void                    _resize(size_t new_size, bool round_up = true);
    void                    _grow();
    size_t                  _final_partition()const;
    size_t                  _find_partition(int value, bool for_insert=false)const;
    size_t                  _partition_start(size_t p)const;
    size_t                  _partition_end(size_t p)const;
    size_t                  _count_in_partition(size_t p)const;
    size_t                  _partition_size(size_t p)const;
    size_t                  _index_to_partition(size_t i)const;
    std::pair<int,int>      _range_in_partition(size_t p)const;
    int                     _max_in_partition(size_t p)const;
    std::tuple<bool, size_t, size_t, size_t> 
                            _find(int value)const;
        
    size_t  storage = 0;
    size_t  count   = 0;
    bool    fixed   = false;
    int* a          = nullptr;
};

HeapArray::HeapArray(int reserve_size, bool allow_resize){
    a       = new int[reserve_size];
    storage = reserve_size;
    fixed   = !allow_resize;
}

HeapArray::HeapArray(int* begin, int* end, int* physical_end, bool allow_resize){ // work in-place on an existing array
    auto new_size = physical_end ? physical_end - begin : end - begin;
    _resize(new_size, allow_resize);  // get space (rounds up if resize is allowed)
    std::copy(begin, end, a);
    count = end - begin;
    _init_heaps();
    fixed = !allow_resize;
}

HeapArray::HeapArray(const HeapArray& rhs){
    *this = rhs;
}

HeapArray& HeapArray::operator=(const HeapArray& rhs){
    if(this != &rhs){
        storage = rhs.storage;
        count   = rhs.count;
        fixed   = rhs.fixed;
        delete [] a;
        a = new int[storage];
        std::copy(rhs.a, rhs.a+count, a);
    }
    return *this;
}

HeapArray::HeapArray(HeapArray&& rhs){
    *this = std::move(rhs);
}

HeapArray& HeapArray::operator=(HeapArray&& rhs){
    if(this != &rhs){
        storage     = rhs.storage;
        count       = rhs.count;
        fixed       = rhs.fixed;
        delete [] a;
        a           = rhs.a;
        rhs.a       = nullptr;
        rhs.storage = rhs.count = 0;
        rhs.fixed   = false;
    }
    return *this;
}

HeapArray::~HeapArray(){
    delete [] a;
}

inline size_t HeapArray::size()const {
    return count;
}

void HeapArray::_init_heaps(){
    std::sort(a, a+count);
    for(size_t p = 1; p <= _final_partition(); ++p){                    // first partition is trivially a heap.
        make_mm_heap(a + _partition_start(p), _count_in_partition(p));  // heapify the rest.
    }
}

void HeapArray::_resize(size_t new_size, bool round_up){
    if(fixed){
        throw std::runtime_error("Resize disabled for this array.");
    }
    if(new_size > 0){
        // Allocation sizes should always be perfect squares.
        if(round_up){ // Round up unless told not to.
            auto rt  = static_cast<size_t>(ceil(sqrt(new_size)));
            new_size = static_cast<size_t>(rt*rt);
        }
        int* victim = a;
        a           = new int[new_size];
        if(victim){
            std::copy(victim, victim + std::min(count, new_size), a);
        }
        delete [] victim;
        storage = new_size;
    }
    else{  // size to zero to clear
        count = storage = 0;
        delete [] a;
        a = nullptr;
    }
}

void HeapArray::_grow(){
    int  next_size = storage * 2;  // double (and then round to next perfect square)
    if(next_size == 0){
        next_size = MIN_HEAPARRAY_ALLOCATION;
    }
    _resize(next_size);
}

void HeapArray::insert(int value){
    if(count == storage){
        if(fixed){
            throw std::length_error("Maximum size exceeded for fixed-size container.");   
        }
        _grow();
    }
    auto partition = _find_partition(value, true);
    bool done      = false;
    do{
        auto p_count = _count_in_partition(partition);
        auto ripple  = mm_heap_ripple_add(value, 
                            a + _partition_start(partition), 
                            p_count, 
                            _partition_size(partition));
        done  = !ripple.first;
        value = ripple.second;
        ++partition;
    }while(!done);
    ++count;
}

bool HeapArray::remove(int value){
    bool removed  = false;
    auto find_res = _find(value);
    if(std::get<0>(find_res)){
        auto partition = std::get<2>(find_res);
        if(partition == _final_partition()){
            // trivial delete value:
            size_t p_count = _count_in_partition(partition);
            mm_heap_remove_at_index(std::get<3>(find_res), a + _partition_start(partition), p_count);
        }
        else{
            // ripple delete, starting from the end:
            size_t p_count = _count_in_partition(_final_partition());
            auto ripple    = mm_heap_remove_min(a + _partition_start(_final_partition()), p_count);                                     // ripple begins at right-most partition
            for(auto p = _final_partition() - 1; p > partition; --p){
                ripple = mm_heap_replace_at_index( ripple, 0, a + _partition_start(p), _count_in_partition(p) );                        // ripples through intermediate partititions
            }
            mm_heap_replace_at_index(ripple, std::get<3>(find_res), a + _partition_start(partition), _count_in_partition(partition));   // and replaces the victim in the destination
        }
        removed = true;
        --count;
    }
    return removed;
}

int  HeapArray::min()const{
    return a[0];  // min is first element.
}

int  HeapArray::max()const{
    return mm_heap_max(a + _partition_start(_final_partition()), _count_in_partition(_final_partition()));
}

std::pair<bool, size_t>  HeapArray::find(int value)const{
    auto t_res = _find(value);
    std::pair<bool, size_t> result{std::get<0>(t_res), std::get<1>(t_res)};
    return result;
}

bool HeapArray::contains(int value)const{
    return count > 0 ? find(value).first : false;
}
/**
 * @brief   read-only random access
 * @details provides read-only access directly to the underlying array
 * 
 * @param  index index to read
 * @return item stored at `index` (by value)
 */
int  HeapArray::operator[](size_t index)const{
    if(index >= count){
        throw std::out_of_range("Index out of range.");
    }
    return a[index];
}


inline size_t HeapArray::_final_partition()const{
    return count > 0 ? static_cast<size_t>(ceil(sqrt(count)) - 1) : 0;
}

inline size_t HeapArray::_partition_size(size_t p)const{
    return p * 2 + 1;
}

inline size_t HeapArray::_partition_start(size_t p)const{
    return p * p;
}

inline size_t HeapArray::_partition_end(size_t p)const{
    return p * p + p * 2;
}

inline size_t HeapArray::_index_to_partition(size_t i)const{
    return i > 0 ? static_cast<size_t>(floor(sqrt(i))) : 0;
}

size_t HeapArray::_count_in_partition(size_t p)const{
    auto c = _partition_size(p); // prior partitions are always full.
    if(p >= _final_partition()){ // final partition may be less than full, find out:
        c = count - (p * p);     // number in whole structure - number in partitions prior to this one
    }
    return c;
}

std::pair<int,int> HeapArray::_range_in_partition(size_t p)const{
    auto start_index = _partition_start(p);
    int  p_min       = a[start_index];
    int  p_max       = mm_heap_max(a+start_index, _count_in_partition(p));
    return std::pair<int,int>{p_min, p_max};
}

int HeapArray::_max_in_partition(size_t p)const{
    auto start_index = _partition_start(p);
    return mm_heap_max(a+start_index, _count_in_partition(p));
}

std::tuple<bool, size_t, size_t, size_t> HeapArray::_find(int value)const{
    auto   p     = _find_partition(value);
    size_t index = 0;
    bool   found = false;
    if(count > 0){
        for(auto i = _partition_start(p); !found && i <= _partition_end(p); ++i){
            if(a[i] == value){
                found = true;
                index = i;
            }
        }
    }
    return std::make_tuple(found, index, p, index - _partition_start(p));
}

size_t HeapArray::_find_partition(int value, bool for_insert)const{
    // binary search for the partition containing value:
    size_t p_index = 0;
    if(count > 0){
        size_t left     = 0;
        size_t right    = _final_partition();
        bool   finished = false;
        while(!finished && left <= right){
            auto mid   = (left + right) / 2;

            auto range = _range_in_partition(mid);
            if((range.first <= value && value <= range.second) ||                             // value within range
                (for_insert &&                                                                // or, if we are inserting               
                    ((mid > 0  && value <= range.second && _max_in_partition(mid-1) <= value) //     follows previous partition
                     || (mid == 0 && value <= range.second)                                   //     or mid is first partition, value <= max
                     || (mid == _final_partition() && range.first <= value))))                //     or mid is last partition, value >= min
            {
                p_index = mid;
                finished   = true;
            }
            else if(range.second < value){
                left  = mid + 1;
            }
            else{
                right = mid - 1;
                if(mid == 0){
                    finished = true;    // edge case: right can't become negative, so it would underflow
                }
            }
        }
    }
    return p_index;
}

#endif