package rmp;

import java.util.*;

public class ExtendedHashSet<K> {
	Hashtable<Integer, ArrayList<K>> set = new Hashtable<Integer, ArrayList<K>>();

	public void add(K elem) {
		ArrayList<K> tempArr;
		if (!set.containsKey(new Integer(elem.hashCode()))) {
			tempArr = new ArrayList<K>();
			set.put(new Integer(elem.hashCode()), tempArr);
		} else {
			tempArr = set.get(new Integer(elem.hashCode()));
		}
		tempArr.add(elem);

	}

	public void remove(K elem) {
		if (set.containsKey(new Integer(elem.hashCode()))) {
			ArrayList<K> tempArr = set.get(new Integer(elem.hashCode()));
			tempArr.remove(elem);
		}
	}

	public int size() {

		int size = 0;
		Collection<ArrayList<K>> collection = set.values();
		for (ArrayList<K> arr : collection)
			size += arr.size();
		return size;

	}

	public boolean contains(K elem) {

		boolean found = false;
		ArrayList<K> tempArr = set.get(new Integer(elem.hashCode()));
		if (tempArr != null) {
			found = tempArr.contains(elem);
		}
		return found;
	}

	public int numOcc(K elem) {
		ArrayList<K> tempArr = set.get(new Integer(elem.hashCode()));
		int count = 0;
		if (tempArr == null)
			return count;
		for (K item : tempArr) {
			if (item.equals(elem))
				count++;
		}
		return count;
	}

	public K get(K elem) {
		ArrayList<K> tempArr = set.get(new Integer(elem.hashCode()));
		if (tempArr != null)
			return tempArr.contains(elem) ? tempArr.get(tempArr.indexOf(elem)) : null;

		return null;
	}

	public ArrayList<K> getArrayList() {
		ArrayList<K> fullList = new ArrayList<K>();
		Collection<ArrayList<K>> collection = set.values();
		for (ArrayList<K> arr : collection)
			fullList.addAll(arr);
		return fullList;
	}
}