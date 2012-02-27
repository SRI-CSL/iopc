package rmp;

import java.util.*;

public class Model {

	private ArrayList<String> speciesList;
	private TransitionList transitions;
	private State initState;
	private int[] goalMask;
	
	public int[] getGoalMask() { return goalMask; }
	
	public TransitionList getProperPretransitions(int place) {
		int[] placemask = BitmaskHelper.makeBitmask(new int[]{place}, speciesList.size());
		TransitionList newList = new TransitionList();
		for (int i = 0; i < transitions.size(); i++) {
			int[] postmask =  transitions.get(i).getPostMask();
			if (BitmaskHelper.EQUALS(BitmaskHelper.AND(postmask, placemask), placemask)) {
				newList.add(transitions.get(i));
			}
		}
		return newList;
	}

	public TransitionList getProperPosttransitions(int place) {
		int[] placemask = BitmaskHelper.makeBitmask(new int[]{place}, speciesList.size());
		TransitionList newList = new TransitionList();
		for (int i = 0; i < transitions.size(); i++) {
			int[] premask =  transitions.get(i).getPreMask();
			if (BitmaskHelper.EQUALS(BitmaskHelper.AND(premask, placemask), placemask)) {
				newList.add(transitions.get(i));
			}
		}
		return newList;
	}
	

	public TransitionList getPosttransitions(int place) {
		int[] placemask = BitmaskHelper.makeBitmask(new int[]{place}, speciesList.size());
		TransitionList newList = new TransitionList();
		for (int i = 0; i < transitions.size(); i++) {
			int[] premask =  transitions.get(i).getPreMask();
			int[] enzymemask = transitions.get(i).getReadMask();
			int[] nonproperPreplaces = BitmaskHelper.OR(premask, enzymemask); 
			if (BitmaskHelper.EQUALS(BitmaskHelper.AND(nonproperPreplaces, placemask), placemask)) 
				newList.add(transitions.get(i));
			
		}
		return newList;
	}
	
	
	
	public State getInitialState() {
		return initState;
	}

	public TransitionList getTransitions() {
		return transitions;
	}

	public ArrayList<String> getSpeciesList() {
		return speciesList;
	}

	public Model(ArrayList<String> speciesList, TransitionList transitions, State initState, int[] goalMask) {
		this.speciesList = speciesList;
		this.transitions = transitions;
		this.initState = initState;
		this.goalMask = goalMask;
	}

	public String speciesNamesToString() {
		String str = "";
		for (int i = 0; i < speciesList.size(); i++)
			str += ((i == 0) ? "" : ",") + speciesList.get(i);

		return str;
	}

	public String toString() {
		String str = "";
		str += this.speciesNamesToString();
		str += "\n" + initState.toString(speciesList) + "\n";
		str += transitions.toString(speciesList);
		str += "Goal:  " + (new State(goalMask)).toString(speciesList) + "\n";

		
		return str;
	}
}
