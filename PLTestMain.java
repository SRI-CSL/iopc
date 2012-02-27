package rmp;

import java.util.ArrayList;

import bp2pl.*;

public class PLTestMain {

	
	
	public static void main(String args[]) {
	
		String filename;
		String goalPlace;
		
		filename = "Rela.pn";
//		filename = "Rela-STM6-IL1-TNF.pn";
		goalPlace = "Rela-act-NUc";
		
		
//		filename = "ErksAct.pn";
//		filename = "ErksAct-STM6.pn";
//		goalPlace = "Erks-act-EgfRC";
		Net net = null;
		try {
			net = (new ParserPN2PL()).readFile(filename);
		} catch (Exception e) {
			System.out.println(e.getMessage());
			System.exit(1);
		}
		
		Occurrence goalOcc = null;
		for (Occurrence occ : net.occmap.values())
			if (occ.longName.equals(goalPlace))
				goalOcc = occ;
		
		net.goals.add(goalOcc);
		

		
		
//		System.out.println(model);
		
		// create an RMP function that takes a net object and 
		// returns a double entry ArrayList of strings for each transition 
		// in a path and cardinality of each transition in the path.
		
		
		long start = System.currentTimeMillis();
//		PathList acceptedPaths = RMPOriginal.compute(net);
//		PathList acceptedPaths = RMPStubbornSets.compute(net);
		ArrayList<ArrayList<ArrayList<String>>> acceptedPaths = RMPDependenceSets.compute(net);
		
		long end = System.currentTimeMillis();
//		System.out.println(acceptedPaths.toString(Net2Model.netToModel(net).getTransitions()));

		for (int i = 0; i < acceptedPaths.get(0).get(1).size(); i++)
		System.out.println(acceptedPaths.get(0).get(1).get(i));

		for (int j = 0; j < acceptedPaths.get(1).size(); j++)
		for (int i = 0; i < acceptedPaths.get(0).get(j).size(); i++)
		if (!acceptedPaths.get(1).get(j).get(i).equals("1"))
		System.out.println(acceptedPaths.get(1).get(j).get(i));
		
		if (true) {
			System.out.println("Execution:  " + ((end - start) / 1000) + "s");
//			System.out.println("Number of paths:  " + acceptedPaths.size());
		}
	
	}
}
