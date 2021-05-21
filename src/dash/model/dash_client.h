#ifndef DASH_CLIENT_H_
#define DASH_CLIENT_H_

#include <assert.h>
#include <algorithm>
#include <stdlib.h>
#include <numeric>
#include <memory>
#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/ipv4.h"
#include "ns3/packet.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-net-device.h"
#include "dash_headers.h"


namespace ns3 {


class DashClient: public Application {
public:
	DashClient();
	virtual ~DashClient();

	void				Setup(	Ipv4Address 	server_ip,
								uint16_t 	 	server_port,
								std::string 	log_file,
								std::string     qmatrix,
								uint16_t        segments,
								uint32_t id);

public:
	virtual void 		StartApplication (void);
	virtual void 		StopApplication (void);

	 void				ConnectionComplete (Ptr<Socket> socket);
	 void				ConnectionFailed (Ptr<Socket> socket);

	// send requests and receive chunk
	void                ReadQMatrix(std::string filename);
	void                WriteQMatrix(std::string filename);
	std::vector<int>    GenerateComplexity(int meanScene);
	void                SetQualityLevels(std::vector<int> complexity);
	void				SendRequest(int action);
	void				RecvChunk(Ptr<Socket> socket);
	void                Decision();
	void 				log();
	

private:
    struct segment {
        int state;
        double capacity;
        int action;
        double buffer;
        double reward;
        double quality;
        int complexity;
        double q_penalty;
        double b_penalty;
        double rebuffering;
        double time;
        double downloadTime;
        bool exploration;
    };

    std::vector<double> rates;
    
    std::vector<int>    m_complexity;    
    
	bool				m_client_running;
	bool				m_accepted; // TCP connection accepted
	uint16_t			m_server_port;
	Ipv4Address			m_server_IP;
	Ptr<Socket> 		m_socket;
	unsigned int		m_id; // id of the user could be useful
	int                 m_state;
	uint16_t            m_segment;
	uint16_t            
	m_segments;
	double              m_buffer;
	uint32_t            m_toDownload;
	bool                m_downloading;
	double              m_tau;
	double              m_alpha;
	double              m_lambda;
	std::string         q_file;
	segment             current;

	// download/buffer management
	std::vector<segment> stats;
	std::vector<std::vector<double> > q_matrix; 
    std::vector<std::vector<double> > qualities;

	// log files
	std::ofstream*		m_log;
	
	int                 FindState(double capacity, double buffer, int complexity, double quality);
	int                 Softmax(int state, double tau, double prevQuality);
	int                 FindPds(int state);
    void                FindReward(segment &prev, double prevQuality, double downloadTime, double prevBuffer);
    void                ParallelUpdate(segment prev, int next_state, double prevBuffer);
    void                SingleUpdate(segment prev, int next_state, double prevBuffer);
    int                 GetComplexityFromState(int state);
    int                 GetBufferFromState(int state);
    int                 GetCapacityFromState(int state);
    int                 GetStateFromBuffer(double buffer);    
    int                 GetStateFromCapacity(double capacity);    
    int                 GetStateFromQuality(double quality);

};

} /* namespace ns3 */

#endif /* DASH_CLIENT_H_ */
