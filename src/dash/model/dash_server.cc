
#include "dash_server.h"

namespace ns3 {

DashServer::DashServer()
:
 	m_server_running(false)
{
}

DashServer::~DashServer()
{
}

void DashServer::Setup (uint16_t in_port,
                        std::string log_file,
                        bool learning,
                        int memory,
                        double aggressiveness,
                        double temp)
{

	// Setup socket and callback
	m_in_port=in_port;
	InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny(), m_in_port);
	m_socket = Socket::CreateSocket (GetNode (),TcpSocketFactory::GetTypeId ());
	assert(m_socket->Bind(local)==0);
	
	m_memory = memory;
	for (int i = 0; i < m_memory; i++) {
	    m_rates.push_back(0);
	}
	
	m_learning = learning;

	m_bytesent=0;
	m_est_cap=0.5e6/8;
	m_last_estcap=0;
	m_toSend=0;
	m_bitratePenalty = aggressiveness;

	m_log=new std::ofstream;
	m_log->open(log_file.c_str());
	
	/*	
	if (m_learning) {
	    // start python learner
		std::cout << "SERVER LEARNING ENABLED!" << std::endl;
	    std::stringstream pyfile;
	    std::ofstream f;
	    f.open("server_listening");
	    f.close();
	    std::string command="python server.py server " + std::to_string(temp) + "&";
	    system(command.c_str());
	}
	*/

//    //schedule the update link capacities method
//	Time tNext_2 (Seconds (4));
//	Simulator::Schedule (tNext_2, &DashServer::est_cap, this);
}

void DashServer::StartApplication (void)
{
	m_server_running=true;

	// start listening
    assert(m_socket->Listen ()==0);

    // setup callbacks for connection request
    m_socket->SetAcceptCallback (MakeCallback (&DashServer::ConnectionRequested, this),
                                 MakeCallback (&DashServer::ConnectionAccepted, this));
                                 
	return;
}

void DashServer::StopApplication (void)
{
	m_server_running=false;

	return;
}

bool DashServer::ConnectionRequested (Ptr<Socket> socket, const Address& address)
{
  std::cout << Simulator::Now().GetSeconds() << " Server: ConnectionRequested\n";
  return true;
}

void DashServer::ConnectionAccepted (Ptr<Socket> socket, const Address& address)
{

	std::cout  << Simulator::Now().GetSeconds() << " Connection Accepted\n";

	// Now we've blocked, so register the up-call for later
	socket->SetRecvCallback (MakeCallback (&DashServer::RecvRequest, this));
    // sent calback
}

void DashServer::AddRequest (uint32_t client)
{
    bool newConn = true;
    double now = Simulator::Now().GetSeconds();
    for (uint32_t i = 0; i < m_clients.size(); i++) {
        if (newConn) {
            if (client == m_clients.at(i).client) {
                newConn = false;
                m_clients.at(i).lastRequest = now;
            }
        }
    }
    if (newConn) {
        clientConn conn;
        conn.client = client;
        conn.lastRequest = now;
        m_clients.push_back(conn);
    }
    UpdateClientList();
    double pastAggr = m_bitratePenalty;
    if (m_learning) {
        UpdateAggressiveness();
    }
    log(pastAggr);
}

void DashServer::UpdateAggressiveness() 
{
    // TODO format request file
    double timestamp = Simulator::Now().GetSeconds();
    req_file.open("server_req");
    req_file << m_clients.size() << std::endl;
    for (int i = 0; i < m_memory; i++) {
        req_file << (float)m_rates.at(i) << std::endl;
    }
    for (uint32_t i = 0; i < m_clients.size(); i++) {
        req_file << m_clients.at(i).client << std::endl;
    }
    req_file << (float)timestamp << std::endl;
    req_file.close();
    
	// Create request file flag
	std::ofstream f;
	f.open("server_req_flag");
	f.close();

	// Wait for action
	while(! std::ifstream("server_resp_flag"));
	
	// Read response
	resp_file.open("server_resp");
	if (resp_file.is_open())
	{
	    std::string action_str;
		getline(resp_file, action_str);
		std::cout << "Received aggressiveness: " << action_str << std::endl;
		m_bitratePenalty = std::stod( action_str );
		resp_file.close();
		std::string resp = "server_resp";
		remove(resp.c_str());
		remove((resp+"_flag").c_str());
	}
}


void DashServer::UpdateClientList ()
{
    std::vector<clientConn> newList;
    double time = Simulator::Now().GetSeconds();
    for (uint32_t i = 0; i < m_clients.size(); i++) {
        clientConn client = m_clients.at(i);
        if (time - client.lastRequest <= 5) {
            newList.push_back(client);
        }
    }
    m_clients = newList;
}

