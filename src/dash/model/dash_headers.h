
#ifndef DASH_HEADERS_H_
#define DASH_HEADERS_H_

#include <stdint.h>

namespace ns3 {

enum download_status {idle,wait_sending,waiting,downloading,downloaded};

struct download_s
{
	download_status	status;
	int 			chunk_num;
	float			download_begin;
	float			download_end;
	float			next_begin;
	int				chunk_size;
	int				total_data;
	int				downloaded_data;
	float			ideal;
	float			current_m_rate;
	float			mul_time;
};


#define T_CLIENT_LOGGING 0.1
#define M 8
#define T_CHUNK 2.0
#define T_CONTROLLER 2
//#define DEB_CLIENT


//---------------------- REQUEST HEADER -----------------------------//
//   0                   1                   2                   3
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                         client_id                             |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                        chunk_number                           |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                         chunk_size                            |
//  +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+

struct DashHeader_req
{
    uint32_t client_id;
    uint32_t chunk_number;
    uint32_t chunk_size;
};

//----------------------- CHUNK HEADER ----------------------------//
//   0                   1                   2                   3
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                         client_id                             |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                        chunk_number                           |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                         chunk_size                            |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                       bitrate_penalty                         |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                         packet_size                           |
//  +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+

struct DashHeader_chunk
{
    uint32_t client_id;
    uint32_t chunk_number;
    uint32_t chunk_size;
    uint32_t packet_size;
    float penalty;
};

struct DashHeader_ctrl_client
{
    uint32_t client_id;
    float rate;
    uint32_t iteration;
    uint32_t downloaded;
    float dtime;
    uint32_t info;
};

struct DashHeader_ctrl_server
{
    float lambda;
    uint32_t iteration;
    uint32_t over;
    float T_start;
};

} /* namespace ns3 */

#endif /*DASH_HEADERS_H_ */
