# HeapArray
An array (priority queue, actually) of $\sqrt{N}$ min-max heaps of increasing (odd) size.

## Why?
This is a C++ implementation of the data structure proposed by Andrei Alexandrescu in the [D-Language forums](http://forum.dlang.org/post/n3iakr$q2g$1@digitalmars.com).

I implemented it to see what its practical performance might be, to help move the discussion along, and just for fun.

### What to call it?
I have called it `HeapArray`, but that name isn't very good.  This thing is really more of a _priority queue of heaps_ built directly on top of a contiguous array, but with some interesting properties with respect to the heap sizes.

## Details

### Building the `HeapArray`
If you build the structure directly from an (unsorted) array, it will actually perform a `std::sort` ( O(n*lg(n)) ) followed by a post-processing Floyd _make heap_ ( O(n) ).

If you build the structure dynamically by inserting values, the cost is closer to polynomial<sup>[1]</sup> due to the effect of values having to "ripple" from one heap to the next until they find the right partition.  Empirical data seems to bear this out.  See chart [here](https://plot.ly/~jcausey-astate/18/fill-container-dynamically-heaparray-vs-vector-and-multiset/), which looks similar to the one shown below in the "Scenario" section.

<sup>[1]</sup>: Probably around O(m^(3/2)), as nicely explained by Timon Gehr [here](http://forum.dlang.org/post/n3qqkm$2c6t$1@digitalmars.com). 

### Search
Search can be performed in (theoretically) O(sqrt(n)) steps; empirical data supports this; see [chart here](https://plot.ly/~jcausey-astate/7/search-timing-vector-vs-heaparray-vs-multiset/) and [chart below](https://plot.ly/~jcausey-astate/10/heaparray-vs-multiset-search-times/).
<div>
    <a href="https://plot.ly/~jcausey-astate/10/" target="_blank" title="HeapArray VS multiset: Search Times" style="display: block; text-align: center;"><img src="https://plot.ly/~jcausey-astate/10.png" alt="HeapArray VS multiset: Search Times" style="max-width: 100%;width: 1620px;"  width="1620" onerror="this.onerror=null;this.src='https://plot.ly/404.png';" /></a>
    <script data-plotly="jcausey-astate:10"  src="https://plot.ly/embed.js" async></script>
</div>

### Insert
Insert may cause a value to "ripple" across $\sqrt{n}-1$ partitions, each of which incur a O(lg(n)) cost, so it is O(sqrt(n)*lg(sqrt(n))) _in theory_.  Empirical evidence seems to support this.

### Delete
Delete must "ripple" from the end of the array toward the location of the delete, so it is the mirror image of insert.  It should also be (theoretically) O(sqrt(n)*lg(sqrt(n))).  I have not profiled deletion yet.

### Scenario
For a real use-case, consider trying to generate a large number of unique values.  Obviously something like `std::set` would be great for this.  In this scenario, I used `std::multiset` (so that I would have to manually cull duplicates) and std::vector (where searches would be linear) to see how the HeapArray performed.  Problem size increased to just over 100000.

##### Chart 1: `vector`-VS-`HeapArray`-VS-`multiset`
<div>
    <a href="https://plot.ly/~jcausey-astate/22/" target="_blank" title="Generate Unique Values - HeapArray VS vector and multiset" style="display: block; text-align: center;"><img src="https://plot.ly/~jcausey-astate/22.png" alt="Generate Unique Values - HeapArray VS vector and multiset" style="max-width: 100%;width: 1620px;"  width="1620" onerror="this.onerror=null;this.src='https://plot.ly/404.png';" /></a>
    <script data-plotly="jcausey-astate:22"  src="https://plot.ly/embed.js" async></script>
</div>
Here, you can see the context -- that both `HeapArray` and `multiset` perform much better than `vector` at this task.  This confirms exactly what you would expect, since `vector` requires one or more linear searches every time a new value is generated.  This gives a good visual line for the O(n^2) complexity this results in.

On the other hand, the plots for the other two data structures are much more well behaved -- it is hard to see their shape with `vector` in the chart, so I created another chart to focus on just these two.

##### Chart 2: `HeapArray`-VS-`multiset`
<div>
    <a href="https://plot.ly/~jcausey-astate/24/" target="_blank" title="Generate Unique Values -- HeapArray VS multiset" style="display: block; text-align: center;"><img src="https://plot.ly/~jcausey-astate/24.png" alt="Generate Unique Values -- HeapArray VS multiset" style="max-width: 100%;width: 1620px;"  width="1620" onerror="this.onerror=null;this.src='https://plot.ly/404.png';" /></a>
    <script data-plotly="jcausey-astate:24"  src="https://plot.ly/embed.js" async></script>
</div>
Here, without `vector` to dominate the Y-axis, you can see that `HeapArray` is still performing about O(n^(3/2)) during this task; so the majority of the time is spent in the addition of new values, not in the searching for collisions.  Also as expected, `multiset` is performing in a more logarathmic fashion.

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