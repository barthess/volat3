// MESSAGE MPIOVD_SENSORS PACKING

#define MAVLINK_MSG_ID_MPIOVD_SENSORS 150

typedef struct __mavlink_mpiovd_sensors_t
{
 uint64_t time_usec; ///< Timestamp (microseconds since Unix epoch)
 uint64_t relay; ///< Relay sensors data
 uint32_t trip; ///< Vehicle mileage. NOT a meters, pulses from speedometer.
 uint32_t engine_uptime; ///< Vehicle motor uptime (seconds)
 uint16_t voltage_battery; ///< Batter voltate in mV. Analog sensor data #0
 uint16_t analog01; ///< Analog sensor data
 uint16_t analog02; ///< Analog sensor data
 uint16_t analog03; ///< Analog sensor data
 uint16_t analog04; ///< Analog sensor data
 uint16_t analog05; ///< Analog sensor data
 uint16_t analog06; ///< Analog sensor data
 uint16_t analog07; ///< Analog sensor data
 uint16_t analog08; ///< Analog sensor data
 uint16_t analog09; ///< Analog sensor data
 uint16_t analog10; ///< Analog sensor data
 uint16_t analog11; ///< Analog sensor data
 uint16_t analog12; ///< Analog sensor data
 uint16_t analog13; ///< Analog sensor data
 uint16_t analog14; ///< Analog sensor data
 uint16_t analog15; ///< Analog sensor data
 uint16_t actuators; ///< Actuators state
 uint16_t speed; ///< Vehicle speed (m/s)
 uint16_t rpm; ///< Vehicle engine RPM
} mavlink_mpiovd_sensors_t;

#define MAVLINK_MSG_ID_MPIOVD_SENSORS_LEN 62
#define MAVLINK_MSG_ID_150_LEN 62



#define MAVLINK_MESSAGE_INFO_MPIOVD_SENSORS { \
	"MPIOVD_SENSORS", \
	23, \
	{  { "time_usec", NULL, MAVLINK_TYPE_UINT64_T, 0, 0, offsetof(mavlink_mpiovd_sensors_t, time_usec) }, \
         { "relay", NULL, MAVLINK_TYPE_UINT64_T, 0, 8, offsetof(mavlink_mpiovd_sensors_t, relay) }, \
         { "trip", NULL, MAVLINK_TYPE_UINT32_T, 0, 16, offsetof(mavlink_mpiovd_sensors_t, trip) }, \
         { "engine_uptime", NULL, MAVLINK_TYPE_UINT32_T, 0, 20, offsetof(mavlink_mpiovd_sensors_t, engine_uptime) }, \
         { "voltage_battery", NULL, MAVLINK_TYPE_UINT16_T, 0, 24, offsetof(mavlink_mpiovd_sensors_t, voltage_battery) }, \
         { "analog01", NULL, MAVLINK_TYPE_UINT16_T, 0, 26, offsetof(mavlink_mpiovd_sensors_t, analog01) }, \
         { "analog02", NULL, MAVLINK_TYPE_UINT16_T, 0, 28, offsetof(mavlink_mpiovd_sensors_t, analog02) }, \
         { "analog03", NULL, MAVLINK_TYPE_UINT16_T, 0, 30, offsetof(mavlink_mpiovd_sensors_t, analog03) }, \
         { "analog04", NULL, MAVLINK_TYPE_UINT16_T, 0, 32, offsetof(mavlink_mpiovd_sensors_t, analog04) }, \
         { "analog05", NULL, MAVLINK_TYPE_UINT16_T, 0, 34, offsetof(mavlink_mpiovd_sensors_t, analog05) }, \
         { "analog06", NULL, MAVLINK_TYPE_UINT16_T, 0, 36, offsetof(mavlink_mpiovd_sensors_t, analog06) }, \
         { "analog07", NULL, MAVLINK_TYPE_UINT16_T, 0, 38, offsetof(mavlink_mpiovd_sensors_t, analog07) }, \
         { "analog08", NULL, MAVLINK_TYPE_UINT16_T, 0, 40, offsetof(mavlink_mpiovd_sensors_t, analog08) }, \
         { "analog09", NULL, MAVLINK_TYPE_UINT16_T, 0, 42, offsetof(mavlink_mpiovd_sensors_t, analog09) }, \
         { "analog10", NULL, MAVLINK_TYPE_UINT16_T, 0, 44, offsetof(mavlink_mpiovd_sensors_t, analog10) }, \
         { "analog11", NULL, MAVLINK_TYPE_UINT16_T, 0, 46, offsetof(mavlink_mpiovd_sensors_t, analog11) }, \
         { "analog12", NULL, MAVLINK_TYPE_UINT16_T, 0, 48, offsetof(mavlink_mpiovd_sensors_t, analog12) }, \
         { "analog13", NULL, MAVLINK_TYPE_UINT16_T, 0, 50, offsetof(mavlink_mpiovd_sensors_t, analog13) }, \
         { "analog14", NULL, MAVLINK_TYPE_UINT16_T, 0, 52, offsetof(mavlink_mpiovd_sensors_t, analog14) }, \
         { "analog15", NULL, MAVLINK_TYPE_UINT16_T, 0, 54, offsetof(mavlink_mpiovd_sensors_t, analog15) }, \
         { "actuators", NULL, MAVLINK_TYPE_UINT16_T, 0, 56, offsetof(mavlink_mpiovd_sensors_t, actuators) }, \
         { "speed", NULL, MAVLINK_TYPE_UINT16_T, 0, 58, offsetof(mavlink_mpiovd_sensors_t, speed) }, \
         { "rpm", NULL, MAVLINK_TYPE_UINT16_T, 0, 60, offsetof(mavlink_mpiovd_sensors_t, rpm) }, \
         } \
}


