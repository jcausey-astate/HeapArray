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
    using namespace mmheap;

const size_t MIN_HEAPARRAY_ALLOCATION = 4;  // TODO: Make this more realistic (based on real cache sizes, etc)

/**
 * An array segmented into sqrt(N) min-max
 * heaps of increasing size (based on odd numbers from 1...2*sqrt(N)).
 * Inspired by the discussion begun by Andrei Alexandrescu here:
 * http://forum.dlang.org/post/n3iakr$q2g$1@digitalmars.com
 * @tparam  DataType    the type of data stored in the heap - must be
 *                      DefaultConstructable, LessThanComparable, Swappable,
 *                      CopyConstructable, and CopyAssignable
 */
template <typename DataType>
class HeapArray{
public:
    HeapArray() = default;
    HeapArray(const HeapArray<DataType>& rhs);
    HeapArray(HeapArray<DataType>&& rhs);
    HeapArray<DataType>& operator=(const HeapArray<DataType>& rhs);
    HeapArray<DataType>& operator=(HeapArray<DataType>&& rhs);
    HeapArray(size_t reserve_size, bool allow_resize = true);
    HeapArray(DataType* begin, DataType* end, DataType* physical_end=nullptr, bool allow_resize = true);
    ~HeapArray();

    void                    insert(DataType value);
    bool                    remove(const DataType& value);
    DataType                min()const;
    DataType                max()const;
    std::pair<bool, size_t> find(const DataType& value)const;
    bool                    contains(const DataType& value)const;
    DataType                operator[](size_t index)const;
    size_t                  size()const;

protected:
    void                    _init_heaps();
    void                    _resize(size_t new_size, bool round_up = true);
    void                    _grow();
    size_t                  _final_partition()const;
    size_t                  _find_partition(const DataType& value, bool for_insert=false)const;
    size_t                  _partition_start(size_t p)const;
    size_t                  _partition_end(size_t p)const;
    size_t                  _count_in_partition(size_t p)const;
    size_t                  _partition_size(size_t p)const;
    size_t                  _index_to_partition(size_t i)const;
    std::pair<DataType,DataType>
                            _range_in_partition(size_t p)const;
    DataType                _max_in_partition(size_t p)const;
    std::tuple<bool, size_t, size_t, size_t>
                            _find(const DataType& value)const;

    size_t    storage = 0;
    size_t    count   = 0;
    bool      fixed   = false;
    DataType* a       = nullptr;
};

/**
 * construct HeapArray given a specfic size (number of elements) to reserve.
 * The size may be set to "static" (which dis-allows all dynamic resizing) by
 * setting `allow_resize` to false.  Resize is allowed by default.
 *
 * @param reserve_size number of elements to reserve for the HeapArray
 * @param allow_resize flag representing whether or not the HeapArray is allowed to dynamically resize
 */
template <typename DataType>
HeapArray<DataType>::HeapArray(size_t reserve_size, bool allow_resize){
    a       = new DataType[reserve_size];
    storage = reserve_size;
    fixed   = !allow_resize;
}

/**
 * Construct a HeapArray given pointers to the beginning and end of an existing
 * array, by copy.
 * The HeapArray will be allowed to dynamically resize unless `allow_resize` is false.
 *
 * @param begin         pointer to the first element of the range to copy into the new HeapArray
 * @param end           pointer to the address following the last data element to copy into the new HeapArray
 * @param physical_end  pointer to the address following the physical end of the array (hints at initial size of the HeapArray)
 * @param allow_resize  flag representing whether or not the HeapArray is allowed to dynamically resize
 */
template <typename DataType>
HeapArray<DataType>::HeapArray(DataType* begin, DataType* end, DataType* physical_end, bool allow_resize){                   // copy existing array (range) into the object
    auto new_size = physical_end ? physical_end - begin : end - begin;
    _resize(new_size, allow_resize);                                                                // get space (rounds up only if resize is allowed)
    std::copy(begin, end, a);                                                                       // copy in the existing range of values
    count = end - begin;
    _init_heaps();                                                                                  // and make-heap in each partition
    fixed = !allow_resize;
}

/**
 * Copy constructor for HeapArray; makes a copy of an existing HeapArray.
 *
 * @param rhs the original HeapArray that will be copied into this new one
 */
template <typename DataType>
HeapArray<DataType>::HeapArray(const HeapArray<DataType>& rhs){
    *this = rhs;
}

