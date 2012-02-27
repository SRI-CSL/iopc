package rmp;

import java.util.ArrayList;

import bp2pl.Net;

public class RMPDependenceSets {
	static class StateTuple {
		State state;
		Path path;

		public StateTuple(State state, Path path) {
			this.state = state;
			this.path = path;
		}

		public int hashCode() {
			return state.hashCode();
		}

		public boolean equals(Object s) {
			return (((StateTuple) s).path).isSuperset(path) && state.equals(((StateTuple) s).state);
		}
	}

	public static boolean debug = true;

	public static ArrayList<ArrayList<ArrayList<String>>> compute(Net net) {
		Model model = Net2Model.netToModel(net);
		HideEdges.hideEdges(model);
		int[] variableMask = model.getGoalMask();
		State initState = model.getInitialState();
		TransitionList transitions = model.getTransitions();

		Queue<StateTuple> queue = new Queue<StateTuple>();
		PathList acceptedPaths = new PathList();
		ExtendedHashSet<StateTuple> visitedTuples = new ExtendedHashSet<StateTuple>();

		queue.enqueue(new StateTuple(initState, new Path(transitions.size())));
		visitedTuples.add(new StateTuple(initState, new Path(transitions.size())));

		int count = 1;
		int numProcessed = 0;
		int depth = 0;
		while (queue.size() > 0) {
			StateTuple s = queue.dequeue();

			numProcessed++;
			if (numProcessed >= count * 10000) {
				count++;
				if (debug)
					System.out.println("Depth:  " + depth + "\tnumProcessed:  " + numProcessed + "   Queue: "
							+ queue.size());
			}
			if (depth < s.path.length()) {
				depth = s.path.length();
				if (debug)
					System.out.println("Depth:  " + depth + "\tnumProcessed:  " + numProcessed + "   Queue: "
							+ queue.size());
			}

			if (BitmaskHelper.EQUALS(BitmaskHelper.AND(s.state.getVars(), variableMask), variableMask)) {
				Path p = RelevantSubnetTransformation.backwardsCollection(model, s.path, variableMask);
				if (!acceptedPaths.isSuperset(p)) {
					acceptedPaths.addPath(p);
					System.out.println("Path " + acceptedPaths.size() + ":  "+p.toString(transitions));
				}
			} else {
//				for (int i = 0; i < transitions.size(); i++) {
//					// System.out.println("Current path: " + s.path);
//					State child = transitions.get(i).applyTransition(s.state);
//					if (child != null) {
//						Path path = (Path) s.path.clone();
//						path.addTransition(i, transitions.size());
//						StateTuple childTuple = new StateTuple(child, path);
//						if (!visitedTuples.contains(childTuple) && !acceptedPaths.isSuperset(path)) {
//							queue.enqueue(childTuple);
//							visitedTuples.add(childTuple);
//							// System.out.println(path.toString());
//							// System.out.println(child.toString(model.getSpeciesList()));
//						}
//					}
//				}
				
				
				
				
				
				
				
				

				boolean newState = false;
				
//					System.out.println("Executing all transitions");
					for (ArrayList<Integer> equiv : transitions.calculateEquivalenceClasses()) {
						boolean allEnabled = true;
						for (int i : equiv) {
							allEnabled = allEnabled && transitions.get(i).applyTransition(s.state) != null;
						}
						if (allEnabled) {
//							System.out.println("Found an equivalence class that is all enabled:  "+equiv);
							
							for (int i : equiv) {
								State child = transitions.get(i).applyTransition(s.state);
								if (child != null) {
									Path path = (Path) s.path.clone();
									path.addTransition(i, transitions.size());
									StateTuple childTuple = new StateTuple(child, path);
									if (!visitedTuples.contains(childTuple) && !acceptedPaths.isSuperset(path)) {
										queue.enqueue(childTuple);
										visitedTuples.add(childTuple);
										newState = true;
//										System.out.println("New state");
		//								System.out.println(path.toString(transitions));
									}
								}
							}
							if (newState)
								break;
						}
					}
//					System.out.println("End loop, queue " + queue.size() + "   num processed " + numProcessed);
				
				
				
				
				if (!newState) {
//					System.out.println("Executing all transitions");
					for (int i = 0; i < transitions.size(); i++) {
						State child = transitions.get(i).applyTransition(s.state);
						if (child != null) {
							Path path = (Path) s.path.clone();
							path.addTransition(i, transitions.size());
							StateTuple childTuple = new StateTuple(child, path);
							if (!visitedTuples.contains(childTuple) && !acceptedPaths.isSuperset(path)) {
								queue.enqueue(childTuple);
								visitedTuples.add(childTuple);
//								System.out.println(path.toString(transitions));
							}
						}
					}
//					System.out.println("End loop" + queue.size() + "   " + numProcessed);
				}
			}
		}

		if (debug)
			System.out.println("Total number of (state, path) tuples:  " + visitedTuples.size());
		if (debug)
			System.out.println("Number of candidate paths:  " + acceptedPaths.size());
			
		PathList reactionMinimalPaths = new PathList();
		for (int i = 0; i < acceptedPaths.size(); i++) {
			Path p = acceptedPaths.get(i);
			
			RMPOriginal.debug = false;
			PathList RMpaths = RMPOriginal.compute(model, variableMask, p.getTransitions());
			for (int j = 0; j < RMpaths.size(); j++) {
				reactionMinimalPaths.addPath(RMpaths.get(j));
				if (debug)				
					System.out.println("Enforced reaction minimal property on candidate path " + (i+1) + ":  " + RMpaths.get(j).toString(transitions));
			}
		}

		if (debug)
			System.out.println("Number of reaction minimal paths:  " + reactionMinimalPaths.size());
		
		return reactionMinimalPaths.asArrayList(model.getTransitions());
	}
}