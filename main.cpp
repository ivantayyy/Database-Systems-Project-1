#include <iostream>
#include <fstream>
#include <bits/stdc++.h>
#include <string>
#include <sstream>
#include <vector>
#include <tuple>
#include <boost/algorithm/string.hpp>
#include <cstddef>
#include <mem.h>
#include <chrono>
#include <ctime>
#include "VirtualDisk.h"
#include "BPTree.h"
#include "Node.h"
#include <filesystem>


using namespace std;
using namespace boost::algorithm;
using std::filesystem::directory_iterator;

const bool PRODUCTION_MODE = false;

vector<vector<string>> readDatafile(string fileDirectory) {
    vector<vector<string>> dataset;
    string line;
    ifstream MyReadFile(fileDirectory);

    getline(MyReadFile, line); //Read Header Line

    while (getline(MyReadFile, line)) {
        // Split line into tab-separated parts
        vector<string> parts;
        split(parts, line, boost::is_any_of("\t"));
        dataset.push_back(parts);
    }
    // Close the file
    MyReadFile.close();
    return dataset;
}

vector<tuple<uint, void *, uint_s> *>
addRecordsToDisk(vector<vector<string>> rawData, vector<tuple<uchar, uchar, size_t>> dataFormat,
                 VirtualDisk *virtualDisk, BPTree *bpTree) {
    // Add datasets to database
//    system("cls");
    cout << "Adding " << rawData.size() << " records to database..." << endl;
    bool success = true;
    int reportInterval = 1000;
    vector<tuple<uint, void *, uint_s> *> mappingTable;
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now(); // Start timer
    std::chrono::steady_clock::time_point end;
    unsigned long long totalTime = 0;
    for (int i = 0; i < rawData.size(); i++) {
        // Report Stats
        if (i != 0 && i % reportInterval == 0) {
            end = std::chrono::steady_clock::now(); // Stop timer
            uint elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
            totalTime += elapsedTime;
            float etc =
                    (float) totalTime / (i / reportInterval) * (rawData.size() - (i + 1)) / reportInterval / 60 / 1000;
            if(PRODUCTION_MODE)system("cls");
            cout << "Added [" << i << "] records. [" << rawData.size() - i << "] records remaining. \t[" << fixed
                 << setprecision(2)
                 << (float) i * 100 / (float) rawData.size() << "%]" << endl;
            cout << fixed << setprecision(2) << "Elapsed time: " << (float) elapsedTime / 1000 << "[s]. ETC: "
                 << setprecision(0) << floor(etc) << "[min] " << (etc - floor(etc)) * 60 << "[s]" << std::endl;
            begin = std::chrono::steady_clock::now(); // Start timer
        }

        // Add Record
        // <recordID, pBlk, recordNum>
        tuple<uint, void *, uint_s> *dataMap = virtualDisk->addRecord(dataFormat, rawData[i]);
        if (dataMap != nullptr) { // If insertion success
            mappingTable.push_back(dataMap); // add dataMap to mappingTable
            int key = stoi(rawData[i][2]); //define key
//            cout << "adding bptree key: " << key << endl;
            bpTree->insertKey(key, mappingTable[i]);
//            bpTree->printTree(bpTree->rootNode, 0);
        } else {
            success = false;
            break;
        }
    }
    if (success) {
        cout << "\nSuccessfully inserted " << mappingTable.size() << " records to virtual disk." << endl;
    } else {
        cout << "Operation aborted. Insertion error." << endl;
    }
    return mappingTable;
}

