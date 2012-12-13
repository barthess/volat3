// MESSAGE OBLIQUE_STORAGE_REQUEST PACKING

#define MAVLINK_MSG_ID_OBLIQUE_STORAGE_REQUEST 158

typedef struct __mavlink_oblique_storage_request_t
{
 uint32_t first; ///< System ID
 uint32_t last; ///< Component ID
 uint8_t target_system; ///< System ID
 uint8_t target_component; ///< Component ID
} mavlink_oblique_storage_request_t;

#define MAVLINK_MSG_ID_OBLIQUE_STORAGE_REQUEST_LEN 10
#define MAVLINK_MSG_ID_158_LEN 10



#define MAVLINK_MESSAGE_INFO_OBLIQUE_STORAGE_REQUEST { \
	"OBLIQUE_STORAGE_REQUEST", \
	4, \
	{  { "first", NULL, MAVLINK_TYPE_UINT32_T, 0, 0, offsetof(mavlink_oblique_storage_request_t, first) }, \
         { "last", NULL, MAVLINK_TYPE_UINT32_T, 0, 4, offsetof(mavlink_oblique_storage_request_t, last) }, \
         { "target_system", NULL, MAVLINK_TYPE_UINT8_T, 0, 8, offsetof(mavlink_oblique_storage_request_t, target_system) }, \
         { "target_component", NULL, MAVLINK_TYPE_UINT8_T, 0, 9, offsetof(mavlink_oblique_storage_request_t, target_component) }, \
         } \
}


/**
 * @brief Pack a oblique_storage_request message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param target_system System ID
 * @param target_component Component ID
 * @param first System ID
 * @param last Component ID
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_oblique_storage_request_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t target_system, uint8_t target_component, uint32_t first, uint32_t last)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[10];
	_mav_put_uint32_t(buf, 0, first);
	_mav_put_uint32_t(buf, 4, last);
	_mav_put_uint8_t(buf, 8, target_system);
	_mav_put_uint8_t(buf, 9, target_component);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 10);
#else
	mavlink_oblique_storage_request_t packet;
	packet.first = first;
	packet.last = last;
	packet.target_system = target_system;
	packet.target_component = target_component;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 10);
#endif

	msg->msgid = MAVLINK_MSG_ID_OBLIQUE_STORAGE_REQUEST;
	return mavlink_finalize_message(msg, system_id, component_id, 10, 237);
}

/**
 * @brief Pack a oblique_storage_request message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param target_system System ID
 * @param target_component Component ID
 * @param first System ID
 * @param last Component ID
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_oblique_storage_request_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t target_system,uint8_t target_component,uint32_t first,uint32_t last)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[10];
	_mav_put_uint32_t(buf, 0, first);
	_mav_put_uint32_t(buf, 4, last);
	_mav_put_uint8_t(buf, 8, target_system);
	_mav_put_uint8_t(buf, 9, target_component);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 10);
#else
	mavlink_oblique_storage_request_t packet;
	packet.first = first;
	packet.last = last;
	packet.target_system = target_system;
	packet.target_component = target_component;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 10);
#endif

	msg->msgid = MAVLINK_MSG_ID_OBLIQUE_STORAGE_REQUEST;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 10, 237);
}

/**
 * @brief Encode a oblique_storage_request struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param oblique_storage_request C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_oblique_storage_request_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_oblique_storage_request_t* oblique_storage_request)
{
	return mavlink_msg_oblique_storage_request_pack(system_id, component_id, msg, oblique_storage_request->target_system, oblique_storage_request->target_component, oblique_storage_request->first, oblique_storage_request->last);
}

/**
 * @brief Send a oblique_storage_request message
 * @param chan MAVLink channel to send the message
 *
 * @param target_system System ID
 * @param target_component Component ID
 * @param first System ID
 * @param last Component ID
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_oblique_storage_request_send(mavlink_channel_t chan, uint8_t target_system, uint8_t target_component, uint32_t first, uint32_t last)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[10];
	_mav_put_uint32_t(buf, 0, first);
	_mav_put_uint32_t(buf, 4, last);
	_mav_put_uint8_t(buf, 8, target_system);
	_mav_put_uint8_t(buf, 9, target_component);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OBLIQUE_STORAGE_REQUEST, buf, 10, 237);
#else
	mavlink_oblique_storage_request_t packet;
	packet.first = first;
	packet.last = last;
	packet.target_system = target_system;
	packet.target_component = target_component;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OBLIQUE_STORAGE_REQUEST, (const char *)&packet, 10, 237);
#endif
}

#endif

// MESSAGE OBLIQUE_STORAGE_REQUEST UNPACKING


/**
 * @brief Get field target_system from oblique_storage_request message
 *
 * @return System ID
 */
static inline uint8_t mavlink_msg_oblique_storage_request_get_target_system(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  8);
}

/**
 * @brief Get field target_component from oblique_storage_request message
 *
 * @return Component ID
 */
static inline uint8_t mavlink_msg_oblique_storage_request_get_target_component(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  9);
}

/**
 * @brief Get field first from oblique_storage_request message
 *
 * @return System ID
 */
static inline uint32_t mavlink_msg_oblique_storage_request_get_first(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint32_t(msg,  0);
}

/**
 * @brief Get field last from oblique_storage_request message
 *
 * @return Component ID
 */
static inline uint32_t mavlink_msg_oblique_storage_request_get_last(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint32_t(msg,  4);
}

/**
 * @brief Decode a oblique_storage_request message into a struct
 *
 * @param msg The message to decode
 * @param oblique_storage_request C-struct to decode the message contents into
 */
static inline void mavlink_msg_oblique_storage_request_decode(const mavlink_message_t* msg, mavlink_oblique_storage_request_t* oblique_storage_request)
{
#if MAVLINK_NEED_BYTE_SWAP
	oblique_storage_request->first = mavlink_msg_oblique_storage_request_get_first(msg);
	oblique_storage_request->last = mavlink_msg_oblique_storage_request_get_last(msg);
	oblique_storage_request->target_system = mavlink_msg_oblique_storage_request_get_target_system(msg);
	oblique_storage_request->target_component = mavlink_msg_oblique_storage_request_get_target_component(msg);
#else
	memcpy(oblique_storage_request, _MAV_PAYLOAD(msg), 10);
#endif
}
