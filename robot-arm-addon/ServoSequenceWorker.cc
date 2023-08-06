#include "ServoSequenceWorker.h"
#include "RobotConnector.h"
#include <chrono>
#include <thread>
#include <utility>
#include <iostream>

using namespace Napi;

ServoSequenceWorker::ServoSequenceWorker(Function& callback, RobotConnector *robotConnector, std::vector<double> servoAngles, int writeDelay)
        : AsyncWorker(callback), robotConnector(robotConnector), servoAngles(std::move(servoAngles)), writeDelay(writeDelay) {};

void ServoSequenceWorker::Execute() {
    std::cout << "Executing servo sequence async worker" << std::endl;

    for (double value : servoAngles) {

        robotConnector->writeServo((int) value);

        std::this_thread::sleep_for(std::chrono::seconds(writeDelay));
    }
};

void ServoSequenceWorker::OnOK() {
    std::string msg = "Servo sequence async worker completed";
    Callback().Call({Env().Null(), String::New(Env(), msg)});
};