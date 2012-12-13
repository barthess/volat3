/** @file
 *	@brief MAVLink comm protocol testsuite generated from oblique.xml
 *	@see http://qgroundcontrol.org/mavlink/
 */
#ifndef OBLIQUE_TESTSUITE_H
#define OBLIQUE_TESTSUITE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAVLINK_TEST_ALL
#define MAVLINK_TEST_ALL
static void mavlink_test_common(uint8_t, uint8_t, mavlink_message_t *last_msg);
static void mavlink_test_oblique(uint8_t, uint8_t, mavlink_message_t *last_msg);

static void mavlink_test_all(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
	mavlink_test_common(system_id, component_id, last_msg);
	mavlink_test_oblique(system_id, component_id, last_msg);
}
#endif

#include "../common/testsuite.h"


static void mavlink_test_mpiovd_sensors(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
	mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
	mavlink_mpiovd_sensors_t packet_in = {
		93372036854775807ULL,
	93372036854776311ULL,
	963498296,
	963498504,
	18483,
	18587,
	18691,
	18795,
	18899,
	19003,
	19107,
	19211,
	19315,
	19419,
	19523,
	19627,
	19731,
	19835,
	19939,
	20043,
	20147,
	20251,
	20355,
	};
	mavlink_mpiovd_sensors_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        	packet1.time_usec = packet_in.time_usec;
        	packet1.relay = packet_in.relay;
        	packet1.trip = packet_in.trip;
        	packet1.engine_uptime = packet_in.engine_uptime;
        	packet1.voltage_battery = packet_in.voltage_battery;
        	packet1.analog01 = packet_in.analog01;
        	packet1.analog02 = packet_in.analog02;
        	packet1.analog03 = packet_in.analog03;
        	packet1.analog04 = packet_in.analog04;
        	packet1.analog05 = packet_in.analog05;
        	packet1.analog06 = packet_in.analog06;
        	packet1.analog07 = packet_in.analog07;
        	packet1.analog08 = packet_in.analog08;
        	packet1.analog09 = packet_in.analog09;
        	packet1.analog10 = packet_in.analog10;
        	packet1.analog11 = packet_in.analog11;
        	packet1.analog12 = packet_in.analog12;
        	packet1.analog13 = packet_in.analog13;
        	packet1.analog14 = packet_in.analog14;
        	packet1.analog15 = packet_in.analog15;
        	packet1.actuators = packet_in.actuators;
        	packet1.speed = packet_in.speed;
        	packet1.rpm = packet_in.rpm;
        
        

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_mpiovd_sensors_encode(system_id, component_id, &msg, &packet1);
	mavlink_msg_mpiovd_sensors_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_mpiovd_sensors_pack(system_id, component_id, &msg , packet1.time_usec , packet1.relay , packet1.trip , packet1.engine_uptime , packet1.voltage_battery , packet1.analog01 , packet1.analog02 , packet1.analog03 , packet1.analog04 , packet1.analog05 , packet1.analog06 , packet1.analog07 , packet1.analog08 , packet1.analog09 , packet1.analog10 , packet1.analog11 , packet1.analog12 , packet1.analog13 , packet1.analog14 , packet1.analog15 , packet1.actuators , packet1.speed , packet1.rpm );
	mavlink_msg_mpiovd_sensors_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_mpiovd_sensors_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.time_usec , packet1.relay , packet1.trip , packet1.engine_uptime , packet1.voltage_battery , packet1.analog01 , packet1.analog02 , packet1.analog03 , packet1.analog04 , packet1.analog05 , packet1.analog06 , packet1.analog07 , packet1.analog08 , packet1.analog09 , packet1.analog10 , packet1.analog11 , packet1.analog12 , packet1.analog13 , packet1.analog14 , packet1.analog15 , packet1.actuators , packet1.speed , packet1.rpm );
	mavlink_msg_mpiovd_sensors_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
        	comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
	mavlink_msg_mpiovd_sensors_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_mpiovd_sensors_send(MAVLINK_COMM_1 , packet1.time_usec , packet1.relay , packet1.trip , packet1.engine_uptime , packet1.voltage_battery , packet1.analog01 , packet1.analog02 , packet1.analog03 , packet1.analog04 , packet1.analog05 , packet1.analog06 , packet1.analog07 , packet1.analog08 , packet1.analog09 , packet1.analog10 , packet1.analog11 , packet1.analog12 , packet1.analog13 , packet1.analog14 , packet1.analog15 , packet1.actuators , packet1.speed , packet1.rpm );
	mavlink_msg_mpiovd_sensors_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
}

