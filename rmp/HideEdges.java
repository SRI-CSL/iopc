package rmp;

import java.util.*;
import java.io.*;
import java.util.regex.*;

public class HideEdges {

	public static void hideEdges(Model m) {
		ArrayList<String> speciesList = m.getSpeciesList();

		ArrayList<ArrayList<Integer>> placeLabels = new ArrayList<ArrayList<Integer>>();

		
//		Seed:
//
//		For each place p not in the initial marking
//			For each t where t is a proper pretransition of p (a transition that produces p)
//				l = the enzymes for t where the enzymes have no proper posttransitions (transitions that consume the enzyme) 
//			intersect all l
//			label p with l (these are the enzymes that must have been present to produce a token on p)

		for (int i = 0; i < m.getSpeciesList().size(); i++) {
			// if not in initial marking
			int[] bitmask = BitmaskHelper.makeBitmask(new int[]{i}, speciesList.size());
			if (!BitmaskHelper.EQUALS(BitmaskHelper.AND(m.getInitialState().getVars(), bitmask), bitmask)) {
				TransitionList properPretransitions = m.getProperPretransitions(i);
				
				ArrayList<Integer> unconsumedEnzymesIntersection = null;
				for (Transition properPreT : properPretransitions) {
					int[] readmask = properPreT.getReadMask();
					
					// go through all the enzymes for this transition and find which are unconsumed.
					ArrayList<Integer> unconsumedEnzymes = new ArrayList<Integer>();
					for (int j = 0; j < m.getSpeciesList().size(); j++) {
						int[] placemask = BitmaskHelper.makeBitmask(new int[]{j}, speciesList.size());
						// if this place was in our readmask
//						System.out.println("Read mask:  "+BitmaskHelper.MASK_TO_INTS(readmask));
						if (BitmaskHelper.EQUALS(BitmaskHelper.AND(placemask, readmask), placemask)) {
//							System.out.println(BitmaskHelper.MASK_TO_INTS(placemask));
//							System.out.println(j);
							if (m.getProperPosttransitions(j).size() == 0)
								unconsumedEnzymes.add(j);
						}
					}

//					System.out.println(i+":  "+unconsumedEnzymes);
					if (unconsumedEnzymesIntersection == null) {
						unconsumedEnzymesIntersection = new ArrayList<Integer>();
						unconsumedEnzymesIntersection.addAll(unconsumedEnzymes);
					} else {
						ArrayList<Integer> newList = new ArrayList<Integer>();
						for (Integer val : unconsumedEnzymesIntersection) {
							if (unconsumedEnzymes.contains(val))
								newList.add(val);
						}
						unconsumedEnzymesIntersection = newList;
					}
				}
				if (unconsumedEnzymesIntersection == null) {
					unconsumedEnzymesIntersection = new ArrayList<Integer>();
				}
				
				placeLabels.add(unconsumedEnzymesIntersection);
			} else {
				placeLabels.add(new ArrayList<Integer>());
			}
		}
//		System.out.println(placeLabels);
		
		
		
//		Propagate:
//
//		For each place p not in the initial marking
//			For each t where t is a proper pretransition of p (a transition that produces p)
//				l = union of the set of labels on the (non-proper) preplaces (all the enzymes that can be assumed by firing t)
//			intersect all l
//			add l to labels on p
		while (true) {
			ArrayList<ArrayList<Integer>> newPlaceLabels = new ArrayList<ArrayList<Integer>>();
			for (int i = 0; i < m.getSpeciesList().size(); i++) {
					
				
				// get the labels from the previous iteration.
				ArrayList<Integer> labels = new ArrayList<Integer>();
				labels.addAll(placeLabels.get(i));
				newPlaceLabels.add(labels);
				
				// if not in initial marking
				int[] bitmask = BitmaskHelper.makeBitmask(new int[]{i}, speciesList.size());
				if (!BitmaskHelper.EQUALS(BitmaskHelper.AND(m.getInitialState().getVars(), bitmask), bitmask)) {
					TransitionList properPretransitions = m.getProperPretransitions(i);
					
					ArrayList<Integer> labelsIntersection = null;
					for (Transition properPreT : properPretransitions) {
					
	//					System.out.println(properPreT);
						int[] properPreplacesMask = properPreT.getPreMask();
						int[] enzymesMask = properPreT.getReadMask();
						int[] preplacesMask = BitmaskHelper.OR(properPreplacesMask, enzymesMask);
						
						ArrayList<Integer> labelsUnion = new ArrayList<Integer>();

						for (int j = 0; j < m.getSpeciesList().size(); j++) {
							int[] bitmask2 = BitmaskHelper.makeBitmask(new int[]{j}, speciesList.size());
							if (BitmaskHelper.EQUALS(BitmaskHelper.AND(preplacesMask, bitmask2), bitmask2)) {
								for (Integer val : placeLabels.get(j))
									if (!labelsUnion.contains(val))
										labelsUnion.add(val);
							}
						}
						
						if (labelsIntersection == null) {
							labelsIntersection = new ArrayList<Integer>();
							labelsIntersection.addAll(labelsUnion);
						} else {
							ArrayList<Integer> newList = new ArrayList<Integer>();
							for (Integer s : labelsIntersection) {
								if (labelsUnion.contains(s))
									newList.add(s);
							}
							labelsIntersection = newList;
						}
	//					System.out.println("Union of labels on the preplaces:  "+labelsUnion);
					}
					if (labelsIntersection == null) {
						labelsIntersection = new ArrayList<Integer>();
					}
//					System.out.println("Labels before:  "+labels);
					for (Integer s : labelsIntersection)
						if (!labels.contains(s))
							labels.add(s);
//					System.out.println("Labels after:  "+labels);
				}
			}
//			System.out.println(newPlaceLabels);
			boolean same = newPlaceLabels.equals(placeLabels);
			placeLabels = newPlaceLabels;
			if (same)
				break;
			
		}
		
//		Hide edges:
//
//		For each transition t
//			l = labels on p where p is a (non-proper) preplace of t
//			union all l
//			remove from t any enzymatic arc from an enzyme in l

		
		for (Transition t : m.getTransitions()) {
			
			int[] properPreplacesMask = t.getPreMask();
			int[] enzymesMask = t.getReadMask();
			int[] preplacesMask = BitmaskHelper.OR(properPreplacesMask, enzymesMask);
			
			ArrayList<Integer> unionLabels = new ArrayList<Integer>();
			for (int j = 0; j < m.getSpeciesList().size(); j++) {
				int[] bitmask2 = BitmaskHelper.makeBitmask(new int[]{j}, speciesList.size());
				if (BitmaskHelper.EQUALS(BitmaskHelper.AND(preplacesMask, bitmask2), bitmask2)) {
					ArrayList<Integer> labels = placeLabels.get(j);
					for (Integer val : labels)
						if (!unionLabels.contains(val))
							unionLabels.add(val);
				}
			}
//			System.out.println(t);
//			System.out.println(unionLabels);
			
			int[] enzymesNew = new int[enzymesMask.length];
			for (int j = 0; j < enzymesMask.length; j++)
				enzymesNew[j] = enzymesMask[j];

			
			for (Integer val : unionLabels) {
				int[] bitmask2 = BitmaskHelper.makeBitmask(new int[]{val.intValue()}, speciesList.size());
				// if enzymes contains this transition
				if (BitmaskHelper.EQUALS(BitmaskHelper.AND(enzymesMask, bitmask2), bitmask2)) {
					
					BitmaskHelper.UNSETBIT(enzymesNew, val.intValue());
//					System.out.println(t.getName()+":  hide edge " + m.getSpeciesList().get(val));
				}
			}
			
			t.setReadMask(enzymesNew);
			
//			System.out.println(t);
//			System.out.println();
		}
	}
	
	
	
}