/**
 * @brief Pack a mpiovd_sensors message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param time_usec Timestamp (microseconds since Unix epoch)
 * @param relay Relay sensors data
 * @param trip Vehicle mileage. NOT a meters, pulses from speedometer.
 * @param engine_uptime Vehicle motor uptime (seconds)
 * @param voltage_battery Batter voltate in mV. Analog sensor data #0
 * @param analog01 Analog sensor data
 * @param analog02 Analog sensor data
 * @param analog03 Analog sensor data
 * @param analog04 Analog sensor data
 * @param analog05 Analog sensor data
 * @param analog06 Analog sensor data
 * @param analog07 Analog sensor data
 * @param analog08 Analog sensor data
 * @param analog09 Analog sensor data
 * @param analog10 Analog sensor data
 * @param analog11 Analog sensor data
 * @param analog12 Analog sensor data
 * @param analog13 Analog sensor data
 * @param analog14 Analog sensor data
 * @param analog15 Analog sensor data
 * @param actuators Actuators state
 * @param speed Vehicle speed (m/s)
 * @param rpm Vehicle engine RPM
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_mpiovd_sensors_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint64_t time_usec, uint64_t relay, uint32_t trip, uint32_t engine_uptime, uint16_t voltage_battery, uint16_t analog01, uint16_t analog02, uint16_t analog03, uint16_t analog04, uint16_t analog05, uint16_t analog06, uint16_t analog07, uint16_t analog08, uint16_t analog09, uint16_t analog10, uint16_t analog11, uint16_t analog12, uint16_t analog13, uint16_t analog14, uint16_t analog15, uint16_t actuators, uint16_t speed, uint16_t rpm)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[62];
	_mav_put_uint64_t(buf, 0, time_usec);
	_mav_put_uint64_t(buf, 8, relay);
	_mav_put_uint32_t(buf, 16, trip);
	_mav_put_uint32_t(buf, 20, engine_uptime);
	_mav_put_uint16_t(buf, 24, voltage_battery);
	_mav_put_uint16_t(buf, 26, analog01);
	_mav_put_uint16_t(buf, 28, analog02);
	_mav_put_uint16_t(buf, 30, analog03);
	_mav_put_uint16_t(buf, 32, analog04);
	_mav_put_uint16_t(buf, 34, analog05);
	_mav_put_uint16_t(buf, 36, analog06);
	_mav_put_uint16_t(buf, 38, analog07);
	_mav_put_uint16_t(buf, 40, analog08);
	_mav_put_uint16_t(buf, 42, analog09);
	_mav_put_uint16_t(buf, 44, analog10);
	_mav_put_uint16_t(buf, 46, analog11);
	_mav_put_uint16_t(buf, 48, analog12);
	_mav_put_uint16_t(buf, 50, analog13);
	_mav_put_uint16_t(buf, 52, analog14);
	_mav_put_uint16_t(buf, 54, analog15);
	_mav_put_uint16_t(buf, 56, actuators);
	_mav_put_uint16_t(buf, 58, speed);
	_mav_put_uint16_t(buf, 60, rpm);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 62);
#else
	mavlink_mpiovd_sensors_t packet;
	packet.time_usec = time_usec;
	packet.relay = relay;
	packet.trip = trip;
	packet.engine_uptime = engine_uptime;
	packet.voltage_battery = voltage_battery;
	packet.analog01 = analog01;
	packet.analog02 = analog02;
	packet.analog03 = analog03;
	packet.analog04 = analog04;
	packet.analog05 = analog05;
	packet.analog06 = analog06;
	packet.analog07 = analog07;
	packet.analog08 = analog08;
	packet.analog09 = analog09;
	packet.analog10 = analog10;
	packet.analog11 = analog11;
	packet.analog12 = analog12;
	packet.analog13 = analog13;
	packet.analog14 = analog14;
	packet.analog15 = analog15;
	packet.actuators = actuators;
	packet.speed = speed;
	packet.rpm = rpm;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 62);
#endif

	msg->msgid = MAVLINK_MSG_ID_MPIOVD_SENSORS;
	return mavlink_finalize_message(msg, system_id, component_id, 62, 18);
}

/**
 * @brief Pack a mpiovd_sensors message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param time_usec Timestamp (microseconds since Unix epoch)
 * @param relay Relay sensors data
 * @param trip Vehicle mileage. NOT a meters, pulses from speedometer.
 * @param engine_uptime Vehicle motor uptime (seconds)
 * @param voltage_battery Batter voltate in mV. Analog sensor data #0
 * @param analog01 Analog sensor data
 * @param analog02 Analog sensor data
 * @param analog03 Analog sensor data
 * @param analog04 Analog sensor data
 * @param analog05 Analog sensor data
 * @param analog06 Analog sensor data
 * @param analog07 Analog sensor data
 * @param analog08 Analog sensor data
 * @param analog09 Analog sensor data
 * @param analog10 Analog sensor data
 * @param analog11 Analog sensor data
 * @param analog12 Analog sensor data
 * @param analog13 Analog sensor data
 * @param analog14 Analog sensor data
 * @param analog15 Analog sensor data
 * @param actuators Actuators state
 * @param speed Vehicle speed (m/s)
 * @param rpm Vehicle engine RPM
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_mpiovd_sensors_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint64_t time_usec,uint64_t relay,uint32_t trip,uint32_t engine_uptime,uint16_t voltage_battery,uint16_t analog01,uint16_t analog02,uint16_t analog03,uint16_t analog04,uint16_t analog05,uint16_t analog06,uint16_t analog07,uint16_t analog08,uint16_t analog09,uint16_t analog10,uint16_t analog11,uint16_t analog12,uint16_t analog13,uint16_t analog14,uint16_t analog15,uint16_t actuators,uint16_t speed,uint16_t rpm)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[62];
	_mav_put_uint64_t(buf, 0, time_usec);
	_mav_put_uint64_t(buf, 8, relay);
	_mav_put_uint32_t(buf, 16, trip);
	_mav_put_uint32_t(buf, 20, engine_uptime);
	_mav_put_uint16_t(buf, 24, voltage_battery);
	_mav_put_uint16_t(buf, 26, analog01);
	_mav_put_uint16_t(buf, 28, analog02);
	_mav_put_uint16_t(buf, 30, analog03);
	_mav_put_uint16_t(buf, 32, analog04);
	_mav_put_uint16_t(buf, 34, analog05);
	_mav_put_uint16_t(buf, 36, analog06);
	_mav_put_uint16_t(buf, 38, analog07);
	_mav_put_uint16_t(buf, 40, analog08);
	_mav_put_uint16_t(buf, 42, analog09);
	_mav_put_uint16_t(buf, 44, analog10);
	_mav_put_uint16_t(buf, 46, analog11);
	_mav_put_uint16_t(buf, 48, analog12);
	_mav_put_uint16_t(buf, 50, analog13);
	_mav_put_uint16_t(buf, 52, analog14);
	_mav_put_uint16_t(buf, 54, analog15);
	_mav_put_uint16_t(buf, 56, actuators);
	_mav_put_uint16_t(buf, 58, speed);
	_mav_put_uint16_t(buf, 60, rpm);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 62);
#else
	mavlink_mpiovd_sensors_t packet;
	packet.time_usec = time_usec;
	packet.relay = relay;
	packet.trip = trip;
	packet.engine_uptime = engine_uptime;
	packet.voltage_battery = voltage_battery;
	packet.analog01 = analog01;
	packet.analog02 = analog02;
	packet.analog03 = analog03;
	packet.analog04 = analog04;
	packet.analog05 = analog05;
	packet.analog06 = analog06;
	packet.analog07 = analog07;
	packet.analog08 = analog08;
	packet.analog09 = analog09;
	packet.analog10 = analog10;
	packet.analog11 = analog11;
	packet.analog12 = analog12;
	packet.analog13 = analog13;
	packet.analog14 = analog14;
	packet.analog15 = analog15;
	packet.actuators = actuators;
	packet.speed = speed;
	packet.rpm = rpm;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 62);
#endif

	msg->msgid = MAVLINK_MSG_ID_MPIOVD_SENSORS;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 62, 18);
}

/**
 * @brief Encode a mpiovd_sensors struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param mpiovd_sensors C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_mpiovd_sensors_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_mpiovd_sensors_t* mpiovd_sensors)
{
	return mavlink_msg_mpiovd_sensors_pack(system_id, component_id, msg, mpiovd_sensors->time_usec, mpiovd_sensors->relay, mpiovd_sensors->trip, mpiovd_sensors->engine_uptime, mpiovd_sensors->voltage_battery, mpiovd_sensors->analog01, mpiovd_sensors->analog02, mpiovd_sensors->analog03, mpiovd_sensors->analog04, mpiovd_sensors->analog05, mpiovd_sensors->analog06, mpiovd_sensors->analog07, mpiovd_sensors->analog08, mpiovd_sensors->analog09, mpiovd_sensors->analog10, mpiovd_sensors->analog11, mpiovd_sensors->analog12, mpiovd_sensors->analog13, mpiovd_sensors->analog14, mpiovd_sensors->analog15, mpiovd_sensors->actuators, mpiovd_sensors->speed, mpiovd_sensors->rpm);
}

/**
 * @brief Send a mpiovd_sensors message
 * @param chan MAVLink channel to send the message
 *
 * @param time_usec Timestamp (microseconds since Unix epoch)
 * @param relay Relay sensors data
 * @param trip Vehicle mileage. NOT a meters, pulses from speedometer.
 * @param engine_uptime Vehicle motor uptime (seconds)
 * @param voltage_battery Batter voltate in mV. Analog sensor data #0
 * @param analog01 Analog sensor data
 * @param analog02 Analog sensor data
 * @param analog03 Analog sensor data
 * @param analog04 Analog sensor data
 * @param analog05 Analog sensor data
 * @param analog06 Analog sensor data
 * @param analog07 Analog sensor data
 * @param analog08 Analog sensor data
 * @param analog09 Analog sensor data
 * @param analog10 Analog sensor data
 * @param analog11 Analog sensor data
 * @param analog12 Analog sensor data
 * @param analog13 Analog sensor data
 * @param analog14 Analog sensor data
 * @param analog15 Analog sensor data
 * @param actuators Actuators state
 * @param speed Vehicle speed (m/s)
 * @param rpm Vehicle engine RPM
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_mpiovd_sensors_send(mavlink_channel_t chan, uint64_t time_usec, uint64_t relay, uint32_t trip, uint32_t engine_uptime, uint16_t voltage_battery, uint16_t analog01, uint16_t analog02, uint16_t analog03, uint16_t analog04, uint16_t analog05, uint16_t analog06, uint16_t analog07, uint16_t analog08, uint16_t analog09, uint16_t analog10, uint16_t analog11, uint16_t analog12, uint16_t analog13, uint16_t analog14, uint16_t analog15, uint16_t actuators, uint16_t speed, uint16_t rpm)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[62];
	_mav_put_uint64_t(buf, 0, time_usec);
	_mav_put_uint64_t(buf, 8, relay);
	_mav_put_uint32_t(buf, 16, trip);
	_mav_put_uint32_t(buf, 20, engine_uptime);
	_mav_put_uint16_t(buf, 24, voltage_battery);
	_mav_put_uint16_t(buf, 26, analog01);
	_mav_put_uint16_t(buf, 28, analog02);
	_mav_put_uint16_t(buf, 30, analog03);
	_mav_put_uint16_t(buf, 32, analog04);
	_mav_put_uint16_t(buf, 34, analog05);
	_mav_put_uint16_t(buf, 36, analog06);
	_mav_put_uint16_t(buf, 38, analog07);
	_mav_put_uint16_t(buf, 40, analog08);
	_mav_put_uint16_t(buf, 42, analog09);
	_mav_put_uint16_t(buf, 44, analog10);
	_mav_put_uint16_t(buf, 46, analog11);
	_mav_put_uint16_t(buf, 48, analog12);
	_mav_put_uint16_t(buf, 50, analog13);
	_mav_put_uint16_t(buf, 52, analog14);
	_mav_put_uint16_t(buf, 54, analog15);
	_mav_put_uint16_t(buf, 56, actuators);
	_mav_put_uint16_t(buf, 58, speed);
	_mav_put_uint16_t(buf, 60, rpm);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_MPIOVD_SENSORS, buf, 62, 18);
#else
	mavlink_mpiovd_sensors_t packet;
	packet.time_usec = time_usec;
	packet.relay = relay;
	packet.trip = trip;
	packet.engine_uptime = engine_uptime;
	packet.voltage_battery = voltage_battery;
	packet.analog01 = analog01;
	packet.analog02 = analog02;
	packet.analog03 = analog03;
	packet.analog04 = analog04;
	packet.analog05 = analog05;
	packet.analog06 = analog06;
	packet.analog07 = analog07;
	packet.analog08 = analog08;
	packet.analog09 = analog09;
	packet.analog10 = analog10;
	packet.analog11 = analog11;
	packet.analog12 = analog12;
	packet.analog13 = analog13;
	packet.analog14 = analog14;
	packet.analog15 = analog15;
	packet.actuators = actuators;
	packet.speed = speed;
	packet.rpm = rpm;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_MPIOVD_SENSORS, (const char *)&packet, 62, 18);
#endif
}

#endif

// MESSAGE MPIOVD_SENSORS UNPACKING


/**
 * @brief Get field time_usec from mpiovd_sensors message
 *
 * @return Timestamp (microseconds since Unix epoch)
 */
