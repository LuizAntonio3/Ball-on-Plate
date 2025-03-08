#include "touchScreen.h"
#include "servoControl.h"
#include "controller.h"
#include "kalmanFilter.h"
#include "stateSpaceMatrices.h"
#include "network.h"
#include "etm.h"
#include "parsers.h"

const char* wifiSSID = "Nidavellir 2G";
const char* wifiPassword = "Mc-Mp4/4@88!.";
const char* serverIP = "192.168.1.27";
const int port = 23;

Network network(wifiSSID, wifiPassword, serverIP, port);

ServoControl servos(23, 19);
TouchScreen ts(27, 26, 32, 33, 25);
screenCoordinates coords;
screenCoordinatesCm coordsCm;

KalmanFilter xFilter(sys.A, sys.B, sys.C, 0.01, 200, 150);
KalmanFilter yFilter(sys.A, sys.B, sys.C, 0.01, 200, 150);

Matrix<1,2> hInfSatGains = {17.8954, 10.0515};

Controller xController(hInfSatGains);
Controller yController(hInfSatGains);

float angleX = 0;
float angleY = 0;

float posX = 0;
float posY = 0;

float uX = 0;
float uY = 0;

float uDegreeX = 0;
float uDegreeY = 0;

float referenceX = 0;
float referenceY = 0;

Matrix<2, 1> statesX   = {0, 0};
Matrix<2, 1> statesY   = {0, 0};

// Event triggering related
Matrix<2, 1> statesXLt = {0, 0};
Matrix<2, 1> statesYLt = {0, 0};
const float lambda = .5;
const float theta = 1;
const float phi = 0.0; // TODO: check this
float GammaX = 0.0;
float etaX = 0.0;
float GammaY = 0.0;
float etaY = 0.0;
unsigned long lastUpdateTime = 0;

void setup() {
  delay(500);
  Serial.begin(1000000);
  network.init();
  delay(500);
  ts.setSamplingTime(35);
  servos.startPosition();
  delay(500);
}

//This is here purely for convenience. Otherwise the functions defined inside won't know about the variables.
#include "printInfo.h" 

void loop() {
  network.keepAlive(); // best placed in an isolated task
  coords = ts.getCoordinates();
 
  if(ts.screenUpdated()){
    
    coordsCm = ts.getCoordinatesCm(coords.x, coords.y);
    
    posX = coordsCm.xCm * 0.01; // meter
    posY = -coordsCm.yCm * 0.01; // meter

    statesX = xFilter.kalman(uX, posX);
    statesY = yFilter.kalman(uY, posY);

    // bool etmX = ETM(statesX, statesXLt, phi, lambda, theta, &etaX, &GammaX);
    // bool etmY = ETM(statesY, statesYLt, phi, lambda, theta, &etaY, &GammaY);

    // if(!etmX && !etmY)
    //   return;

    char data[60];
    sprintf(data, "<%.4f,%.4f,%.4f,%.4f,%.4f>\0", statesX(0), statesX(1), statesY(0), statesY(1), millis() - lastUpdateTime);
    if(!network.sendData(data))
      return; // failed to send data
    
    data[0] = '\0'; // if this causes any bugs consider erasing the complete buffer
    if(!network.readData(data))
      return; // failed to receive new data

    // ETM_UpdateLt(statesX, statesXLt, &GammaX);
    // ETM_UpdateLt(statesY, statesYLt, &GammaY);
    lastUpdateTime = millis();
    
    parseUxUyFromString(data, &uX, &uY);
    
    uDegreeX = rad2deg(uX);
    saturate(&uDegreeX, -25, 25);
    uX = deg2rad(uDegreeX);

    uDegreeY = rad2deg(uY);
    saturate(&uDegreeY, -25, 25);
    uY = deg2rad(uDegreeY);

    angleX = (uDegreeX) + servos.offset1;
    angleY = (uDegreeY) + servos.offset2;

    servos.moveServos(angleX , angleY);
    printXY();
  }
}