/**
 * Move constructot for HeapArray; creates a new HeapArray by moving an
 * existing one into the new object.
 *
 * @param rhs the original HeapArray to move into the new one (rhs is left in an empty state)
 */
template <typename DataType>
HeapArray<DataType>::HeapArray(HeapArray<DataType>&& rhs){
    *this = std::move(rhs);
}

/**
 * Copy assignment operator; makes a complete deep copy of a HeapArray, including all data,
 * the physical allocation size, and the "allow_resize" trait.
 *
 * @param rhs the original HeapArray to copy into the left-hand operand
 * @return    a reference to the new copy
 */
template <typename DataType>
HeapArray<DataType>& HeapArray<DataType>::operator=(const HeapArray<DataType>& rhs){
    if(this != &rhs){
        storage = rhs.storage;
        count   = rhs.count;
        fixed   = rhs.fixed;
        delete [] a;
        a = new DataType[storage];
        std::copy(rhs.a, rhs.a+count, a);
    }
    return *this;
}

/**
 * Move assignment operator; moves an existing HeapArray into the left-hand operand,
 * replacing whatever was there.  The right-hand operand is left in an empty state.
 *
 * @param rhs the original HeapArray to move into the left-hand operand; after the
 *            operation `rhs` is left in an empty state
 * @return    a reference to the left-hand operand (containing the moved data)
 */
