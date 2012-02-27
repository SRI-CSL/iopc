package rmp;

import java.util.*;

public class Queue<K> {
	ArrayList<K> queue = new ArrayList<K>();

	public void enqueue(K elem) {
		queue.add(0, elem);
	}

	public K dequeue() {
		K elem = queue.get(queue.size() - 1);
		queue.remove(queue.size() - 1);
		return elem;
	}

	public int size() {
		return queue.size();
	}

	public boolean isEmpty() {
		return queue.isEmpty();
	}
}