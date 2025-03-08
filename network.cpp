#include "network.h"

Network::Network(const char *ssid, const char *password, const char *serverIP, int port) {
  this->ssid = ssid;
  this->password = password;

  this->serverIP = serverIP;
  this->port = port;
}

Network::~Network() {
  this->client.stop();
  WiFi.disconnect();
}

void Network::init() {
  while(!this->isConnected()) {
    connectToWiFi();
    connectClientToServer();
    delay(200);
  }
}

void Network::keepAlive() {
  if((millis() - this->lastKeepAliveMillis) < this->keepAliveInterval)
    return;

  this->checkWifiConnection();
  this->checkClientConnection();

  this->lastKeepAliveMillis = millis();
}

int Network::sendData(char *data) {
  if(!this->isConnected())
    return -1;
  // Serial.print("sending: ");
  // Serial.println(data);
  return this->client.println(data);
}

/*
* Returns number of bytes available
* if -1 there is no new data
*/
  int Network::readData(char *data, int maxSize) {

  unsigned long timer = millis();

  bool flagStartOfMsg = false;
  bool flagEndOfMsg = false;

  int i = 0;
  while(millis() - timer <= Network::sTimeout) {
    if(!this->client.available())
      continue;

    char reading = this->client.read();

    if(reading == '<')
      flagStartOfMsg = true;
    else if(flagStartOfMsg && reading == '>')
      flagEndOfMsg = true;
    
    if(!flagStartOfMsg)
      continue;
    
    data[i] = reading;
    i++;
    
    if(!flagEndOfMsg && i >= maxSize - 1)
      return -1;

    if(flagEndOfMsg) {
      data[i] = '\0';

      // Serial.print("Received: ");
      // Serial.println(data);

      return i;
    }
  }

  // Serial.println("Received Timed out");

  return 0;
}

bool Network::isWifiConnected() {
  return WiFi.status() == WL_CONNECTED;
}
bool Network::isClientConnected() {
  return this->client.connected();
}

bool Network::isConnected() {
  return this->isWifiConnected() && this->isClientConnected();
}

void Network::connectToWiFi() {
  if(this->isWifiConnected())
    return;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.println(WiFi.localIP());
}

void Network::checkWifiConnection() {
  if(this->isWifiConnected())
    return;
  
  Serial.print(millis());
  Serial.println("ms - Reconnecting to WiFi...");
  WiFi.disconnect();
  WiFi.reconnect();
}

void Network::connectClientToServer() {
  if(!this->isWifiConnected())
    return;

  Serial.println("Connecting to server...");
  
  if(this->client.connect(this->serverIP, this->port)) {
    this->sendHandShake();
    this->client.setTimeout(Network::sTimeout);
    Serial.println("connected to server");
  }
}

void Network::checkClientConnection() {
  if(!this->isClientConnected()) {
    this->connectClientToServer();
  }

  // Send keepalive package -> check if this is really needed
  // this->sendHandShake();
}

void Network::sendHandShake() {
  this->client.println(Network::sHandshakeData);
}
