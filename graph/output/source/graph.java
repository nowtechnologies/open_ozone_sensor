import processing.core.*; 
import processing.data.*; 
import processing.event.*; 
import processing.opengl.*; 

import processing.serial.*; 
import java.nio.ByteBuffer; 
import java.nio.ByteOrder; 
import java.util.ArrayList; 

import java.util.HashMap; 
import java.util.ArrayList; 
import java.io.File; 
import java.io.BufferedReader; 
import java.io.PrintWriter; 
import java.io.InputStream; 
import java.io.OutputStream; 
import java.io.IOException; 

public class graph extends PApplet {






String  newLine = null;
Serial  serialPort;
float   ozonePPM     = 0.0f;
float   temperature  = 0.0f;
float   humidity     = 0.0f;
float   ratio        = 0.0f;
float   lastO3Disp, lastTempDisp, lastHumiDisp;
boolean logEnabled  = false;

datalogger file;

// packet ids
static final byte PID_SENSOR    = 1;
// header index
static final byte leadIndex     = 0;
static final byte headIndex     = 1;
static final byte lengthIndex   = 2;
static final byte packetIDindex = 3;
static final byte CRCIndex      = 4;
// data index
static final byte ozoneIndex    = 0;
static final byte tempIndex     = 4;
static final byte humidIndex    = 8;
static final byte ratioIndex    = 12;

class UHR {
  public int firstLeadIn = 0;
  public int lastLeadIn = 0;
  public int packetLength = 0;
  public int packetID = 0;
  public int crc = 0;

  public final int sizeOf = 5;
  public final int IN_L = 0x55;
  public final int IN_H = 0xAA;

  public String toString() {
    return "L:[0x"+hex(firstLeadIn,2)+""+hex(lastLeadIn,2)+"], " + "ID:0x"+hex(packetID, 2)+", L:"+packetLength+", C:"+hex(crc, 2);
  }

  public UHR() {
    clear();
  }

  public void clear() {
    firstLeadIn = 0;
    lastLeadIn = 0;
    packetLength = 0;
    packetID = 0;
    crc = 0;
  }

  public void set(int[] data) {
    firstLeadIn = data[leadIndex];
    lastLeadIn = data[headIndex];
    packetLength = data[lengthIndex];
    packetID = data[packetIDindex];
    crc = data[CRCIndex];
  }

