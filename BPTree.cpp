#include <climits>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cmath>
#include "BPTree.h"
#include "Node.h"
#include <tuple>
#include <queue>

using namespace std;
typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned short int uint_s;

const bool DEBUG_MODE = false;

BPTree::BPTree(size_t blockSize) {
    this->nodeSize = (blockSize - 13) / 12;
    this->blockSize = blockSize;
    this->rootNode = nullptr;

    cout << "BP Tree Created." << endl;
    printTreeStats();
}

void BPTree::insertKey(int newKey, tuple<uint, void *, uint_s> *keyPtr) {
//        cout<< (void*)keyPtr<<endl;
    if (DEBUG_MODE) cout << "Inserting new key to tree: " << newKey << endl;
    if (rootNode == nullptr) {
        rootNode = new Node(blockSize);
        rootNode->setLeaf(true);
        rootNode->setKey(0, newKey);
        rootNode->setCurSize(1);
        vector<tuple<uint, void *, uint_s> *> *vecKeys = new vector<tuple<uint, void *, uint_s> *>;
        vecKeys->push_back(keyPtr);
        rootNode->setChildNode(0, (Node *) vecKeys);
        if (DEBUG_MODE)cout << "New Tree " << endl;
        if (DEBUG_MODE)printNode(rootNode, "debug");
    } else //root node not null
    {

        Node *ptrNode = rootNode;
        Node *parent;
        int childPtr;

        while (ptrNode->isLeaf() == false) //find the leaf node to insert new key
        {
            parent = ptrNode;
            for (int i = 0; i < ptrNode->getCurSize(); i++) {
                childPtr = i;
                if (newKey < ptrNode->getKey(i)) //if newkey smaller than current key, traverse to ith child node
                {
                    ptrNode = ptrNode->getChildNode(i);
                    break;
                }
                if (i == ptrNode->getCurSize() - 1) //if reached last key, traverse to most right node
                {
                    ptrNode = ptrNode->getChildNode(i + 1);
                    break;
                }
            }
        }
        bool keyDuplicate = false;
        for (int i = 0; i < ptrNode->getCurSize(); i++) {
            if (newKey == ptrNode->getKey(i))
                keyDuplicate = true;
        }

        if (!keyDuplicate) {
            if (ptrNode->getCurSize() < nodeSize) //if leaf node still has space
            {
                if (DEBUG_MODE)cout << "leaf node still has space" << endl;

                int keyPointer = 0;
                for (int i = 0; i < ptrNode->getCurSize(); i++) { //find position to insert key
                    if (newKey < ptrNode->getKey(i)) break;
                    else keyPointer++;
                }

                if (DEBUG_MODE)cout << "keyPointer: " << keyPointer << endl;
                if (DEBUG_MODE)cout << "ptrNode->curSize: " << ptrNode->getCurSize() << endl;

                for (int j = ptrNode->getCurSize(); j > keyPointer; j--) { //insert new key
                    //move all keys back by 1
                    ptrNode->setKey(j, ptrNode->getKey(j - 1));
                    ptrNode->setChildNode(j, ptrNode->getChildNode(j - 1));
                }
                ptrNode->setKey(keyPointer, newKey);
                vector<tuple<uint, void *, uint_s> *> *vecKeys = new vector<tuple<uint, void *, uint_s> *>;
                vecKeys->push_back(keyPtr);
                ptrNode->setChildNode(keyPointer, (Node *) vecKeys);
                ptrNode->incCurSize();

            } else //if leaf node no space, make new node
            {
                if (DEBUG_MODE)cout << "leaf node no space, make new node" << endl;
                if (DEBUG_MODE)printNode(ptrNode, "orig leaf node");
                Node *newLeaf = new Node(nodeSize);
                newLeaf->setLeaf(true);
                Node *tempNode = new Node(nodeSize + 1);
                tempNode->setCurSize(0);
                vector<tuple<uint, void *, uint_s> *> *vecKeys = new vector<tuple<uint, void *, uint_s> *>;
                vecKeys->push_back(keyPtr);

                int keyCursor = 0;
                for (int i = 0; i < ptrNode->getCurSize(); i++) //find position to input new key
                {
                    if (newKey < ptrNode->getKey(i))break;
                    else keyCursor++;
                }
//            cout << "keyCursor: " << keyCursor << endl;

                int j = 0;
                for (int i = 0; i < nodeSize + 1; i++) //copy all keys and insert new key into tempNode
                {
//                cout << "i,j: " << i << " " << j << endl;
                    if (keyCursor != i) {
                        tempNode->setKey(i, ptrNode->getKey(j));
                        tempNode->setChildNode(i, ptrNode->getChildNode(j));
                        if (DEBUG_MODE)cout << "add " << ptrNode->getKey(j) << endl;
                        j++;
                    } else {
                        tempNode->setKey(i, newKey);
                        tempNode->setChildNode(i, (Node *) vecKeys);
                        if (DEBUG_MODE)cout << "add new " << newKey << endl;
                    }
                    tempNode->incCurSize();
                }
                tempNode->setChildNode(nodeSize + 1, ptrNode->getChildNode(nodeSize));

                if (DEBUG_MODE)printNode(tempNode, "temp leaf node");

                ptrNode->setCurSize(ceil(((float)nodeSize + 1) / 2));
                newLeaf->setCurSize(floor(((float)nodeSize + 1) / 2));

                if (DEBUG_MODE)cout << "ptrNode->curSize: " << ptrNode->getCurSize() << endl;
                if (DEBUG_MODE)cout << "newLeaf->curSize: " << newLeaf->getCurSize() << endl;

                for (int i = 0; i < nodeSize; i++) //copy from temp node to initial node, clearing excess keys
                {
                    if (i < ptrNode->getCurSize()) {
                        ptrNode->setKey(i, tempNode->getKey(i));
                        ptrNode->setChildNode(i, tempNode->getChildNode(i));
                    } else {
                        ptrNode->setKey(i, NULL);
                        ptrNode->setChildNode(i, nullptr);
                    }
                }
                for (int i = 0; i < newLeaf->getCurSize(); i++) //copy from temp node to new node
                {
                    newLeaf->setKey(i, tempNode->getKey(i + ptrNode->getCurSize()));
                    newLeaf->setChildNode(i, tempNode->getChildNode(i + ptrNode->getCurSize()));
                }

                ptrNode->setChildNode(nodeSize, newLeaf); //set last node ptr to point to next leaf node
                newLeaf->setChildNode(nodeSize, tempNode->getChildNode(nodeSize + 1));

                if (DEBUG_MODE)printNode(ptrNode, "orig leaf node");
                if (DEBUG_MODE)printNode(newLeaf, "new leaf node");

                if (ptrNode == rootNode) {
                    Node *newRoot = new Node(nodeSize);
                    newRoot->setLeaf(false);
                    newRoot->setKey(0, newLeaf->getKey(0)); //key of new root is smallest key on right node
                    newRoot->setCurSize(1);
                    newRoot->setChildNode(0, ptrNode); //set child pointers
                    newRoot->setChildNode(1, newLeaf);
                    rootNode = newRoot;
                } else {
                    insertInternal(newLeaf->getKey(0), parent, newLeaf);
                }
            }
        } else {//key is duplicate
            if (DEBUG_MODE)cout << "Key duplicate. Inserting to same key" << endl;
            for (int i = 0; i < ptrNode->getCurSize(); i++) {
                if (ptrNode->getKey(i) == newKey) {
                    vector<tuple<uint, void *, uint_s> *> *vecKeys =
                            (vector<tuple<uint, void *, uint_s> *> *) ptrNode->getChildNode(i);
                    vecKeys->push_back(keyPtr);
                    if (DEBUG_MODE)cout << "Current vector size: " << vecKeys->size() << endl;
                }
            }
        }
    }
}

