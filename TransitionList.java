package rmp;

import java.util.*;

public class TransitionList extends ArrayList<Transition> {


	private ArrayList<ArrayList<Integer>> classes = null;
	
	public ArrayList<ArrayList<Integer>> calculateEquivalenceClasses() {
		if (classes == null) {
		classes = new ArrayList<ArrayList<Integer>>();
		
		for (int i = 0; i < this.size(); i++) {
			
			// an equivalence class is a set of transitions that interact with each other
			// at least transition one can disable a transition in the class
			
			ArrayList<Integer> equiv = new ArrayList<Integer>(); 
			equiv.add(i);
			Transition trans = this.get(i);
			int[] properPre = trans.getPreMask();
			int[] pre = trans.getPreMask();
			pre = BitmaskHelper.OR(pre, trans.getReadMask());
			
			
			// go through all the transitions that are not this transition
			for (int j = 0; j < this.size(); j++) {
				if (j != i) {
//
//					ArrayList<Integer> preTest = this.get(j).getPreplacesList();
//					ArrayList<Integer> properPreTest = this.get(j).getProperPreplaces();
					int[] properPreTest = this.get(j).getPreMask();
					int[] preTest = this.get(j).getPreMask();
					preTest = BitmaskHelper.OR(preTest, this.get(j).getReadMask());
					
					boolean ind = true;
					int[] mask = BitmaskHelper.AND(preTest, properPre); 
					for (int val : mask)
						if (val > 0)
							ind = false;

					
					mask = BitmaskHelper.AND(properPreTest, pre); 
					for (int val : mask)
						if (val > 0)
							ind = false;
					
					if (!ind)
						equiv.add(j);
				}
			}
			
			Collections.sort(equiv);
			
			
//			if (equiv.size() > 1) {
				if (!classes.contains(equiv)) {
					int index = 0;
					for (int j = 0; j < classes.size(); j++) {
						if (classes.get(j).size() < equiv.size())
							index++;
						else 
							break;
					}
					classes.add(index, equiv);
				}
//			}
			
		}
		
		for (ArrayList<Integer> equiv : classes) 
			System.out.println(equiv);
		
//		System.exit(1);
		
		}

		return classes;
	}
	
	public String toString(ArrayList<String> speciesList) {
		String str = "";
		for (Transition t : this)
			str += t.toString(speciesList) + "\n";

		return str;
	}


	public int indexOfName(String name) {
		int count = 0;
		for (Transition t : this) {
			if (t.getName().equals(name))
				return count;
			count++;
		}
		return -1;
	}
	
	public boolean isIndependent(int index) {

		Transition trans = this.get(index);
		int[] properPre = trans.getPreMask();
		for (int i = 0; i < this.size(); i++) {
			if (i != index) {
				int[] premask = this.get(i).getPreMask();
				int[] readmask = this.get(i).getReadMask();
				int[] bitmask = BitmaskHelper.OR(premask, readmask);
				
				int[] result = BitmaskHelper.AND(bitmask, properPre);
				
				for (int val : result)
					if (val > 0)
						return false;
				
			}
		}
		return true;

	}
}
