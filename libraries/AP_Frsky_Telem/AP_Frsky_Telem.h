// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include <AP_HAL/AP_HAL.h>
#include <AP_AHRS/AP_AHRS.h>
#include <AP_BattMonitor/AP_BattMonitor.h>
#include <AP_Notify/AP_Notify.h>
#include <AP_RangeFinder/AP_RangeFinder.h>
#include <AP_SerialManager/AP_SerialManager.h>

#define MSG_BUFFER_LENGTH           5 // size of the message buffer queue (number of messages waiting to be sent)

/* 
for FrSky D protocol (D-receivers)
*/
// FrSky sensor hub data IDs
#define DATA_ID_GPS_ALT_BP          0x01
#define DATA_ID_TEMP1               0x02
#define DATA_ID_FUEL                0x04
#define DATA_ID_TEMP2               0x05
#define DATA_ID_GPS_ALT_AP          0x09
#define DATA_ID_BARO_ALT_BP         0x10
#define DATA_ID_GPS_SPEED_BP        0x11
#define DATA_ID_GPS_LONG_BP         0x12
#define DATA_ID_GPS_LAT_BP          0x13
#define DATA_ID_GPS_COURS_BP        0x14
#define DATA_ID_GPS_SPEED_AP        0x19
#define DATA_ID_GPS_LONG_AP         0x1A
#define DATA_ID_GPS_LAT_AP          0x1B
#define DATA_ID_BARO_ALT_AP         0x21
#define DATA_ID_GPS_LONG_EW         0x22
#define DATA_ID_GPS_LAT_NS          0x23
#define DATA_ID_CURRENT             0x28
#define DATA_ID_VFAS                0x39

#define START_STOP_D                0x5E
#define BYTESTUFF_D                 0x5D

/* 
for FrSky SPort and SPort Passthrough (OpenTX) protocols (X-receivers)
*/
// FrSky Sensor IDs
#define SENSOR_ID_VARIO             0x00 // Sensor ID  0
#define SENSOR_ID_FAS               0x22 // Sensor ID  2
#define SENSOR_ID_GPS               0x83 // Sensor ID  3
#define SENSOR_ID_SP2UR             0xC6 // Sensor ID  6
#define SENSOR_ID_28                0x1B // Sensor ID 28

// FrSky data IDs
#define ALT_FIRST_ID                0x0100
#define VARIO_FIRST_ID              0x0110
#define VFAS_FIRST_ID               0x0210
#define GPS_LONG_LATI_FIRST_ID      0x0800
#define DIY_FIRST_ID                0x5000

#define START_STOP_SPORT            0x7E
#define BYTESTUFF_SPORT             0x7D

/* 
for FrSky SPort Passthrough
*/
// data bits preparation
// for gps status data
#define GPS_SATS_LIMIT              0xF
#define GPS_STATUS_LIMIT            0x3
#define GPS_STATUS_OFFSET           4
#define GPS_HDOP_OFFSET             6
#define GPS_VDOP_OFFSET             14
#define GPS_ALTMSL_OFFSET           22
// for battery data
#define BATT_VOLTAGE_LIMIT          0x1FF
#define BATT_CURRENT_OFFSET         9
#define BATT_TOTALMAH_LIMIT         0x7FFF
#define BATT_TOTALMAH_OFFSET        17
// for autopilot status data
#define AP_CONTROL_MODE_LIMIT       0x1F
#define AP_SSIMPLE_FLAGS            0x6
#define AP_SSIMPLE_OFFSET           4
#define AP_LANDCOMPLETE_FLAG        0x80
#define AP_ARMED_OFFSET             8
#define AP_BATT_FS_OFFSET           9
#define AP_EKF_FS_OFFSET            10
// for home position related data
#define HOME_ALT_OFFSET             12
#define HOME_BEARING_LIMIT          0x7F
#define HOME_BEARING_OFFSET         25
// for velocity and yaw data
#define VELANDYAW_XYVEL_OFFSET      9
#define VELANDYAW_YAW_LIMIT         0x7FF
#define VELANDYAW_YAW_OFFSET        17
// for attitude (roll, pitch) and range data
#define ATTIANDRNG_ROLL_LIMIT       0x7FF
#define ATTIANDRNG_PITCH_LIMIT      0x3FF
#define ATTIANDRNG_PITCH_OFFSET     11
#define ATTIANDRNG_RNGFND_OFFSET    21



class AP_Frsky_Telem
{
public:
    //constructor
    AP_Frsky_Telem(AP_AHRS &ahrs, const AP_BattMonitor &battery, const RangeFinder &rng);

    // init - perform required initialisation
    void init(const AP_SerialManager &serial_manager, const char *firmware_str, const uint8_t mav_type, AP_Float *fs_batt_voltage, AP_Float *fs_batt_mah, uint32_t *ap_value, int32_t *home_distance, int32_t *home_bearing);
    void init(const AP_SerialManager &serial_manager);

