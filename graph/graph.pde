import processing.serial.*;

String newLine = null;
Serial serialPort;
int    x = 1;
int    rawOzoneData = 0;
float  ozonePPM     = 0.0;
float  temperature  = 0.0;
float  humidity     = 0.0;

float  lastO3Disp, lastTempDisp, lastHumiDisp;

void drawGrid(){
  stroke(50);
  for (int dx=0; dx<=width; dx+=10){
    line(dx,0,dx,height);
  }
  for (int dy=0; dy<=height; dy+=10){
    line(0,dy,width,dy);
  }
}

void setup() {
  size(800, 600);
  printArray(Serial.list());
  serialPort = new Serial(this, Serial.list()[32], 115200);
  serialPort.clear();
  newLine = null;
  background(42);
  drawGrid();
}

float getOzonePPM(){
  return map(rawOzoneData, 2097152, 0, 10, 1000); // ?
}

void readDataFromPort(){
  while (serialPort.available() > 0) {
    newLine = serialPort.readStringUntil(10);
    if (newLine != null) {
      if (newLine.indexOf("O3")!=-1){
        String d = newLine.substring(4);
        rawOzoneData = Integer.parseInt(d.trim());
      }
      if (newLine.indexOf("TC")!=-1){
        String d = newLine.substring(4);
        temperature = Float.parseFloat(d.trim());
      }
      if (newLine.indexOf("RH")!=-1){
        String d = newLine.substring(4);
        humidity = Float.parseFloat(d.trim());
      }
    }
  }
}

void draw() {

  readDataFromPort();

  float t = map(temperature, 0, 100, height, 0);
  stroke(250,50,50); line(x-1,lastTempDisp,x,t);
  lastTempDisp = t;

  float h = map(humidity,    0, 100, height, 0);
  stroke(50,250,50); line(x-1,lastHumiDisp,x,h);
  lastHumiDisp = h;

  float o = map(getOzonePPM(), 10, 1000, height, 0);
  stroke(50,50,250); line(x-1,lastO3Disp,x,o);
  lastO3Disp = o;


  if (frameCount%10==0){
    x++;
    if (x>=width) {
      x=1;
      background(42);
      drawGrid();
    }
  }
}
