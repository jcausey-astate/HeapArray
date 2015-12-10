# HeapArray
An array (priority queue, actually) of $\sqrt{N}$ min-max heaps of increasing (odd) size.

## Why?
This is a C++ implementation of the data structure proposed by Andrei Alexandrescu in the [D-Language forums](http://forum.dlang.org/post/n3iakr$q2g$1@digitalmars.com).

I implemented it to see what its practical performance might be, to help move the discussion along, and just for fun.

### What to call it?
I have called it `HeapArray`, but that name isn't very good.  This thing is really more of a _priority queue of heaps_ built directly on top of a contiguous array, but with some interesting properties with respect to the heap sizes.

## Details

### Building the `HeapArray`
If you build the structure directly from an (unsorted) array, it will actually perform a `std::sort` ($\mathrm{O}(n\cdot\mathrm{lg}(n))$) followed by a post-processing Floyd _make heap_ ($\mathrm{O}(n)$).

If you build the structure dynamically by inserting values, the cost is closer to polynomial<sup>[1]</sup> due to the effect of values having to "ripple" from one heap to the next until they find the right partition.

<sup>[1]</sup>: Probably around $\mathrm{O}(m^{3\over{2}})$, as nicely explained by Timon Gehr [here](http://forum.dlang.org/post/n3qqkm$2c6t$1@digitalmars.com). 

### Search
Search can be performed in (theoretically) $\mathrm{O}(\sqrt{n}))$ steps, but actual empirical performance (so far) looks much closer to $\mathrm{O}(\mathrm{lg}(n))$.

### Insert
Insert may cause a value to "ripple" across $\sqrt{n}-1$ partitions, each of which incur a $\mathrm{O}(\mathrm{lg}(n))$ cost, so it is $\mathrm{O}(\sqrt{n}\cdot\mathrm{lg}(\sqrt{n}))$ _in theory_.  Actual empirical results seem to show worse performance (at least when adding many values at once). Perhaps my algorithm is missing an optimization.

### Delete
Delete must "ripple" from the end of the array toward the location of the delete, so it is the mirror image of insert.  It should also be (theoretically) $\mathrm{O}(\sqrt{n}\cdot\mathrm{lg}(\sqrt{n}))$.  I have not profiled deletion yet.

## Dependency
This data structure depends on a Min-Max heap to perform a lot of the fast search (and insert/remove) magic.  I've included my implementation of a Min-Max heap in this repository (<tt>mmheap.h</tt>).

All other dependencies are standard C++ libraries.

## Big Disclaimer
This data structure is still in the _research_ stage -- you should obviously not be using it for anything (other than curiosity) yet.

## License

     Released under the MIT License: http://opensource.org/licenses/MIT
     Copyright (c) 2015 Jason L Causey, Arkansas State University 
     
     Permission is hereby granted, free of charge, to any person obtaining a copy
     of this software and associated documentation files (the "Software"), to deal
     in the Software without restriction, including without limitation the rights
     to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
     copies of the Software, and to permit persons to whom the Software is
     furnished to do so, subject to the following conditions:
     
     The above copyright notice and this permission notice shall be included in
     all copies or substantial portions of the Software.
     
     THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
     IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
     FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
     AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
     LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
     OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
     THE SOFTWARE.