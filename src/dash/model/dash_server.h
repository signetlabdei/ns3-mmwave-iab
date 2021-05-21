#ifndef DASH_SERVER_H_
#define DASH_SERVER_H_

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

class DashServer: public Application {
public:
	DashServer();
	virtual ~DashServer();

	void 				    Setup(uint16_t 	 	in_port,
							    std::string 	log_file,
							    bool            learning,
							    int             memory,
							    double          aggressiveness,
								double temp);

public:
	virtual void 		    StartApplication (void);
	virtual void 		    StopApplication (void);
	
	struct clientConn {
	    double lastRequest;
	    uint32_t client;
	};

	// connection
	bool				    ConnectionRequested (Ptr<Socket> socket, const Address& address);
	void				    ConnectionAccepted (Ptr<Socket> socket, const Address& address);
	void 				    log(double past_aggr);
	void 				    est_cap(Ptr<Socket> socket, uint32_t dataSent);
	


	// receive request
	void				    RecvRequest(Ptr<Socket> socket);
	//void 				    SendRequest();

	bool				    m_server_running;
	uint16_t			    m_in_port;
	Ptr<Socket> 		    m_socket;
	//DashCoordinator*	    m_coordinator;
	std::ofstream*		    m_log;
	
	// Q-learning python management
	std::ofstream       req_file;
	std::ifstream       resp_file;
	
	std::vector<double> m_rates;
	int m_memory;

	double				    m_bytesent;
	bool                    m_learning;
	float                   m_bitratePenalty;
	double				    m_est_cap;
    int                     m_connected;
    double                  m_lastCheck;
    std::vector<clientConn> m_clients;
	double				    m_last_estcap;
	
private:    
    int                     m_toSend;
    void                    UpdateAggressiveness();
    void                    AddRequest(uint32_t client);
    void                    UpdateClientList();

};

} /* namespace ns3 */

#endif /* DASH_SERVER_H_ */
