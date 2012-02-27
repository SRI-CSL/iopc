The RMP code is in a package called rmp:

import rmp.*;

There are two RMP algorithms in this package.  The RMP algorithm with stubborn 
sets and the RMP algorithm with dependence sets.  The algorithms can be called as follows: 

ArrayList<ArrayList<ArrayList<String>>> rmps = RMPStubbornSets.compute(net);
ArrayList<ArrayList<ArrayList<String>>> rmps = RMPDependenceSets.compute(net);

where net is a bp2pl.Net object.

Each algorithm uses the Hide Edges algorithm, so there is no need to preprocess.

Each algorithm returns a ArrayList<ArrayList<ArrayList<String>>>.  This is an ArrayList of
size 2, where each element is a ArrayList<ArrayList<String>>.  The first element
is a list of lists of transition names, for each RMP.  The second element is a list of
lists of cardinalities for the corresponding transition name, for each RMP.

Hence, 
rmps.get(0) is the list of lists of transition names.
rmps.get(0).get(10) is the list of transition names for the 11th RMP.
rmps.get(1).get(10) is the corresponding list of cardinalities for the 11th RMP. 

The debug messages can be turned off as follows:
RMPStubbornSets.debug = false;
RMPDependenceSets.debug = false;