package rmp;

import java.util.ArrayList;

import bp2pl.Net;


public class RMPOriginal {

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
			return  (((StateTuple) s).path).isSuperset(path) && state.equals(((StateTuple) s).state);
		}
	}
	public static boolean debug = true;

	public static PathList compute(Net net) {
		Model model = Net2Model.netToModel(net);
		HideEdges.hideEdges(model);
		int[] variableMask = model.getGoalMask();		
		return compute(model, variableMask, null);
	}
	
	public static PathList compute(Model model, int[] variableMask, int[] allowedTransitionsMask) {
		
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
				if (!acceptedPaths.isSuperset(s.path)) {
					acceptedPaths.addPath(s.path);
				}
			} else {
				for (int i = 0; i < transitions.size(); i++) {
					int[] mask = BitmaskHelper.makeBitmask(new int[]{i}, transitions.size());
					if (allowedTransitionsMask == null || BitmaskHelper.EQUALS(BitmaskHelper.AND(mask, allowedTransitionsMask), mask)) {
	//					System.out.println("Current path: " + s.path);
						State child = transitions.get(i).applyTransition(s.state);
						if (child != null) {
							Path path = (Path) s.path.clone();
							path.addTransition(i, transitions.size());
							StateTuple childTuple = new StateTuple(child, path);
							if (!visitedTuples.contains(childTuple) && !acceptedPaths.isSuperset(path)) {
								queue.enqueue(childTuple);
								visitedTuples.add(childTuple);
	//							System.out.println(path.toString());
	//							System.out.println(child.toString(model.getSpeciesList()));
							}
						}
					}
				}
			}
		}

		if (debug)
			System.out.println("Total number of (state,path) tuples:  " + visitedTuples.size());
		if (debug)
			System.out.println("Number of accepted paths:  " + acceptedPaths.size());
		
		return acceptedPaths;
	}
}