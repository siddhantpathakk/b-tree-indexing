
package components.BPTree;

import java.util.ArrayList;
import java.util.List;

import components.Database.Address;
import components.Nodes.*;
import utils.Reader;

public class BPlusTree {

    public static final int SizeofNode = (Reader.SizeofBlock - Reader.SizeofPointer)
            / (Reader.SizeofPointer + Reader.SizeofKey);
    public static NodeFunctions rootNode;
    NodeFunctions nodeToInsertTo;

    public BPlusTree() {
        LeafNode root = new LeafNode();
        root.setRoot(true);
        root.setLeaf(true);
        rootNode.setRoot(true);
        rootNode = root;
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
            rootNode.setRoot(true);
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

    public static NodeFunctions getRoot() {
        return rootNode;
    }

    public ArrayList<Address> deleteKeyRecursive(NodeFunctions node, InternalNode parent, int parentPointerIndex,
            int parentKeyIndex,
            Float key, Float lowerbound) {

        ArrayList<Address> addressesToDelete = new ArrayList<>();

        if (node.isLeaf()) {
            LeafNode leafNode = (LeafNode) node;
            int keyIndex = node.getIndexOfKey(key, false);
            if ((keyIndex == leafNode.keys.size()) || (!key.equals(leafNode.keys.get(keyIndex)))) {
                return null;
            }

            addressesToDelete.addAll(leafNode.getAddressesForKey(key));
            leafNode.keys.remove(keyIndex);
            leafNode.removeKeyFromMap(key);

            int pointerIndex = node.getIndexOfKey(key, true);
            keyIndex = pointerIndex - 1;

            if (leafNode.keys.size() >= (keyIndex + 1)) {
                Float newLowerBound = lowerbound;
                List<Float> keys = leafNode.keys;
                leafNode.updateKeyAt(pointerIndex - 1, keys.get(0), false, newLowerBound);
            } else {
                Float newLowerBound = BPTFunctions.checkForLowerbound(leafNode.keys.get(keyIndex + 1));
                List<Float> keys = leafNode.keys;
                leafNode.updateKeyAt(pointerIndex - 1, keys.get(0), true, newLowerBound);
            }
        } else {

            InternalNode nonLeafNode = (InternalNode) node;
            int pointerIndex = node.getIndexOfKey(key, true);
            int keyIndex = pointerIndex - 1;

            NodeFunctions next = nonLeafNode.getChild(pointerIndex);
            addressesToDelete = deleteKeyRecursive(next, nonLeafNode, pointerIndex, keyIndex, key, lowerbound);
        }

        if (node.hasInsufficientKeys(SizeofNode)) {
            BPTFunctions.fixInvalidTree(node, parent, parentPointerIndex, parentKeyIndex);
        }

        return addressesToDelete;
    }

    public ArrayList<Address> getAddresses(Float key) {
        return (searchValue(rootNode, key));
    }

    public ArrayList<Address> searchValue(NodeFunctions node, Float key) {
        BPTFunctions.incrementNodeReads();
        if (node.isLeaf()) {
            int pointerIndex = node.getIndexOfKey(key, false);
            if (pointerIndex >= 0 && pointerIndex < node.keys.size() && key.equals(node.keys.get(pointerIndex))) {
                return ((LeafNode) node).getAddressesForKey(key);
            }
            return null;
        } else {
            int pointerIndex = node.getIndexOfKey(key, false);
            NodeFunctions childNode = ((InternalNode) node).getChild(pointerIndex);
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
}
