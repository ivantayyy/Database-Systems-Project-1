//
// Created by jingh on 30/9/2021.
//

#ifndef DATABASE_BPTREE_H
#define DATABASE_BPTREE_H
#include "Node.h"
#include "Node.h"
#include<vector>

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned short int uint_s;
using namespace std;

class BPTree
{
private:
    int nodeSize;
    size_t blockSize;

public :
    Node* rootNode;

public:
    BPTree(size_t blockSize);
    void insertKey(int,tuple<uint, void *, uint_s>*);
    int heightOfTree(Node* cursor);
    vector<tuple<uint, void *, uint_s> *> deleteKey(int);
    Node* searchForNode(int key);
    vector<tuple<uint, void *, uint_s> *> searchForRange(int start, int end);
    void printTree(Node*,uint);
    void printNode(Node* node, string label);
    void printTreeStats();
    uint getNumOfNodes(Node *);

private:
    void insertInternal(int newKey, Node* ptrNode, Node* child);
    Node* findParent(Node*, Node*);
    int deleteInternal(int deleteKey, Node* ptrNode, Node* child);
    void updateTreeAftDelete(int deleteKey, int newKey);
    int getMinKey(Node* ptrNode);
};


#endif //DATABASE_BPTREE_H