    // add statustext message to FrSky lib queue.
    void queue_message(MAV_SEVERITY severity, const char *text);

    // update flight control mode. The control mode is vehicle type specific
    void update_control_mode(uint8_t mode) { _ap.control_mode = mode; }

    // update error mask of sensors and subsystems. The mask uses the
    // MAV_SYS_STATUS_* values from mavlink. If a bit is set then it
    // indicates that the sensor or subsystem is present but not
    // functioning correctly
    void update_sensor_status_flags(uint32_t error_mask) { _ap.sensor_status_error_flags = error_mask; }
    
    struct msg_t
    {
        struct {
            const char *text;
            uint8_t severity;
        } data[MSG_BUFFER_LENGTH];
        uint8_t queued_idx;
        uint8_t sent_idx;
    };
    
private:
    AP_AHRS &_ahrs;
    const AP_BattMonitor &_battery;
    const RangeFinder &_rng;
    AP_HAL::UARTDriver *_port;                  // UART used to send data to FrSky receiver
    AP_SerialManager::SerialProtocol _protocol; // protocol used - detected using SerialManager's SERIAL#_PROTOCOL parameter
    bool _initialised_uart;
    uint16_t _crc;

    struct
    {
        uint8_t mav_type; // frame type (see MAV_TYPE in Mavlink definition file common.h)
        AP_Float *fs_batt_voltage; // failsafe battery voltage in volts
        AP_Float *fs_batt_mah; // failsafe reserve capacity in mAh
    } _params;
    
    struct
    {
        uint8_t control_mode;
        uint32_t *value;
        uint32_t sensor_status_error_flags;
        int32_t *home_distance;
        int32_t *home_bearing;
    } _ap;
    
    float _relative_home_altitude; // altitude in centimeters above home
    uint32_t _control_sensors_timer;
    uint8_t _paramID;
    
    struct
    {
        char lat_ns, lon_ew;
        uint16_t latdddmm;
        uint16_t latmmmm;
        uint16_t londddmm;
        uint16_t lonmmmm;
        uint16_t alt_gps_meters;
        uint16_t alt_gps_cm;
        uint16_t alt_nav_meters;
        uint16_t alt_nav_cm;
        int16_t speed_in_meter;
        uint16_t speed_in_centimeter;
    } _gps;

    struct
    {
        uint8_t new_byte;
        bool send_attiandrng;
        bool send_latitude;
        uint32_t timer_params;
        uint32_t timer_ap_status;
        uint32_t timer_batt;
        uint32_t timer_gps_status;
        uint32_t timer_home;
        uint32_t timer_velandyaw;
        uint32_t timer_gps_latlng;
        uint32_t timer_vario;
        uint32_t timer_alt;
        uint32_t timer_vfas;
    } _passthrough;
    
    struct
    {
        bool sport_status;
        uint8_t fas_call;
        uint8_t gps_call;
        uint8_t vario_call;
        uint8_t various_call;
    } _SPort;
    
    struct
    {
        uint32_t last_200ms_frame;
        uint32_t last_1000ms_frame;
    } _D;
    
    struct
    {
        uint32_t chunk; // a "chunk" (four characters/bytes) at a time of the mavlink message to be sent
        uint8_t repeats; // send each message "chunk" 3 times to make sure the entire messsage gets through without getting cut
        uint8_t char_index; // index of which character to get in the message
    } _msg_chunk;

    msg_t _msg;
    
    // main transmission function when protocol is FrSky SPort Passthrough (OpenTX)
    void send_SPort_Passthrough(void);
    // main transmission function when protocol is FrSky SPort
    void send_SPort(void);
    // main transmission function when protocol is FrSky D
    void send_D(void);
    // tick - main call to send updates to transmitter (called by scheduler at 1kHz)
    void tick(void);

    // methods related to the nuts-and-bolts of sending data
    void calc_crc(uint8_t byte);
    void send_crc(void);
    void send_byte(uint8_t value);
    void send_uint32(uint16_t id, uint32_t data);
    void send_uint16(uint16_t id, uint16_t data);

    // methods to convert flight controller data to FrSky SPort Passthrough (OpenTX) format
    uint32_t get_next_msg_chunk(void);
    void control_sensors_check(void);
    uint32_t calc_param(void);
    uint32_t calc_gps_latlng(bool *send_latitude);
    uint32_t calc_gps_status(void);
    uint32_t calc_batt(void);
    uint32_t calc_ap_status(void);
    uint32_t calc_home(void);
    uint32_t calc_velandyaw(void);
    uint32_t calc_attiandrng(void);
    uint16_t prep_number(int32_t number, uint8_t digits, uint8_t power);

    // methods to convert flight controller data to FrSky D or SPort format
    void calc_nav_alt(void);
    float format_gps(float dec);
    void calc_gps_position(void);
};
