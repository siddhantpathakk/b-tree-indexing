package components.Nodes;

import java.util.ArrayList;
import java.util.TreeMap;

import components.Database.Address;

public class LeafNode extends NodeFunctions {

    public TreeMap<Float, ArrayList<Address>> keyAddressMap;
    protected ArrayList<Address> addresses;
    private LeafNode rightSibling;
    private LeafNode leftSibling;

    public LeafNode() {
        super();
        setLeaf(true);
        setRightSibling(null);
        setLeftSibling(null);
    }

    public ArrayList<Address> findRecord(Float key) {
        return this.keyAddressMap.containsKey(key) || this.keys.contains(key) ? keyAddressMap.get(key) : null;
    }

    public void addRecord(Float key, Address addr) {
        int n = SizeofNode;
        if (this.keys == null) {
            this.keyAddressMap = new TreeMap<Float, ArrayList<Address>>();
            this.addresses = new ArrayList<Address>();

            this.addresses.add(addr);
            this.keyAddressMap.put(key, addresses);

            this.keys = new ArrayList<Float>();
            insertInOrder(this.keys, key);
        } else if (this.keyAddressMap.containsKey(key) || this.keys.contains(key)) {
            ArrayList<Address> existingRecords = keyAddressMap.get(key);
            existingRecords.add(addr);
            keyAddressMap.put(key, existingRecords);
        } else if (this.keys.size() < n) {
            this.addresses = new ArrayList<Address>();
            this.addresses.add(addr);
            this.keyAddressMap.put(key, addresses);
            insertInOrder(this.keys, key);
        } else {
            this.splitLeaf(key, addr);
        }

    }

    public static void insertInOrder(ArrayList<Float> keys, Float key) {
        int i = 0;
        while (i < keys.size() && keys.get(i) < key)
            i++;
        keys.add(i, key);
    }

    public void insertToKeyAddressMap(Float key, ArrayList<Address> addr) {
        keyAddressMap.put(key, addr);
    }

    public void removeKeyFromMap(Float key) {
        keyAddressMap.remove(key);
    }

    public void clear() {
        keys.clear();
        addresses.clear();
    }

    public LeafNode getRightSibling() {
        return rightSibling;
    }

    public void setRightSibling(LeafNode right) {
        rightSibling = right;
    }

    public LeafNode getLeftSibling() {
        return leftSibling;
    }

    public void setLeftSibling(LeafNode left) {
        leftSibling = left;
    }

    @Override
    public String toString() {
        return String.format("\n\tContents of Leaf Node: map %s records %s, rightSibling\n",
                keyAddressMap.toString(),
                addresses, rightSibling);
    }

}
