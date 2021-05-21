#ifndef DDASH_H_
#define DDASH_H_

#include <assert.h>
#include <algorithm>
#include <stdlib.h>
#include <math.h>
#include <numeric>
#include <memory>
#include <string>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/ipv4.h"
#include "ns3/packet.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-net-device.h"
#include "dash_headers.h"


namespace ns3 {


class DdashClient: public Application {
public:
	DdashClient();
	virtual ~DdashClient();

	void				Setup(	Ipv4Address 	server_ip,
								uint16_t 	 	server_port,
								std::string 	log_file,
								uint16_t        segments,
								uint32_t id,
								double temp,
								int history);

public:
	virtual void 		StartApplication (void);
	virtual void 		StopApplication (void);

	 void				ConnectionComplete (Ptr<Socket> socket);
	 void				ConnectionFailed (Ptr<Socket> socket);

	// send requests and receive chunk
	std::vector<int>    GenerateComplexity(int meanScene);
	void                SetQualityLevels(std::vector<int> complexity);
	void				SendRequest(int action);
	void				RecvChunk(Ptr<Socket> socket);
	void                Decision();
	void 				Log();
	void                SetAggressiveness(double aggr);
	

private:
    struct segment {
        std::vector<double> capacity;
        int action;
        double buffer;
        double reward;
        double quality;
        int complexity;
        double q_penalty;
        double b_penalty;
        double rebuffering;
        double time;
        double download_time;
    };

    std::vector<double> rates;
    std::vector<int>    m_complexity;    
    
	bool				m_client_running;
	bool				m_accepted; // TCP connection accepted
	uint16_t			m_server_port;
	Ipv4Address			m_server_IP;
	Ptr<Socket> 		m_socket;
	unsigned int		m_id; // id of the user could be useful
	uint16_t            m_segment;
	uint16_t            m_segments;
	double              m_buffer;
	uint32_t            m_toDownload;
	bool                m_downloading;
	double              m_tau;
	int                 m_steps;
	double              m_aggr;
	int                 m_memory;
	segment             current;
	
	// Q-learning python management
	std::ofstream       req_file;
	std::ifstream       resp_file;
	std::string         m_name;

	// download/buffer management
	std::vector<segment> stats;
	std::vector<std::vector<double> > q_matrix; 
    std::vector<std::vector<double> > qualities;

	// log files
	std::ofstream*		m_log;
	
    void                FindReward(segment &prev, double prevQuality, double downloadTime, double prevBuffer);
	int                 ChooseAction(segment last);
};

} /* namespace ns3 */

#endif /* RATE_BASED_H_ */
