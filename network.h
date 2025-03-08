#include <WiFi.h>


class Network {
  public:
    Network(const char *ssid, const char *password, const char *serverIP, int port);
    ~Network();

    void init();
    void keepAlive();
    int sendData(char *data);
    int readData(char *data, int maxSize = 40);

  private:
    static const char sTimeout = 500; // in ms
    inline static const char sHandshakeData[] = "<ALIVE>";

    const char *ssid;
    const char *password;

    unsigned long lastKeepAliveMillis = 0;
    const int keepAliveInterval = 2000;

    const char *serverIP;
    int port;
    WiFiClient client;

    bool isWifiConnected();
    bool isClientConnected();
    bool isConnected();
    void connectToWiFi();
    void connectClientToServer();
    void checkWifiConnection();
    void checkClientConnection();
    void sendHandShake();
};
