package rmp;

import java.util.*;

public class ComputeStubbornSet {

	public static int countEnabledTransitions(Model model, State state, boolean[] arr) {
		TransitionList transitions = model.getTransitions();
		int count = 0;
		for (int i = 0; i < arr.length; i++) {
			if (arr[i] && transitions.get(i).isEnabled( state))
				count++;
		}
		return count;
	}
	// This is compute smallest set rather than smallest number of enabled transitions!!
	public static boolean[] computeSmallestStubbornSet(Model model, State state) {
		TransitionList transitions = model.getTransitions();
		
		int currentCount = -1;
		boolean[] returnStub = null;
		
		for (int i = 0; i < transitions.size(); i++) {
			if (transitions.get(i).isEnabled(state)) {
				boolean[] stub = computeStubbornSet(model, state, i);
				int newCount = countEnabledTransitions(model, state, stub);
				
				// if the size is 1 it has to be *a* smallest stubborn set.
				if (newCount == 1)
					return stub;
				
				if (currentCount == -1 || newCount < currentCount) {
					returnStub = stub;
					currentCount = newCount;
				}
			}
		}
		return returnStub;
	}
	
	public static boolean[] computeStubbornSet(Model model, State state, int transIndex) {
		boolean[] stub = new boolean[model.getTransitions().size()];
		for (int i = 0; i < stub.length; i++)
			stub[i] = false;

		try {
			computeStubbornSet(model, state, stub, transIndex);
		} catch (Exception e) {
			for (int i = 0; i < stub.length; i++)
				stub[i] = true;
		}
		return stub;

	}

	public static void computeStubbornSet(Model model, State state, boolean[] stub, int transIndex)
			throws Exception {
		TransitionList transitions = model.getTransitions();
		
		if (!stub[transIndex]) {
			stub[transIndex] = true;

			// if this new transition is enabled in this state
			if (transitions.get(transIndex).isEnabled(state)) {
				int[] enzymes = transitions.get(transIndex).getReadMask();

				int[] properPreplaces = transitions.get(transIndex).getPreMask();
				int[] nonproperPreplaces = BitmaskHelper.OR(enzymes, properPreplaces);

				int[] properPostplaces = transitions.get(transIndex).getPostMask();
				int[] nonproperPostplaces = BitmaskHelper.OR(enzymes, properPostplaces);

				for (int i = 0; i < model.getSpeciesList().size(); i++) {
					int[] placemask = BitmaskHelper.makeBitmask(new int[] { i }, model.getSpeciesList()
							.size());
					if (BitmaskHelper.EQUALS(BitmaskHelper.AND(placemask, nonproperPreplaces), placemask)) {
						// place i is a nonproperPreplace

						// for each transition != t
						for (int j = 0; j < model.getTransitions().size(); j++) {
							if (j != transIndex) {
								int[] enzymesT2 = transitions.get(j).getReadMask();

								int[] properPreplacesT2 = transitions.get(j).getPreMask();
								int[] nonproperPreplacesT2 = BitmaskHelper.OR(enzymesT2, properPreplacesT2);

								int[] properPostplacesT2 = transitions.get(j).getPostMask();
								int[] nonproperPostplacesT2 = BitmaskHelper.OR(enzymesT2, properPostplacesT2);

								// If both t1,t2 have p as a post place, return
								// false.
								// else return (return t1 and t2 have p as a
								// preplace.)
								if (!BitmaskHelper.EQUALS(
										BitmaskHelper.AND(placemask, nonproperPostplacesT2), placemask)
										|| !BitmaskHelper.EQUALS(BitmaskHelper.AND(placemask,
												nonproperPostplaces), placemask)) {

									if (BitmaskHelper.EQUALS(BitmaskHelper.AND(placemask,
											nonproperPreplacesT2), placemask)) {
										computeStubbornSet(model, state, stub, j);
									}

								}

							}
						}

					}
				}

			} else {
				// else it is not enabled in this state
				int violatingPlaceIndex = transitions.get(transIndex).getViolatingPreplace(model, state);
				if (violatingPlaceIndex == -1) {
					throw new Exception();
				}

				TransitionList producingTransitions = model.getProperPretransitions(violatingPlaceIndex);
				for (int j = 0; j < producingTransitions.size(); j++) {
					int index = model.getTransitions().indexOf(producingTransitions.get(j));							
					computeStubbornSet(model, state, stub, index);
				}
			}

		}
	}
	

	
	public static boolean[] computeStubbornSet(Model model, State state) {
		TransitionList transitions = model.getTransitions();
		for (int i = 0; i < transitions.size(); i++)
			if (transitions.get(i).isEnabled(state))
				return computeStubbornSet(model, state, i);
			
		return null;
	}
}
