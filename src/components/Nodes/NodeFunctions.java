package components.Nodes;

import java.util.ArrayList;
import java.util.TreeMap;

import components.BPTree.BPlusTree;
import components.DB.Address;

import java.util.Map;
import java.util.Set;
import java.util.SortedMap;

public class NodeFunctions {

    private int minLeafNodeSize;
    private int minInternalNodeSize;
    static final int SizeofNode = BPlusTree.SizeofNode;
    private boolean isLeaf;
    private boolean isRoot;
    private InternalNode parent;
    public ArrayList<Float> keys;
    NodeFunctions rootNode;

    public NodeFunctions() {
        this.rootNode = BPlusTree.getRoot();
        this.isLeaf = false;
        this.isRoot = false;
        this.minLeafNodeSize = (SizeofNode + 1) / 2;
        this.minInternalNodeSize = SizeofNode / 2;
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
        Float childToSort = child.keys.get(0);
        while (i < parent.keys.size() && parent.keys.get(i) < childToSort) {
            i++;
        }
        parent.children.add(i + 1, child);
    }

    public void updateKeyAt(int keyIndex, Float newKey, boolean leafNotUpdated, Float lowerbound) {
        if (keyIndex >= 0 && keyIndex < keys.size() && !leafNotUpdated) {
            keys.set(keyIndex, newKey);
        }
        if (parent != null && !parent.isLeaf()) {
            int childIndex = parent.getChildren().indexOf(this);
            if (childIndex >= 0) {
                if (childIndex > 0) {
                    parent.keys.set(childIndex - 1, keys.get(0));
                }
                parent.updateKeyAt(childIndex - 1, newKey, false, lowerbound);
            }
        } else if (parent != null && parent.isLeaf()) {
            parent.updateKeyAt(keyIndex, newKey, false, lowerbound);
        }
    }

    public boolean isUnderUtilized(int maxKeyCount) {
        if (isRoot()) {
            return (this.keys.size() < 1);
        } else if (isLeaf()) {
            return (this.keys.size() < (maxKeyCount + 1) / 2);
        } else {
            return (this.keys.size() < maxKeyCount / 2);
        }
    }

    public boolean canDonate(int maxKeyCount) {
        if (!isLeaf())
            return keys.size() - 1 >= maxKeyCount / 2;
        return keys.size() - 1 >= (maxKeyCount + 1) / 2;
    }

    public void insertNewNodeToParent(LeafNode newNode) {
        int index = 0;
        boolean insertedNode = false;

        try {
            for (NodeFunctions currentNode : this.getParent().getChildren()) {
                if (newNode.keys.get(newNode.keys.size() - 1) < currentNode.keys.get(0)) {
                    this.getParent().getChildren().add(index, newNode);
                    this.getParent().keys.add(index - 1, newNode.keys.get(0));
                    insertedNode = true;
                    break;
                }
                index++;
            }

            if (insertedNode == false) {
                this.getParent().getChildren().add(newNode);
                this.getParent().keys.add(newNode.keys.get(0));
            }

        } catch (Exception e) {
            this.getParent().getChildren().add(newNode);
            this.getParent().keys.add(newNode.keys.get(0));
        }

        newNode.setParent(this.getParent());

        if (this.getParent().keys.size() > SizeofNode) {
            this.getParent().splitInternalNode();
        }
    }

    public void splitLeafNode(Float key, Address addr) {

        LeafNode newNode = this.splitLeafNodeHelper(key, addr);

        if (this.getParent() != null) {
            this.insertNewNodeToParent(newNode);
            if (this.getParent().keys.size() > SizeofNode) {
                this.getParent().splitInternalNode();
            }
        } else {
            this.createFirstParentNode(newNode);
        }

    }

