#include <napi.h>
#include "RobotConnector.h"

using namespace Napi;

#ifndef SERIAL_PORT_ADDON_SERVOSEQUENCEWORKER_H
#define SERIAL_PORT_ADDON_SERVOSEQUENCEWORKER_H

class ServoSequenceWorker : public AsyncWorker {
public:
    ServoSequenceWorker(Function& callback, RobotConnector *robotConnector, std::vector<double> servoAngles, int writeDelay);

    virtual ~ServoSequenceWorker(){};

    void Execute();
    void OnOK();

private:
    RobotConnector *robotConnector;
    std::vector<double> servoAngles;
    int writeDelay;

};


#endif //SERIAL_PORT_ADDON_SERVOSEQUENCEWORKER_H
