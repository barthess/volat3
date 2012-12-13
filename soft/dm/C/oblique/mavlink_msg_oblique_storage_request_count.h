// MESSAGE OBLIQUE_STORAGE_REQUEST_COUNT PACKING

#define MAVLINK_MSG_ID_OBLIQUE_STORAGE_REQUEST_COUNT 156

typedef struct __mavlink_oblique_storage_request_count_t
{
 uint8_t target_system; ///< System ID
 uint8_t target_component; ///< Component ID
} mavlink_oblique_storage_request_count_t;

#define MAVLINK_MSG_ID_OBLIQUE_STORAGE_REQUEST_COUNT_LEN 2
#define MAVLINK_MSG_ID_156_LEN 2



#define MAVLINK_MESSAGE_INFO_OBLIQUE_STORAGE_REQUEST_COUNT { \
	"OBLIQUE_STORAGE_REQUEST_COUNT", \
	2, \
	{  { "target_system", NULL, MAVLINK_TYPE_UINT8_T, 0, 0, offsetof(mavlink_oblique_storage_request_count_t, target_system) }, \
         { "target_component", NULL, MAVLINK_TYPE_UINT8_T, 0, 1, offsetof(mavlink_oblique_storage_request_count_t, target_component) }, \
         } \
}


/**
 * @brief Pack a oblique_storage_request_count message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param target_system System ID
 * @param target_component Component ID
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_oblique_storage_request_count_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t target_system, uint8_t target_component)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[2];
	_mav_put_uint8_t(buf, 0, target_system);
	_mav_put_uint8_t(buf, 1, target_component);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 2);
#else
	mavlink_oblique_storage_request_count_t packet;
	packet.target_system = target_system;
	packet.target_component = target_component;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 2);
#endif

	msg->msgid = MAVLINK_MSG_ID_OBLIQUE_STORAGE_REQUEST_COUNT;
	return mavlink_finalize_message(msg, system_id, component_id, 2, 130);
}

/**
 * @brief Pack a oblique_storage_request_count message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param target_system System ID
 * @param target_component Component ID
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_oblique_storage_request_count_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t target_system,uint8_t target_component)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[2];
	_mav_put_uint8_t(buf, 0, target_system);
	_mav_put_uint8_t(buf, 1, target_component);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 2);
#else
	mavlink_oblique_storage_request_count_t packet;
	packet.target_system = target_system;
	packet.target_component = target_component;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 2);
#endif

	msg->msgid = MAVLINK_MSG_ID_OBLIQUE_STORAGE_REQUEST_COUNT;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 2, 130);
}

/**
 * @brief Encode a oblique_storage_request_count struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param oblique_storage_request_count C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_oblique_storage_request_count_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_oblique_storage_request_count_t* oblique_storage_request_count)
{
	return mavlink_msg_oblique_storage_request_count_pack(system_id, component_id, msg, oblique_storage_request_count->target_system, oblique_storage_request_count->target_component);
}

/**
 * @brief Send a oblique_storage_request_count message
 * @param chan MAVLink channel to send the message
 *
 * @param target_system System ID
 * @param target_component Component ID
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_oblique_storage_request_count_send(mavlink_channel_t chan, uint8_t target_system, uint8_t target_component)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[2];
	_mav_put_uint8_t(buf, 0, target_system);
	_mav_put_uint8_t(buf, 1, target_component);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OBLIQUE_STORAGE_REQUEST_COUNT, buf, 2, 130);
#else
	mavlink_oblique_storage_request_count_t packet;
	packet.target_system = target_system;
	packet.target_component = target_component;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OBLIQUE_STORAGE_REQUEST_COUNT, (const char *)&packet, 2, 130);
#endif
}

#endif

// MESSAGE OBLIQUE_STORAGE_REQUEST_COUNT UNPACKING


/**
 * @brief Get field target_system from oblique_storage_request_count message
 *
 * @return System ID
 */
static inline uint8_t mavlink_msg_oblique_storage_request_count_get_target_system(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  0);
}

/**
 * @brief Get field target_component from oblique_storage_request_count message
 *
 * @return Component ID
 */
static inline uint8_t mavlink_msg_oblique_storage_request_count_get_target_component(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  1);
}

/**
 * @brief Decode a oblique_storage_request_count message into a struct
 *
 * @param msg The message to decode
 * @param oblique_storage_request_count C-struct to decode the message contents into
 */
static inline void mavlink_msg_oblique_storage_request_count_decode(const mavlink_message_t* msg, mavlink_oblique_storage_request_count_t* oblique_storage_request_count)
{
#if MAVLINK_NEED_BYTE_SWAP
	oblique_storage_request_count->target_system = mavlink_msg_oblique_storage_request_count_get_target_system(msg);
	oblique_storage_request_count->target_component = mavlink_msg_oblique_storage_request_count_get_target_component(msg);
#else
	memcpy(oblique_storage_request_count, _MAV_PAYLOAD(msg), 2);
#endif
}
