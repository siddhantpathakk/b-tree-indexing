#include "BPTree.h"
#include <vector>
#include <iostream>

// print all the keys contained within a node
void BPTree::printKeys(Node *node)
{
    std::cout << "| ";
    for (float key : node->keys)
        std::cout << key << " | ";

    std::cout << "\n";
}

// // display the b+ tree
// void BPTree::printTree(Node *tmp)
// {
//     std::vector<Node *> n;
//     n.push_back(tmp);
//     n.push_back(nullptr);
//     while (!n.empty())
//     {
//         tmp = n.front();
//         n.erase(n.begin());
//         if (tmp != nullptr)
//         {
//             if (tmp->isLeaf)
//                 break;
//             for (int i = 0; static_cast<unsigned long>(i) < tmp->pointers.size(); i++)

//                 n.push_back(tmp->pointers.at(i));

//             n.push_back(nullptr);
//             printKeys(tmp);
//         }
//         else
//             std::cout << "Go To Next Level. \n";
//     }

//     while (!tmp->isLeaf)
//         tmp = tmp->pointers.at(0);

//     do
//     {
//         printKeys(tmp);
//         std::cout << "  ";
//         tmp = tmp->nextLeaf;
//     } while (tmp != nullptr);
//     std::cout << "\n";
// }
void BPTree::printTree(Node *tmp)
{
    static int level = 0; // Static variable to track the current level
    if (tmp == nullptr)
        return;

    level == 0? std::cout << "\nRoot: ": std::cout << "    Lvl " << level << ": [";
    

    for (int i = 0; static_cast<unsigned long>(i) < tmp->keys.size(); i++)
    {
        std::cout << tmp->keys[i];
        if (i < tmp->keys.size() - 1)
            std::cout << " | ";
    }
    std::cout << "] ("
              << tmp->keys.size()<<" keys)"
              << "\n";

    if (!tmp->isLeaf)
    {
        level++;
        for (int i = 0; static_cast<unsigned long>(i) < tmp->pointers.size(); i++)
            printTree(tmp->pointers[i]);
        level--; // Decrement the level when returning from a child node
    }
}
