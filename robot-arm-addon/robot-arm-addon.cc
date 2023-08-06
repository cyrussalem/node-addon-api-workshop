#include <iostream>
#include <napi.h>
#include "RobotConnector.h"
#include "ServoSequenceWorker.h"
#include <thread>

using namespace Napi;

// Data structure representing our thread-safe function context.
struct TsfnContext {
    TsfnContext(Env env) : deferred(Promise::Deferred::New(env)) {
    };

    // Native Promise returned to JavaScript
    Promise::Deferred deferred;

    // Native thread
    std::thread nativeThread;

    RobotConnector *robotConnector;
    // used for benchmark testing
    const char *buffer;
    int bufferLength;
    int messageCount;
    int32_t minResponseTime;
    int32_t maxResponseTime;
    double avgResponseTime;

    ThreadSafeFunction tsfn;
};

struct EventData {
    std::string path;
    std::string message;
};

static void Connect(const CallbackInfo &info, RobotConnector *robotConnector) {
    Env env = info.Env();

    Function callback = info[0].As<Function>();


    robotConnector->connect();

    callback.Call({env.Null(), String::New(env, "Robot connected successfully")});

//    Napi::Error error = Napi::Error::New(env, "Connection failed");
//    callback.Call({error.Value(), String::New(env, "Some error message")});
//
//    callback.Call({});
}

static void Disconnect(const CallbackInfo &info, RobotConnector *robotConnector) {
    Env env = info.Env();

    Function callback = info[0].As<Function>();

    robotConnector->disconnect();

    callback.Call({});
}

static void WriteSerial(const CallbackInfo &info, RobotConnector *robotConnector) {
    Env env = info.Env();

    Buffer<char> buffer = info[0].As<Buffer<char>>();
    Function callback = info[1].As<Function>();

    const char *dataBuffer = buffer.Data();
    int length = buffer.Length();

    robotConnector->writeSerialPort(dataBuffer, length);

    callback.Call({});
}

static void WriteServo(const CallbackInfo &info, RobotConnector *robotConnector) {
    Env env = info.Env();

    int servoAngle = info[0].As<Number>().Int32Value();

    Function callback = info[1].As<Function>();

    robotConnector->writeServo(servoAngle);

    callback.Call({});
}

static void WriteServoSequence(const CallbackInfo &info, RobotConnector *robotConnector) {
    Env env = info.Env();

    Array jsArray = info[0].As<Array>();
    int writeDelay = info[1].As<Number>().Int32Value();

    Function callback = info[2].As<Function>();

    // Get the length of the JavaScript array
    int length = jsArray.Length();

    // Create a C++ vector to store the numbers
    std::vector<double> servoAngles(length);

    // Extract elements from the JavaScript array and store them in the C++ vector
    for (uint32_t i = 0; i < length; i++) {
        Value value = jsArray.Get(i);
        if (value.IsNumber()) {
            double number = value.As<Napi::Number>().DoubleValue();
            servoAngles[i] = number;
        } else {
            // Handle error if the element is not a number
            Napi::TypeError::New(env, "Array elements must be numbers").ThrowAsJavaScriptException();
            return;
        }
    }

    ServoSequenceWorker* servoSequenceWorker = new ServoSequenceWorker(callback, robotConnector, servoAngles, writeDelay);
    servoSequenceWorker->Queue();
}

/**
 * Thread-safe function entry.
 */
void robotEventsThreadEntry(TsfnContext *context) {

    /*
 * This callback transforms the native EventData to JavaScript
 * values. It also receives the treadsafe-function's registered callback, and
 * will call it. This is the emit function in this case.
 */
    auto callback = [&](Env env, Function jsCallback, EventData *eventData) {
        String path = String::New(env, eventData->path);
        String message = String::New(env, eventData->message);

        jsCallback.Call({path, message});

        delete eventData;
        eventData = nullptr;
    };

    while (context->robotConnector->isConnected()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        EventData *eventData = new EventData();
        eventData->path = "data";

        eventData->message = context->robotConnector->readSerialPort();

        context->tsfn.BlockingCall(eventData, callback);
    }

    context->tsfn.Release();
}

/**
 * Finalizer callback for thread-safe function.
 */
void RobotEventsFinalizerCallback(Env env,
                                  void *finalizeData,
                                  TsfnContext *context) {
    context->nativeThread.join();

    context->deferred.Resolve(Boolean::New(env, true));
    delete context;
}

Value CreateRobotEventsTSFN(const CallbackInfo &info, RobotConnector *robotConnector) {
    Env env = info.Env();

    auto tsfnContext = new TsfnContext(env);

    tsfnContext->robotConnector = robotConnector;

    // Create a new ThreadSafeFunction.
    tsfnContext->tsfn = ThreadSafeFunction::New(
            env,                           // Environment
            info[0].As<Function>(),        // JS function from caller
            "TSFN",                        // Resource name
            0,                             // Max queue size (0 = unlimited).
            1,                             // Initial thread count
            tsfnContext,                   // Context,
            RobotEventsFinalizerCallback,             // Finalizer
            (void *) nullptr               // Finalizer data
    );
    tsfnContext->nativeThread = std::thread(robotEventsThreadEntry, tsfnContext);

    return tsfnContext->deferred.Promise();
}

Napi::Value SubscribeToEventsBlocking(const Napi::CallbackInfo& info, RobotConnector *robotConnector) {
    Napi::Env env = info.Env();
    Napi::Function emit = info[0].As<Napi::Function>();

    for (int i = 0; i < 3; i++) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::string message = robotConnector->readSerialPort();

        emit.Call(
                {Napi::String::New(env, "data"), Napi::String::New(env, message)});
    }
}

Object CreateRobotArmObject(const CallbackInfo &info) {
    Env env = info.Env();
    Object obj = Object::New(env);

    Object settingsObject = info[0].As<Object>();

    // extract settings data
    std::string serialPath = settingsObject.Get("serialPath").As<String>().Utf8Value();
    int baudRate = settingsObject.Get("baudRate").As<Number>().Int32Value();
    int servoPin = settingsObject.Get("servoPin").As<Number>().Int32Value();

    printf("Settings: %s %d %d\n", serialPath.c_str(), baudRate, servoPin);

    auto* robotController = new RobotConnector(serialPath, baudRate, servoPin);

    obj.Set(String::New(env, "connect"),
            Function::New(env, [robotController](const CallbackInfo &info) {
                return Connect(info, robotController);
            }));

    obj.Set(String::New(env, "disconnect"),
            Function::New(env, [robotController](const CallbackInfo &info) {
                return Disconnect(info, robotController);
            }));

    obj.Set(String::New(env, "writeSerial"),
            Function::New(env, [robotController](const CallbackInfo &info) {
                return WriteSerial(info, robotController);
            }));

    obj.Set(String::New(env, "writeServo"),
            Function::New(env, [robotController](const CallbackInfo &info) {
                return WriteServo(info, robotController);
            }));

    obj.Set(String::New(env, "writeServoSequence"),
            Function::New(env, [robotController](const CallbackInfo &info) {
                return WriteServoSequence(info, robotController);
            }));

    obj.Set(String::New(env, "subscribeToEvents"),
            Function::New(env, [robotController](const CallbackInfo &info) {
                return CreateRobotEventsTSFN(info, robotController);
            }));

    return obj;
}

Object Init(Env env, Object exports) {
    return Function::New(env, CreateRobotArmObject, "createObject");
}

NODE_API_MODULE(addon, Init)