int BPTree::heightOfTree(Node *cursor) //initial node should be root
{
    if (cursor == NULL)
        return 0;
    if (cursor->isLeaf()) {
        return 1;
    } else {
        return heightOfTree(cursor->getChildNode(0)) + 1;
    }
}

void BPTree::insertInternal(int newKey, Node *ptrNode, Node *child) {
    if (DEBUG_MODE)cout << "insert internal" << endl;
    if (DEBUG_MODE)printNode(ptrNode, "orig internal node");
    if (ptrNode->getCurSize() < nodeSize) //if internal node still has space
    {
        if (DEBUG_MODE)cout << "internal node still has space" << endl;

        int keyPointer = 0;
        for (int i = 0; i < ptrNode->getCurSize(); i++) { //find position to insert key
            if (newKey < ptrNode->getKey(i)) break;
            else keyPointer++;
        }

//        cout << "keyPointer: " << keyPointer << endl;
//        cout << "ptrNode->curSize: " << ptrNode->curSize << endl;
//        cout << "newkey: " << newKey << endl;
//        cout << "child: " << child << endl;

        for (int j = ptrNode->getCurSize(); j > keyPointer; j--) { //insert new key
            //move all keys back by 1
            ptrNode->setKey(j, ptrNode->getKey(j - 1));
            ptrNode->setChildNode(j + 1, ptrNode->getChildNode(j));
        }
        ptrNode->setKey(keyPointer, newKey);
        ptrNode->setChildNode(keyPointer + 1, child);
        ptrNode->incCurSize();

        if (DEBUG_MODE)printNode(ptrNode, "new internal node");

    } else //if internal node no space, make new node
    {
        if (DEBUG_MODE)cout << "internal node no space, make new node" << endl;
        Node *newNode = new Node(nodeSize);
        newNode->setLeaf(false);
        Node *tempNode = new Node(nodeSize + 1);
        tempNode->setCurSize(0);

        int keyCursor = 0;
        for (int i = 0; i < ptrNode->getCurSize() + 1; i++) { //find position to input new key
//            cout << "key: " << getMinKey(ptrNode->childNode[i]) << endl;
            if (newKey < getMinKey(ptrNode->getChildNode(i))) break;
            else keyCursor++;
        }

//        cout << "keyCursor: " << keyCursor << endl;
//        cout << "newkey: " << newKey << endl;
//        cout << "child: " << child << endl;

        int j = 0;
        for (int i = 0; i < nodeSize + 2; i++) //copy all keys and insert new key into tempNode
        {
//            cout << "i: " << i << endl;
            if (keyCursor != i) {
                if (i != 0)tempNode->setKey(i - 1, getMinKey(ptrNode->getChildNode(j)));
                tempNode->setChildNode(i, ptrNode->getChildNode(j));
//                cout << "insert " << ptrNode->key[j] << endl;
                j++;
            } else {
                if (i != 0) tempNode->setKey(i - 1, newKey);
                tempNode->setChildNode(i, child);
//                cout << "insert new " << newKey << endl;
            }
            if (i != nodeSize + 1) tempNode->incCurSize();
        }

//        cout << ptrNode->key[0];
//        printNode(tempNode, "temp node");

        ptrNode->setCurSize(ceil((float) nodeSize / 2));
        newNode->setCurSize(floor((float) nodeSize / 2));
        //ptrNode->childNode[nodeSize] = newLeaf; //set last node ptr to point to next leaf node
        //newLeaf->childNode[nodeSize] = ptrNode->childNode[nodeSize];

//        cout << "Cur node size: " << ptrNode->curSize << endl;
//        cout << "New node size:" << newLeaf->curSize << endl;
        for (int i = 0; i < nodeSize; i++) //copy from temp node to initial node, clearing excess keys
        {
            if (i < ptrNode->getCurSize() + 1) {
                if (i < ptrNode->getCurSize()) ptrNode->setKey(i, tempNode->getKey(i));
                ptrNode->setChildNode(i, tempNode->getChildNode(i));
            } else {
                if (i < ptrNode->getCurSize()) ptrNode->setKey(i, NULL);
                ptrNode->setChildNode(i, nullptr);
            }
        }
        for (int i = 0; i < newNode->getCurSize() + 1; i++) //copy from temp node to new node
        {
            if (i < newNode->getCurSize())
                newNode->setKey(i, tempNode->getKey(i + ptrNode->getCurSize() + 1));
            newNode->setChildNode(i, tempNode->getChildNode(i + ptrNode->getCurSize() + 1));
        }

//        printNode(ptrNode, "ptr internal node");
//        printNode(newNode, "new internal node");

        if (ptrNode == rootNode) {
            Node *newRoot = new Node(nodeSize);
            newRoot->setLeaf(false);
            newRoot->setKey(0, getMinKey(newNode)); //key of new root is smallest key on right node
            newRoot->setCurSize(1);
            newRoot->setChildNode(0, ptrNode); //set child pointers
            newRoot->setChildNode(1, newNode);
            rootNode = newRoot;
        } else {
            Node *parent = findParent(rootNode, ptrNode);
//            printTree(parent);
            insertInternal(getMinKey(newNode), parent, newNode);
        }
    }
}

