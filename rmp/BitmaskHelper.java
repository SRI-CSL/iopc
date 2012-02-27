package rmp;

import java.util.ArrayList;


public class BitmaskHelper {

	private static final int BITS_PER_ELEMENT = 31;


	public static ArrayList<Integer> MASK_TO_INTS(int[] bitmask) {
		
		
		
		ArrayList<Integer> list = new ArrayList<Integer>(); 
		for (int i = 0; i < (bitmask.length*BITS_PER_ELEMENT); i++) {
			
			int[] mask = BitmaskHelper.makeBitmask(new int[]{i}, bitmask.length*BITS_PER_ELEMENT);
			if (BitmaskHelper.EQUALS(BitmaskHelper.AND(mask, bitmask), mask))
				list.add(i);
			
			
			
//			System.out.println("Bitmask is now:  " + mask[index]+"\n");
		}
		
//		int[] returnArr = new int[list.size()];
//		for (int i = 0; i < returnArr.length; i++)
//			returnArr[i] = list.get(i);
		
		return list;
	}
	

	public static void UNSETBIT(int[] mask, int bit) {
		int index = (int) Math.floor(bit/(double)BITS_PER_ELEMENT);

		int distanceIntoElem = bit-BITS_PER_ELEMENT*index;

		int bitmask = (int)Math.pow(2, distanceIntoElem);
		mask[index] &= ~bitmask;
	}
	
	

	public static int[] NOT(int[] arr) {
		
		int[] result = new int[arr.length];
		for (int i = 0; i < arr.length; i++) {
			result[i] = ~arr[i];
		}
		return result;
		
	}

	public static boolean EQUALS(int[] arr1, int[] arr2) {
		if (arr1.length != arr2.length)
			return false;
		
		for (int i = 0; i < arr1.length; i++) {
			if (arr1[i] != arr2[i])
				return false;
		}
		return true;
		
	}


	public static int[] AND(int[] arr1, int[] arr2) {
		
		int[] result = new int[arr1.length];
		for (int i = 0; i < arr1.length; i++) {
			result[i] = arr1[i] & arr2[i];
		}
		return result;
		
	}

	public static int[] OR(int[] arr1, int[] arr2) {
		
		int[] result = new int[arr1.length];
		for (int i = 0; i < arr1.length; i++) {
			result[i] = arr1[i] | arr2[i];
		}
		return result;
		
	}
	
	public static int[] makeBitmask(int[] bitsToSet, int length) {
		
		int numElements = (int) Math.ceil(length/(double)BITS_PER_ELEMENT);
//		System.out.println("Number of elements in the bitmask:  " + numElements);
		int[] mask = new int[numElements];
		
		for (int i = 0; i < mask.length; i++) 
			mask[i] = 0;
		
		for (int val : bitsToSet) {
			int index = (int) Math.floor(val/(double)BITS_PER_ELEMENT);
//			System.out.println(val + " in index "+index);
			int distanceIntoElem = val-BITS_PER_ELEMENT*index;
//			System.out.println(distanceIntoElem);
			int bitmask = (int)Math.pow(2, distanceIntoElem);
			mask[index] |= bitmask;
//			System.out.println("Bitmask is now:  " + mask[index]+"\n");
		}
		
		return mask;
	}
	
	
}
