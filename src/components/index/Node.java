package components.index;

import java.util.ArrayList;
import java.util.TreeMap;
import java.util.Map;
import java.util.Set;
import java.util.SortedMap;
import components.storage.Address;

public class Node {

    private int minLeafNodeSize;
    private int minInternalNodeSize;
    static final int NODE_SIZE = BPlusTree.NODE_SIZE;
    private boolean isLeaf;
    private boolean isRoot;
    private InternalNode parent;
    protected ArrayList<Float> keys;
    Node rootNode;


    public Node() {
        this.rootNode = BPlusTree.getRoot();
        this.isLeaf = false;
        this.isRoot = false;
        this.minLeafNodeSize = (NODE_SIZE + 1) / 2;
        this.minInternalNodeSize = NODE_SIZE / 2;
    }

    public void removeLastKey() {
        this.keys.remove(keys.size() - 1);
    }

    void replaceKeyAt(int index, Float key) {
        keys.set(index, key);
    }

    public int getIdxOfKey(Float key, boolean upperBound) {
        int keyCount = keys.size();
        return binSearchKeyIdx(0, keyCount - 1, key, upperBound);
    }

    public void insertKeyAt(int index, Float key) {
        keys.add(index, key);
    }
    
    public static void insertKeyInOrder(ArrayList<Float> keys, Float key) {
        int i = 0;
        while (i < keys.size() && keys.get(i) < key) {
            i++;
        }
        keys.add(i, key);
    }

    public void insertChildInOrder(InternalNode parent, InternalNode child) {
        int i = 0;
        Float childToSort = child.getKeyAt(0);
        while (i < parent.getKeyCount() && parent.getKeyAt(i) < childToSort) {
            i++;
        }
        parent.children.add(i + 1, child);
    }

    // update tree after removal of key at index
    public void updateKeyAt(int keyIndex, Float newKey, boolean leafNotUpdated, Float lowerbound) {
        // run only once to make leaf updated
        if (keyIndex >= 0 && keyIndex < keys.size() && !leafNotUpdated) {
            keys.set(keyIndex, newKey);
        }
        if (parent != null && !parent.isLeaf()) {
            int childIndex = parent.getChildren().indexOf(this);
            if (childIndex >= 0) {
                if (childIndex > 0) {
                    parent.replaceKeyAt(childIndex - 1, keys.get(0));
                }
                parent.updateKeyAt(childIndex - 1, newKey, false, lowerbound);
                // System.out.println(parent.keys);
                // System.out.println(parent.getChild(0).keys);
            }
        } else if (parent != null && parent.isLeaf()) {
            parent.updateKeyAt(keyIndex, newKey, false, lowerbound);
        }
    }

    // boolean to check if node is not valid / underutilised
    public boolean isUnderUtilized(int maxKeyCount) {
        if (isRoot()) {
            return (this.getKeyCount() < 1);
        } else if (isLeaf()) {
            return (this.getKeyCount() < (maxKeyCount + 1) / 2);
        } else {
            return (this.getKeyCount() < maxKeyCount / 2);
        }
    }

    public boolean canDonate(int maxKeyCount) {
        if (!isLeaf()) return getKeyCount() - 1 >= maxKeyCount / 2;
        return getKeyCount() - 1 >= (maxKeyCount + 1) / 2;
    }
  
