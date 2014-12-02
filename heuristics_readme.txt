*** Simple heuristics

Two obvious lower bounds:
- Depth of graph.
- Number of distinct colours.

<Double-check this>: In the paper that shows NP-hardness of the problem in a grid, they construct
approximation algorithms using these heuristics.


*** Combined heuristic

Motivating example:
    ...BR
    ...BB
    RR...
    BR...
Both the simple heuristics would give 2 for this instance. The following heuristic gives the opt 3.

Using both of the simple ideas in a single heuristic, we can do better.

Another clear lower bound for a graph of depth d:
- max over 0 <= k <= d { k + (# of distinct colours at depth at least k) }

This can be considered as a generous simulation of solving. Namely,
a single move can do no better than clearing all adjacent colour blobs (the first term) and also
can do no better than clearing all remaining blobs of a particular colour (the second term).

Computing this heuristic is fast. We may precalculate the maximum depth blob of each colour using
BFS in O(n) time. Then, observe that the maximum value of the heuristic can only be attained when
n is one less than the maximum depth of some colour. Since the max depth values are O(n), they
can be sorted using bucket sort if c is large (and in practice, any sort is fine for small c),
giving O(n) time overall.


*** Simulation heuristic

Motivating example:
    RGR.BGR
The combined heuristc gives 3 for this instance. The following heuristic gives the opt 4.

We can exploit the simulation viewpoint to make a tighter heuristic.

Observe that if all the blobs of a particular colour are currently adjacent, then without loss of
generality, any optimal solution clears that colour first.

The heuristic is the duration of the following simulation:
- Until no blobs remain, repeat:
  1. If a colour can be cleared thsi move, do so.
  2. Otherwise, clear all adjacent blobs (even if they are different colours).

This is clearly a lower bound on any optimal solution. Also, it's an upper bound of the
combined heuristic. Why: TODO.

Computing this heuristic is not as convenient. O(n) is still possible, but the constant is larger.

This heuristic happens to give an exact answer in the special case of filling from one end of an
interval graph. In that case, a greedy algorithm is optimal: clear a colour immediately if possible,
and otherwise clear the colour of the adjacent interval having the largest right endpoint.

(Note: The honey-bee paper considers filling from any node and also general co-comparability graphs
(a generalization of interval graphs), but to fill from the middle you need a DP approach rather
than greedy, and I don't think their generalization to co-comparability graphs works as written.
In any case, I doubt that this heuristic is optimal for filling from the middle of an interval
graph.)
