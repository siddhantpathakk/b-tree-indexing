package components.BPTree;

import java.util.ArrayList;

public class InternalNode extends Node {

    ArrayList<Node> children;

    public InternalNode() {
        super();
        children = new ArrayList<Node>();
        setLeaf(false);
    }

    
    public ArrayList<Node> getChildren() {
        return children;
    }
    public Node getChild(int index) {
        return children.get(index);
    }
    public void addChild(Node child) {
        this.children.add(child);
    }
    public void removeChild(Node child) {
        this.children.remove(child);
    }

}