Node *BPTree::searchForNode(int key) {
    if (rootNode == NULL) {
        return nullptr;
    } else {
        Node *ptrNode = rootNode;
        int pointer = 0;
        while (!ptrNode->isLeaf()) //find the leaf node with delete key
        {
            if (DEBUG_MODE)cout << "debug node " << ptrNode->getKey(0) << endl;
            pointer = 0;

            for (int i = 0; i < ptrNode->getCurSize(); i++) {
                if (DEBUG_MODE)cout << "key: " << key << " nodeKey: " << ptrNode->getKey(i) << endl;
                if (key >= ptrNode->getKey(i)) {
                    //cout<<"in if"<<endl;
                    pointer = i + 1;

                }
                //cout<<i<<" "<<pointer<<endl;

            }
            if (DEBUG_MODE)cout << ptrNode->getKey(0) << " pointer " << pointer << endl;
            ptrNode = ptrNode->getChildNode(pointer);
        }


        return ptrNode;
    }
}

vector<tuple<uint, void *, uint_s> *> BPTree::searchForRange(int start, int end) {
    Node *searchNode;
    searchNode = searchForNode(start);
    if (DEBUG_MODE)cout << searchNode->getKey(0) << endl;
    int nodeAccessed = 1;
    if (searchNode == nullptr)
        return {};
    vector<tuple<uint, void *, uint_s> *> rangeOfRecords;
    int startKeyPos = 0;
    for (int i = 0; i < searchNode->getCurSize(); i++) //find position of first key >= start in node
    {
        if (start <= searchNode->getKey(i)) {
            startKeyPos = i;
            break;
        }
    }
    printNode(searchNode, "Contents of Node searched");
    int cursorKey = startKeyPos;
    while (searchNode->getKey(cursorKey) <= end) //find all keys in range
    {
        //cout << "search " << searchNode->getKey(cursorKey) << " pointer " << cursorKey << endl;
        if (searchNode->getKey(cursorKey) >= start) {
            vector<tuple<uint, void *, uint_s> *> *keyPtr = (vector<tuple<uint, void *, uint_s> *> *) searchNode->getChildNode(
                    cursorKey);
            rangeOfRecords.insert(rangeOfRecords.end(), keyPtr->begin(), keyPtr->end());
            //cout << "search " << searchNode->getKey(cursorKey) << " pointer " << cursorKey << endl;
        }
        //rangeOfRecords.push_back(*keyPtr);


        if (cursorKey == searchNode->getCurSize() - 1) //if reach last key in node, jump to next node
        {

            searchNode = searchNode->getChildNode(nodeSize);

            printNode(searchNode, "Contents of Node searched");
            if (DEBUG_MODE)cout << "Jump to Next Node " << (void *) searchNode << endl;

            nodeAccessed++;

            cursorKey = 0;
        } else {
            cursorKey++;

        }

    }
    cout << "Total Number of Nodes Accessed is " << nodeAccessed << endl;
    return rangeOfRecords;
}

