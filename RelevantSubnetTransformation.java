package rmp;

import java.util.*;

public class RelevantSubnetTransformation {

	public static Path backwardsCollection(Model model, Path path, int[] goalMask) {
		
		int[] goalsPrevious = new int[goalMask.length];
		for (int i = 0; i < goalsPrevious.length; i++)
			goalsPrevious[i] = 0;
		
		int[] acceptedTransitions = BitmaskHelper.makeBitmask(new int[0], model.getTransitions().size());
		
		while (!BitmaskHelper.EQUALS(goalMask, goalsPrevious)) {

			goalsPrevious = goalMask;
			
			goalMask = new int[goalsPrevious.length];
			for (int i = 0; i < goalsPrevious.length; i++)
				goalMask[i] = goalsPrevious[i];
			
			int[] transitionMask = path.getTransitions();
			
			for (int i = 0; i < model.getTransitions().size(); i++) {
				int[] bitmask = BitmaskHelper.makeBitmask(new int[]{i}, model.getTransitions().size());
				// if the transition i was in the path
				
				if (BitmaskHelper.EQUALS(BitmaskHelper.AND(bitmask, transitionMask), bitmask)) {
					Transition transition = model.getTransitions().get(i);
					int[] producedMask = transition.getPostMask();
					int[] andMask = BitmaskHelper.AND(producedMask, goalMask);
					boolean zero = true;
					for (int val : andMask)
						if (val > 0)
							zero = false;
					if (!zero) {
						
						// accept this transition
						int[] mask = BitmaskHelper.makeBitmask(new int[]{i}, model.getTransitions().size());
						acceptedTransitions = BitmaskHelper.OR(acceptedTransitions, mask);
						
						// add the non proper preplaces to the goalset
						goalMask = BitmaskHelper.OR(goalMask, transition.getPreMask());
						goalMask = BitmaskHelper.OR(goalMask, transition.getReadMask());
					}
					
					
				}
			}

		}
		
		ArrayList<Path.Tuple> cardinalities = path.getCardinalities();
		
		ArrayList<Path.Tuple> acceptedCardinalities = new ArrayList<Path.Tuple>();
		
		for (Path.Tuple tuple : cardinalities) {
			int[] mask = BitmaskHelper.makeBitmask(new int[]{tuple.transID}, model.getTransitions().size());
			if (BitmaskHelper.EQUALS(BitmaskHelper.AND(acceptedTransitions, mask), mask)) {
				acceptedCardinalities.add(tuple);	
			}
		}
		
		Path returnPath = new Path(acceptedTransitions, cardinalities);
		
		return returnPath;
	}
}
