
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

    private void handleInvalidTree(NodeFunctions underUtilizedNode, InternalNode parent, int parentPointerIndex,
            int parentKeyIndex) throws IllegalStateException {
        if (parent == null) {
            handleInvalidRootNode(underUtilizedNode);
        } else if (underUtilizedNode.isLeaf()) {
            handleInvalidLeafNode(underUtilizedNode, parent,
                    parentPointerIndex, parentKeyIndex);
        } else if (!underUtilizedNode.isLeaf()) {
            handleInvalidInternalNode(underUtilizedNode, parent,
                    parentPointerIndex, parentKeyIndex);
        } else {
            throw new IllegalStateException("state is wrong!");
        }
    }

    public static void setRoot(NodeFunctions root) {
        rootNode = root;
        rootNode.setRoot(true);
    }

    public static NodeFunctions getRoot() {
        return rootNode;
    }

    private Float checkForLowerbound(Float key) {
        System.out.println("Entering checkForLowerbound function");
        InternalNode node = (InternalNode) rootNode;
        NodeFunctions targetNode = node;
        boolean found = false;
        for (int i = node.keys.size() - 1; i >= 0; i--) {
            if (key >= node.keys.get(i)) {
                targetNode = node.getChild(i + 1);
                found = true;
                break;
            }
        }
        ;
        if (!found && key < node.keys.get(0)) {
            targetNode = ((InternalNode) node).getChild(0);
        }
        System.out.println("found the largest key in node smaller than key");
        while (!node.getChild(0).isLeaf()) {
            targetNode = ((InternalNode) node).getChild(0);
        }
        System.out.println("looped completely till found leftmost key");

        if (targetNode.isLeaf()) {
            return targetNode.keys.get(0);
        } else {
            return ((InternalNode) targetNode).getChild(0).keys.get(0);
        }
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
                Float newLowerBound = checkForLowerbound(leafNode.keys.get(keyIdx + 1));
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
            handleInvalidTree(node, parent, parentPointerIndex, parentKeyIndex);
        }

        return addressesToDel;
    }

    public void handleInvalidRootNode(NodeFunctions underUtilizedNode) {
        if (underUtilizedNode.isLeaf()) {
            ((LeafNode) underUtilizedNode).clear();
        } else {
            InternalNode nonLeafRoot = (InternalNode) underUtilizedNode;
            NodeFunctions newRoot = nonLeafRoot.getChild(0);
            newRoot.setParent(null);
            rootNode = newRoot;
        }
    }

    private void handleInvalidLeafNode(NodeFunctions underUtilizedNode,
            InternalNode parent,
            int parentPointerIndex,
            int parentKeyIndex) {
        LeafNode underUtilizedLeaf = (LeafNode) underUtilizedNode;
        LeafNode leftSibling = (LeafNode) underUtilizedLeaf.getLeftSibling();
        LeafNode rightSibling = (LeafNode) underUtilizedLeaf.getRightSibling();
        if (leftSibling != null && leftSibling.canDonate(SizeofNode)) {
            moveOneKeyLeafNode(leftSibling, underUtilizedLeaf, true, parent, parentKeyIndex);
        } else if (rightSibling != null && rightSibling.canDonate(SizeofNode)) {
            moveOneKeyLeafNode(rightSibling, underUtilizedLeaf, false, parent, parentKeyIndex + 1);

        } else if (leftSibling != null && (leftSibling.keys.size() + underUtilizedLeaf.keys.size()) <= SizeofNode) {
            mergeLeafNodes(leftSibling, underUtilizedLeaf, parent, parentPointerIndex, parentKeyIndex, false);
        } else if (rightSibling != null
                && (rightSibling.keys.size() + underUtilizedLeaf.keys.size()) <= SizeofNode) {
            mergeLeafNodes(underUtilizedLeaf, rightSibling, parent, parentPointerIndex + 1, parentKeyIndex + 1, true);
        }
    }

    private void handleInvalidInternalNode(NodeFunctions underUtilizedNode,
            InternalNode parent,
            int parentPointerIndex,
            int parentKeyIndex) {

        NodeFunctions underUtilizedInternalNode = underUtilizedNode;

        InternalNode leftInNodeSibling = null;
        InternalNode rightInNodeSibling = null;
        try {
            rightInNodeSibling = (InternalNode) parent.getChild(parentPointerIndex + 1);
        } catch (Exception e) {
            System.out.print(e);
        }

        try {
            leftInNodeSibling = (InternalNode) parent.getChild(parentPointerIndex - 1);
        } catch (Exception e) {
            System.out.print(e);
        }

        if (rightInNodeSibling == null && leftInNodeSibling == null)
            throw new IllegalStateException(
                    "Both leftInNodeSibling and rightInNodeSibling is null for " + underUtilizedNode);

        if (leftInNodeSibling != null && leftInNodeSibling.canDonate(SizeofNode)) {
            moveOneKeyInternalNode(leftInNodeSibling, (InternalNode) underUtilizedInternalNode, true, parent,
                    parentKeyIndex);
        } else if (rightInNodeSibling != null && rightInNodeSibling.canDonate(SizeofNode)) {
            moveOneKeyInternalNode(rightInNodeSibling, (InternalNode) underUtilizedInternalNode, false, parent,
                    parentKeyIndex + 1);

        } else if (leftInNodeSibling != null
                && (underUtilizedInternalNode.keys.size() + leftInNodeSibling.keys.size()) <= SizeofNode) {
            mergeInternalNodes(leftInNodeSibling, (InternalNode) underUtilizedInternalNode, parent, parentPointerIndex,
                    parentKeyIndex, true);
        } else if (rightInNodeSibling != null
                && (underUtilizedInternalNode.keys.size() + rightInNodeSibling.keys.size()) <= SizeofNode) {
            mergeInternalNodes((InternalNode) underUtilizedInternalNode, rightInNodeSibling, parent,
                    parentPointerIndex + 1, parentKeyIndex + 1, false);
        }
    }

    private void moveOneKeyInternalNode(InternalNode donor, InternalNode receiver,
            boolean donorOnLeft, InternalNode parent,
            int inBetweenKeyIdx) {
        Float key;

        if (donorOnLeft) {
            donor.keys.remove(donor.keys.size() - 1);
            NodeFunctions nodeToMove = donor.getChild(donor.keys.size());
            donor.removeChild(nodeToMove);
            receiver.addChild(nodeToMove);

            receiver.keys.add(receiver.keys.size(), receiver.getChild(1).keys.get(0));
            key = receiver.keys.get(0);
        } else {
            donor.keys.remove(0);
            NodeFunctions nodeToMove = donor.getChild(0);
            donor.removeChild(nodeToMove);
            receiver.addChild(nodeToMove);

            receiver.keys.add(receiver.keys.size(), receiver.getChild(1).keys.get(0));
            key = receiver.keys.get(0);
        }

        int ptrIdx = receiver.getIdxOfKey(key, true);
        int keyIdx = ptrIdx - 1;

        Float lowerbound = checkForLowerbound(key);
        Float newLowerBound;
        if (receiver.keys.size() >= (keyIdx + 1)) {
            newLowerBound = lowerbound;
        } else {
            newLowerBound = checkForLowerbound(receiver.keys.get(keyIdx + 1));
            parent.updateKeyAt(inBetweenKeyIdx - 1, key, false, checkForLowerbound(key));
        }
        parent.keys.set(inBetweenKeyIdx, newLowerBound);

    }

    private void mergeInternalNodes(InternalNode targetNode, InternalNode sacrificialNode, InternalNode parent,
            int rightPointerIdx,
            int inBetweenKeyIdx, boolean targetNodeInsufficient) {
        if (targetNodeInsufficient) {
            targetNode.keys.add(parent.keys.get(inBetweenKeyIdx));
            targetNode.keys.addAll(sacrificialNode.keys);
            targetNode.getChildren().addAll(sacrificialNode.getChildren());

        } else {
            targetNode.keys.add(0, parent.keys.get(inBetweenKeyIdx));
            targetNode.keys.addAll(0, sacrificialNode.keys);
            targetNode.getChildren().addAll(0, sacrificialNode.getChildren());
        }

        parent.getChildren().remove(sacrificialNode);
        parent.keys.remove(inBetweenKeyIdx);

        sacrificialNode = null;
    }

    private void mergeLeafNodes(LeafNode targetNode, LeafNode sacrificialNode, InternalNode parent,
            int rightPointerIdx, int inBetweenKeyIdx, boolean targetNodeInsufficient) {

        targetNode.keys.addAll(sacrificialNode.keys);
        targetNode.keyAddrMap.putAll(sacrificialNode.keyAddrMap);

        if (sacrificialNode.getRightSibling() != null) {
            sacrificialNode.getRightSibling().setLeftSibling(targetNode);
        }
        targetNode.setRightSibling(sacrificialNode.getRightSibling());

        parent.getChildren().remove(sacrificialNode);
        parent.keys.remove(inBetweenKeyIdx);

        sacrificialNode = null;

    }

    private void moveOneKeyLeafNode(LeafNode donor, LeafNode receiver,
            boolean donorOnLeft, InternalNode parent,
            int inBetweenKeyIdx) {
        Float key;
        if (donorOnLeft) {
            Float donorKey = donor.keys.get(donor.keys.size() - 1);
            receiver.insertKeyAddrArrPair(donorKey, donor.getAddressesForKey(donorKey));
            donor.removeKeyInMap(donorKey);

            receiver.insertKeyAt(0, donorKey);
            donor.keys.remove(donor.keys.size() - 1);
            key = receiver.keys.get(0);
        } else {
            Float donorKey = donor.keys.get(0);
            receiver.insertKeyAddrArrPair(donorKey, donor.getAddressesForKey(donorKey));
            donor.removeKeyInMap(donorKey);

            receiver.insertKeyAt(receiver.keys.size(), donorKey);
            donor.keys.remove(0);
            key = donor.keys.get(0);
        }

        if (inBetweenKeyIdx == -1) {
        } else if (inBetweenKeyIdx >= 0) {
            if (parent.keys.size() == inBetweenKeyIdx) {
                parent.keys.set(inBetweenKeyIdx - 1, key);

                int lastParentChild = receiver.getParent().keys.size() - 1;
                Float lastParentChildKey = receiver.getParent().getChild(receiver.getParent().keys.size()).keys.get(0);
                if (!(donor.getParent().getChild(donor.getParent().getChildren().size() - 1).keys.get(0))
                        .equals(key)) {
                    receiver.getParent().keys.set(lastParentChild, lastParentChildKey);
                }
            } else {
                parent.keys.set(inBetweenKeyIdx, key);

                if (!(donor.getParent().getChild(inBetweenKeyIdx + 1).keys.get(0)).equals(key)) {
                    donor.getParent().keys.set(inBetweenKeyIdx,
                            donor.getParent().getChild(inBetweenKeyIdx + 1).keys.get(0));
                }
            }
        } else {
            parent.keys.set(inBetweenKeyIdx - 1, key);
        }

        int ptrIdx = receiver.getIdxOfKey(key, true);
        int keyIdx = ptrIdx - 1;

        LeafNode LeafNode = (LeafNode) receiver;

        if (LeafNode.keys.size() < (keyIdx + 1))
            parent.updateKeyAt(inBetweenKeyIdx - 1, parent.getChild(inBetweenKeyIdx).keys.get(0), false,
                    checkForLowerbound(key));

    }

    public ArrayList<Address> getAddresses(Float key) {
        return (searchValue(rootNode, key));
    }

    public ArrayList<Address> searchValue(NodeFunctions node, Float key) {
        BPTFunctions.addNodeReads();
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

    public ArrayList<Address> getAddressesForKeysBetween(NodeFunctions node,
            float minKey, float maxKey) {
        BPTFunctions.addIndexNodeReads();
        if (!node.isLeaf()) {
            int ptr = node.getIdxOfKey(minKey, true);
            NodeFunctions childNode = ((InternalNode) node).getChild(ptr);
            return getAddressesForKeysBetween(childNode, minKey, maxKey);
        } else {
            ArrayList<Address> addresses = new ArrayList<>();
            int ptr = node.getIdxOfKey(minKey, false);
            LeafNode leafNode = (LeafNode) node;
            while (true) {
                if (ptr == leafNode.keys.size()) {
                    if (leafNode.getRightSibling() == null)
                        break;
                    leafNode = (LeafNode) leafNode.getRightSibling();
                    BPTFunctions.addIndexNodeReads();
                    ptr = 0;
                }
                if (leafNode.keys.get(ptr) > maxKey)
                    break;
                Float key = leafNode.keys.get(ptr);
                addresses.addAll(leafNode.getAddressesForKey(key));
                ptr++;
            }
            return (addresses.isEmpty() ? null : addresses);
        }
    }

    public Collection<? extends Address> removeKeys(NodeFunctions rootNode2, Object object, int i, int j, Float key,
            Float key2) {
        // TODO Auto-generated method stub
        throw new UnsupportedOperationException("Unimplemented method 'removeKeys'");
    }
}
