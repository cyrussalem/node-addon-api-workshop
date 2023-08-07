const EventEmitter = require('events').EventEmitter

const RobotArmAddon = require('bindings')('robot-arm-addon');

this.options = {
    serialPath: '/dev/ttyACM0',
    baudRate: 115200,
    servoPin: 1
};

this.robotArm = new RobotArmAddon(this.options);

this.robotArm.connect((err, data) => {
    console.log('ERROR:', err);
    console.log('DATA:', data);
});

const serialData = Buffer.from('Sending message to robot', 'utf8');
this.robotArm.writeSerial(serialData , (err, data) => {
    console.log('ERROR:', err);
    console.log('DATA:', data);
    console.log('Message write callback received');
});
this.robotArm.writeServo(60 , (err, data) => {
    console.log('ERROR:', err);
    console.log('DATA:', data);
    console.log('Servo write callback received');
});
this.robotArm.writeServoSequence([60, 61, 62, 63, 64, 65] , 1,  (err, data) => {
    console.log('ERROR:', err);
    console.log('DATA:', data);
});

const emitter = new EventEmitter()

emitter.on('data', (data) => {
    console.log('DATA EVENT:', data);
})

this.robotArm.subscribeToEvents(emitter.emit.bind(emitter));

setTimeout(() => {
    this.robotArm.disconnect((err, data) => {
        console.log('ERROR:', err);
        console.log('DATA:', data);
    });
}, 15000);


console.log('I WANT TO GET HERE');