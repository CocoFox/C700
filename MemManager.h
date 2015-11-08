//
//  MemManager.h
//  C700
//
//  Created by osoumen on 2015/11/07.
//
//

#ifndef __C700__MemManager__
#define __C700__MemManager__

#include "C700DSP.h"
#include <map>

class BrrRegion {
public:
    BrrRegion(const unsigned char *data=NULL, int size=0, int loopPoint=0);
    //BrrRegion(const BrrRegion &obj);
    BrrRegion &operator=(const BrrRegion &obj);
    ~BrrRegion();
    
    bool SetAddr(int addr);
    int GetAddr();
    void ReadData(unsigned char *outBuf);
    int GetSize() { return mSize; }
    void SetLoopPoint(int lp) { mLoopPoint = lp; }
    int GetLoopPoint() { return mLoopPoint; }
    
private:
    int mAddr;
    int mSize;
    int mLoopPoint;
    unsigned char *mData;
};

class MemManager {
public:
    MemManager(int size=0x10000);
    ~MemManager();
    
    void SetDirAddr(int addr) { mDirAddr = addr; }
    bool WriteData(int srcn, const unsigned char *data, int size, int loopPoint);
    void DeleteData(int srcn);
    void UpdateMem(C700DSP *dsp);
    const unsigned char *GetMemImage() const { return mMem; }
    void ChangeLoopPoint(int srcn, int lp, C700DSP *dsp);
    
    void SetBrrEndAddr(int addr);
    int CalcBrrSize() { return (mBrrEndAddr - mBrrStartAddr); }
    
private:
    std::map<int, BrrRegion>            mRegions;
    unsigned char                       *mMem;
    int                                 mMemSize;
    int                                 mDirAddr;
    int                                 mTotalSize;
    int                                 mBrrStartAddr;
    int                                 mBrrEndAddr;
};

#endif /* defined(__C700__MemManager__) */