Node *BPTree::findParent(Node *ptrNode, Node *child) {
    Node *parent;
    if (ptrNode->isLeaf() || (ptrNode->getChildNode(0)->isLeaf()))//stop recurssion when hit last 2 levels
    {
        return NULL;
    }
    for (int i = 0; i < ptrNode->getCurSize() + 1; i++) //iterate through all pointers in node
    {
        if (ptrNode->getChildNode(i) == child) {
            parent = ptrNode;
            return parent;
        } else // if not parent, call findParent recursively
        {
            parent = findParent(ptrNode->getChildNode(i), child);
            if (parent != NULL)
                return parent;
        }
    }
    return parent;
}

vector<tuple<uint, void *, uint_s> *> BPTree::deleteKey(int deleteKey) {
    if (rootNode == NULL) {
        return {};
    } else {
        Node *ptrNode = rootNode;
        Node *parent;
        int leftSibling = 0, rightSibling = 0;
        int pointer = 0;

        while (!ptrNode->isLeaf()) //find the leaf node with delete key
        {
            pointer = 0;
            if (DEBUG_MODE)cout << ptrNode->getKey(0) << " leaf? = " << ptrNode->isLeaf() << endl;
            for (int i = 0; i < ptrNode->getCurSize(); i++) {
                if (deleteKey < ptrNode->getKey(i)) {
                    pointer = i;
                    leftSibling = pointer - 1;
                    rightSibling = pointer + 1;
                    break;
                } else {
                    pointer = ptrNode->getCurSize();
                    leftSibling = pointer - 1;
                    rightSibling = pointer + 1;
                }

            }
            parent = ptrNode;
            ptrNode = ptrNode->getChildNode(pointer);
        }

        int keyPointer = 0;
        bool keyExist = false;
        for (int i = 0; i < ptrNode->getCurSize(); i++) //find key in leaf node
        {
            if (deleteKey == ptrNode->getKey(i)) {
                keyPointer = i;

                keyExist = true;
                break;
            }
        }
        if (!keyExist) {
            cout << "key to be deleted cannot be found" << endl;
            return {};
        } else { //key exist

            vector<tuple<uint, void *, uint_s> *> *deletedRecord = (vector<tuple<uint, void *, uint_s> *> *) ptrNode->getChildNode(
                    keyPointer);
            ptrNode->setKey(keyPointer, NULL); //delete key
            ptrNode->decCurSize();
            ptrNode->setChildNode(keyPointer, nullptr);

            // CASE 1
            if (ptrNode->getCurSize() >= floor((nodeSize + 1) / 2)) //if enough keys in node after deleting
            {
                for (int i = keyPointer; i < nodeSize; i++) //squeeze keys in node
                {
                    ptrNode->setKey(i, ptrNode->getKey(i + 1));
                    ptrNode->setChildNode(i, ptrNode->getChildNode(i + 1));
                }
                //cout<<"debug1: "<<pointer<<" "<<keyPointer<<endl;
                if (keyPointer == 0) {
                    //parent->key[pointer - 1] = ptrNode->key[0]; //update parent key if neccessary
                    // TODO update tree
                    updateTreeAftDelete(deleteKey, ptrNode->getKey(0));
                }
                cout << "Deleted 0 number of nodes" << endl;
                return *deletedRecord;
            }
                //CASE 2 borrow from sibling node
            else {
                Node *rightSiblingNode = nullptr;
                Node *leftSiblingNode = nullptr;
                bool cannotShare = false;
                if (leftSibling >= 0) //if left sibling exist
                    leftSiblingNode = parent->getChildNode(leftSibling);
                if (rightSibling < parent->getCurSize() + 1) //if right sibling exist
                    rightSiblingNode = parent->getChildNode(rightSibling);
                if (DEBUG_MODE)
                    cout << "debug " << leftSibling << " " << parent->getCurSize() + 1 << " " << rightSibling << endl;
                if (leftSiblingNode != nullptr) {
                    if (DEBUG_MODE)
                        cout << "debugLSN " << leftSiblingNode->getCurSize() - 1 << " vs " << floor((nodeSize + 1) / 2)
                             << endl;
                    if (leftSiblingNode->getCurSize() - 1 >= floor((nodeSize + 1) / 2)) //if left sibling can share keys
                    {
                        if (DEBUG_MODE)cout << "debug in left if" << endl;

                        int shareKey = leftSiblingNode->getKey(leftSiblingNode->getCurSize() - 1);
                        for (int i = 0;
                             i < ptrNode->getCurSize(); i++) //squeeze keys in delete node and leave first key empty
                        {
                            if (ptrNode->getKey(i) == NULL) {
                                if (i == 0)
                                    break;
                                else {
                                    for (int j = i; j > 0; j--) {
                                        ptrNode->setKey(j, ptrNode->getKey(j - 1));
                                        ptrNode->setChildNode(j, ptrNode->getChildNode(j - 1));
                                    }
                                    break;
                                }
                            }
                        }
                        //insert shared key in delete node
                        ptrNode->setKey(0, shareKey);
                        ptrNode->setChildNode(0, leftSiblingNode->getChildNode(leftSiblingNode->getCurSize() - 1));
                        ptrNode->incCurSize();
                        //delete shared key from sibling
                        leftSiblingNode->setKey(leftSiblingNode->getCurSize() - 1, NULL);
                        leftSiblingNode->setChildNode(leftSiblingNode->getCurSize() - 1, nullptr);
                        leftSiblingNode->decCurSize();
                        if (DEBUG_MODE)cout << "before update " << ptrNode->getKey(0) << endl;
                        //TODO update tree function
                        updateTreeAftDelete(deleteKey, ptrNode->getKey(0));
                        cout << "Deleted 0 number of nodes" << endl;
                        return *deletedRecord;
                    } else {
                        cannotShare = true;
                    }
                }
                if (rightSiblingNode != NULL) //if right sibling can share keys
                {
                    if (DEBUG_MODE)cout << "debugRN" << endl;
                    if (rightSiblingNode->getCurSize() - 1 >= floor((nodeSize + 1) / 2)) {
                        cannotShare = false;
                        int shareKey = rightSiblingNode->getKey(0);
                        for (int i = 0;
                             i < ptrNode->getCurSize(); i++) //squeeze keys in delete node and leave last key empty
                        {
                            if (ptrNode->getKey(i) == NULL) {
                                for (int j = i; j < nodeSize - 1; j--) {
                                    ptrNode->setKey(j, ptrNode->getKey(j + 1));
                                    ptrNode->setChildNode(j, ptrNode->getChildNode(j + 1));
                                }
                                break;
                            }
                        }
                        //insert shared key in delete node
                        ptrNode->setKey(ptrNode->getCurSize(), shareKey);
                        ptrNode->setChildNode(ptrNode->getCurSize(), rightSiblingNode->getChildNode(0));
                        ptrNode->incCurSize();
                        //delete shared key from sibling
                        for (int i = 0; i < rightSiblingNode->getCurSize(); i++) {
                            rightSiblingNode->setKey(i, rightSiblingNode->getKey(i + 1));
                            rightSiblingNode->setChildNode(i, rightSiblingNode->getChildNode(i + 1));
                        }
                        rightSiblingNode->decCurSize();
                        //TODO update tree function
                        updateTreeAftDelete(shareKey, rightSiblingNode->getKey(0));
                        cout << "Deleted 0 number of nodes" << endl;
                        return *deletedRecord;
                    } else {
                        cannotShare = true;
                    }
                }
                if (cannotShare) //both left and right siblings unable to share keys
                {
                    int deleteNodeCount = 0;
                    if (DEBUG_MODE)cout << "debug cannot share" << endl;
                    for (int i = keyPointer; i < ptrNode->getCurSize(); i++) //squeeze keys in node
                    {
                        ptrNode->setKey(i, ptrNode->getKey(i + 1));
                    }
                    if (leftSiblingNode != NULL) {
                        if (DEBUG_MODE)cout << "debug cannot share left" << endl;
                        cout << leftSiblingNode->getKey(leftSiblingNode->getCurSize() - 1) << endl;
                        for (int i = 0; i < ptrNode->getCurSize(); i++) {
                            //transfer all keys into left sibling
                            leftSiblingNode->setKey(leftSiblingNode->getCurSize() + i, ptrNode->getKey(i));
                            leftSiblingNode->setChildNode(leftSiblingNode->getCurSize() + i, ptrNode->getChildNode(i));
                            leftSiblingNode->incCurSize();
                        }
                        //printNode(leftSiblingNode,"hello");
                        //shift last ptr over to left sibling node
                        leftSiblingNode->setChildNode(nodeSize, ptrNode->getChildNode(nodeSize));
                        //leftSiblingNode->curSize += ptrNode->curSize;
                        delete ptrNode;
                        deleteNodeCount = deleteInternal(parent->getKey(leftSibling), parent, leftSiblingNode) + 1;
                        if (DEBUG_MODE)cout << "Deleted " << deleteNodeCount << " number of nodes" << endl;
                        return *deletedRecord;
                    } else if (rightSiblingNode != NULL) {
                        //cout<<"debug cannot share right "<<ptrNode->curSize<<endl;
                        //squeeze front
                        for (int i = 0; i < ptrNode->getCurSize(); i++) {
                            ptrNode->setKey(i, ptrNode->getKey(i + 1));
                        }
                        //cout<<"debug cannot share right2 "<<ptrNode->key[0]<<endl;
                        for (int i = 0; i < rightSiblingNode->getCurSize(); i++) {
                            //transfer all keys from right sibling
                            //cout<<"debug for "<<ptrNode->key[ptrNode->curSize]<<" "<<rightSiblingNode->key[i]<<endl;
                            ptrNode->setKey(ptrNode->getCurSize(), rightSiblingNode->getKey(i));
                            ptrNode->setChildNode(ptrNode->getCurSize() + 1, rightSiblingNode->getChildNode(i + 1));
                            ptrNode->incCurSize();
                        }
                        //printNode(ptrNode,"hello");
                        //shift last ptr over from right sibling node
                        ptrNode->setChildNode(nodeSize, rightSiblingNode->getChildNode(nodeSize));
                        //ptrNode->curSize += rightSiblingNode->curSize;
                        delete rightSiblingNode;
                        deleteNodeCount = deleteInternal(parent->getKey(rightSibling - 1), parent, ptrNode) + 1;
                        if (DEBUG_MODE)cout << "Deleted " << deleteNodeCount << " number of nodes" << endl;
                        return *deletedRecord;
                    }
                }
            }
        }

    }
}

