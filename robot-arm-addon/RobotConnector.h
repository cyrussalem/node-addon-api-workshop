//
// Created by Cyrus on 2023/08/06.
//

#ifndef SERIAL_PORT_ADDON_ROBOTCONNECTOR_H
#define SERIAL_PORT_ADDON_ROBOTCONNECTOR_H


#include <string>

class RobotConnector {
private:
    std::string serialPath;
    int baudRate;
    int servoPin;
    bool connected;
public:
    RobotConnector(std::string serialPath, int baudRate, int servoPin);
    ~RobotConnector();

    int connect();

    int disconnect();

    bool isConnected() const;

    int writeServo(int servoAngle);

    std::string readSerialPort();

    int writeSerialPort(const char* data, int length);
};


#endif //SERIAL_PORT_ADDON_ROBOTCONNECTOR_H
