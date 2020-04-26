# gift
This was some dabbling the author did about how to build an extensible type system interface that can be used in a data platform. It was called Gift as an acronym for `G`ather `i`nsights `f`rom `t`hings to capture that a type system is trying to model a large number of "things" and do so in way that: 
1. It is easy to add a new type without having to rip through a large amount of code (e.g. touch a switch statement in each operator implementation). Imagine adding spatial types for example to a data platform, or adding IP address as a type. 
2. Make it easy to allow efficient batch/vectorized operations on a column/array of new "things"/typed objects. 
3. Allow the developer to add a new thing to the type system easily, so that it works with very minimal code, leaving the room open to optimize vectorized operations later (and in a clean way)

In Beta (internal to UW for now)
