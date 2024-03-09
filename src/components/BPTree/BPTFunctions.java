package components.BPTree;

import java.util.ArrayList;

import components.Database.Address;
import components.Nodes.InternalNode;
import components.Nodes.LeafNode;
import components.Nodes.NodeFunctions;
import utils.Driver;

public class BPTFunctions {

    private static int nodeReads;

    public int getNodeReads() {
        return nodeReads;
    }

    static void incrementNodeReads() {
        nodeReads++;
    }

    public static ArrayList<Address> getAddressesForKeysBetween(NodeFunctions node,
            float minKey, float maxKey) {
        Driver.indexCalls++;
        BPTFunctions.incrementNodeReads();
        if (!node.isLeaf()) {
            int ptr = node.getIndexOfKey(minKey, true);
            NodeFunctions childNode = ((InternalNode) node).getChild(ptr);
            return getAddressesForKeysBetween(childNode, minKey, maxKey);
        } else {
            ArrayList<Address> addresses = new ArrayList<>();
            int ptr = node.getIndexOfKey(minKey, false);
            LeafNode leafNode = (LeafNode) node;
            while (true) {
                if (ptr == leafNode.keys.size()) {
                    if (leafNode.getRightSibling() == null)
                        break;
                    leafNode = (LeafNode) leafNode.getRightSibling();
                    Driver.indexCalls++;
                    BPTFunctions.incrementNodeReads();
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

    public static void handleInvalidTree(NodeFunctions underUtilizedNode, InternalNode parent, int parentPointerIndex,
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

    public static void handleInvalidRootNode(NodeFunctions underUtilizedNode) {
        if (underUtilizedNode.isLeaf()) {
            ((LeafNode) underUtilizedNode).clear();
        } else {
            InternalNode nonLeafRoot = (InternalNode) underUtilizedNode;
            NodeFunctions newRoot = nonLeafRoot.getChild(0);
            newRoot.setParent(null);
            BPlusTree.rootNode = newRoot;
        }
    }

    private static void handleInvalidLeafNode(NodeFunctions underUtilizedNode,
            InternalNode parent,
            int parentPointerIndex,
            int parentKeyIndex) {
        LeafNode underUtilizedLeaf = (LeafNode) underUtilizedNode;
        LeafNode leftSibling = (LeafNode) underUtilizedLeaf.getLeftSibling();
        LeafNode rightSibling = (LeafNode) underUtilizedLeaf.getRightSibling();
        if (leftSibling != null && leftSibling.canGiveKey()) {
            moveOneKeyLeafNode(leftSibling, underUtilizedLeaf, true, parent, parentKeyIndex);
        } else if (rightSibling != null && rightSibling.canGiveKey()) {
            moveOneKeyLeafNode(rightSibling, underUtilizedLeaf, false, parent, parentKeyIndex + 1);

        } else if (leftSibling != null
                && (leftSibling.keys.size() + underUtilizedLeaf.keys.size()) <= BPlusTree.SizeofNode) {
            mergeLeafNodes(leftSibling, underUtilizedLeaf, parent, parentPointerIndex, parentKeyIndex, false);
        } else if (rightSibling != null
                && (rightSibling.keys.size() + underUtilizedLeaf.keys.size()) <= BPlusTree.SizeofNode) {
            mergeLeafNodes(underUtilizedLeaf, rightSibling, parent, parentPointerIndex + 1, parentKeyIndex + 1, true);
        }
    }

    private static void handleInvalidInternalNode(NodeFunctions underUtilizedNode,
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

        if (leftInNodeSibling != null && leftInNodeSibling.canGiveKey()) {
            moveOneKeyInternalNode(leftInNodeSibling, (InternalNode) underUtilizedInternalNode, true, parent,
                    parentKeyIndex);
        } else if (rightInNodeSibling != null && rightInNodeSibling.canGiveKey()) {
            moveOneKeyInternalNode(rightInNodeSibling, (InternalNode) underUtilizedInternalNode, false, parent,
                    parentKeyIndex + 1);

        } else if (leftInNodeSibling != null
                && (underUtilizedInternalNode.keys.size() + leftInNodeSibling.keys.size()) <= BPlusTree.SizeofNode) {
            mergeInternalNodes(leftInNodeSibling, (InternalNode) underUtilizedInternalNode, parent, parentPointerIndex,
                    parentKeyIndex, true);
        } else if (rightInNodeSibling != null
                && (underUtilizedInternalNode.keys.size() + rightInNodeSibling.keys.size()) <= BPlusTree.SizeofNode) {
            mergeInternalNodes((InternalNode) underUtilizedInternalNode, rightInNodeSibling, parent,
                    parentPointerIndex + 1, parentKeyIndex + 1, false);
        }
    }

    private static void moveOneKeyInternalNode(InternalNode donor, InternalNode receiver,
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

        int pointerIndex = receiver.getIndexOfKey(key, true);
        int keyIndex = pointerIndex - 1;

        Float lowerbound = checkForLowerbound(key);
        Float newLowerBound;
        if (receiver.keys.size() >= (keyIndex + 1)) {
            newLowerBound = lowerbound;
        } else {
            newLowerBound = checkForLowerbound(receiver.keys.get(keyIndex + 1));
            parent.updateKeyAt(inBetweenKeyIdx - 1, key, false, checkForLowerbound(key));
        }
        parent.keys.set(inBetweenKeyIdx, newLowerBound);

    }

    private static void mergeInternalNodes(InternalNode targetNode, InternalNode sacrificialNode, InternalNode parent,
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

    private static void mergeLeafNodes(LeafNode targetNode, LeafNode sacrificialNode, InternalNode parent,
            int rightPointerIdx, int inBetweenKeyIdx, boolean targetNodeInsufficient) {

        targetNode.keys.addAll(sacrificialNode.keys);
        targetNode.keyAddressMap.putAll(sacrificialNode.keyAddressMap);

        if (sacrificialNode.getRightSibling() != null) {
            sacrificialNode.getRightSibling().setLeftSibling(targetNode);
        }
        targetNode.setRightSibling(sacrificialNode.getRightSibling());

        parent.getChildren().remove(sacrificialNode);
        parent.keys.remove(inBetweenKeyIdx);

        sacrificialNode = null;

    }

    private static void moveOneKeyLeafNode(LeafNode donor, LeafNode receiver,
            boolean donorOnLeft, InternalNode parent,
            int inBetweenKeyIdx) {
        Float key;
        if (donorOnLeft) {
            Float donorKey = donor.keys.get(donor.keys.size() - 1);
            receiver.insertToKeyAddressMap(donorKey, donor.getAddressesForKey(donorKey));
            donor.removeKeyFromMap(donorKey);

            receiver.insertKeyAt(0, donorKey);
            donor.keys.remove(donor.keys.size() - 1);
            key = receiver.keys.get(0);
        } else {
            Float donorKey = donor.keys.get(0);
            receiver.insertToKeyAddressMap(donorKey, donor.getAddressesForKey(donorKey));
            donor.removeKeyFromMap(donorKey);

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

        int pointerIndex = receiver.getIndexOfKey(key, true);
        int keyIndex = pointerIndex - 1;

        LeafNode LeafNode = (LeafNode) receiver;

        if (LeafNode.keys.size() < (keyIndex + 1))
            parent.updateKeyAt(inBetweenKeyIdx - 1, parent.getChild(inBetweenKeyIdx).keys.get(0), false,
                    checkForLowerbound(key));

    }

    public static Float checkForLowerbound(Float key) {
        System.out.println("Entering checkForLowerbound function");
        InternalNode node = (InternalNode) BPlusTree.rootNode;
        NodeFunctions targetNode = node;
        boolean found = false;
        for (int i = node.keys.size() - 1; i >= 0; i--) {
            if (key >= node.keys.get(i)) {
                targetNode = node.getChild(i + 1);
                found = true;
                break;
            }
        }
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

}
