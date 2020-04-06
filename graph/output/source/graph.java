import processing.core.*; 
import processing.data.*; 
import processing.event.*; 
import processing.opengl.*; 

import processing.serial.*; 

import java.util.HashMap; 
import java.util.ArrayList; 
import java.io.File; 
import java.io.BufferedReader; 
import java.io.PrintWriter; 
import java.io.InputStream; 
import java.io.OutputStream; 
import java.io.IOException; 

public class graph extends PApplet {



String newLine = null;
Serial serialPort;
int    x = 1;
float  ozonePPM     = 0.0f;
float  temperature  = 0.0f;
float  humidity     = 0.0f;

float  lastO3Disp, lastTempDisp, lastHumiDisp;

public void drawGrid(){
  stroke(150);
  for (int dx=0; dx<=width; dx+=10){
    line(dx,0,dx,height);
  }
  for (int dy=0; dy<=height; dy+=10){
    line(0,dy,width,dy);
  }
}

public void setup() {
  size(800, 600);
  printArray(Serial.list());
  serialPort = new Serial(this, Serial.list()[32], 115200);
  serialPort.clear();
  newLine = null;
  background(200);
  drawGrid();
}

public void readDataFromPort(){
  while (serialPort.available() > 0) {
    newLine = serialPort.readStringUntil(10);
    if (newLine != null) {
      if (newLine.indexOf("O3")!=-1){
        String d = newLine.substring(4);
        ozonePPM = Float.parseFloat(d.trim());
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

public void draw() {

  readDataFromPort();

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
  static public void main(String[] passedArgs) {
    String[] appletArgs = new String[] { "graph" };
    if (passedArgs != null) {
      PApplet.main(concat(appletArgs, passedArgs));
    } else {
      PApplet.main(appletArgs);
    }
  }
}