static void mavlink_test_oblique_agps(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
	mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
	mavlink_oblique_agps_t packet_in = {
		5,
	72,
	"CDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQ",
	};
	mavlink_oblique_agps_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        	packet1.target_system = packet_in.target_system;
        	packet1.target_component = packet_in.target_component;
        
        	mav_array_memcpy(packet1.message, packet_in.message, sizeof(char)*250);
        

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_oblique_agps_encode(system_id, component_id, &msg, &packet1);
	mavlink_msg_oblique_agps_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_oblique_agps_pack(system_id, component_id, &msg , packet1.target_system , packet1.target_component , packet1.message );
	mavlink_msg_oblique_agps_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_oblique_agps_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.target_system , packet1.target_component , packet1.message );
	mavlink_msg_oblique_agps_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
        	comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
	mavlink_msg_oblique_agps_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_oblique_agps_send(MAVLINK_COMM_1 , packet1.target_system , packet1.target_component , packet1.message );
	mavlink_msg_oblique_agps_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
}

static void mavlink_test_oblique_rssi(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
	mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
	mavlink_oblique_rssi_t packet_in = {
		5,
	72,
	};
	mavlink_oblique_rssi_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        	packet1.rssi = packet_in.rssi;
        	packet1.ber = packet_in.ber;
        
        

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_oblique_rssi_encode(system_id, component_id, &msg, &packet1);
	mavlink_msg_oblique_rssi_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_oblique_rssi_pack(system_id, component_id, &msg , packet1.rssi , packet1.ber );
	mavlink_msg_oblique_rssi_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_oblique_rssi_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.rssi , packet1.ber );
	mavlink_msg_oblique_rssi_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
        	comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
	mavlink_msg_oblique_rssi_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_oblique_rssi_send(MAVLINK_COMM_1 , packet1.rssi , packet1.ber );
	mavlink_msg_oblique_rssi_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
}

static void mavlink_test_oblique_storage_request_count(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
	mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
	mavlink_oblique_storage_request_count_t packet_in = {
		5,
	72,
	};
	mavlink_oblique_storage_request_count_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        	packet1.target_system = packet_in.target_system;
        	packet1.target_component = packet_in.target_component;
        
        

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_oblique_storage_request_count_encode(system_id, component_id, &msg, &packet1);
	mavlink_msg_oblique_storage_request_count_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_oblique_storage_request_count_pack(system_id, component_id, &msg , packet1.target_system , packet1.target_component );
	mavlink_msg_oblique_storage_request_count_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_oblique_storage_request_count_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.target_system , packet1.target_component );
	mavlink_msg_oblique_storage_request_count_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
        	comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
	mavlink_msg_oblique_storage_request_count_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_oblique_storage_request_count_send(MAVLINK_COMM_1 , packet1.target_system , packet1.target_component );
	mavlink_msg_oblique_storage_request_count_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
}

static void mavlink_test_oblique_storage_count(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
	mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
	mavlink_oblique_storage_count_t packet_in = {
		963497464,
	};
	mavlink_oblique_storage_count_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        	packet1.count = packet_in.count;
        
        

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_oblique_storage_count_encode(system_id, component_id, &msg, &packet1);
	mavlink_msg_oblique_storage_count_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_oblique_storage_count_pack(system_id, component_id, &msg , packet1.count );
	mavlink_msg_oblique_storage_count_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_oblique_storage_count_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.count );
	mavlink_msg_oblique_storage_count_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
        	comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
	mavlink_msg_oblique_storage_count_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_oblique_storage_count_send(MAVLINK_COMM_1 , packet1.count );
	mavlink_msg_oblique_storage_count_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
}

static void mavlink_test_oblique_storage_request(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
	mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
	mavlink_oblique_storage_request_t packet_in = {
		963497464,
	963497672,
	29,
	96,
	};
	mavlink_oblique_storage_request_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        	packet1.first = packet_in.first;
        	packet1.last = packet_in.last;
        	packet1.target_system = packet_in.target_system;
        	packet1.target_component = packet_in.target_component;
        
        

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_oblique_storage_request_encode(system_id, component_id, &msg, &packet1);
	mavlink_msg_oblique_storage_request_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_oblique_storage_request_pack(system_id, component_id, &msg , packet1.target_system , packet1.target_component , packet1.first , packet1.last );
	mavlink_msg_oblique_storage_request_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_oblique_storage_request_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.target_system , packet1.target_component , packet1.first , packet1.last );
	mavlink_msg_oblique_storage_request_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
        	comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
	mavlink_msg_oblique_storage_request_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
	mavlink_msg_oblique_storage_request_send(MAVLINK_COMM_1 , packet1.target_system , packet1.target_component , packet1.first , packet1.last );
	mavlink_msg_oblique_storage_request_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
}

static void mavlink_test_oblique(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
	mavlink_test_mpiovd_sensors(system_id, component_id, last_msg);
	mavlink_test_oblique_agps(system_id, component_id, last_msg);
	mavlink_test_oblique_rssi(system_id, component_id, last_msg);
	mavlink_test_oblique_storage_request_count(system_id, component_id, last_msg);
	mavlink_test_oblique_storage_count(system_id, component_id, last_msg);
	mavlink_test_oblique_storage_request(system_id, component_id, last_msg);
}

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // OBLIQUE_TESTSUITE_H