int main() {
    string fileDirectory = "..\\data";
    string selectedFilePath;
    size_t diskSize, blockSize;
    float blkHeaderRatio = 0.20;

    while (selectedFilePath.empty()) {
        if (PRODUCTION_MODE)system("cls");
        cout << "############## CZ4031 Database Project 1 ##############" << endl;
        cout << "Please Select a Data File (.tsv) [../data]:" << endl;
        int i = 1;
        vector<string> paths;
        for (const auto &file: directory_iterator(fileDirectory)) {
            if (file.path().extension() == ".tsv") {
                cout << i << ". " << file.path().string() << endl;
                paths.push_back(file.path().string());
                i++;
            }
        }
        int selection;
        cin >> selection;
        if (selection <= paths.size() && selection > 0) {
            selectedFilePath = paths[selection - 1];
        }
    }
    if (PRODUCTION_MODE)system("cls");
    cout << "Selected data file: " << selectedFilePath << endl;

    vector<vector<string>> rawData = readDatafile(selectedFilePath);

    cout << "Loaded " << rawData.size() << " records." << endl << endl;
    cout << "Virtual Disk configurations: " << endl;
    cout << "Disk Size (MB): ";
    cin >> diskSize;
    cout << "Block Size (B): ";
    cin >> blockSize;

    // Create virtual disk
    VirtualDisk virtualDisk(diskSize * 1000000, blockSize, blkHeaderRatio);

    // Create B+Tree for indexing
    BPTree bpTree(blockSize);

    cout << endl << "Press any key to proceed inserting records into virtual disk..." << endl;
    system("pause");

    vector<tuple<uchar, uchar, size_t>> dataFormat;
    dataFormat.push_back(make_tuple(1, 's', 9));
    dataFormat.push_back(make_tuple(2, 'f', 1));
    dataFormat.push_back(make_tuple(3, 'i', 3));

    // Add records to disk
    vector<tuple<uint, void *, uint_s> *> mappingTable = addRecordsToDisk(rawData, dataFormat, &virtualDisk, &bpTree);
    virtualDisk.reportStats();
    system("pause");

    bpTree.printTreeStats();
    bpTree.printNode(bpTree.rootNode, "Root Node");
    bpTree.printNode(bpTree.rootNode->getChildNode(0), "1st Child Node");
    system("pause");

    while (true) {
        if (PRODUCTION_MODE)system("cls");
        cout << "============== Database Menu ===============" << endl;
        cout << "1. Print virtual disk statistics" << endl;
        cout << "2. Print B+Tree index statistics" << endl;
        cout << "3. Print virtual disk allocated blocks" << endl;
        cout << "4. Print B+Tree" << endl;
        cout << "============================================" << endl;
        cout << "5. Fetch record (single)" << endl;
        cout << "6. Fetch record (range)" << endl;
        cout << "7. Delete record (single)" << endl;
        cout << "============================================" << endl;
        cout << "0. Quit" << endl;
        cout << "============================================" << endl;
        int selection;
        cin >> selection;
        if (selection == 0) break;

        vector<tuple<uint, void *, uint_s> *> results;
        uint startKey, endKey, height, blocks;
        vector<uchar *> blksAccessed;
        double totalAvgRating;

        switch (selection) {
            case 1: // Print virtual disk statistics
                virtualDisk.reportStats();
                break;
            case 2: // Print B+Tree index statistics
                bpTree.printTreeStats();
                break;
            case 3: // Print virtual disk allocated blocks
                virtualDisk.reportStats();
                cout << "No. of allocated blocks to print (0 to print everything):";
                cin >> blocks;
                virtualDisk.printAllocatedBlocks();
                break;
            case 4: // Print B+Tree
                cout << "Current height of tree: " << bpTree.heightOfTree(bpTree.rootNode) << endl;
                cout << "Height of tree to print (0 to print whole tree):";
                cin >> height;
                bpTree.printTree(bpTree.rootNode, height);
                break;
            case 5: // Fetch record (single)
                cout << "Search Key: ";
                cin >> startKey;
                endKey = startKey;
                totalAvgRating = 0;
                blksAccessed.clear();
                results = bpTree.searchForRange(startKey, endKey);
                for (int i = 0; i < results.size(); i++) {
                    vector<tuple<uchar, string>> record = virtualDisk.fetchRecord(*results[i]);
                    string tconst = get<1>(record[0]);
                    double avgRating = atof(get<1>(record[1]).c_str());
                    uint numVotes = stof(get<1>(record[2]));
//                    cout << "tconst: " << tconst << endl;
//                    cout << "avgRating: " << avgRating << endl;
//                    cout << "avgRating: " << get<1>(record[1]) << endl;
//                    cout << "numVotes: " << numVotes << endl;
                    totalAvgRating += avgRating;

                    uchar *pBlk = (uchar *) get<1>(*results[i]);
                    bool isUnique = true;
                    for (int i; i < blksAccessed.size(); i++) {
                        if (blksAccessed[i] == pBlk) {
                            isUnique = false;
                            break;
                        }
                    }
                    if (isUnique) blksAccessed.push_back(pBlk);
                    if (i < 5) { //Print content of data blocks
                        virtualDisk.printHex(pBlk, blockSize, "Fetched record from block");
                        cout << endl;
                    }
                }
                cout << "Printed first 5 accessed blocks" << endl;
                cout << "Found " << results.size() << " records." << endl;
                cout << "No. of data blocks accessed: " << blksAccessed.size() << endl;
                cout << "Average of averageRatings: " << totalAvgRating / results.size() << endl;
                break;
            case 6: // Fetch record (range)
                cout << "Start Key: ";
                cin >> startKey;
                cout << "End Key: ";
                cin >> endKey;
                totalAvgRating = 0;
                blksAccessed.clear();
                results = bpTree.searchForRange(startKey, endKey);
                for (int i = 0; i < results.size(); i++) {
                    vector<tuple<uchar, string>> record = virtualDisk.fetchRecord(*results[i]);
                    string tconst = get<1>(record[0]);
                    double avgRating = atof(get<1>(record[1]).c_str());
                    uint numVotes = stod(get<1>(record[2]));
//                    cout << "tconst: " << tconst << endl;
//                    cout << "avgRating: " << avgRating << endl;
//                    cout << "avgRating: " << get<1>(record[1]) << endl;
//                    cout << "numVotes: " << numVotes << endl;
                    totalAvgRating += avgRating;

                    uchar *pBlk = (uchar *) get<1>(*results[i]);
                    bool isUnique = true;
                    for (int i; i < blksAccessed.size(); i++) {
                        if (blksAccessed[i] == pBlk) {
                            isUnique = false;
                            break;
                        }
                    }
                    if (isUnique) blksAccessed.push_back(pBlk);
                    if (i < 5) { //Print content of data blocks
                        virtualDisk.printHex(pBlk, blockSize, "Fetched record from block");
                        cout << endl;
                    }
                }
                cout << "Found " << results.size() << " records." << endl;
                cout << "No. of data blocks accessed: " << blksAccessed.size() << endl;
                cout << "Average of averageRatings: " << totalAvgRating / results.size() << endl;
                break;
            case 7: // Delete record (single)
                cout << "Delete Key: ";
                cin >> startKey;
                results = bpTree.deleteKey(startKey);
                for (int i = 0; i < results.size(); i++) {
                    virtualDisk.deleteRecord(*results[i]);
                }
                cout << "Deleted " << results.size() << " records." << endl;
                bpTree.printTreeStats();
                bpTree.printNode(bpTree.rootNode, "Root Node");
                bpTree.printNode(bpTree.rootNode->getChildNode(0), "1st Child Node");
                break;
            default:
                cout << "Invalid input." << endl;
        }
        system("pause");
        cout << endl;
    }

    if (PRODUCTION_MODE)system("cls");
    system("pause");
    return 0;

}



