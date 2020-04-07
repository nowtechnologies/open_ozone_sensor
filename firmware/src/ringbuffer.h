#ifndef _RING_BUFFER_H
#define _RING_BUFFER_H

#include <stdlib.h>
#include <stdint.h>
#include "crc.h"

class RingBuffer {
private:
    uint8_t *mData;
    int mSize;
    int mHead;
    int mTail;

public:
    RingBuffer(int size) {
        mSize = size;
        mData = (uint8_t*)malloc(mSize+1);
        mHead = mTail = 0;
    }

    ~RingBuffer() {
        free(mData);
    }

    size_t capacity() {
        if (mHead > mTail) {
            return mSize - (mHead - mTail);
        } else {
            return mTail - mHead;
        }
    }

    size_t available() {
        return mSize - capacity();
    }

    void clear() {
      mHead = mTail = 0;
    }

    bool push(uint8_t v) {
        int tail = (mTail + 1) % mSize;
        if(tail == mHead) {
            // full
            return false;
        } else {
            mData[mTail] = v;
            mTail = tail;
            return true;
        }
    }

    bool pop(uint8_t &v) {
        if(mHead == mTail) {
            // empty
            return false;
        } else {
            v = mData[mHead];
            mHead = (mHead + 1) % mSize;
            return true;
        }
    }

    bool pop() {
        if(mHead != mTail) {
            mHead = (mHead + 1) % mSize;
            return true;
        }
        return false;
    }

    bool pop(size_t length) {
        if(capacity() >= length) {
            mHead = (mHead + length) % mSize;
            return true;
        }
        return false;
    }

    bool get(size_t index, uint8_t &v) {
        v = mData[(mHead + index) % mSize];
        return capacity() > 0;
    }

    bool get(size_t index, uint8_t *buffer, size_t length) {
        if(capacity() >= length) {
            while (length--) {
                get(index++, *buffer++);
            }
            return true;
        }
        return  false;
    }

    uint16_t crc16(size_t index, size_t length) {
      static CRCContext ctx;
      static uint8_t d;
      crc16Init(ctx);
      if(capacity() >= length) {
          while (length--) {
              get(index++, d);
              crc16Add(ctx, d);
          }
      }
      return ctx.checksum;
    }
};

#endif
