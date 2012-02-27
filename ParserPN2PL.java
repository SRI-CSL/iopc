package rmp;

import java.util.*;
import java.io.*;
import java.util.regex.*;
import bp2pl.*;

public class ParserPN2PL {

	private static final boolean debug = false;

	public static boolean speciesListContains(ArrayList<Occurrence> list, Occurrence occ) {
		for (Occurrence occ2 : list)
			if (occ2.longName.equals(occ.longName))
				return true;
		
		return false;
	}
	
	public static Occurrence getOccurrenceFromList(ArrayList<Occurrence> list, String longname) {
		for (Occurrence occ : list)
			if (occ.longName.equals(longname))
				return occ;
		
		return null;
	}
	
	
	public Net readFile(String filename) throws Exception {
		
		Net net = new Net();
		
		// TODO:  Remove Model
		// TODO:
		// Set net.goals manually.
//		net.init<Occurance>
//		net.occmap<somename, Occurrance>
		
		// First step, create an array list of occurrences and initially present occurrences.
		ArrayList<Occurrence> speciesList = new ArrayList<Occurrence>();
		ArrayList<Occurrence> speciesInit = new ArrayList<Occurrence>();
		
		int lineCount = 0;
		try {
			BufferedReader in = new BufferedReader(new FileReader(filename));

			if (!in.ready())
				throw new IOException();
			
			String line;

			while ((line = in.readLine()) != null) {
				lineCount++;
				if (!line.trim().equals("")) {
					line = line.trim();

					if (line.matches("^\\s*initial\\s*=\\s*.*$")) {
						Pattern p = Pattern.compile("^\\s*initial\\s*=\\s*");
						Matcher m = p.matcher(line);
						String str = m.replaceFirst("");


						Occurrence occ = new Occurrence();
						occ.longName = str;
//						System.out.println("Init:  "+str);
						
						speciesList.add(occ);
						speciesInit.add(occ);
						
					} else if (line.matches("^\\s*input\\s*=\\s*.*$")) {

						Pattern p = Pattern.compile("^\\s*input\\s*=\\s*");
						Matcher m = p.matcher(line);
						String str = m.replaceFirst("");

						Occurrence occ = new Occurrence();
						occ.longName = str;
						
						if (!ParserPN2PL.speciesListContains(speciesList, occ))							
							speciesList.add(occ);
							
					} else if (line.matches("^\\s*output\\s*=\\s*.*$")) {

						Pattern p = Pattern.compile("^\\s*output\\s*=\\s*");
						Matcher m = p.matcher(line);
						String str = m.replaceFirst("");

						Occurrence occ = new Occurrence();
						occ.longName = str;
						
						if (!ParserPN2PL.speciesListContains(speciesList, occ))
							speciesList.add(occ);

					}
				}

			}


//			
			in.close();
		} catch (Exception e) {
			throw new Exception("Error at line " + lineCount + ":  " + e.getMessage());
		}

		

		ArrayList<Rule> rules = new ArrayList<Rule>();
		lineCount = 0;
		try {
			BufferedReader in = new BufferedReader(new FileReader(filename));

			if (!in.ready())
				throw new IOException();

			Rule rule = null;
			String longname = null;
			ArrayList<Occurrence> consumed = new ArrayList<Occurrence>();
			ArrayList<Occurrence> produced = new ArrayList<Occurrence>();
			ArrayList<Occurrence> controls = new ArrayList<Occurrence>();
			
			
			
			String line;

			while ((line = in.readLine()) != null) {
				lineCount++;
				if (!line.trim().equals("")) {
					line = line.trim();

					if (line.matches("^\\s*initial\\s*=\\s*.*$")) {						
//						***IGNORE THIS LINE IN THE MODEL - HANDLED ABOVE***
//						Pattern p = Pattern.compile("^\\s*initial\\s*=\\s*");
//						Matcher m = p.matcher(line);
//						String str = m.replaceFirst("");
//						if (debug)
//							System.out.println("Got the initial:  " + str);
//						speciesPresent.add(str);
//						speciesList.add(str);						
					} else if (line.matches("^-+$")) {
						
						
						// if we have a rule name, then store the rule in the rules arraylist
						if (longname != null) {
							// then we have a rulestored in the variables.
							rule = new Rule();
							rule.consumed = consumed;
							rule.produced = produced;
							rule.controls = controls;
							rule.longName = longname;
							rules.add(rule);
							
						}

						// reset the datastructures to store the rule
						rule = null;
						longname = null;
						consumed = new ArrayList<Occurrence>();
						produced = new ArrayList<Occurrence>();
						controls = new ArrayList<Occurrence>();

					} else if (line.matches("^\\s*transition\\s*=\\s*.*$")) {

						Pattern p = Pattern.compile("^\\s*transition\\s*=\\s*");
						Matcher m = p.matcher(line);
						String str = m.replaceFirst("");

						if (debug)
							System.out.println("Got the transition name:  " + str);
						longname = str;

					} else if (line.matches("^\\s*input\\s*=\\s*.*$")) {

						Pattern p = Pattern.compile("^\\s*input\\s*=\\s*");
						Matcher m = p.matcher(line);
						String str = m.replaceFirst("");

						if (debug)
							System.out.println("Got the input:  " + str);

						
						consumed.add(ParserPN2PL.getOccurrenceFromList(speciesList, str));
//						preIndices.add(new Integer(speciesList.indexOf(str)));

					} else if (line.matches("^\\s*output\\s*=\\s*.*$")) {

						Pattern p = Pattern.compile("^\\s*output\\s*=\\s*");
						Matcher m = p.matcher(line);
						String str = m.replaceFirst("");

						if (debug)
							System.out.println("Got the output:  " + str);

						Occurrence occ = ParserPN2PL.getOccurrenceFromList(speciesList, str);
						
						produced.add(occ);
						// TODO:  Handle controls too!
						if (ParserPN2PL.speciesListContains(consumed, occ))
							controls.add(occ);
//						System.out.println("Controls size:  "+controls.size());
					}
				}

			}

			// if we have a transition stored in the data structures but we have
			// not handled it yet, then handle it now
			if (longname != null) {
					// then we have a rulestored in the variables.
					rule = new Rule();
					rule.consumed = consumed;
					rule.produced = produced;
					rule.controls = controls;
					rule.longName = longname;
					rules.add(rule);
			}

			in.close();

		} catch (Exception e) {
			throw new Exception("Error at line " + lineCount + ":  " + e.getMessage());
		}
		
		
		
		
		System.out.println("Parsed " + rules.size() + " rules.");
		

		// Add the occurrences to the net.
		net.init = speciesInit;
		net.occmap = new HashMap<String,Occurrence>();
		for (Occurrence occ : speciesList)
			net.occmap.put(occ.toString(), occ);
		
		net.rulemap = new HashMap<String,Rule>();
		for (Rule rule : rules)
			net.rulemap.put(rule.toString(), rule);
		
		net.goals = new ArrayList<Occurrence>();
		
	
		// process the set of produced and consumed occurrences for each rule
		// and remove from these lists those occurrences that are controls.
		for (Rule rule : rules) {
			for (int i = 0; i < rule.consumed.size(); i++) {
				Occurrence occ = rule.consumed.get(i);
				if (rule.controls.contains(occ))
					rule.consumed.remove(occ);
			}
			
			for (int i = 0; i < rule.produced.size(); i++) {
				Occurrence occ = rule.produced.get(i);
				if (rule.controls.contains(occ))
					rule.produced.remove(occ);
			}
		}
		
//		for (Occurrence occ : speciesList) 
//			System.out.println(occ.longName);
		
//		for (Occurrence occ : net.occmap.values()) 
//			System.out.println(occ.longName);

		System.out.println("Number of species (list):  " +speciesList.size());
		
		System.out.println("Number of species (net):  " +net.occmap.values().size());
		
		
		
		System.out.println("Number of rules (list):  " +rules.size());
		
		System.out.println("Number of rules (net):  " +net.rulemap.values().size());
		
		
		return net;
	}

}
