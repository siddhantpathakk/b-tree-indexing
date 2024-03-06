package components.BPTree;

import java.util.ArrayList;
import java.util.TreeMap;

import components.DB.Address;

public class LeafNode extends Node {

    protected TreeMap<Float, ArrayList<Address>> keyAddrMap;
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
        return this.keyAddrMap.containsKey(key) || this.keys.contains(key) ?  keyAddrMap.get(key) :  null;
    }

    public ArrayList<Address> getAddressesForKey(Float key) {
        return keyAddrMap.get(key);
    }

    public void addRecord(Float key, Address addr) {
        int n = NODE_SIZE;

        // new node (no map)
        if (this.keys == null) {
            this.keyAddrMap = new TreeMap<Float, ArrayList<Address>>();
            this.addresses = new ArrayList<Address>();

            this.addresses.add(addr);
            this.keyAddrMap.put(key, addresses);
            
            this.keys = new ArrayList<Float>();
            insertInOrder(this.keys, key);
        // handle duplicate key key -> [addr1, addr2, ...]
        } else if (this.keyAddrMap.containsKey(key) || this.keys.contains(key)) {
            ArrayList<Address> existingRecords = keyAddrMap.get(key);
            existingRecords.add(addr);
            keyAddrMap.put(key, existingRecords);
        // no duplicates but has space
        } else if (this.keys.size() < n) {
            this.addresses = new ArrayList<Address>();
            this.addresses.add(addr);
            this.keyAddrMap.put(key, addresses);
            insertInOrder(this.keys, key);
         // need to split since full
        } else {
            this.splitLeafNode(key, addr);
        }

    }

    public static void insertInOrder(ArrayList<Float> keys, Float key) {
        int i = 0;
        while (i < keys.size() && keys.get(i) < key) i++;
        keys.add(i, key);
    }

    public void insertKeyAddrArrPair(Float key, ArrayList<Address> addr) {
        keyAddrMap.put(key, addr);
    }

    public void removeKeyInMap(Float key) {
        keyAddrMap.remove(key);
    }

    public void clear() {
        keys.clear();
        addresses.clear();
    }


    // GETTERS AND SETTERS
    public LeafNode getRightSibling() {
        return rightSibling;
    }

    public void setRightSibling(LeafNode sibling) {
        rightSibling = sibling;
    }

    public LeafNode getLeftSibling() {
        return leftSibling;
    }

    public void setLeftSibling(LeafNode sibling) {
        leftSibling = sibling;
    }

    
    @Override
    public String toString() {
        return String.format("\n--------LEAF NODE CONTAINS: map %s records %s, rightSibling ------------\n", keyAddrMap.toString(),
                addresses, rightSibling);
    }

}