static inline uint64_t mavlink_msg_mpiovd_sensors_get_time_usec(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint64_t(msg,  0);
}

/**
 * @brief Get field relay from mpiovd_sensors message
 *
 * @return Relay sensors data
 */
static inline uint64_t mavlink_msg_mpiovd_sensors_get_relay(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint64_t(msg,  8);
}

/**
 * @brief Get field trip from mpiovd_sensors message
 *
 * @return Vehicle mileage. NOT a meters, pulses from speedometer.
 */
static inline uint32_t mavlink_msg_mpiovd_sensors_get_trip(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint32_t(msg,  16);
}

/**
 * @brief Get field engine_uptime from mpiovd_sensors message
 *
 * @return Vehicle motor uptime (seconds)
 */
static inline uint32_t mavlink_msg_mpiovd_sensors_get_engine_uptime(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint32_t(msg,  20);
}

/**
 * @brief Get field voltage_battery from mpiovd_sensors message
 *
 * @return Batter voltate in mV. Analog sensor data #0
 */
static inline uint16_t mavlink_msg_mpiovd_sensors_get_voltage_battery(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint16_t(msg,  24);
}

/**
 * @brief Get field analog01 from mpiovd_sensors message
 *
 * @return Analog sensor data
 */
static inline uint16_t mavlink_msg_mpiovd_sensors_get_analog01(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint16_t(msg,  26);
}