  public void set(byte[] data) {
    firstLeadIn = data[leadIndex];
    lastLeadIn = data[headIndex];
    packetLength = data[lengthIndex];
    packetID = data[packetIDindex];
    crc = data[CRCIndex];
  }
};

UHR header = new UHR();
RingBuffer buffer = new RingBuffer(256);
long shpCounter = 0;
boolean gotPacket = false;

public void serialEvent(Serial serialPort) {
  while (serialPort.available () > 0) {
    int p = serialPort.read();
    print("0x" + hex(p,2) +"\t");

    if(buffer.push(p)) {
      int required = header.sizeOf + header.packetLength;

      while(buffer.size() >= required) {
        println("BUFFER: " + buffer.size() + "/" + required);

        try {
          int[] pkt = buffer.get(0, header.sizeOf);
          header.set(pkt);

          println("Header: " + header.toString());

          if(header.firstLeadIn == header.IN_L && header.lastLeadIn == header.IN_H) {
            int crc = crc8(pkt, 0, header.sizeOf - 1);

            println("HEADER CRC: "+hex(crc, 4)+"<=>"+hex(header.crc, 4));

            if(crc == header.crc) {
              if(buffer.size() >= (header.sizeOf + header.packetLength)) {
                  buffer.popN(header.sizeOf);

                  int crcdata[] = buffer.get(header.packetLength - 2, 2);
                  int checksum = buffer.crc16(0, header.packetLength - 2);
                  int datacrc = getInt(crcdata, 0);

                  println("DATA CRC: ("+datacrc+") => " + hex(datacrc, 8) + " / ("+checksum+") => " + hex(checksum, 8) + "");

                  if(datacrc == checksum) {
                    process(header, buffer);
                  } else {
                    println("DATA CHECKSUM MISMATCH");
                  }

                  buffer.popN(header.packetLength);
                  header.clear();
              } else {
                // need more bytes
                break;
              }
            } else {
              println("HEADER CRC ERROR");
              header.clear();
              buffer.pop();
            }
          } else {
            println("Invalid LeadIn");
            header.clear();
            buffer.pop();
          }
        } catch(Exception e) {
          /// ERROR
          println("RingBuffer underflow");
          buffer.clear();
          break;
        } // try
      } // while
    } else {
      // buffer overflow
      println("RingBuffer overflow");
      buffer.clear();
    } // push
  } // serialPort.available
}

String packetContentString;
public void printPacketContent(int[] data){
  packetContentString = "";
    print("(HEADER: " + header.toString() + ") DATA: [");
    for(int i=0; i<data.length - 2; i++) {
      if(i>0) {
        print(", ");
      }
      String hexS = "0x"+hex(data[i],2)+"\t";
      packetContentString=packetContentString+hexS;
      print(hexS);
    }
    println("]");
}

public void process(UHR header, RingBuffer buffer) throws Exception {
  int[] data = buffer.get(0, header.packetLength);
  if (header.packetID == PID_SENSOR) {
    printPacketContent(data);
    println("VALID SENSOR DATA");
    temperature = getFloat(data, tempIndex);
    humidity    = getFloat(data, humidIndex);
    ozonePPM    = getFloat(data, ozoneIndex);
    ratio       = getFloat(data, ratioIndex);
    if (logEnabled){
      file.add(hour()+":"+minute()+":"+second() +","+ ozonePPM +","+ temperature +","+ humidity);
    }
  }
}

public void drawGrid(){
  stroke(150);
  for (int dx=0; dx<=width; dx+=10){
    line(dx,0,dx,height);
  }
  for (int dy=0; dy<=height; dy+=10){
    line(0,dy,width,dy);
  }
}

public void beginLog(){
  file.beginSave();
  file.add("TIME,O3PPM,TMP,RH%");
  logEnabled = true;
}

public void endLog(){
  file.endSave( file.getIncrementalFilename( sketchPath("LOG" + java.io.File.separator + "log###.csv" )));
}

public void keyPressed(){
  if (key == 'l') {
    logEnabled = !logEnabled;
    println("Log: "+logEnabled);
    if (logEnabled){
      beginLog();
    }
    else {
      endLog();
    }
  }
}

public void setup() {
  size(800, 600);
  printArray(Serial.list());
  serialPort = new Serial(this, Serial.list()[32], 9600);
  serialPort.clear();
  newLine = null;
  background(200);
  drawGrid();
  file = new datalogger();
}

int x = 1;

public void draw() {

  fill(42); stroke(42);
  rect(0,0,140,100);
  fill(255);
  text("T="+temperature+" C", 10, 20);
  text("H="+humidity+" %", 10, 40);
  text("O="+ozonePPM+" ppm", 10, 60);
  text("R="+ratio, 10, 80);

  float t = map(temperature, 0, 100, height, 0);
  stroke(250,50,50); line(x-1,lastTempDisp,x,t);
  lastTempDisp = t;

  float h = map(humidity,    0, 100, height, 0);
  stroke(50,250,50); line(x-1,lastHumiDisp,x,h);
  lastHumiDisp = h;

  float o = map(ozonePPM, 0, 1000, height, 0);
  stroke(50,50,250); line(x-1,lastO3Disp,x,o);
  lastO3Disp = o;

  if (frameCount%10==0){
    x++;
    if (x>=width) {
      x=1;
      background(200);
      drawGrid();
    }
  }
}
// untidy java.nio functions for converting the headset byte stream into floats and shorts and stuff (could be much nicer, sorry)
public float getFloat( int[] data, int from ) {
  float result = 0;
  byte[] array = new byte[4];
  for (int to = from; to <= from + 3; to ++) {
    array[to - from] = PApplet.parseByte(data[to]);
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
    array[to - from] = PApplet.parseByte(data[to]);
  }
  ByteBuffer b_array = ByteBuffer.wrap(array);
  b_array.order(ByteOrder.LITTLE_ENDIAN);
  result = b_array.getShort();
  return result;
}

public int getInt( int[] data, int from) {
  int result = 0;
  byte low = PApplet.parseByte(data[from]);
  byte high = PApplet.parseByte(data[from+1]);
  result = ((high & 0xFF) << 8) | (low & 0xFF);
  return result;
}

public int getSignedInt( int[] data, int from) {
  int result = 0;
  byte low = PApplet.parseByte(data[from]);
  byte high = PApplet.parseByte(data[from+1]);
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
  byte a = PApplet.parseByte(data[from]);
  byte b = PApplet.parseByte(data[from+1]);
  byte c = PApplet.parseByte(data[from+2]);
  byte d = PApplet.parseByte(data[from+3]);
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
// ----------------------------------------------------------------------------
int[] primeTable = {0x049D, 0x0C07, 0x1591, 0x1ACF, 0x1D4B, 0x202D, 0x2507, 0x2B4B, 0x34A5, 0x38C5, 0x3D3F, 0x4445, 0x4D0F, 0x538F, 0x5FB3, 0x6BBF};

public int crc(byte[] out) {
  int sum = 0;
  int ret = 0;
  for ( int i = 0; i < CRCIndex; i++ ) { sum += out[i]; }
  ret = (byte)((~(sum & 0xff)) + 1);
  return ret;
}

public int crc8(int[] data, int start, int count) {
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

public int crc16(byte[] out) {
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

  public int crc16(int index, int count) throws Exception {
    Checksum crc = new Checksum();
    for(int i=0; i<count; i++) {
      crc.append(get(index + i));
    }
    return crc.getResult();
  }
}
  static public void main(String[] passedArgs) {
    String[] appletArgs = new String[] { "graph" };
    if (passedArgs != null) {
      PApplet.main(concat(appletArgs, passedArgs));
    } else {
      PApplet.main(appletArgs);
    }
  }
}
