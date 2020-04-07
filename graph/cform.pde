// untidy java.nio functions for converting the headset byte stream into floats and shorts and stuff (could be much nicer, sorry)
public float getFloat( int[] data, int from ) {
  float result = 0;
  byte[] array = new byte[4];
  for (int to = from; to <= from + 3; to ++) {
    array[to - from] = byte(data[to]);
  }
  ByteBuffer b_array = ByteBuffer.wrap(array);
  b_array.order(ByteOrder.LITTLE_ENDIAN);
  result = b_array.getFloat();
  return result;
}

public short getShort( int[] data, int from ) {
  short result = 0;
  byte[] array = new byte[2];
  for (int to = from; to <= from + 1; to ++) {
    array[to - from] = byte(data[to]);
  }
  ByteBuffer b_array = ByteBuffer.wrap(array);
  b_array.order(ByteOrder.LITTLE_ENDIAN);
  result = b_array.getShort();
  return result;
}

public int getInt( int[] data, int from) {
  int result = 0;
  byte low = byte(data[from]);
  byte high = byte(data[from+1]);
  result = ((high & 0xFF) << 8) | (low & 0xFF);
  return result;
}

public int getSignedInt( int[] data, int from) {
  int result = 0;
  byte low = byte(data[from]);
  byte high = byte(data[from+1]);
  result = ((high & 0xFF) << 8) | (low & 0xFF);
  if((result & 0x8000) > 0) {
    result ^= 0xFFFF;
    result *= -1;
  }
  return result;
}

public int getInt( byte[] data, int from) {
  int result = 0;
  byte low = data[from];
  byte high = data[from+1];
  result = ((high & 0xFF) << 8) | (low & 0xFF);
  return result;
}

public long getDWord( int[] data, int from) {
  long result = 0;
  // byte[] array = new byte[4];
  // for (int to = from; to <= from + 3; to ++) {
  //   array[to - from] = byte(data[to]);
  // }
  // ByteBuffer b_array = ByteBuffer.wrap(array);
  // b_array.order(ByteOrder.LITTLE_ENDIAN);
  byte a = byte(data[from]);
  byte b = byte(data[from+1]);
  byte c = byte(data[from+2]);
  byte d = byte(data[from+3]);
  result = (((d & 0xFF) << 24) | ((c & 0xFF) << 16) | ((b & 0xFF) << 8) | (a & 0xFF));
  return result;
}

public byte lowByte(int i) {
  byte result = 0;
  result = (byte) (i & 0xFF);
  return result;
}

public byte highByte(int i) {
  byte result = 0;
  result = (byte) ((i & 0xFF00) >> 8);
  return result;
}

public String hex(long v, int count) {
  ByteBuffer bb = ByteBuffer.allocate(8);
  bb.putLong(v);

  String result = "";
  for(int i=7; (i>=0) && (count > 0); i--) {
    int s = min(2, count);
    result = hex(bb.get(i), s) + result;
    count -= 2;
  }

  return result;
}