/**
 * @brief Get field analog02 from mpiovd_sensors message
 *
 * @return Analog sensor data
 */
static inline uint16_t mavlink_msg_mpiovd_sensors_get_analog02(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint16_t(msg,  28);
}

/**
 * @brief Get field analog03 from mpiovd_sensors message
 *
 * @return Analog sensor data
 */
static inline uint16_t mavlink_msg_mpiovd_sensors_get_analog03(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint16_t(msg,  30);
}

/**
 * @brief Get field analog04 from mpiovd_sensors message
 *
 * @return Analog sensor data
 */
static inline uint16_t mavlink_msg_mpiovd_sensors_get_analog04(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint16_t(msg,  32);
}

/**
 * @brief Get field analog05 from mpiovd_sensors message
 *
 * @return Analog sensor data
 */
static inline uint16_t mavlink_msg_mpiovd_sensors_get_analog05(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint16_t(msg,  34);
}

/**
 * @brief Get field analog06 from mpiovd_sensors message
 *
 * @return Analog sensor data
 */
static inline uint16_t mavlink_msg_mpiovd_sensors_get_analog06(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint16_t(msg,  36);
}

/**
 * @brief Get field analog07 from mpiovd_sensors message
 *
 * @return Analog sensor data
 */
static inline uint16_t mavlink_msg_mpiovd_sensors_get_analog07(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint16_t(msg,  38);
}

