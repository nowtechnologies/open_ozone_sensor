
public class RingBuffer {
  private final int mSize;
  private final int[] mData;
  private int mHead = 0;
  private int mTail = 0;

  public RingBuffer(int size) {
    mSize = size;
    mData = new int[size + 1];
  }

  private int capacity() {
    if(mHead > mTail) {
      return mSize - (mHead - mTail);
    } else {
      return mTail - mHead;
    }
  }

  private int available() {
    return mSize - capacity();
  }

  public void clear() {
    mHead = mTail = 0;
  }

  public boolean push(int v) {
    int tail = (mTail + 1) % mSize;
    if(tail == mHead) {
      return false;
    } else {
      mData[mTail] = v;
      mTail = tail;
      return true;
    }
  }

  public boolean pop() {
    if(mHead != mTail) {
      mHead = (mHead + 1) % mSize;
      return true;
    } else {
      return false;
    }
  }

  public boolean popN(int count) throws Exception {
    if(capacity() >= count) {
      mHead = (mHead + count) % mSize;
      return true;
    } else {
      throw new Exception("POP " + count + ", not enough data ("+capacity()+")");
    }
  }

  public int size() {
    return capacity();
  }

  public int space() {
    return available();
  }

  public int get(int i) throws Exception {
    int v = 0;
    if(capacity() > 0) {
      v = mData[(mHead + i) % mSize];
    } else {
      throw new Exception("Ring Buffer underrun");
    }
    return v;
  }

  public int[] get(int index, int count) throws Exception {
    int[] result = new int[count];
    int i = 0;
    if(capacity() >= count) {
      while(i < count) {
        result[i] = get(index);
        i += 1;
        index += 1;
      }
    }
    return result;
  }

  int crc16(int index, int count) throws Exception {
    Checksum crc = new Checksum();
    for(int i=0; i<count; i++) {
      crc.append(get(index + i));
    }
    return crc.getResult();
  }
}
