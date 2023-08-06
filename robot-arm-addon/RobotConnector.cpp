#include "RobotConnector.h"
#include <iostream>
#include <utility>


RobotConnector::RobotConnector(std::string serialPath, int baudRate, int servoPin)
    : serialPath(std::move(serialPath)), baudRate(baudRate), servoPin(servoPin) {}

RobotConnector::~RobotConnector() {
    std::cout << "Shutting down connector" << std::endl;
}

int RobotConnector::connect() {
    this->connected = true;

    std::cout << "Robot connected..." << std::endl;

    return 0;
}

int RobotConnector::disconnect() {
    this->connected = false;

    std::cout << "Robot disconnected..." << std::endl;

    return 0;
}

bool RobotConnector::isConnected() const {
    return this->connected;
}

int RobotConnector::writeServo(int servoAngle) {
    std::cout << "Writing servo angle: " << servoAngle << std::endl;

    return 0;
}

std::string RobotConnector::readSerialPort() {
    return "The robot is running";
}

int RobotConnector::writeSerialPort(const char* data, int length) {

    std::cout << "Sending message to robot: ";

    for (int i = 0; i < length; ++i) {
        std::cout << data[i];
    }

    std::cout << std::endl;

    return 0;
}
