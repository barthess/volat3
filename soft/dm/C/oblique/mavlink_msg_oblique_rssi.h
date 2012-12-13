// MESSAGE OBLIQUE_RSSI PACKING

#define MAVLINK_MSG_ID_OBLIQUE_RSSI 155

typedef struct __mavlink_oblique_rssi_t
{
 uint8_t rssi; ///< RSSI field from command response
 uint8_t ber; ///< BER field from command response
} mavlink_oblique_rssi_t;

#define MAVLINK_MSG_ID_OBLIQUE_RSSI_LEN 2
#define MAVLINK_MSG_ID_155_LEN 2



#define MAVLINK_MESSAGE_INFO_OBLIQUE_RSSI { \
	"OBLIQUE_RSSI", \
	2, \
	{  { "rssi", NULL, MAVLINK_TYPE_UINT8_T, 0, 0, offsetof(mavlink_oblique_rssi_t, rssi) }, \
         { "ber", NULL, MAVLINK_TYPE_UINT8_T, 0, 1, offsetof(mavlink_oblique_rssi_t, ber) }, \
         } \
}


/**
 * @brief Pack a oblique_rssi message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param rssi RSSI field from command response
 * @param ber BER field from command response
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_oblique_rssi_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t rssi, uint8_t ber)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[2];
	_mav_put_uint8_t(buf, 0, rssi);
	_mav_put_uint8_t(buf, 1, ber);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 2);
#else
	mavlink_oblique_rssi_t packet;
	packet.rssi = rssi;
	packet.ber = ber;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 2);
#endif

	msg->msgid = MAVLINK_MSG_ID_OBLIQUE_RSSI;
	return mavlink_finalize_message(msg, system_id, component_id, 2, 94);
}

/**
 * @brief Pack a oblique_rssi message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param rssi RSSI field from command response
 * @param ber BER field from command response
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_oblique_rssi_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t rssi,uint8_t ber)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[2];
	_mav_put_uint8_t(buf, 0, rssi);
	_mav_put_uint8_t(buf, 1, ber);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 2);
#else
	mavlink_oblique_rssi_t packet;
	packet.rssi = rssi;
	packet.ber = ber;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 2);
#endif

	msg->msgid = MAVLINK_MSG_ID_OBLIQUE_RSSI;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 2, 94);
}

/**
 * @brief Encode a oblique_rssi struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param oblique_rssi C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_oblique_rssi_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_oblique_rssi_t* oblique_rssi)
{
	return mavlink_msg_oblique_rssi_pack(system_id, component_id, msg, oblique_rssi->rssi, oblique_rssi->ber);
}

/**
 * @brief Send a oblique_rssi message
 * @param chan MAVLink channel to send the message
 *
 * @param rssi RSSI field from command response
 * @param ber BER field from command response
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_oblique_rssi_send(mavlink_channel_t chan, uint8_t rssi, uint8_t ber)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[2];
	_mav_put_uint8_t(buf, 0, rssi);
	_mav_put_uint8_t(buf, 1, ber);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OBLIQUE_RSSI, buf, 2, 94);
#else
	mavlink_oblique_rssi_t packet;
	packet.rssi = rssi;
	packet.ber = ber;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_OBLIQUE_RSSI, (const char *)&packet, 2, 94);
#endif
}

#endif

// MESSAGE OBLIQUE_RSSI UNPACKING


/**
 * @brief Get field rssi from oblique_rssi message
 *
 * @return RSSI field from command response
 */
static inline uint8_t mavlink_msg_oblique_rssi_get_rssi(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  0);
}

/**
 * @brief Get field ber from oblique_rssi message
 *
 * @return BER field from command response
 */
static inline uint8_t mavlink_msg_oblique_rssi_get_ber(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  1);
}

/**
 * @brief Decode a oblique_rssi message into a struct
 *
 * @param msg The message to decode
 * @param oblique_rssi C-struct to decode the message contents into
 */
static inline void mavlink_msg_oblique_rssi_decode(const mavlink_message_t* msg, mavlink_oblique_rssi_t* oblique_rssi)
{
#if MAVLINK_NEED_BYTE_SWAP
	oblique_rssi->rssi = mavlink_msg_oblique_rssi_get_rssi(msg);
	oblique_rssi->ber = mavlink_msg_oblique_rssi_get_ber(msg);
#else
	memcpy(oblique_rssi, _MAV_PAYLOAD(msg), 2);
#endif
}