int main1() {

    string fileDirectory = "..\\data\\data_tree_4.tsv";

    // Data Format: uchar fieldID, uchar dataType, size_t dataSize
    vector<tuple<uchar, uchar, size_t>> dataFormat;
    dataFormat.push_back(make_tuple(1, 's', 9));
    dataFormat.push_back(make_tuple(2, 'f', 1));
    dataFormat.push_back(make_tuple(3, 'i', 3));

    // Read and parse tsv
    vector<vector<string>> rawData = readDatafile(fileDirectory);

    // Create virtual disk of 50MB, block size 100B
    VirtualDisk virtualDisk(50000000, 100, 0.20);

    // Create B+Tree Index of node size 3
    BPTree bpTree(100);

    // Add datasets to database
    cout << "Adding " << rawData.size() << " records to database..." << endl;
    bool success = true;
    int reportInterval = 1000;
    vector<tuple<uint, void *, uint_s> *> mappingTable;
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now(); // Start timer
    std::chrono::steady_clock::time_point end;
    uint totalTime = 0;
    for (int i = 0; i < rawData.size(); i++) {
        // Report Stats
        if (i != 0 && i % reportInterval == 0) {
            end = std::chrono::steady_clock::now(); // Stop timer
            uint elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(end - begin).count();
            totalTime += elapsedTime;
            uint etc = totalTime / (i / reportInterval) * (rawData.size() - (i + 1)) / reportInterval / 60;
            system("cls");
            cout << "Added [" << i << "] records. [" << rawData.size() - i << "] records left. \t[" << fixed
                 << setprecision(2)
                 << (float) i * 100 / (float) rawData.size() << "%]" << endl;
            cout << "Elapsed time: " << elapsedTime << "[s]. ETC: " << etc << "[min]" << std::endl;
            begin = std::chrono::steady_clock::now(); // Start timer
        }

        // Add Record
        // <recordID, pBlk, recordNum>
        tuple<uint, void *, uint_s> *dataMap = virtualDisk.addRecord(dataFormat, rawData[i]);
        if (dataMap != nullptr) { // If insertion success
            mappingTable.push_back(dataMap); // add dataMap to mappingTable
            int key = stoi(rawData[i][2]); //define key
            cout << "adding bptree key: " << key << endl;
            bpTree.insertKey(key, mappingTable[i]);
            bpTree.printTree(bpTree.rootNode, 0);
        } else {
            success = false;
            break;
        }
    }
    if (success) {
        cout << "Successfully added " << mappingTable.size() << " records" << endl;

        //bpTree.deleteKey(120);
        //bpTree.printTree(bpTree.rootNode);
        //bpTree.deleteKey(115);
        //bpTree.printTree(bpTree.rootNode);
        //bpTree.deleteKey(652);
        //bpTree.printTree(bpTree.rootNode);

        vector<tuple<uint, void *, uint_s> *> results = bpTree.searchForRange(800, 5000);
        cout << "num of results: " << results.size() << endl;
        for (int i = 0; i < results.size(); i++) {
            vector<tuple<uchar, string>> record = virtualDisk.fetchRecord(*results[i]);
            string tconst = get<1>(record[0]);
            float avgRating = stoi(get<1>(record[1]));
            uint numVotes = stof(get<1>(record[2]));
            cout << "tconst: " << tconst << endl;
            cout << "avgRating: " << avgRating << endl;
            cout << "numVotes: " << numVotes << endl;
        }
    } else {
        cout << "Operation aborted. Insertion error." << endl;
    }

    virtualDisk.reportStats();
    bpTree.printTree(bpTree.rootNode, 0);

//     Fetch Record
//    cout << "Fetching rec1" << endl;
//    vector<tuple<uchar, string>> rec1 = virtualDisk.fetchRecord(mappingTable[0]);
//    for (int i = 0; i < rec1.size(); i++) {
//        cout << "Field ID: " << (int) get<0>(rec1[i]) << endl;
//        cout << "Data: " << get<1>(rec1[i]) << endl;
//    }

    // Delete Record
//    virtualDisk.deleteRecord(mappingTable[6]);
//    virtualDisk.deleteRecord(mappingTable[7]);
//    virtualDisk.deleteRecord(mappingTable[8]);

//    vector<tuple<uint, void *, uint_s> *> results = bpTree.deleteKey(1342);

//    virtualDisk.reportStats();


    system("pause");
    return 0;
}