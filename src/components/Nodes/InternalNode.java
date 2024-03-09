package components.Nodes;

import java.util.ArrayList;

public class InternalNode extends NodeFunctions {

    ArrayList<NodeFunctions> children;

    public InternalNode() {
        super();
        children = new ArrayList<NodeFunctions>();
        setLeaf(false);
    }

    public ArrayList<NodeFunctions> getChildren() {
        return children;
    }

    public NodeFunctions getChild(int index) {
        return children.get(index);
    }

    public void addChild(NodeFunctions child) {
        this.children.add(child);
    }

    public void removeChild(NodeFunctions child) {
        this.children.remove(child);
    }

}