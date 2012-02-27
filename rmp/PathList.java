package rmp;

import java.util.*;

import rmp.Path.Tuple;

public class PathList implements Cloneable {
	private ArrayList<Path> paths;

	public PathList() {
		paths = new ArrayList<Path>();
	}

	public Object clone() {
		PathList newPathList = new PathList();
		for (Path p : paths) {
			Path copiedPath = (Path) p.clone();
			newPathList.paths.add(copiedPath);

		}
		return newPathList;
	}


	public boolean isSuperset(Path p) {
		for (Path test : paths) {
			if (test.isSuperset(p)) {
				return true;
			}
		}
		return false;
	}

	public void addPath(Path p) {
		paths.add(p);
	}

	public Path get(int i) {
		return paths.get(i);
	}

	public int size() {
		return paths.size();
	}
	
	public ArrayList<ArrayList<ArrayList<String>>> asArrayList(TransitionList transitionList) { 
		ArrayList<ArrayList<String>> allCardinalities = new ArrayList<ArrayList<String>>();
		ArrayList<ArrayList<String>> allTransNames = new ArrayList<ArrayList<String>>();
		
		for (Path path : paths) {
			ArrayList<String> newTransNames = new ArrayList<String>();
			ArrayList<String> newCardinalities = new ArrayList<String>();
			for (int i = 0; i < transitionList.size(); i++) {
				int[] bitmask = BitmaskHelper.makeBitmask(new int[]{i}, transitionList.size());
				if (BitmaskHelper.EQUALS(BitmaskHelper.AND(bitmask, path.getTransitions()), bitmask)) {
					newTransNames.add(transitionList.get(i).getName());
					
					String cardinality = "1"; 
					for (Path.Tuple tuple : path.getCardinalities())
						if (tuple.transID == i)
							cardinality = tuple.card + "";
					newCardinalities.add(cardinality);
				}
			}
			allCardinalities.add(newCardinalities);
			allTransNames.add(newTransNames);
		}
		
		ArrayList<ArrayList<ArrayList<String>>> returnTuple = new ArrayList<ArrayList<ArrayList<String>>>();
		returnTuple.add(allTransNames);
		returnTuple.add(allCardinalities);
		return returnTuple;
		
	}

	public String toString(TransitionList transitionList) {
		String str = "";
		for (Path p : paths) {
			str += p.toString(transitionList) + "\n";
		}
		return str;
	}
}