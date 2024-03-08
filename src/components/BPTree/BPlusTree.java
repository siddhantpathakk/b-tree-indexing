
package components.BPTree;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import components.DB.Address;
import components.Nodes.*;
import utils.Reader;

public class BPlusTree {

    public static final int SizeofNode = (Reader.SizeofBlock - Reader.SizeofPointer)
            / (Reader.SizeofPointer + Reader.SizeofKey);
    public static NodeFunctions rootNode;
    NodeFunctions nodeToInsertTo;

    public BPlusTree() {
        rootNode = createFirstNode();
    }

    public LeafNode createFirstNode() {
        LeafNode newNode = new LeafNode();
        newNode.setRoot(true);
        newNode.setLeaf(true);
        setRoot(newNode);
        return newNode;
    }

    public static NodeFunctions createNode() {
        NodeFunctions newNode = new NodeFunctions();
        return newNode;
    }

    public void insertKeyAddrPair(float key, Address add) {
        nodeToInsertTo = searchNodeContaining(key);
        ((LeafNode) nodeToInsertTo).addRecord(key, add);
    }

    public LeafNode searchNodeContaining(Float key) {
        ArrayList<Float> keys;

        if (BPlusTree.rootNode.isLeaf()) {
            setRoot(rootNode);
            return (LeafNode) rootNode;
        } else {
            NodeFunctions nodeToInsertTo = (InternalNode) getRoot();

            while (!((InternalNode) nodeToInsertTo).getChild(0).isLeaf()) {
                keys = nodeToInsertTo.keys;
                for (int i = keys.size() - 1; i >= 0; i--) {

                    if (nodeToInsertTo.keys.get(i) <= key) {
                        nodeToInsertTo = ((InternalNode) nodeToInsertTo).getChild(i + 1);
                        break;
                    } else if (i == 0) {
                        nodeToInsertTo = ((InternalNode) nodeToInsertTo).getChild(0);
                    }
                }
                if (nodeToInsertTo.isLeaf()) {
                    break;
                }
            }

            keys = nodeToInsertTo.keys;
            for (int i = keys.size() - 1; i >= 0; i--) {
                if (keys.get(i) <= key) {
                    return (LeafNode) ((InternalNode) nodeToInsertTo).getChild(i + 1);
                }
            }
            return (LeafNode) ((InternalNode) nodeToInsertTo).getChild(0);
        }
    }

    public static void setRoot(NodeFunctions root) {
        rootNode = root;
        rootNode.setRoot(true);
    }

    public static NodeFunctions getRoot() {
        return rootNode;
    }

    public ArrayList<Address> deleteKeyRecursive(NodeFunctions node, InternalNode parent, int parentPointerIndex,
            int parentKeyIndex,
            Float key, Float lowerbound) {

        ArrayList<Address> addressesToDel = new ArrayList<>();

        if (node.isLeaf()) {
            LeafNode leafNode = (LeafNode) node;
            int keyIdx = node.getIdxOfKey(key, false);
            if ((keyIdx == leafNode.keys.size()) || (!key.equals(leafNode.keys.get(keyIdx)))) {
                return null;
            }

            addressesToDel.addAll(leafNode.getAddressesForKey(key));
            leafNode.keys.remove(keyIdx);
            leafNode.removeKeyInMap(key);

            int ptrIdx = node.getIdxOfKey(key, true);
            keyIdx = ptrIdx - 1;

            if (leafNode.keys.size() >= (keyIdx + 1)) {
                Float newLowerBound = lowerbound;
                List<Float> keys = leafNode.keys;
                leafNode.updateKeyAt(ptrIdx - 1, keys.get(0), false, newLowerBound);
            } else {
                Float newLowerBound = BPTFunctions.checkForLowerbound(leafNode.keys.get(keyIdx + 1));
                List<Float> keys = leafNode.keys;
                leafNode.updateKeyAt(ptrIdx - 1, keys.get(0), true, newLowerBound);
            }
        } else {

            InternalNode nonLeafNode = (InternalNode) node;
            int ptrIdx = node.getIdxOfKey(key, true);
            int keyIdx = ptrIdx - 1;

            NodeFunctions next = nonLeafNode.getChild(ptrIdx);
            addressesToDel = deleteKeyRecursive(next, nonLeafNode, ptrIdx, keyIdx, key, lowerbound);
        }

        if (node.isUnderUtilized(SizeofNode)) {
            BPTFunctions.handleInvalidTree(node, parent, parentPointerIndex, parentKeyIndex);
        }

        return addressesToDel;
    }

    public ArrayList<Address> getAddresses(Float key) {
        return (searchValue(rootNode, key));
    }

    public ArrayList<Address> searchValue(NodeFunctions node, Float key) {
        BPTFunctions.incrementNodeReads();
        if (node.isLeaf()) {
            int ptrIdx = node.getIdxOfKey(key, false);
            if (ptrIdx >= 0 && ptrIdx < node.keys.size() && key.equals(node.keys.get(ptrIdx))) {
                return ((LeafNode) node).getAddressesForKey(key);
            }
            return null;
        } else {
            int ptrIdx = node.getIdxOfKey(key, false);
            NodeFunctions childNode = ((InternalNode) node).getChild(ptrIdx);
            return (searchValue(childNode, key));
        }
    }

    public int countNodes(NodeFunctions node) {
        int count = 1;
        if (node.isLeaf()) {
            return count;
        }
        for (NodeFunctions child : ((InternalNode) node).getChildren()) {
            count += countNodes(child);
        }
        return count;
    }

    public int getDepth(NodeFunctions node) {
        int level = 0;
        while (!node.isLeaf()) {
            node = ((InternalNode) node).getChild(0);
            level++;
        }
        level++;
        return level;
    }

    public Collection<? extends Address> removeKeys(NodeFunctions rootNode2, Object object, int i, int j, Float key,
            Float key2) {
        // TODO Auto-generated method stub
        throw new UnsupportedOperationException("Unimplemented method 'removeKeys'");
    }
}