void DashServer::RecvRequest (Ptr<Socket> socket)
{
	if (m_server_running)
	{
	    
	
		Ptr<Packet> r_packet;
		Address s_address;

		// get the packet
		r_packet=socket->RecvFrom(s_address);
		assert(r_packet);

		// assert this is request
		assert(r_packet->GetSize()>=sizeof(DashHeader_req));

		// read packet
		uint32_t p_size=r_packet->GetSize();
		uint8_t *packet_buffer;
		packet_buffer=(uint8_t *)malloc(p_size);
		r_packet->CopyData(packet_buffer,p_size);

		// get the feedback header
		DashHeader_req r_header;
		memcpy(&r_header,packet_buffer,sizeof(DashHeader_req));
		


		std::cout<<Simulator::Now().GetSeconds() << ": received a request of a: " << r_header.chunk_size << " size chunk from " << r_header.client_id << "\n";
		//std::cout.flush();
	    AddRequest(r_header.client_id);
		// create header
		DashHeader_chunk h_chunk;
		h_chunk.client_id=r_header.client_id;
		h_chunk.chunk_number=r_header.chunk_number;
		h_chunk.chunk_size=r_header.chunk_size;
		h_chunk.packet_size=sizeof(DashHeader_chunk)+r_header.chunk_size;
		h_chunk.penalty=(float)m_bitratePenalty;

		// create the request
		uint8_t* packet_buffer_chunk;
		packet_buffer_chunk=(uint8_t *)malloc(sizeof(DashHeader_chunk)+r_header.chunk_size);
		memcpy(packet_buffer_chunk, &h_chunk, sizeof(DashHeader_chunk));
		Ptr<Packet> packet = Create<Packet> (packet_buffer_chunk,h_chunk.packet_size);
        

		// send the chunk
		int psize=socket->Send(packet);
		//std::cout << socket->GetTxAvailable() << "tx available \n";
		//assert( socket->GetTxAvailable()== 131072); //<< "\n";
		//std::cout << socket->GetTxAvailable() << "\n";
		assert(psize==(int)(packet->GetSize()));
		m_toSend=psize;
		socket->SetDataSentCallback(MakeCallback(&DashServer::est_cap,this));

		//m_bytesent+=(double)packet->GetSerializedSize();
        
	}

	return;
}

// void DashServer::log(double past_aggr)
// {
//     double currentTime = Simulator::Now().GetSeconds();
// 	*(m_log) << "{\"capacity\": ";
// 	*(m_log) << m_est_cap;
// 	*(m_log) << ", \"past_aggressiveness\": ";
// 	*(m_log) << past_aggr;
//     *(m_log) << ", \"aggressiveness\": ";
// 	*(m_log) << m_bitratePenalty;
//     *(m_log) << ", \"clients\": ";
// 	*(m_log) << m_clients.size();
//     *(m_log) << ", \"timestamp\": ";
// 	*(m_log) << currentTime;
//     *(m_log) << ", \"client_ids\": [";
//     for (uint32_t i = 0;i < m_clients.size() - 1; i++) {
//         (*m_log) << m_clients.at(i).client << ", ";
//     }
//     *(m_log) << m_clients.at(m_clients.size() - 1).client << "]";
//     *(m_log) << ", \"state\": [";
//     *(m_log) << m_clients.size() << ", ";
//     for (int i = 0; i < m_memory; i++) {
//         *(m_log) << m_rates.at(i) << ", ";
//     }
//     *(m_log) << currentTime << "]}\n" << std::endl;
// 	return;    
// }

void DashServer::log(double past_aggr)
{
    double currentTime = Simulator::Now().GetSeconds();
	// *(m_log) << "{\"capacity\": ";
	*(m_log) << m_est_cap << " ";
	// *(m_log) << ", \"past_aggressiveness\": ";
	*(m_log) << past_aggr << " ";
    // *(m_log) << ", \"aggressiveness\": ";
	*(m_log) << m_bitratePenalty << " ";
    // *(m_log) << ", \"clients\": ";
	*(m_log) << m_clients.size() << " ";
    // *(m_log) << ", \"timestamp\": ";
	*(m_log) << currentTime << std::endl;
    // *(m_log) << ", \"client_ids\": [";
    // for (uint32_t i = 0;i < m_clients.size() - 1; i++) {
    //     (*m_log) << m_clients.at(i).client << ", ";
    // }
    // *(m_log) << m_clients.at(m_clients.size() - 1).client << "]";
    // *(m_log) << ", \"state\": [";
    // *(m_log) << m_clients.size() << ", ";
    // for (int i = 0; i < m_memory; i++) {
    //     *(m_log) << m_rates.at(i) << ", ";
    // }
    // *(m_log) << currentTime << "]}\n" << std::endl;
	return;    
}

void DashServer::est_cap(Ptr<Socket> socket, uint32_t dataSent)
{
    //std::cout<<Simulator::Now().GetSeconds() << ": server sent a packet\n";
	m_bytesent+=dataSent;
	m_toSend-=dataSent;

	float Delta=Simulator::Now().GetSeconds()-m_last_estcap;

	if (Delta>=2)
	{
		m_last_estcap=Simulator::Now().GetSeconds();

		m_est_cap=m_bytesent/Delta/125000;
		m_bytesent=0.0;
		for (int i = m_memory - 1; i > 0; i--) {
	        m_rates.at(i)=m_rates.at(i-1);
	    }
        m_rates.at(0)=m_est_cap;

		//std::cout << Delta << " " << dataSent << "\n";
	}
	//m_coordinator->m_cap=m_est_cap/8.0;


}

} /* namespace ns3 */
