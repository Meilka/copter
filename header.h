
#define VERSION "0.1"

// AVR runtime
//#include <avr/io.h>
//#include <avr/eeprom.h>
//#include <avr/pgmspace.h>
#include <math.h>

#define ESC_ARM_VAL 16
#define ESC_MAX_VAL 179
#define PIN_ARM_BUTTON 22

#define DEBUG 0
#define ALLOW_PHYSICAL_ARMING 0

// some math stuff
#define ToRad(x) (x*0.01745329252)
#define ToDeg(x) (x*57.2957795131)

// led pins
#define LED_STATUS 13
#define LED_ARMED 23

// in case i get lazy
#define TRUE (1)
#define FALSE (0)

// flight modes
#define SAFEMODE 0
#define LANDED 1
#define STABILIZE 2
#define ALT_HOLD 3
#define RTL 4

// serial ports
#define SERIAL_DEBUG Serial
#define SERIAL_WIRELESS Serial3
#define SERIAL_IMU Serial2

#define WIRELESS_BAUD 38400
#define WIRELESS_BYTELIMIT 8
#define IMU_BAUD 115200
#define DEBUG_BAUD 38400

// heartbeat timeout in microseconds
// set to 2x the heartbeat time or something
#define HEARTBEAT_TIMEOUT (2000000) // 2 secs

// Arduino stuff
#include "Arduino.h"

// wireless comm is in the form:
// START, OPCODE, VALUE, END (kind of like assembly)
// START is a 1 byte signal for the start of a message
// OPCODE is 1 byte for the operation to do
// VALUE is some number of bytes of data
// END is another 1 byte signal for the end of a message

// wireless opcodes
#define WIRELESS_START 0x53 // S
#define WIRELESS_END 0x45 // E

// make sure to keep NOP to 0x00, some logic stuff depends on it
#define OPCODE_NOP 0x00 // no operation, just for fun
#define OPCODE_HEARTBEAT 0x48 // heartbeat (H)
#define OPCODE_ARM 0x02 // arm motors
#define OPCODE_KILL 0x03 // kill motors
#define OPCODE_CALIB 0x04 // run ESC calibration
#define OPCODE_THROTTLE 0x05 // set throttle

// for sending data back to the base station
#define COMM_START 0x53
#define COMM_END 0x45
#define COMM_MODE_FREE 0x01
#define COMM_MODE_IMU 0x02
#define COMM_MODE_POS 0x03
#define COMM_MODE_MOTOR 0x04
#define COMM_MODE_BATT 0x05
#define COMM_MODE_HELLO 0x06
#define COMM_MODE_STATS 0x07

// Motor Control
#define ESC_ARM_VAL 20
#define ESC_MAX_VAL 179

// // // Variables

// state variables
float pitch, roll, yaw;
uint8_t gps_quality;
float gps_xpos, gps_ypos, gps_zpos;
float gps_xvel, gps_yvel;
uint8_t batterylevel[6] = {100,100,100,100,100,100};

// motor control
Servo motor[6];
uint8_t motorval[6] = {0,0,0,0,0,0};
uint8_t ESC_PIN[6] = {7,8,9,10,11,13};
uint8_t GND_PIN[6] = {26,27,28,29,30,31};
uint8_t armed = 0;
uint8_t throttle = 0;

// flight status
// 0 - armed
// 1 - hitting motor bounds
uint8_t flightStats = 0x00;

// wireless
uint8_t wirelessOpcode = 0x00;
uint8_t wirelessLength = 0;
uint8_t wirelessPackage[WIRELESS_BYTELIMIT];
uint8_t debugmode = 0;

// wireless heartbeat
uint8_t heartbeat = 0;
uint32_t lastHeartbeat = 0;

// debug info
// 0 - IMU info
// 1 - position info
// 2 - motor values
// 3 - battery levels
// 4 - flight info
uint8_t debugFlag = 0xff;

// function prototypes

// copter.pde
static void quick_start();
// wireless.pde
static void checkWireless();
static void parseCommand();
static void sendDebug();
// motors.pde TODO: make my naming convention sane
static void write_motors();
static void init_motors();
static void arm_motors();
static void disarm_motors();
static void calibrate_motors();
// state.pde
void checkIMU();