    /** 
     * Inserts new node to parent node in ascending order based on key values.
     * 
     * @param newNode is the leaf node to be inserted to the parent node.
     */
    public void insertNewNodeToParent(LeafNode newNode) {
        int index = 0;
        boolean insertedNode = false;

        try {
            for (Node currentNode : this.getParent().getChildren()) {

                // if there is a node > than newNode, insert inbetween that node
                if (newNode.getKeyAtIdx(newNode.getKeyCount() - 1) < currentNode.getKeyAtIdx(0)) {
                    this.getParent().getChildren().add(index, newNode);
                    this.getParent().keys.add(index - 1, newNode.getKeyAtIdx(0));
                    insertedNode = true;
                    break;
                }
                index++;
            }

            if (insertedNode == false) {
                this.getParent().getChildren().add(newNode);
                this.getParent().keys.add(newNode.getKeyAtIdx(0));
            }

        } catch (Exception e) {
            this.getParent().getChildren().add(newNode);
            this.getParent().keys.add(newNode.getKeyAtIdx(0));
        }

        newNode.setParent(this.getParent());

        if (this.getParent().getKeyCount() > NODE_SIZE) {
            this.getParent().splitInternalNode();
        }
    }

    
    // split leaf node and add to current node's parent if present, else create first parent
    public void splitLeafNode(Float key, Address addr) {

        LeafNode newNode = this.splitLeafNodeHelper(key, addr);

        // if parent is present, add to parent, split if necessary
        if (this.getParent() != null) {
            this.insertNewNodeToParent(newNode);
            if (this.getParent().getKeyCount() > NODE_SIZE) {
                this.getParent().splitInternalNode();
            }
        } else {
            this.createFirstParentNode(newNode);
        }

    }

    // similar to prev
    public void splitInternalNode() {
        InternalNode sibling = this.splitInternalNodeHelper();
        // if node has parent
        if (this.getParent() != null) {
            // add sibling node to parent
            insertChildInOrder(this.getParent(), sibling);
            // set sibling node's parent as this node's parent
            sibling.setParent(this.getParent());
            // insert this node's first key to parent
            insertKeyInOrder(this.getParent().keys, sibling.getKeyAt(0));
            sibling.keys.remove(0);
            if (this.getParent().getKeyCount() > NODE_SIZE) {
                this.getParent().splitInternalNode();
            }
        } else {
            // this node is root (no parent)
            InternalNode newRoot = new InternalNode();
            newRoot.keys = new ArrayList<Float>();
            // add sibling's key to root
            newRoot.keys.add(sibling.getKeyAt(0));
            sibling.keys.remove(0);
            // create parent child relationship between both siblings and parent
            newRoot.addChild(this);
            newRoot.addChild(sibling);
            this.setParent(newRoot);
            sibling.setParent(newRoot);

            BPlusTree.setRoot(newRoot);
        }
    }

    // GETTERS AND SETTERS + misc and helper
    public int getMinLeafNodeSize() {
        return this.minLeafNodeSize;
    }

    public int getMinInternalNodeSize() {
        return this.minInternalNodeSize;
    }

    public boolean isLeaf() {
        return isLeaf;
    }

    public boolean isRoot() {
        return isRoot;
    }

    public void setLeaf(boolean isLeaf) {
        this.isLeaf = isLeaf;
    }

    public void setRoot(boolean isRoot) {
        this.isRoot = isRoot;
    }

    public InternalNode getParent() {
        return this.parent;
    }

    public void setParent(InternalNode parentNode) {
        if (this.isRoot()) {
            this.setRoot(false);
            parentNode.setRoot(true);
            parentNode.setLeaf(false);
            BPlusTree.setRoot(parentNode);
        } else {
            parentNode.setLeaf(false);
        }
        this.parent = parentNode;
    }

    public ArrayList<Float> getKeys() {
        return this.keys;
    }
    
    public Float getKeyAtIdx(int index) {
        return this.keys.get(index);
    }

    public int getKeyCount() {
        return this.keys.size();
    }

    // functions for neater code
    public Float getFirstKey() {
        return this.keys.get(0);
    }
    public Float getLastKey() {
        return this.keys.get(keys.size() - 1);
    }

    public Float removeKeyAt(int index) {
        return this.keys.remove(index);
    }

    private int binSearchKeyIdx(int left, int right, Float key, boolean upperBound) {
        if (left > right)
            return left;
        int middle = (left + right) / 2;
        Float middleKey = getKeyAt(middle);

        if (middleKey < key) {
            return binSearchKeyIdx(middle + 1, right, key, upperBound);
        } else if (middleKey > key) {
            return binSearchKeyIdx(left, middle - 1, key, upperBound);
        } else {
            while (middle < keys.size() && keys.get(middle).equals(key))
                middle++;
            if (!upperBound)
                return middle - 1;
            return middle;
        }
    }