int BPTree::deleteInternal(int deleteKey, Node *ptrNode, Node *child) {
    if (ptrNode == rootNode) {
        if (ptrNode->getCurSize() == 1) {
            rootNode = child;
            delete ptrNode;
            return 0;
        }
    } else {
        int keyPointer;
        for (int i = 0; i < ptrNode->getCurSize(); i++) //find key in internal node
        {
            if (deleteKey == ptrNode->getKey(i)) {
                keyPointer = i;
                break;
            }
        }
        if (DEBUG_MODE)cout << "parent internal keyPTR " << keyPointer << endl;
        for (int i = keyPointer; i < nodeSize - 1; i++)//adjust parent(current node)
        {//shift keys and pointers (delete key)

            ptrNode->setKey(i, ptrNode->getKey(i + 1));
            ptrNode->setChildNode(i + 1, ptrNode->getChildNode(i + 2));
        }
        if (keyPointer == nodeSize - 1)//if last ptr
        {
            ptrNode->setKey(keyPointer, NULL);
            ptrNode->setChildNode(keyPointer, nullptr);
        }
        ptrNode->decCurSize();

        int leftSibling = 0, rightSibling = 0, pointer = 0;
        Node *parent = findParent(rootNode, ptrNode);

        // CASE 2
        if (ptrNode->getCurSize() < floor(nodeSize / 2)) //if not enough keys in node after deleting
        {
            for (int i = 0; i < parent->getCurSize(); i++) {
                if (parent->getChildNode(i) == ptrNode) {
                    leftSibling = i - 1;
                    rightSibling = i + 1;
                }
            }
            Node *rightSiblingNode = nullptr;
            Node *leftSiblingNode = nullptr;
            bool cannotShare = false;
            if (leftSibling >= 0) //if left sibling exist
                leftSiblingNode = parent->getChildNode(leftSibling);
            if (rightSibling < parent->getCurSize() + 1) //if right sibling exist
                rightSiblingNode = parent->getChildNode(rightSibling);

            if (leftSiblingNode != nullptr) {
                if (leftSiblingNode->getCurSize() - 1 > floor(nodeSize / 2)) //if left sibling can share keys
                {
                    int shareKey = leftSiblingNode->getKey(leftSiblingNode->getCurSize() - 1);

                    for (int j = ptrNode->getCurSize() + 1; j > 0; j--) {//shift keys to leave first slot open
                        ptrNode->setKey(j, ptrNode->getKey(j - 1));
                        ptrNode->setChildNode(j, ptrNode->getChildNode(j - 1));
                    }

                    //insert shared key in delete node
                    ptrNode->setKey(0, shareKey);
                    ptrNode->setChildNode(0, leftSiblingNode->getChildNode(leftSiblingNode->getCurSize() - 1));
                    ptrNode->incCurSize();
                    //delete shared key from sibling
                    leftSiblingNode->setKey(leftSiblingNode->getCurSize() - 1, NULL);
                    leftSiblingNode->setChildNode(leftSiblingNode->getCurSize() - 1, nullptr);
                    leftSiblingNode->decCurSize();

                    return 0;
                } else {
                    cannotShare = true;
                }
            } else if (rightSiblingNode != NULL) //if right sibling can share keys
            {
                if (rightSiblingNode->getCurSize() - 1 >= floor(nodeSize / 2)) {
                    cannotShare = false;
                    int shareKey = rightSiblingNode->getKey(0);
                    //no need to squeeze keys as they are alr squeezed
                    //insert shared key in delete node
                    ptrNode->setKey(ptrNode->getCurSize(), shareKey);
                    ptrNode->setChildNode(ptrNode->getCurSize(), rightSiblingNode->getChildNode(0));
                    ptrNode->incCurSize();
                    //delete shared key from sibling
                    for (int i = 0; i < rightSiblingNode->getCurSize(); i++) {
                        rightSiblingNode->setKey(i, rightSiblingNode->getKey(i + 1));
                        rightSiblingNode->setChildNode(i, rightSiblingNode->getChildNode(i + 1));
                    }
                    rightSiblingNode->decCurSize();
                    return 0;
                } else {
                    cannotShare = true;
                }
            }
            if (cannotShare) //CASE 3 both left and right siblings unable to share keys
            {

                if (leftSiblingNode != NULL) {
                    for (int i = 0; i < ptrNode->getCurSize(); i++) {
                        //transfer all keys into left sibling
                        leftSiblingNode->setKey(leftSiblingNode->getCurSize() + i, ptrNode->getKey(i));
                        leftSiblingNode->setChildNode(leftSiblingNode->getCurSize() + i, ptrNode->getChildNode(i));
                        leftSiblingNode->incCurSize();
                    }
                    //leftSiblingNode->curSize += ptrNode->curSize;
                    delete ptrNode;
                    return deleteInternal(parent->getKey(leftSibling), parent, leftSiblingNode) + 1;

                } else if (rightSiblingNode != NULL) {
                    for (int i = 0; i < rightSiblingNode->getCurSize(); i++) {
                        //transfer all keys from right sibling
                        ptrNode->setKey(ptrNode->getCurSize(), rightSiblingNode->getKey(i));
                        ptrNode->setChildNode(ptrNode->getCurSize(), rightSiblingNode->getChildNode(i));
                        ptrNode->incCurSize();
                    }
                    //ptrNode->curSize += rightSiblingNode->curSize;
                    delete rightSiblingNode;
                    return deleteInternal(parent->getKey(rightSibling - 1), parent, ptrNode) + 1;

                }
            }
        }
        return 0;
    }
}