template <typename DataType>
HeapArray<DataType>& HeapArray<DataType>::operator=(HeapArray<DataType>&& rhs){
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

/**
 * Destroy the HeapArray; deallocates all memory associated with the data structure.
 */
template <typename DataType>
HeapArray<DataType>::~HeapArray(){
    delete [] a;
}

/**
 * Get the logical size (number of elements) for the HeapArray
 * @return the current number of elements contained in the HeapArray
 */
template <typename DataType>
inline size_t HeapArray<DataType>::size()const {
    return count;
}

/**
 * @brief   read-only random access
 * @details provides read-only access directly to the underlying array
 *
 * @param  index index to read
 * @return item stored at `index` (by value)
 * @throws std::out_of_range is thrown if `index` is beyond the end of the logical
 *         size of the HeapArray
 */
template <typename DataType>
DataType  HeapArray<DataType>::operator[](size_t index)const{
    if(index >= count){
        throw std::out_of_range("Index out of range.");
    }
    return a[index];
}

/**
 * @brief   Insert a new item into the HeapArray.
 * @details Inserts a new value.  If the container is full, inserting a new item
 *          will increase its size unless the "allow_resize" option is set to
 *          `false`, in which case the insert will fail with a std::length_error
 *          exception.
 * @param   value  the new value to insert
 * @throws  std::length_error  if the container is already full and isn't allowed to resize
 */
template <typename DataType>
void HeapArray<DataType>::insert(DataType value){
    if(count == storage){                                                                           // if the container is full, resize
        if(fixed){                                                                                  // unless it is fixed size, in which
            throw std::length_error("Maximum size exceeded for fixed-size container.");             // case throw an exception
        }
        _grow();
    }
    auto partition = _find_partition(value, true);                                                  // find which partition the new value
    bool done      = false;                                                                         // belongs in
    do{                                                                                             // then add it to that partition, and
        auto p_count = _count_in_partition(partition);                                              // "ripple" the maximum value (which
        auto ripple  = heap_insert_circular(value,                                                  // will be displaced if the partition is
                            a + _partition_start(partition),                                        // non-final and thus full)
                            p_count,                                                                // down to subsequent partitons,
                            _partition_size(partition));                                            // until the final partition is reached
        done  = !ripple.first;
        value = ripple.second;
        ++partition;
    }while(!done);
    ++count;
}

/**
 * @brief   Remove an element from the HeapArray, given its value.
 * @details If `value` is present in the HeapArray, it is removed (if there are duplicates,
 *          one of the instances of `value` will be removed).  An indication of whether
 *          the value was found and removed is returned.
 *
 * @param value  the value to remove
 * @return       true if `value` is removed, `false` otherwise
 */
template <typename DataType>
bool HeapArray<DataType>::remove(const DataType& value){
    bool removed  = false;
    auto find_res = _find(value);
    if(std::get<0>(find_res)){
        auto partition = std::get<2>(find_res);
        if(partition == _final_partition()){                                                        // if the delete happens to be in the final
            size_t p_count = _count_in_partition(partition);                                        // partition, no "ripple" is necessary,
            heap_remove_at_index(                                                                   // and the element can be trivially
                std::get<3>(find_res),                                                              // removed
                a + _partition_start(partition), p_count);
        }
        else{                                                                                       // non-trivial ripple delete, starting from the end:
            size_t p_count = _count_in_partition(_final_partition());
            auto ripple    = heap_remove_min(                                                       // ripple begins at right-most partition
                                a + _partition_start(_final_partition()), p_count);
            for(auto p = _final_partition() - 1; p > partition; --p){
                ripple = heap_replace_at_index(                                                     // ripples through intermediate partititions
                    ripple,
                    0,
                    a + _partition_start(p),
                    _count_in_partition(p) );
            }
            heap_replace_at_index(                                                                  // and replaces the victim in the destination
                ripple,
                std::get<3>(find_res),
                a + _partition_start(partition),
                _count_in_partition(partition));
        }
        removed = true;
        --count;
    }
    return removed;
}

/**
 * Get the minimum value contained in the HeapArray
 * @return the minimum value in the container
 */
template <typename DataType>
DataType  HeapArray<DataType>::min()const{
    return a[0];                                                                                    // min is first element.
}

/**
 * Get the maximum value contained in the HeapArray
 * @return the maximum value in the container
 */
template <typename DataType>
DataType  HeapArray<DataType>::max()const{
    return heap_max(a +                                                                          // max is maximum element in
        _partition_start(_final_partition()),                                                       // the final partition
        _count_in_partition(_final_partition()));                                                   // (mmheap can access it in O(1))
}

/**
 * @brief   Find the location of a particular value in the HeapArray.
 * @details Searches for `value` in the HeapArray, returning a pair indicating
 *          whether or not the value was found, and if so, the index at which
 *          it was found.
 *
 * @param value  the value to search for
 * @return       a std::pair where the `first` attribute is true if `value` was
 *               found (false otherwise) and the `second` attribute is the index
 *               at which `value` was located (only if it was found).
 */
template <typename DataType>
std::pair<bool, size_t>  HeapArray<DataType>::find(const DataType& value)const{
    auto t_res = _find(value);
    std::pair<bool, size_t> result{std::get<0>(t_res), std::get<1>(t_res)};
    return result;
}

/**
 * Determine whether or not the HeapArray contains the value passes as an argument.
 *
 * @param value  the value to search for
 * @return       true if `value` is found, false otherwise
 */
template <typename DataType>
bool HeapArray<DataType>::contains(const DataType& value)const{
    return count > 0 ? find(value).first : false;
}

/*
 * turns an arbitrary array of values into the appropriate list-of-contiguous-heaps structure
 */
template <typename DataType>
void HeapArray<DataType>::_init_heaps(){
    std::sort(a, a+count);
    for(size_t p = 1; p <= _final_partition(); ++p){                                                // first partition is trivially a heap.
        make_heap(a + _partition_start(p), _count_in_partition(p));                                 // heapify the rest.
    }
}

/*
 * resizes the underlying array container
 *     new_size  new size of the physical container
 *     round_up  set to `true` to round size up to the next perfect square (default=true)
 *     throws    std::runtime_error if the HeapArray is set to "fixed" size mode
 */
template <typename DataType>
void HeapArray<DataType>::_resize(size_t new_size, bool round_up){
    if(fixed){
        throw std::runtime_error("Resize disabled for this array.");
    }
    if(new_size > 0){
        // Allocation sizes should always be perfect squares.
        if(round_up){                                                                               // Round up unless told not to.
            auto rt  = static_cast<size_t>(ceil(sqrt(new_size)));
            new_size = static_cast<size_t>(rt*rt);
        }
        DataType* victim = a;
        a           = new DataType[new_size];
        if(victim){
            std::copy(victim, victim + std::min(count, new_size), a);                               // copy existing values (as many as will fit if sizing down)
        }
        delete [] victim;
        storage = new_size;
    }
    else{                                                                                           // size to zero to clear
        count = storage = 0;
        delete [] a;
        a = nullptr;
    }
}

/*
 * Increases the size of the HeapArray to the next incremental size,
 * by doubling the current physical allocation (rounded up to the next
 * perfect square).
 */
template <typename DataType>
void HeapArray<DataType>::_grow(){
    size_t  next_size = storage * 2;                                                                // double (and then round to next perfect square)
    if(next_size == 0){                                                                             // or set to a minimum size if the container is new
        next_size = MIN_HEAPARRAY_ALLOCATION;
    }
    _resize(next_size);
}

/*
 * Get the partition-index of the final partition in the HeapArray
 */
template <typename DataType>
inline size_t HeapArray<DataType>::_final_partition()const{
    return count > 0 ? static_cast<size_t>(ceil(sqrt(count)) - 1) : 0;
}

/*
 * Get the size of the partition given by the partition-index `p`.
 */
template <typename DataType>
inline size_t HeapArray<DataType>::_partition_size(size_t p)const{
    return p * 2 + 1;
}

/*
 * Get the array index of the first element contained in the partition whose
 * partition-index is `p`.
 */
template <typename DataType>
inline size_t HeapArray<DataType>::_partition_start(size_t p)const{
    return p * p;
}

/*
 * Get the array index of the last element contained in the partition whose
 * partition-index is `p`.
 */
template <typename DataType>
inline size_t HeapArray<DataType>::_partition_end(size_t p)const{
    return p * p + p * 2;
}

/*
 * Convert an array index to a partition-index (i.e. determine which partition
 * a particular array index falls within).
 */
template <typename DataType>
inline size_t HeapArray<DataType>::_index_to_partition(size_t i)const{
    return i > 0 ? static_cast<size_t>(floor(sqrt(i))) : 0;
}

/*
 * Determine how many elements are actually contained in the partition whose
 * partition-index is `p`.
 * NOTE:  All partitions except the final one are always completely full.
 */
template <typename DataType>
size_t HeapArray<DataType>::_count_in_partition(size_t p)const{
    auto c = _partition_size(p);                                                                    // prior partitions are always full.
    if(p >= _final_partition()){                                                                    // final partition may be less than full, find out:
        c = count - (p * p);                                                                        // number in whole structure - number in partitions prior to this one
    }
    return c;
}

/*
 * Get the minimum and maximum values contained in the partition whose
 * partition-index is `p`.
 */
template <typename DataType>
std::pair<DataType,DataType> HeapArray<DataType>::_range_in_partition(size_t p)const{
    auto start_index = _partition_start(p);
    auto p_min       = a[start_index];
    auto p_max       = heap_max(a+start_index, _count_in_partition(p));
    return std::pair<DataType,DataType>{p_min, p_max};
}

/*
 * Get the maximum value contained in the partition whose partition-index is `p`.
 */
template <typename DataType>
DataType HeapArray<DataType>::_max_in_partition(size_t p)const{
    auto start_index = _partition_start(p);
    return heap_max(a+start_index, _count_in_partition(p));
}

/*
 * Finds several pieces of information about a particular value, and returns it
 * as a tuple:
 *     <0> flag indicating whether or not the value was found
 *     <1> the array index at which the value was found (only valid if it was found)
 *     <2> the partition-index of the partition containing the value (or the
 *         partition into which the value should fall if it was not there)
 *     <3> the offset index inside the partition where the value was found
 *         (only valid if it was found)
 *
 *     value    the value to find
 */
template <typename DataType>
std::tuple<bool, size_t, size_t, size_t> HeapArray<DataType>::_find(const DataType& value)const{
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

/*
 * Finds the partition-index of the partition that contains `value`, or
 * (optionally) the partition-index of the partition that _should_ contain
 * that value if it were inserted.
 *     value        the value to search for
 *     for_insert   flag indicating whether this is a speculative search prior
 *                  to an insert.
 */
template <typename DataType>
size_t HeapArray<DataType>::_find_partition(const DataType& value, bool for_insert)const{
    size_t p_index = 0;
    if(count > 0){
        size_t left     = 0;
        size_t right    = _final_partition();
        bool   finished = false;
        while(!finished && left <= right){                                                          // binary search for the partition containing value:
            auto mid   = (left + right) / 2;

            auto range = _range_in_partition(mid);
            if(((range.first < value || range.first == value)
                && (value < range.second || value == range.second)) ||                              // value within range
                (for_insert &&                                                                      // or, if we are inserting
                    ((mid > 0  && (value < range.second || value == range.second)
                        && _max_in_partition(mid-1) <= value)                                       //     follows previous partition
                     || (mid == 0 && (value < range.second || value == range.second))               //     or mid is first partition, value <= max
                     || (mid == _final_partition()
                        && (range.first < value || range.first == value)))))                        //     or mid is last partition, value >= min
            {
                p_index  = mid;
                finished = true;
            }
            else if(range.second < value){
                left = mid + 1;
            }
            else{
                right = mid - 1;
                if(mid == 0){
                    finished = true;                                                                // edge case: right can't become negative, so it would underflow
                }
            }
        }
    }
    return p_index;
}

#endif
