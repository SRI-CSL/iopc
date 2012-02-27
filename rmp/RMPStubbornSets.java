package rmp;

import java.util.ArrayList;

import bp2pl.Net;

public class RMPStubbornSets {

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
			return state.equals(((StateTuple) s).state) && (((StateTuple) s).path).isSuperset(path);
		}
		public String toString(TransitionList transitionList) {
			return state + "\t" + path.toString();
		}
		public String toString() {
			return toString(null);
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
		PathList provisionalPaths = new PathList();
		ExtendedHashSet<StateTuple> visitedTuples = new ExtendedHashSet<StateTuple>();

		queue.enqueue(new StateTuple(initState, new Path(model.getTransitions().size())));
		visitedTuples.add(new StateTuple(initState, new Path(model.getTransitions().size())));

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
//				Path p = RelevantSubnetTransformation.backwardsCollection(model, s.path, variableMask);
				Path p = RelevantSubnetTransformation.backwardsCollection(model, s.path, variableMask);
//				System.out.println(p.toString(transitions));
				if (!provisionalPaths.isSuperset(p)) {
					provisionalPaths.addPath(p);
				}
			} else {

				boolean addedNew = false;
				
						boolean[] stubbornSet = ComputeStubbornSet.computeSmallestStubbornSet(model, s.state);
						for (int i = 0; i < transitions.size(); i++) {
							if (stubbornSet != null && stubbornSet[i]) {
								
								State child = transitions.get(i).applyTransition(s.state);
								if (child != null) {
									Path path = (Path) s.path.clone();
									path.addTransition(i, transitions.size());
									StateTuple childTuple = new StateTuple(child, path);
									if (!visitedTuples.contains(childTuple) && !provisionalPaths.isSuperset(path)) {
										queue.enqueue(childTuple);
										visitedTuples.add(childTuple);
										addedNew = true;
									}
								}
							}
						}
						
					
				
				// Property S:  Using Enabled(T, m)
				if (!addedNew) {
					for (int i = 0; i < transitions.size(); i++) {
						State child = transitions.get(i).applyTransition(s.state);
						if (child != null) {
							Path path = (Path) s.path.clone();
							path.addTransition(i, transitions.size());
							StateTuple childTuple = new StateTuple(child, path);
							if (!visitedTuples.contains(childTuple) && !provisionalPaths.isSuperset(path)) {
								queue.enqueue(childTuple);
								visitedTuples.add(childTuple);
							}
						}
					}
				}
				
				// Property S:  Using stub'(m)
//				if (!addedNew) {
//
//				for (int j = 0; j < transitions.size(); j++) {
//					if (transitions.get(j).isEnabled(s.state)) {
//						boolean[] stubbornSet = ComputeStubbornSet.computeStubbornSet(transitions, s.state, j);
//						for (int i = 0; i < transitions.size(); i++) {
//							if (stubbornSet != null && stubbornSet[i]) {
//								
//								if (transitions.get(i).isEnabled(s.state)) {
//									State child = transitions.get(i).applyTransition(s.state);
//									Path path = (Path) s.path.clone();
//									path.addTransition(i);
//									StateTuple childTuple = new StateTuple(child, path);
//									if (!visitedTuples.contains(childTuple) && !provisionalPaths.isSuperset(path)) {
//										queue.enqueue(childTuple);
//										visitedTuples.add(childTuple);
//										addedNew = true;
//									}
//								}
//							}
//						}
//						if (addedNew)
//							break;
//					}
//				}
//				}
			}
		}

		if (debug)
			System.out.println("Total number of (state, path) tuples:  " + visitedTuples.size());
		if (debug)
			System.out.println("Number of candidate paths:  " + provisionalPaths.size());
			
		PathList reactionMinimalPaths = new PathList();
		for (int i = 0; i < provisionalPaths.size(); i++) {
			Path p = provisionalPaths.get(i);
			
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