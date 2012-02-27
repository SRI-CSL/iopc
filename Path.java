package rmp;

import java.util.*;

public class Path implements Cloneable {

	public class Tuple {
		public int transID, card;
		public String toString() {
			return "("+transID+","+card+")";
		}
	}
	private int length;
	
	private int[] transitions;
	private ArrayList<Tuple> cardinalities;

	private Path() {
	}

	public Path(int maxSize) {
		this.transitions = BitmaskHelper.makeBitmask(new int[0], maxSize);
		this.cardinalities = new ArrayList<Tuple>();
	}
	

	public Path(int[] transitions, ArrayList<Tuple> cardinalities) {
		this.transitions = transitions;
		this.cardinalities = cardinalities;
	}

	
	public int[] getTransitions() {
		return transitions;
	}
	
	public ArrayList<Tuple> getCardinalities() {
		return cardinalities; 
	}
	// public Path(ArrayList<Integer> transitions, ArrayList<Integer>
	// transitionCardinalities) {
	// this.transitions = transitions;
	// this.transitionCardinalities = transitionCardinalities;
	// }
	//
	// public ArrayList<Integer> getTransitions() {
	// return transitions;
	// }

	// public int length() {
	// int len = 0;
	// for (int i = 0; i < transitionCardinalities.size(); i++) {
	// len += transitionCardinalities.get(i).intValue();
	// }
	//
	// return len;
	// }

	public Object clone() {
		Path newPath = new Path();

		newPath.transitions = new int[this.transitions.length];
		for (int i = 0; i < newPath.transitions.length; i++)
			newPath.transitions[i] = this.transitions[i];

		newPath.cardinalities = new ArrayList<Tuple>();
		for (Tuple elem : this.cardinalities) {
			Tuple t = new Tuple();
			t.card = elem.card;
			t.transID = elem.transID;
			newPath.cardinalities.add(t);
		}
		
		newPath.length = this.length;
		
		return newPath;
	}

	public void addTransition(int transition, int maxSize) {
		
		int[] transitionBitmask = BitmaskHelper.makeBitmask(new int[] { transition }, maxSize);

		boolean transitionInPath = BitmaskHelper.EQUALS(BitmaskHelper.AND(transitionBitmask, transitions),
				transitionBitmask);

		if (transitionInPath) {
			// Either find the Tuple in the ArrayList or else put an item in the
			// array list for the cardinality.
			boolean found = false;
			for (Tuple t : cardinalities) {
				if (t.transID == transition) {
					t.card++;
					found = true;
					break;
				}
			}
			if (!found) {
				Tuple t = new Tuple();
				t.transID = transition;
				t.card = 2;
				cardinalities.add(t);
			}

		} else {
			transitions = BitmaskHelper.OR(transitionBitmask, transitions);
		}
		
		length++;

	}
	
	public int length() { return length; }

	// checks whether testPath is a superset of this path
	public boolean isSuperset(Path testPath) {
		
//		if (testPath.length < this.length)
//			return false;
		
		boolean transitionsOK = BitmaskHelper.EQUALS(BitmaskHelper.AND(testPath.transitions, transitions),
				transitions);
		if (!transitionsOK)
			return false;

		// if the cardinality of a transition in this path is > than a
		// cardinality in the test path, return false
		for (Tuple t : cardinalities) {
			boolean found = false;
			for (Tuple testT : testPath.cardinalities) {
				if (testT.transID == t.transID) {
					// if a cardinality in this path is greater than a
					// cardinality in the path we are testing for the superset
					// property, it isn't a superset
					if (t.card > testT.card) {
						System.out.println(t.transID);
						System.out.println(t.card);
						System.out.println(testT.transID);
						System.out.println(testT.card+"\n");
						return false;
					}

					found = true;
				}
			}
			if (!found) {
//				System.out.println(t);
//				System.out.println(testPath.cardinalities);
				return false;
			}
		}

		return true;
	}

	public String toString(TransitionList transitionList) {
		
		String str = "";
		boolean init = true;
		for (int i = 0; i < transitionList.size(); i++) {
			int[] bitmask = BitmaskHelper.makeBitmask(new int[]{i}, transitionList.size());
			if (BitmaskHelper.EQUALS(BitmaskHelper.AND(bitmask, transitions), bitmask)) {
				String cardinality = ""; 
				for (Tuple tuple : cardinalities)
					if (tuple.transID == i)
						cardinality = tuple.card + "*";
				str +=  ((init) ? "" : ", ") + cardinality + transitionList.get(i).getName();
				init = false;
			}
		}
		return str;
	}

}
