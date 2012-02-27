package rmp;

import java.util.ArrayList;

public class Transition {
	private int[] preMask, postMask, readMask;
	private String name;

	public Transition(int[] preMask, int[] postMask, int[] readMask, String name) {
		this.preMask = preMask;
		this.postMask = postMask;
		this.readMask = readMask;
		this.name = name;
	}
	
	public void setReadMask(int[] readMask) { this.readMask = readMask; }
	
	public String getName() {
		return name;
	}


	public int getViolatingPreplace(Model model, State state) {
		int[] nonproperPreplaces = BitmaskHelper.OR(preMask, readMask);
		
		
		for (int i = 0; i < model.getSpeciesList().size(); i++) {
			int[] placemask = BitmaskHelper.makeBitmask(new int[]{i}, model.getSpeciesList().size());
			if (BitmaskHelper.EQUALS(BitmaskHelper.AND(placemask, nonproperPreplaces), placemask)) {
				// Then i is a non proper preplace, therefore i must be set in state.
				
				// if it is not set in state, then this is a violating place
				if (!BitmaskHelper.EQUALS(BitmaskHelper.AND(placemask, state.getVars()), placemask))
					return i;
				
			}
		}
		
		return -1;
	}

	

	public int[] getPreMask() {
		return preMask;
	}

	public int[] getPostMask() {
		return postMask;
	}

	public int[] getReadMask() {
		return readMask;
	}

	public State applyTransition(State state) {

		if (!this.isEnabled(state))
			return null;

		int[] stateMask = state.getVars();
		
		stateMask = BitmaskHelper.AND(BitmaskHelper.NOT(preMask), stateMask);
		stateMask = BitmaskHelper.OR(postMask, stateMask);
		
		return new State(stateMask);
	}

	public boolean isEnabled(State state) {

		int[] stateMask = state.getVars();
		boolean pre = BitmaskHelper.EQUALS(BitmaskHelper.AND(preMask, stateMask), preMask);
		if (!pre)
			return false;
		
		boolean post = BitmaskHelper.EQUALS(BitmaskHelper.AND(postMask, BitmaskHelper.NOT(stateMask)), postMask);
		if (!post)
			return false;
		
		boolean read = BitmaskHelper.EQUALS(BitmaskHelper.AND(readMask, stateMask), readMask);
		if (!read)
			return false;
		
		return true;
	}
	

	public String toString(ArrayList<String> speciesList) {
		String str = "Transition:  " + name;


		boolean preMaskPresent = false;
		for (int val : preMask)
			if (val > 0)
				preMaskPresent = true;
		
		if (preMaskPresent) {
		str += "  Pre:  ";
		{
			boolean init = true;
			for (int i = 0; i < speciesList.size(); i++) {
				int[] bitmask = BitmaskHelper.makeBitmask(new int[]{i}, speciesList.size());
				if (BitmaskHelper.EQUALS(BitmaskHelper.AND(preMask, bitmask), bitmask)) {
					str += (init ? "" : ",") + speciesList.get(i);
					init = false;
				}
				
			}
		}
		}
		
		boolean postMaskPresent = false;
		for (int val : postMask)
			if (val > 0)
				postMaskPresent = true;
		
		if (postMaskPresent) {
		str += "  Post:  ";
		{
			boolean init = true;
			for (int i = 0; i < speciesList.size(); i++) {
				int[] bitmask = BitmaskHelper.makeBitmask(new int[]{i}, speciesList.size());
				if (BitmaskHelper.EQUALS(BitmaskHelper.AND(postMask, bitmask), bitmask)) {
					str += (init ? "" : ",") + speciesList.get(i);
					init = false;
				}
				
			}
		}
		}
		
		boolean readMaskPresent = false;
		for (int val : readMask)
			if (val > 0)
				readMaskPresent = true;
		
		if (readMaskPresent) {
		str += "  Read:  ";
		{
			boolean init = true;
			for (int i = 0; i < speciesList.size(); i++) {
				int[] bitmask = BitmaskHelper.makeBitmask(new int[]{i}, speciesList.size());
				if (BitmaskHelper.EQUALS(BitmaskHelper.AND(readMask, bitmask), bitmask)) {
					str += (init ? "" : ",") + speciesList.get(i);
					init = false;
				}
				
			}
		}
		}
		
		
		return str;
	}

}