     public Float getKeyAt(int index) {
        return keys.get(index);
    }

    public int getLastIdx() {
        return keys.size() - 1;
    }
    
    public void printNode() {
        Set<Float> keys = ((LeafNode) this).keyAddrMap.keySet();
        System.out.println(keys);
    }
    
    public void createFirstParentNode(LeafNode newNode) {
        InternalNode newParent = new InternalNode();
        newParent.keys = new ArrayList<Float>();
        newParent.addChild(this);
        newParent.addChild(newNode);
        newParent.keys.add(newNode.getKeyAtIdx(0));
        this.setParent(newParent);
        newNode.setParent(newParent);
    }

    public void createRootNode(InternalNode newNode) {
        InternalNode newParent = new InternalNode();
        newParent.keys = new ArrayList<Float>();
        newParent.addChild(this);
        newParent.addChild(newNode);
        newParent.keys.add(newNode.getKeyAtIdx(0));
        this.setParent(newParent);
        newNode.setParent(newParent);

    }
 
    // split itself (this node) into two, and return the sibling
    public LeafNode splitLeafNodeHelper(Float key, Address addr) {
        LeafNode newNode = new LeafNode();
        ((LeafNode) this).addresses = new ArrayList<Address>();
        ((LeafNode) this).addresses.add(addr);
        ((LeafNode) this).keyAddrMap.put(key, ((LeafNode) this).addresses);

        // moves the remaining keys into a sibling node so that current node has the  min required number of keys
        int n = NODE_SIZE - minLeafNodeSize + 1;
        int i = 0;
        Float fromKey = 0.0f;

        for (Map.Entry<Float, ArrayList<Address>> entry : ((LeafNode) this).keyAddrMap.entrySet()) {
            if (i == n) {
                fromKey = entry.getKey();
                break;
            }
            i++;
        }

        // save the last n keys as a temporary variable to be added to newNode
        SortedMap<Float, ArrayList<Address>> lastnKeys = ((LeafNode) this).keyAddrMap.subMap(fromKey, true,
                ((LeafNode) this).keyAddrMap.lastKey(), true);
        newNode.keyAddrMap = new TreeMap<Float, ArrayList<Address>>(lastnKeys);
        lastnKeys.clear();

        insertKeyInOrder(this.keys, key);

        // move keys after the nth index from this node into sibling
        newNode.keys = new ArrayList<Float>(this.keys.subList(n, this.keys.size()));
        this.keys.subList(n, this.keys.size()).clear();

        // shift the leaf nodes to the right and set new node as this node's right sibling and vice versa
        if (((LeafNode) this).getRightSibling() != null) {
            newNode.setRightSibling(((LeafNode) this).getRightSibling());
            ((LeafNode) this).getRightSibling().setLeftSibling(newNode);
        }
        ((LeafNode) this).setRightSibling(newNode);
        newNode.setLeftSibling(((LeafNode) this));
        return newNode;
    }

    // similar to prev helper
    public InternalNode splitInternalNodeHelper() {

        InternalNode currentParent = (InternalNode) (this);
        InternalNode newParent = new InternalNode();
        newParent.keys = new ArrayList<Float>();

        Float keyToSplitAt = currentParent.getKeyAt(minInternalNodeSize);
        for (int k = currentParent.getKeyCount(); k > 0; k--) {
            if (currentParent.getKeyAt(k - 1) < keyToSplitAt) {
                break;
            }
            Float currentKey = currentParent.getKeyAt(k - 1);
            Node currentChild = currentParent.getChild(k);

            // add node and keys to new parent
            newParent.children.add(0, currentChild);
            newParent.keys.add(0, currentKey);
            currentChild.setParent(newParent);

            // remove node and keys from old parent
            currentParent.removeChild(currentParent.getChild(k));
            currentParent.keys.remove(k - 1);
        }

        return newParent;
    }
}