void BPTree::updateTreeAftDelete(int deleteKey, int newKey) {
    Node *ptrNode = rootNode;
    bool keyFound = false;
    int pointer = 0;
    while (!keyFound && !ptrNode->isLeaf()) {
        pointer = 0;
        for (int i = 0; i < ptrNode->getCurSize(); i++) //search within the node
        {
            if (DEBUG_MODE)
                cout << "loop " << ptrNode->getKey(i) << " " << deleteKey << "leaf? " << ptrNode->isLeaf() << endl;
            if (ptrNode->getKey(i) == deleteKey) //if key found
            {
                ptrNode->setKey(i, newKey);
                keyFound = true;
                break;
            } else if (deleteKey > ptrNode->getKey(i)) //find which child pointer to go
            {
                pointer = i + 1;
            }

        }
        ptrNode = ptrNode->getChildNode(pointer);
    }

}


int BPTree::getMinKey(Node *ptrNode) {
    if (ptrNode == nullptr) return NULL;
    while (!ptrNode->isLeaf()) ptrNode = ptrNode->getChildNode(0);
    return ptrNode->getKey(0);
}

void BPTree::printTree(Node *root, uint height) {
    bool printAddress = false;
    uint numNodes = 0;
    uint curHeight = 0;
    queue<Node *> printQueue;

    cout << "+++++++++++++++++ Printing B+Tree +++++++++++++++++" << endl;
    if (root != nullptr) {
        printQueue.push(root);
        printQueue.push(nullptr);
    } else {
        cout << "Nothing to print." << endl;
        return;
    }
    while (!printQueue.empty()) {
        Node *curNode = printQueue.front();
        printQueue.pop();

        if (curNode == nullptr) {
            if (printQueue.size() > 1) {
//                cout << " \\n ";
                cout << endl << "  \\" << endl;
                if (height != 0 && curHeight == height) {
                    cout << "Tree stopped printing at height " << curHeight << endl;
                    break;
                }
            }
        } else {
            numNodes++;
            if (printAddress) cout << curNode << " ";
            if (printAddress) cout << "[" << (curNode->isLeaf() ? "L" : "N") << curNode->getCurSize() << " ";
            if (!printAddress) cout << "[";
            int i;
            for (i = 0; i < curNode->getCurSize(); i++) {
                if (printAddress) cout << curNode->getChildNode(i) << " ";
                if (printAddress) cout << curNode->getKey(i) << " ";
                if (!printAddress) cout << curNode->getKey(i) << ((i < curNode->getCurSize() - 1) ? " " : "");
                if (!curNode->isLeaf() && curNode->getChildNode(i) != nullptr) {
                    printQueue.push(curNode->getChildNode(i));
                }
            }
            if (curNode->isLeaf()) {
                if (printAddress) cout << curNode->getChildNode(curNode->getMaxSize());
            } else {
                if (printAddress) cout << curNode->getChildNode(i);
                printQueue.push(curNode->getChildNode(i));
            }
            cout << "] ";
            if (printQueue.front() == nullptr && printQueue.size() > 1) {
                //                cout<<" (NL) ";
                printQueue.push(nullptr);
                curHeight++;
            }
        }
    }
    cout << "\n++++++++++++++++++ No. of Nodes: " << numNodes << " ++++++++++++++++" << endl
         << endl;
}

