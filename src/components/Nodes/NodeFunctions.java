package components.Nodes;

import java.util.ArrayList;
import java.util.TreeMap;

import components.BPTree.BPlusTree;
import components.Database.Address;

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

    public void insertKeyloc(int index, Float key) {
        keys.add(index, key);
    }

    public static void insertKeyInOrder(ArrayList<Float> keys, Float key) {
        int index = 0;
        while (index < keys.size() && keys.get(index) < key) {
            index++;
        }
        keys.add(index, key);
    }

    public void insertChildOrder(InternalNode parent, InternalNode child) {
        int index = 0;
        Float childToInsert = child.keys.get(0);
        while (index < parent.keys.size() && parent.keys.get(index) < childToInsert) {
            index++;
        }
        parent.children.add(index + 1, child);
    }
    public void insertNewNode(LeafNode newNode) {
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

            if (!insertedNode) {
                this.getParent().getChildren().add(newNode);
                this.getParent().keys.add(newNode.keys.get(0));
            }

        } catch (Exception e) {
            this.getParent().getChildren().add(newNode);
            this.getParent().keys.add(newNode.keys.get(0));
        }

        newNode.setParent(this.getParent());

        if (this.getParent().keys.size() > SizeofNode) {
            this.getParent().splitInternal();
        }
    }

    public int getIndexOfKey(Float key, boolean upperBound) {
        return binarySearchKeyIndex(0, keys.size() - 1, key, upperBound);
    }


    public void updateKeyAt(int keyIndex, Float newKey, boolean isLeafUpdated, Float lowerbound) {
        if (keyIndex >= 0 && keyIndex < keys.size() && !isLeafUpdated) {
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

    public boolean hasInsufficientKeys(int maxKeyCount) {
        if (isRoot()) {
            return (this.keys.size() < 1);
        } else if (isLeaf()) {
            return (this.keys.size() < (maxKeyCount + 1) / 2);
        } else {
            return (this.keys.size() < maxKeyCount / 2);
        }
    }

    public boolean canGiveKey() {
        int maxKeyCount = BPlusTree.SizeofNode;
        if (!isLeaf())
            return keys.size() - 1 >= maxKeyCount / 2;
        return keys.size() - 1 >= (maxKeyCount + 1) / 2;
    }

    public void splitLeaf(Float key, Address address) {

        LeafNode newNode = this.getNewLeaf(key, address);

        if (this.getParent() != null) {
            this.insertNewNode(newNode);
            if (this.getParent().keys.size() > SizeofNode) {
                this.getParent().splitInternal();
            }
        } else {
            this.createParentNode(newNode);
        }

    }

    public void splitInternal() {
        InternalNode sibling = this.getNewSibling();
        if (this.getParent() != null) {
            insertChildOrder(this.getParent(), sibling);
            sibling.setParent(this.getParent());
            insertKeyInOrder(this.getParent().keys, sibling.keys.get(0));
            sibling.keys.remove(0);
            if (this.getParent().keys.size() > SizeofNode) {
                this.getParent().splitInternal();
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

            BPlusTree.rootNode = newRoot;
            BPlusTree.rootNode.setRoot(true);
        }
    }

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
            BPlusTree.rootNode = parentNode;
            BPlusTree.rootNode.setRoot(true);
        } else {
            parentNode.setLeaf(false);
        }
        this.parent = parentNode;
    }

    private int binarySearchKeyIndex(int left, int right, Float key, boolean upperBound) {
        if (left > right)
            return left;
        int middle = (left + right) / 2;
        Float middleKey = keys.get(middle);

        if (middleKey < key) {
            return binarySearchKeyIndex(middle + 1, right, key, upperBound);
        } else if (middleKey > key) {
            return binarySearchKeyIndex(left, middle - 1, key, upperBound);
        } else {
            while (middle < keys.size() && keys.get(middle).equals(key))
                middle++;
            if (!upperBound)
                return middle - 1;
            return middle;
        }
    }

    public void printNode() {
        Set<Float> keys = ((LeafNode) this).keyAddressMap.keySet();
        System.out.println(keys);
    }

    public void createParentNode(LeafNode newNode) {
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

    public LeafNode getNewLeaf(Float key, Address address) {
        LeafNode newNode = new LeafNode();
        ((LeafNode) this).addresses = new ArrayList<Address>();
        ((LeafNode) this).addresses.add(address);
        ((LeafNode) this).keyAddressMap.put(key, ((LeafNode) this).addresses);
        int n = SizeofNode - minLeafNodeSize + 1;
        int index = 0;
        Float fromKey = 0.0f;

        for (Map.Entry<Float, ArrayList<Address>> entry : ((LeafNode) this).keyAddressMap.entrySet()) {
            if (index == n) {
                fromKey = entry.getKey();
                break;
            }
            index++;
        }

        SortedMap<Float, ArrayList<Address>> lastnKeys = ((LeafNode) this).keyAddressMap.subMap(fromKey, true,
                ((LeafNode) this).keyAddressMap.lastKey(), true);
        newNode.keyAddressMap = new TreeMap<Float, ArrayList<Address>>(lastnKeys);
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

    public InternalNode getNewSibling() {

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