/**
 * @brief Get field analog08 from mpiovd_sensors message
 *
 * @return Analog sensor data
 */
static inline uint16_t mavlink_msg_mpiovd_sensors_get_analog08(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint16_t(msg,  40);
}

/**
 * @brief Get field analog09 from mpiovd_sensors message
 *
 * @return Analog sensor data
 */
static inline uint16_t mavlink_msg_mpiovd_sensors_get_analog09(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint16_t(msg,  42);
}

/**
 * @brief Get field analog10 from mpiovd_sensors message
 *
 * @return Analog sensor data
 */
static inline uint16_t mavlink_msg_mpiovd_sensors_get_analog10(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint16_t(msg,  44);
}

/**
 * @brief Get field analog11 from mpiovd_sensors message
 *
 * @return Analog sensor data
 */
static inline uint16_t mavlink_msg_mpiovd_sensors_get_analog11(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint16_t(msg,  46);
}

/**
 * @brief Get field analog12 from mpiovd_sensors message
 *
 * @return Analog sensor data
 */
static inline uint16_t mavlink_msg_mpiovd_sensors_get_analog12(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint16_t(msg,  48);
}

/**
 * @brief Get field analog13 from mpiovd_sensors message
 *
 * @return Analog sensor data
 */
static inline uint16_t mavlink_msg_mpiovd_sensors_get_analog13(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint16_t(msg,  50);
}

