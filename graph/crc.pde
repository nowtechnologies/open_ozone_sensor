// ----------------------------------------------------------------------------
int[] primeTable = {0x049D, 0x0C07, 0x1591, 0x1ACF, 0x1D4B, 0x202D, 0x2507, 0x2B4B, 0x34A5, 0x38C5, 0x3D3F, 0x4445, 0x4D0F, 0x538F, 0x5FB3, 0x6BBF};

int crc(byte[] out) {
  int sum = 0;
  int ret = 0;
  for ( int i = 0; i < CRCIndex; i++ ) { sum += out[i]; }
  ret = (byte)((~(sum & 0xff)) + 1);
  return ret;
}

int crc8(int[] data, int start, int count) {
  int sum = 0;
  int n = start;
  int i = 0;
  while(i < count) {
    sum += data[n] & 0xFF;
    i++;
    n++;
  }
  return ((~(sum & 0xff)) + 1) & 0xFF;
}

int crc16(byte[] out) {
  int result = 0;
  int primeIndex = 0;
	for(int i=0; i<out.length; i++) {
		result += ((out[i] & 0xFF) ^ 0x5A) * primeTable[primeIndex & 0x0F];
    primeIndex++;
	}
	return (result & 0x0000FFFF);
}

public class Checksum {
  protected int prime_index = 0;
  protected int checksum = 0;

  public Checksum() {
    clear();
  }

  public void clear() {
    prime_index = 0;
    checksum = 0;
  }

  public int append(int data) {
    checksum += ((data & 0xFF) ^ 0x5A) * primeTable[prime_index & 0x0F];
    prime_index++;
    return getResult();
  }

  public int getResult() {
    return (checksum & 0x0000FFFF);
  }
}
