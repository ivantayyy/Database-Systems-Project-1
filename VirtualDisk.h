//
// Created by jingh on 20/9/2021.
//

#ifndef DATABASE_VIRTUALDISK_H
#define DATABASE_VIRTUALDISK_H

#include <iostream>
#include <string>
#include <vector>
#include <tuple>

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned short int uint_s;

using namespace std;

class VirtualDisk {
private:
    uchar *pDisk;

    vector<bool> blkValidity;

    uint diskSize;
    uint blkSize;
    uint blkHeaderSize;
    uint numMaxRecPerBlk;

    uint numTotalBlk;
    uint numFreeBlk;
    uint numAllocBlk;

    uint curRecordID;

public:
    VirtualDisk(uint diskSize, uint blockSize, float blkHeaderRatio);

    ~VirtualDisk();

    tuple<uint, void *, uint_s>*
    addRecord(vector<tuple<uchar, uchar, size_t>> dataFormat, vector<string> data);

    bool deleteRecord(tuple<uint, void *, uint_s> recordDirectory);

    vector<tuple<uchar, string>> fetchRecord(tuple<uint, void *, uint_s> recordDirectory);

    void reportStats();

    size_t getBlockSize();

    void printHex(uchar *target, size_t size, string label);

    void printAllocatedBlocks();

private:
    int allocBlk();

    bool deallocBlk(int blkOffset);

    uchar *readBlock(uchar *pBlk);

    void writeBlock(uchar *pBlk, uchar *block);

    void intToBytes(uint integer, uchar *bytes, size_t numBytes);

    void fixedStringToBytes(string s, uint length, uchar *bytes);

    void floatToBytes(float f, uchar *bytes);

    float bytesToFloat(uchar *bytes);

    uint bytesToInt(uchar *bytes, size_t numBytes);

    string bytesToFixedString(uchar *bytes, size_t numBytes);

    void packToField(string data, uchar fieldID, uchar type, size_t dataSize, uchar *field);

    uint packToRecord(vector<uchar *> fields, uchar numFields, uchar *record);

    int insertRecordToBlock(uchar *targetBlock, uchar *targetRecord, int recordSize);

    vector<tuple<uchar, uchar, size_t, uchar *>> fetchRecordFromBlock(uchar *targetBlock, uint recordNum);

    bool removeRecordFromBlock(uchar *targetBlock, uint recordNum);

    vector<tuple<uchar, string>> decodeRecord(vector<tuple<uchar, uchar, size_t, uchar *>> recordSet);

};


#endif //DATABASE_VIRTUALDISK_H