/**
 * @brief Get field analog14 from mpiovd_sensors message
 *
 * @return Analog sensor data
 */
static inline uint16_t mavlink_msg_mpiovd_sensors_get_analog14(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint16_t(msg,  52);
}

/**
 * @brief Get field analog15 from mpiovd_sensors message
 *
 * @return Analog sensor data
 */
static inline uint16_t mavlink_msg_mpiovd_sensors_get_analog15(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint16_t(msg,  54);
}

/**
 * @brief Get field actuators from mpiovd_sensors message
 *
 * @return Actuators state
 */
static inline uint16_t mavlink_msg_mpiovd_sensors_get_actuators(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint16_t(msg,  56);
}

/**
 * @brief Get field speed from mpiovd_sensors message
 *
 * @return Vehicle speed (m/s)
 */
static inline uint16_t mavlink_msg_mpiovd_sensors_get_speed(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint16_t(msg,  58);
}

/**
 * @brief Get field rpm from mpiovd_sensors message
 *
 * @return Vehicle engine RPM
 */
static inline uint16_t mavlink_msg_mpiovd_sensors_get_rpm(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint16_t(msg,  60);
}

/**
 * @brief Decode a mpiovd_sensors message into a struct
 *
 * @param msg The message to decode
 * @param mpiovd_sensors C-struct to decode the message contents into
 */
