//
// Created by jingh on 2/10/2021.
//

#include <climits>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cmath>
#include "Node.h"
#include <tuple>

using namespace std;
typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned short int uint_s;

const bool DEBUG_MODE = false;

Node::Node(size_t blockSize) {
    this->nodeData = (uchar *) calloc(blockSize, 1);
    uint maxKeys = (blockSize - 13) / 12;
    setMaxSize(maxKeys);
    if(DEBUG_MODE)cout<<"New node with block size "<<blockSize<<", maxKeys "<<maxKeys<<endl;
}

Node::Node(int maxKeys) {
    uint blockSize = (maxKeys * 12) + 13;
    this->nodeData = (uchar *) calloc(blockSize, 1);
    setMaxSize(maxKeys);
    if(DEBUG_MODE)cout<<"New node with maxKeys "<<maxKeys<<", block size "<<blockSize<<endl;
}

bool Node::isLeaf() {
    return nodeData[0];
}

void Node::setLeaf(bool isLeaf) {
    nodeData[0] = isLeaf;
}

uint_s Node::getCurSize() {
    return (nodeData[1] << 8) | nodeData[2];
}

void Node::setCurSize(uint_s curSize) {
    nodeData[1] = curSize >> 8 & 0xFF;
    nodeData[2] = curSize & 0xFF;
}

void Node::incCurSize(){
    setCurSize(getCurSize()+1);
}

void Node::decCurSize(){
    setCurSize(getCurSize()-1);
}

uint_s Node::getMaxSize() {
    return (nodeData[3] << 8) | nodeData[4];
}

void Node::setMaxSize(uint_s maxSize) {
    nodeData[3] = maxSize >> 8 & 0xFF;
    nodeData[4] = maxSize & 0xFF;
}

uint Node::getKey(uint index) {
    if (index > getMaxSize() - 1) return NULL;
    uint numBytes = 4;
    uint offset = 5 + (index * numBytes);
    uint key = 0;
    for (int i = 0; i < numBytes; i++) {
        key = (key << 8) | nodeData[offset + i];
    }
    return key;
}

void Node::setKey(uint index, uint key) {
    if (index > getMaxSize() - 1) return;
    uint numBytes = 4;
    uint offset = 5 + (index * numBytes);
    for (int i = 0; i < numBytes; i++) {
        nodeData[offset + (numBytes - i - 1)] = (key >> (8 * i)) & 0xFF;
    }
}

Node *Node::getChildNode(uint index) {
    if (index > getMaxSize()) return NULL;
    uint numBytes = 8;
    uint offset = 5 + (getMaxSize() * 4) + (index * numBytes);
    unsigned long long address = 0x0;
    for (int i = 0; i < numBytes; i++) {
        address = (address << 8) | nodeData[offset + i];
    }
    return (Node *) address;
}

void Node::setChildNode(uint index, Node *childNode) {
    if (index > getMaxSize()) return;
    uint numBytes = 8;
    uint offset = 5 + (getMaxSize() * 4) + (index * numBytes);
    auto address = (unsigned long long) childNode;
    for (int i = 0; i < numBytes; i++) {
        nodeData[offset + (numBytes - i - 1)] = (address >> (8 * i)) & 0xFF;
    }
}