    public void splitInternalNode() {
        InternalNode sibling = this.splitInternalNodeHelper();
        if (this.getParent() != null) {
            insertChildInOrder(this.getParent(), sibling);
            sibling.setParent(this.getParent());
            insertKeyInOrder(this.getParent().keys, sibling.keys.get(0));
            sibling.keys.remove(0);
            if (this.getParent().keys.size() > SizeofNode) {
                this.getParent().splitInternalNode();
            }
        } else {
            InternalNode newRoot = new InternalNode();
            newRoot.keys = new ArrayList<Float>();
            newRoot.keys.add(sibling.keys.get(0));
            sibling.keys.remove(0);
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

    private int binSearchKeyIdx(int left, int right, Float key, boolean upperBound) {
        if (left > right)
            return left;
        int middle = (left + right) / 2;
        Float middleKey = keys.get(middle);

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

    public void printNode() {
        Set<Float> keys = ((LeafNode) this).keyAddrMap.keySet();
        System.out.println(keys);
    }

    public void createFirstParentNode(LeafNode newNode) {
        InternalNode newParent = new InternalNode();
        newParent.keys = new ArrayList<Float>();
        newParent.addChild(this);
        newParent.addChild(newNode);
        newParent.keys.add(newNode.keys.get(0));
        this.setParent(newParent);
        newNode.setParent(newParent);
    }

    public void createRootNode(InternalNode newNode) {
        InternalNode newParent = new InternalNode();
        newParent.keys = new ArrayList<Float>();
        newParent.addChild(this);
        newParent.addChild(newNode);
        newParent.keys.add(newNode.keys.get(0));
        this.setParent(newParent);
        newNode.setParent(newParent);

    }

    public LeafNode splitLeafNodeHelper(Float key, Address addr) {
        LeafNode newNode = new LeafNode();
        ((LeafNode) this).addresses = new ArrayList<Address>();
        ((LeafNode) this).addresses.add(addr);
        ((LeafNode) this).keyAddrMap.put(key, ((LeafNode) this).addresses);
        int n = SizeofNode - minLeafNodeSize + 1;
        int i = 0;
        Float fromKey = 0.0f;

        for (Map.Entry<Float, ArrayList<Address>> entry : ((LeafNode) this).keyAddrMap.entrySet()) {
            if (i == n) {
                fromKey = entry.getKey();
                break;
            }
            i++;
        }

        SortedMap<Float, ArrayList<Address>> lastnKeys = ((LeafNode) this).keyAddrMap.subMap(fromKey, true,
                ((LeafNode) this).keyAddrMap.lastKey(), true);
        newNode.keyAddrMap = new TreeMap<Float, ArrayList<Address>>(lastnKeys);
        lastnKeys.clear();

        insertKeyInOrder(this.keys, key);

        newNode.keys = new ArrayList<Float>(this.keys.subList(n, this.keys.size()));
        this.keys.subList(n, this.keys.size()).clear();

        if (((LeafNode) this).getRightSibling() != null) {
            newNode.setRightSibling(((LeafNode) this).getRightSibling());
            ((LeafNode) this).getRightSibling().setLeftSibling(newNode);
        }
        ((LeafNode) this).setRightSibling(newNode);
        newNode.setLeftSibling(((LeafNode) this));
        return newNode;
    }

    public InternalNode splitInternalNodeHelper() {

        InternalNode currentParent = (InternalNode) (this);
        InternalNode newParent = new InternalNode();
        newParent.keys = new ArrayList<Float>();

        Float keyToSplitAt = currentParent.keys.get(minInternalNodeSize);
        for (int k = currentParent.keys.size(); k > 0; k--) {
            if (currentParent.keys.get(k - 1) < keyToSplitAt) {
                break;
            }
            Float currentKey = currentParent.keys.get(k - 1);
            NodeFunctions currentChild = currentParent.getChild(k);

            newParent.children.add(0, currentChild);
            newParent.keys.add(0, currentKey);
            currentChild.setParent(newParent);

            currentParent.removeChild(currentParent.getChild(k));
            currentParent.keys.remove(k - 1);
        }

        return newParent;
    }
}