static inline void mavlink_msg_mpiovd_sensors_decode(const mavlink_message_t* msg, mavlink_mpiovd_sensors_t* mpiovd_sensors)
{
#if MAVLINK_NEED_BYTE_SWAP
	mpiovd_sensors->time_usec = mavlink_msg_mpiovd_sensors_get_time_usec(msg);
	mpiovd_sensors->relay = mavlink_msg_mpiovd_sensors_get_relay(msg);
	mpiovd_sensors->trip = mavlink_msg_mpiovd_sensors_get_trip(msg);
	mpiovd_sensors->engine_uptime = mavlink_msg_mpiovd_sensors_get_engine_uptime(msg);
	mpiovd_sensors->voltage_battery = mavlink_msg_mpiovd_sensors_get_voltage_battery(msg);
	mpiovd_sensors->analog01 = mavlink_msg_mpiovd_sensors_get_analog01(msg);
	mpiovd_sensors->analog02 = mavlink_msg_mpiovd_sensors_get_analog02(msg);
	mpiovd_sensors->analog03 = mavlink_msg_mpiovd_sensors_get_analog03(msg);
	mpiovd_sensors->analog04 = mavlink_msg_mpiovd_sensors_get_analog04(msg);
	mpiovd_sensors->analog05 = mavlink_msg_mpiovd_sensors_get_analog05(msg);
	mpiovd_sensors->analog06 = mavlink_msg_mpiovd_sensors_get_analog06(msg);
	mpiovd_sensors->analog07 = mavlink_msg_mpiovd_sensors_get_analog07(msg);
	mpiovd_sensors->analog08 = mavlink_msg_mpiovd_sensors_get_analog08(msg);
	mpiovd_sensors->analog09 = mavlink_msg_mpiovd_sensors_get_analog09(msg);
	mpiovd_sensors->analog10 = mavlink_msg_mpiovd_sensors_get_analog10(msg);
	mpiovd_sensors->analog11 = mavlink_msg_mpiovd_sensors_get_analog11(msg);
	mpiovd_sensors->analog12 = mavlink_msg_mpiovd_sensors_get_analog12(msg);
	mpiovd_sensors->analog13 = mavlink_msg_mpiovd_sensors_get_analog13(msg);
	mpiovd_sensors->analog14 = mavlink_msg_mpiovd_sensors_get_analog14(msg);
	mpiovd_sensors->analog15 = mavlink_msg_mpiovd_sensors_get_analog15(msg);
	mpiovd_sensors->actuators = mavlink_msg_mpiovd_sensors_get_actuators(msg);
	mpiovd_sensors->speed = mavlink_msg_mpiovd_sensors_get_speed(msg);
	mpiovd_sensors->rpm = mavlink_msg_mpiovd_sensors_get_rpm(msg);
#else
	memcpy(mpiovd_sensors, _MAV_PAYLOAD(msg), 62);
#endif
}
