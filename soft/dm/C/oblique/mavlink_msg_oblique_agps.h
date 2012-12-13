// MESSAGE OBLIQUE_AGPS PACKING

#define MAVLINK_MSG_ID_OBLIQUE_AGPS 154

typedef struct __mavlink_oblique_agps_t
{
 uint8_t target_system; ///< System ID
 uint8_t target_component; ///< Component ID
 char message[250]; ///< Message with correction data.
} mavlink_oblique_agps_t;

#define MAVLINK_MSG_ID_OBLIQUE_AGPS_LEN 252
#define MAVLINK_MSG_ID_154_LEN 252

#define MAVLINK_MSG_OBLIQUE_AGPS_FIELD_MESSAGE_LEN 250

#define MAVLINK_MESSAGE_INFO_OBLIQUE_AGPS { \
	"OBLIQUE_AGPS", \
	3, \
	{  { "target_system", NULL, MAVLINK_TYPE_UINT8_T, 0, 0, offsetof(mavlink_oblique_agps_t, target_system) }, \
         { "target_component", NULL, MAVLINK_TYPE_UINT8_T, 0, 1, offsetof(mavlink_oblique_agps_t, target_component) }, \
         { "message", NULL, MAVLINK_TYPE_CHAR, 250, 2, offsetof(mavlink_oblique_agps_t, message) }, \
         } \
}


/**
 * @brief Pack a oblique_agps message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param target_system System ID
 * @param target_component Component ID
 * @param message Message with correction data.
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_oblique_agps_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t target_system, uint8_t target_component, const char *message)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[252];
	_mav_put_uint8_t(buf, 0, target_system);
	_mav_put_uint8_t(buf, 1, target_component);
	_mav_put_char_array(buf, 2, message, 250);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 252);
#else
	mavlink_oblique_agps_t packet;
	packet.target_system = target_system;
	packet.target_component = target_component;
	mav_array_memcpy(packet.message, message, sizeof(char)*250);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 252);
#endif

	msg->msgid = MAVLINK_MSG_ID_OBLIQUE_AGPS;
	return mavlink_finalize_message(msg, system_id, component_id, 252, 87);
}

/**
 * @brief Pack a oblique_agps message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param target_system System ID
 * @param target_component Component ID
 * @param message Message with correction data.
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_oblique_agps_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t target_system,uint8_t target_component,const char *message)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[252];
	_mav_put_uint8_t(buf, 0, target_system);
	_mav_put_uint8_t(buf, 1, target_component);
	_mav_put_char_array(buf, 2, message, 250);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 252);
#else
	mavlink_oblique_agps_t packet;
	packet.target_system = target_system;
	packet.target_component = target_component;
	mav_array_memcpy(packet.message, message, sizeof(char)*250);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 252);
#endif

	msg->msgid = MAVLINK_MSG_ID_OBLIQUE_AGPS;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 252, 87);
}

/**
 * @brief Encode a oblique_agps struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param oblique_agps C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_oblique_agps_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_oblique_agps_t* oblique_agps)
{
	return mavlink_msg_oblique_agps_pack(system_id, component_id, msg, oblique_agps->target_system, oblique_agps->target_component, oblique_agps->message);
}

/**
 * @brief Send a oblique_agps message
 * @param chan MAVLink channel to send the message
 *
 * @param target_system System ID
 * @param target_component Component ID
 * @param message Message with correction data.
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_oblique_agps_send(mavlink_channel_t chan, uint8_t target_system, uint8_t target_component, const char *message)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[252];
	_mav_put_uint8_t(buf, 0, target_system);
	_mav_put_uint8_t(buf, 1, target_component);
	_mav_put_char_array(buf, 2, message, 250);
	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OBLIQUE_AGPS, buf, 252, 87);
#else
	mavlink_oblique_agps_t packet;
	packet.target_system = target_system;
	packet.target_component = target_component;
	mav_array_memcpy(packet.message, message, sizeof(char)*250);
	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OBLIQUE_AGPS, (const char *)&packet, 252, 87);
#endif
}

#endif

// MESSAGE OBLIQUE_AGPS UNPACKING


/**
 * @brief Get field target_system from oblique_agps message
 *
 * @return System ID
 */
static inline uint8_t mavlink_msg_oblique_agps_get_target_system(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  0);
}

/**
 * @brief Get field target_component from oblique_agps message
 *
 * @return Component ID
 */
static inline uint8_t mavlink_msg_oblique_agps_get_target_component(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  1);
}

/**
 * @brief Get field message from oblique_agps message
 *
 * @return Message with correction data.
 */
static inline uint16_t mavlink_msg_oblique_agps_get_message(const mavlink_message_t* msg, char *message)
{
	return _MAV_RETURN_char_array(msg, message, 250,  2);
}

/**
 * @brief Decode a oblique_agps message into a struct
 *
 * @param msg The message to decode
 * @param oblique_agps C-struct to decode the message contents into
 */
static inline void mavlink_msg_oblique_agps_decode(const mavlink_message_t* msg, mavlink_oblique_agps_t* oblique_agps)
{
#if MAVLINK_NEED_BYTE_SWAP
	oblique_agps->target_system = mavlink_msg_oblique_agps_get_target_system(msg);
	oblique_agps->target_component = mavlink_msg_oblique_agps_get_target_component(msg);
	mavlink_msg_oblique_agps_get_message(msg, oblique_agps->message);
#else
	memcpy(oblique_agps, _MAV_PAYLOAD(msg), 252);
#endif
}
