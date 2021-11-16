//
// Created by jingh on 30/9/2021.
//

#ifndef DATABASE_NODE_H
#define DATABASE_NODE_H

#include <tuple>

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned short int uint_s;
using namespace std;

class Node {
public:
    uchar *nodeData;

public:
    Node(size_t);

    Node(int);

    bool isLeaf();

    void setLeaf(bool);

    uint_s getMaxSize();

    uint_s getCurSize();

    void incCurSize();
    void decCurSize();

    void setCurSize(uint_s);

    uint getKey(uint index);

    void setKey(uint index, uint key);

    Node *getChildNode(uint index);

    void setChildNode(uint index, Node *childNode);

private:
    void setMaxSize(uint_s);

};

#endif //DATABASE_NODE_H
