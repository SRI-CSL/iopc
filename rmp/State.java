package rmp;

import java.util.*;

public class State {

	private int[] variables;

	public State(int[] variables) {
		this.variables = variables;
	}

	public int[] getVars() {
		return variables;
	}

	public boolean equals(Object s) {
		for (int i = 0; i < variables.length; i++)
			if (variables[i] != ((State) s).variables[i])
				return false;

		return true;
	}

	public int hashCode() {
		return variables[0];
	}
	
	public String toString(ArrayList<String> speciesList) {
		String str = "";
		boolean init = true;
		for (int i = 0; i < speciesList.size(); i++) {
			int[] bitmask = BitmaskHelper.makeBitmask(new int[]{i}, speciesList.size());
			if (BitmaskHelper.EQUALS(BitmaskHelper.AND(variables, bitmask), bitmask)) {
				str += (init ? "" : ",") + speciesList.get(i);
				init = false;
			}
			
		}
		return str;
		
	}
}