void BPTree::printNode(Node *node, string label) {
    bool printAddress = false;
    if (node != nullptr) {
        cout << "Print " << label << ": ";
        if (printAddress)cout << node;
        cout << "[";
        if (printAddress)cout << (node->isLeaf() ? "L" : "N") << node->getCurSize() << "/" << node->getMaxSize() << " ";
        int i;
        for (i = 0; i < node->getCurSize(); i++) {
            if (printAddress)cout << node->getChildNode(i) << " ";
            cout << node->getKey(i) << " ";
        }
        if (node->isLeaf())
            if (printAddress)cout << node->getChildNode(node->getMaxSize());
            else if (printAddress)cout << node->getChildNode(i);
        cout << "] " << endl;
    } else {
        cout << "Nothing to print." << endl;
        return;
    }
}

uint BPTree::getNumOfNodes(Node *root) {
    bool printAddress = false;
    uint numNodes = 0;
    queue<Node *> printQueue;

    if (root != nullptr) {
        printQueue.push(root);
        printQueue.push(nullptr);
    } else {
        return 0;
    }
    while (!printQueue.empty()) {
        Node *curNode = printQueue.front();
        printQueue.pop();
        if (curNode != nullptr) {
            numNodes++;
            int i;
            for (i = 0; i < curNode->getCurSize(); i++) {
                if (!curNode->isLeaf() && curNode->getChildNode(i) != nullptr) {
                    printQueue.push(curNode->getChildNode(i));
                }
            }
            if (!curNode->isLeaf()) {
                printQueue.push(curNode->getChildNode(i));
            }
        }
    }
    return numNodes;
}

void BPTree::printTreeStats() {
    int treeHeight = heightOfTree(rootNode);
    int numNodes = getNumOfNodes(rootNode);

    cout << "+++++++++++++++++ B+Tree Stats +++++++++++++++++" << endl;
    cout << "| Tree Height:\t\t\t\t" << treeHeight << endl;
    cout << "| Total Node Count:\t\t\t" << numNodes << endl;
    cout << "++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
    cout << "| Max no. of keys in a node:\t\t" << nodeSize << endl;
    cout << "| Max no. of children in a node: \t" << nodeSize + 1 << endl;
    cout << "| Max size of node: \t\t\t" << blockSize << " B" << endl;
    cout << "++++++++++++++++++++++++++++++++++++++++++++++++" << endl;

}
