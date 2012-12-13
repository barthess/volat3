// MESSAGE OBLIQUE_STORAGE_COUNT PACKING

#define MAVLINK_MSG_ID_OBLIQUE_STORAGE_COUNT 157

typedef struct __mavlink_oblique_storage_count_t
{
 uint32_t count; ///< System ID
} mavlink_oblique_storage_count_t;

#define MAVLINK_MSG_ID_OBLIQUE_STORAGE_COUNT_LEN 4
#define MAVLINK_MSG_ID_157_LEN 4



#define MAVLINK_MESSAGE_INFO_OBLIQUE_STORAGE_COUNT { \
	"OBLIQUE_STORAGE_COUNT", \
	1, \
	{  { "count", NULL, MAVLINK_TYPE_UINT32_T, 0, 0, offsetof(mavlink_oblique_storage_count_t, count) }, \
         } \
}


/**
 * @brief Pack a oblique_storage_count message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param count System ID
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_oblique_storage_count_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint32_t count)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[4];
	_mav_put_uint32_t(buf, 0, count);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 4);
#else
	mavlink_oblique_storage_count_t packet;
	packet.count = count;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 4);
#endif

	msg->msgid = MAVLINK_MSG_ID_OBLIQUE_STORAGE_COUNT;
	return mavlink_finalize_message(msg, system_id, component_id, 4, 5);
}

/**
 * @brief Pack a oblique_storage_count message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param count System ID
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_oblique_storage_count_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint32_t count)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[4];
	_mav_put_uint32_t(buf, 0, count);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 4);
#else
	mavlink_oblique_storage_count_t packet;
	packet.count = count;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 4);
#endif

	msg->msgid = MAVLINK_MSG_ID_OBLIQUE_STORAGE_COUNT;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 4, 5);
}

/**
 * @brief Encode a oblique_storage_count struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param oblique_storage_count C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_oblique_storage_count_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_oblique_storage_count_t* oblique_storage_count)
{
	return mavlink_msg_oblique_storage_count_pack(system_id, component_id, msg, oblique_storage_count->count);
}

/**
 * @brief Send a oblique_storage_count message
 * @param chan MAVLink channel to send the message
 *
 * @param count System ID
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_oblique_storage_count_send(mavlink_channel_t chan, uint32_t count)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[4];
	_mav_put_uint32_t(buf, 0, count);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OBLIQUE_STORAGE_COUNT, buf, 4, 5);
#else
	mavlink_oblique_storage_count_t packet;
	packet.count = count;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OBLIQUE_STORAGE_COUNT, (const char *)&packet, 4, 5);
#endif
}

#endif

// MESSAGE OBLIQUE_STORAGE_COUNT UNPACKING


/**
 * @brief Get field count from oblique_storage_count message
 *
 * @return System ID
 */
static inline uint32_t mavlink_msg_oblique_storage_count_get_count(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint32_t(msg,  0);
}

/**
 * @brief Decode a oblique_storage_count message into a struct
 *
 * @param msg The message to decode
 * @param oblique_storage_count C-struct to decode the message contents into
 */
static inline void mavlink_msg_oblique_storage_count_decode(const mavlink_message_t* msg, mavlink_oblique_storage_count_t* oblique_storage_count)
{
#if MAVLINK_NEED_BYTE_SWAP
	oblique_storage_count->count = mavlink_msg_oblique_storage_count_get_count(msg);
#else
	memcpy(oblique_storage_count, _MAV_PAYLOAD(msg), 4);
#endif
}
