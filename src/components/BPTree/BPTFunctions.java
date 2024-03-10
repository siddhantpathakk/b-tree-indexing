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
            int pointer = node.getIndexOfKey(minKey, true);
            NodeFunctions childNode = ((InternalNode) node).getChild(pointer);
            return getAddressesForKeysBetween(childNode, minKey, maxKey);
        } else {
            ArrayList<Address> addresses = new ArrayList<>();
            int pointer = node.getIndexOfKey(minKey, false);
            LeafNode leafNode = (LeafNode) node;
            while (true) {
                if (pointer == leafNode.keys.size()) {
                    if (leafNode.getRightSibling() == null)
                        break;
                    leafNode = (LeafNode) leafNode.getRightSibling();
                    Driver.indexCalls++;
                    BPTFunctions.incrementNodeReads();
                    pointer = 0;
                }
                if (leafNode.keys.get(pointer) > maxKey)
                    break;
                Float key = leafNode.keys.get(pointer);
                addresses.addAll(leafNode.keyAddressMap.get(key));
                pointer++;
            }
            return (addresses.isEmpty() ? null : addresses);
        }
    }

    public static void fixInvalidTree(NodeFunctions invalidNode, InternalNode parent, int parentPointerIndex,
            int parentKeyIndex) throws IllegalStateException {
        if (parent == null) {
            fixInvalidRootNode(invalidNode);
        } else if (invalidNode.isLeaf()) {
            fixInvalidLeafNode(invalidNode, parent,
                    parentPointerIndex, parentKeyIndex);
        } else if (!invalidNode.isLeaf()) {
            handleInvalidInternalNode(invalidNode, parent,
                    parentPointerIndex, parentKeyIndex);
        } else {
            throw new IllegalStateException("state is wrong!");
        }
    }

    public static void fixInvalidRootNode(NodeFunctions invalidNode) {
        if (invalidNode.isLeaf()) {
            ((LeafNode) invalidNode).clear();
        } else {
            InternalNode nonLeafRoot = (InternalNode) invalidNode;
            NodeFunctions newRoot = nonLeafRoot.getChild(0);
            newRoot.setParent(null);
            BPlusTree.rootNode = newRoot;
        }
    }

    private static void fixInvalidLeafNode(NodeFunctions invalidNode,
            InternalNode parent,
            int parentPointerIndex,
            int parentKeyIndex) {
        LeafNode underUtilizedLeaf = (LeafNode) invalidNode;
        LeafNode leftSibling = (LeafNode) underUtilizedLeaf.getLeftSibling();
        LeafNode rightSibling = (LeafNode) underUtilizedLeaf.getRightSibling();
        if (leftSibling != null && leftSibling.canGiveKey()) {
            moveKeyInLeaf(leftSibling, underUtilizedLeaf, true, parent, parentKeyIndex);
        } else if (rightSibling != null && rightSibling.canGiveKey()) {
            moveKeyInLeaf(rightSibling, underUtilizedLeaf, false, parent, parentKeyIndex + 1);

        } else if (leftSibling != null
                && (leftSibling.keys.size() + underUtilizedLeaf.keys.size()) <= BPlusTree.SizeofNode) {
            mergeLeafNodes(leftSibling, underUtilizedLeaf, parent, parentPointerIndex, parentKeyIndex, false);
        } else if (rightSibling != null
                && (rightSibling.keys.size() + underUtilizedLeaf.keys.size()) <= BPlusTree.SizeofNode) {
            mergeLeafNodes(underUtilizedLeaf, rightSibling, parent, parentPointerIndex + 1, parentKeyIndex + 1, true);
        }
    }

    private static void handleInvalidInternalNode(NodeFunctions invalidNode,
            InternalNode parent,
            int parentPointerIndex,
            int parentKeyIndex) {

        NodeFunctions underUtilizedInternalNode = invalidNode;

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
                    "Both leftInNodeSibling and rightInNodeSibling is null for " + invalidNode);

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

    private static void moveOneKeyInternalNode(InternalNode lender, InternalNode borrower,
            boolean leftLender, InternalNode parent,
            int insideKeyIndex) {
        Float key;

        if (leftLender) {
            lender.keys.remove(lender.keys.size() - 1);
            NodeFunctions nodeToMove = lender.getChild(lender.keys.size());
            lender.removeChild(nodeToMove);
            borrower.addChild(nodeToMove);

            borrower.keys.add(borrower.keys.size(), borrower.getChild(1).keys.get(0));
            key = borrower.keys.get(0);
        } else {
            lender.keys.remove(0);
            NodeFunctions nodeToMove = lender.getChild(0);
            lender.removeChild(nodeToMove);
            borrower.addChild(nodeToMove);

            borrower.keys.add(borrower.keys.size(), borrower.getChild(1).keys.get(0));
            key = borrower.keys.get(0);
        }

        int pointerIndex = borrower.getIndexOfKey(key, true);
        int keyIndex = pointerIndex - 1;

        Float lowerbound = checkForLowerbound(key);
        Float newLowerBound;
        if (borrower.keys.size() >= (keyIndex + 1)) {
            newLowerBound = lowerbound;
        } else {
            newLowerBound = checkForLowerbound(borrower.keys.get(keyIndex + 1));
            parent.updateKeyAt(insideKeyIndex - 1, key, false, checkForLowerbound(key));
        }
        parent.keys.set(insideKeyIndex, newLowerBound);

    }

    private static void mergeInternalNodes(InternalNode targetNode, InternalNode sacrificialNode, InternalNode parent,
            int rightPointerIdx,
            int insideKeyIndex, boolean targetNodeInsufficient) {
        if (targetNodeInsufficient) {
            targetNode.keys.add(parent.keys.get(insideKeyIndex));
            targetNode.keys.addAll(sacrificialNode.keys);
            targetNode.getChildren().addAll(sacrificialNode.getChildren());

        } else {
            targetNode.keys.add(0, parent.keys.get(insideKeyIndex));
            targetNode.keys.addAll(0, sacrificialNode.keys);
            targetNode.getChildren().addAll(0, sacrificialNode.getChildren());
        }

        parent.getChildren().remove(sacrificialNode);
        parent.keys.remove(insideKeyIndex);

        sacrificialNode = null;
    }

    private static void mergeLeafNodes(LeafNode targetNode, LeafNode sacrificialNode, InternalNode parent,
            int rightPointerIdx, int insideKeyIndex, boolean targetNodeInsufficient) {

        targetNode.keys.addAll(sacrificialNode.keys);
        targetNode.keyAddressMap.putAll(sacrificialNode.keyAddressMap);

        if (sacrificialNode.getRightSibling() != null) {
            sacrificialNode.getRightSibling().setLeftSibling(targetNode);
        }
        targetNode.setRightSibling(sacrificialNode.getRightSibling());

        parent.getChildren().remove(sacrificialNode);
        parent.keys.remove(insideKeyIndex);

        sacrificialNode = null;

    }

    private static void moveKeyInLeaf(LeafNode lender, LeafNode borrower,
            boolean leftLender, InternalNode parent,
            int insideKeyIndex) {
        Float key;
        if (leftLender) {
            Float keyLender = lender.keys.get(lender.keys.size() - 1);
            borrower.insertToKeyAddressMap(keyLender, lender.keyAddressMap.get(keyLender));
            lender.removeKeyFromMap(keyLender);

            borrower.insertKeyAt(0, keyLender);
            lender.keys.remove(lender.keys.size() - 1);
            key = borrower.keys.get(0);
        } else {
            Float keyLender = lender.keys.get(0);
            borrower.insertToKeyAddressMap(keyLender, lender.keyAddressMap.get(keyLender));
            lender.removeKeyFromMap(keyLender);

            borrower.insertKeyAt(borrower.keys.size(), keyLender);
            lender.keys.remove(0);
            key = lender.keys.get(0);
        }

        if (insideKeyIndex == -1) {
        } else if (insideKeyIndex >= 0) {
            if (parent.keys.size() == insideKeyIndex) {
                parent.keys.set(insideKeyIndex - 1, key);

                int lastParentChild = borrower.getParent().keys.size() - 1;
                Float lastParentChildKey = borrower.getParent().getChild(borrower.getParent().keys.size()).keys.get(0);
                if (!(lender.getParent().getChild(lender.getParent().getChildren().size() - 1).keys.get(0))
                        .equals(key)) {
                    borrower.getParent().keys.set(lastParentChild, lastParentChildKey);
                }
            } else {
                parent.keys.set(insideKeyIndex, key);

                if (!(lender.getParent().getChild(insideKeyIndex + 1).keys.get(0)).equals(key)) {
                    lender.getParent().keys.set(insideKeyIndex,
                            lender.getParent().getChild(insideKeyIndex + 1).keys.get(0));
                }
            }
        } else {
            parent.keys.set(insideKeyIndex - 1, key);
        }

        int pointerIndex = borrower.getIndexOfKey(key, true);
        int keyIndex = pointerIndex - 1;

        LeafNode LeafNode = (LeafNode) borrower;

        if (LeafNode.keys.size() < (keyIndex + 1))
            parent.updateKeyAt(insideKeyIndex - 1, parent.getChild(insideKeyIndex).keys.get(0), false,
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
