package rmp;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import bp2pl.*;

public class Net2Model {

	
	public static Model netToModel(Net net) {
		
		int numberOfSpecies = net.occmap.size();

		// init species
		ArrayList<String> speciesInit = new ArrayList<String>();
		for (Occurrence occ : net.init)
			speciesInit.add(occ.longName);

		// all species
		ArrayList<String> speciesList = new ArrayList<String>();
		Collection<Occurrence> occs = net.occmap.values();
		for (Occurrence occ : occs)
			speciesList.add(occ.longName);
		
		
		
		TransitionList transitions = new TransitionList();

		
		for (Rule rule : net.rulemap.values()) {
			
			String transitionName = rule.longName;
			
			int[] preArr = new int[rule.consumed.size() + rule.controls.size()];
			for (int i = 0; i < rule.consumed.size(); i++)
				preArr[i] = speciesList.indexOf(rule.consumed.get(i).longName);
			
			for (int i = 0; i < rule.controls.size(); i++)
				preArr[rule.consumed.size()+i] = speciesList.indexOf(rule.controls.get(i).longName);
			
			int[] postArr = new int[rule.produced.size() + rule.controls.size()];
			for (int i = 0; i < rule.produced.size(); i++)
				postArr[i] = speciesList.indexOf(rule.produced.get(i).longName);
			
			for (int i = 0; i < rule.controls.size(); i++)
				postArr[rule.produced.size()+i] = speciesList.indexOf(rule.controls.get(i).longName);
			
			
			
			for (int val : preArr)
			System.out.print(val + ",");
			System.out.println();
			

			for (int val : postArr)
			System.out.print(val + ",");
			System.out.println();
			
			// Convert them to proper pre and post arrays and a read array.
			preArr = BitmaskHelper.makeBitmask(preArr, numberOfSpecies);
			postArr = BitmaskHelper.makeBitmask(postArr, numberOfSpecies);
			int[] readArr = BitmaskHelper.AND(preArr, postArr);
			
			preArr = BitmaskHelper.AND(preArr, BitmaskHelper.NOT(readArr));
			postArr = BitmaskHelper.AND(postArr, BitmaskHelper.NOT(readArr));

			transitions.add(new Transition(preArr, postArr, readArr, transitionName));	
		}
		
		
		int[] initStateVars = new int[speciesInit.size()];
		for (int i = 0; i < initStateVars.length; i++)
			initStateVars[i] = speciesList.indexOf(speciesInit.get(i));
		
		
		initStateVars = BitmaskHelper.makeBitmask(initStateVars, numberOfSpecies);
		
		State initState = new State(initStateVars);

		
		int[] goalIndices = new int[net.goals.size()];
		for (int i = 0; i < goalIndices.length; i++)
			goalIndices[i] = speciesList.indexOf(net.goals.get(i).longName);
		int[] goalMask = BitmaskHelper.makeBitmask(goalIndices, numberOfSpecies);
		System.out.println(new Model(speciesList, transitions, initState, goalMask));
		System.out.println(numberOfSpecies);
		
		return new Model(speciesList, transitions, initState, goalMask);
	}
}
