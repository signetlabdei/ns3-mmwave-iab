
#include "ddash_client.h"

namespace ns3 {

DdashClient::DdashClient()
:
 	m_client_running(false)
{
}

DdashClient::~DdashClient()
{
}

void DdashClient::Setup (Ipv4Address server_ip,
                        uint16_t server_port,
                        std::string log_file,
                        uint16_t segments,
                        uint32_t id,
                        double temp,
                        int history)
{
	// Setup socket and callback
	m_server_port=server_port;
	m_server_IP=server_ip;
	m_socket = Socket::CreateSocket (GetNode (),TcpSocketFactory::GetTypeId ());
	assert(m_socket->Bind()==0);

	int mean_scene=5;
	m_memory = history;
	m_steps = 5;
	m_aggr = 0;
	
	m_accepted=false;
	
	for (int i = 0; i < m_memory; i++) {
	    current.capacity.push_back(0);
	}
	
	rates.push_back(10);
	rates.push_back(6);
	rates.push_back(4);
	rates.push_back(3);
	rates.push_back(2);
	rates.push_back(1);
	rates.push_back(0.5);
	rates.push_back(0.3);
	
	m_buffer=0;
	m_toDownload=0;
	m_segments=segments;
	m_log=new std::ofstream;
	m_log->open(log_file.c_str());
	
	// start python learner
	std::stringstream client_name;
	client_name << "client" << id;
	m_name = client_name.str();
	std::stringstream pyfile;
	pyfile << m_name << "_listening";
	std::ofstream f;
	f.open(pyfile.str().c_str());
	f.close();
	system(("python client.py " + m_name + " " + std::to_string(temp) + "&").c_str());
	
    std::string qualityfile="quality_matrix.csv";
    std::ifstream qualitystream;
    qualitystream.open(qualityfile.c_str());
    for (int i = 0; i < 5; i++) {
        std::vector<double> v;
	    qualities.push_back(v);
		for (int j = 0; j < 8; j++) {
		    double quality = 0;
			qualitystream >> quality;
			qualities.at(i).push_back(quality);
		}
	}
	qualitystream.close();
	
	// set quality levels
	std::vector<int> complexity = GenerateComplexity(mean_scene);
	SetQualityLevels(complexity);

	// flow id
	m_id = id; //rand();

	m_segment=0;
}

void DdashClient::SetAggressiveness (double aggr) 
{
	std::cout << "ns3 - " << m_name << " - SetAggressiveness: " << aggr << std::endl;
    m_aggr = aggr;
}

void DdashClient::StartApplication (void)
{
	m_client_running=true;

	m_socket->Connect (InetSocketAddress(m_server_IP, m_server_port));

	m_socket->SetConnectCallback (MakeCallback (&DdashClient::ConnectionComplete, this),
								  MakeCallback (&DdashClient::ConnectionFailed, this));

	std::cout << "CLIENT " << m_id << " (D-DASH) STARTED\n";

	return;
}

void DdashClient::StopApplication (void)
{
	m_client_running=false;
	m_client_running=false;
	m_log->flush();
	m_log->close();
	std::cout << "CLIENT "<< m_id << " STOPPED\n";
	remove((m_name + "_listening").c_str());
	return;
}

void DdashClient::ConnectionComplete (Ptr<Socket> socket)
{
	socket->SetRecvCallback (MakeCallback (&DdashClient::RecvChunk, this));
	m_accepted=true;

	std::cout << "CLIENT "<< m_id << ": CONNECTION ACCEPTED\n";

    //schedule request
    current.time=Simulator::Now().GetSeconds();
	Decision();

	return;
}

void DdashClient::ConnectionFailed (Ptr<Socket> socket)
{
	std::cout << "CLIENT " << m_id << ": CONNECTION FAILED\n";

	m_client_running = false;
}

std::vector<int> DdashClient::GenerateComplexity(int mean_scene) {
    std::vector<int> complexity;
    
    Ptr<ExponentialRandomVariable> sceneRng = CreateObject<ExponentialRandomVariable> ();
    Ptr<UniformRandomVariable> compRng = CreateObject<UniformRandomVariable> ();
    uint16_t i=0;
    int previous = 0;
    int sceneComp=0;
    while(i < m_segments) {
        int nextScene = sceneRng->GetInteger(mean_scene,m_segments);
        previous=sceneComp;
        while(sceneComp==previous){
            sceneComp=(int)compRng->GetValue(0,5);
        }
        for(int j=0;j<nextScene;j++){
            complexity.push_back(sceneComp);
        }
        i+=nextScene;
    }
    return complexity;
}

void DdashClient::SetQualityLevels(std::vector<int> complexity) {
    m_complexity = complexity;
}

void DdashClient::SendRequest (int action)
{
    double rate=rates.at(action);
	if (m_client_running && m_accepted)
	{
		// create header
		DashHeader_req h_req;

		h_req.client_id=m_id;
		h_req.chunk_number=m_segment;
		h_req.chunk_size=rate*T_CHUNK*125000;
		m_toDownload=h_req.chunk_size;

		// create the request
		uint8_t packet_buffer[sizeof(DashHeader_req)];
		memcpy(packet_buffer, &h_req, sizeof(DashHeader_req));
		Ptr<Packet> packet = Create<Packet> (packet_buffer,sizeof(DashHeader_req));
		// send the request
		m_socket->Send(packet);
    }
	return;
}

void DdashClient::RecvChunk(Ptr<Socket> socket)
{

	Ptr<Packet> r_packet;
	Address r_address;
    //std::cout<<Simulator::Now().GetSeconds() << ": client received a packet\n";
	//get the packet
	r_packet=socket->RecvFrom(r_address);
	assert(r_packet);

	if (!m_client_running)
	{
		std::cout << "packet received, but sender is not running\n";
		return;
	}
    uint32_t p_size=r_packet->GetSize();
    // read packet
	uint8_t *packet_buffer;
	packet_buffer=(uint8_t *)malloc(p_size);
	r_packet->CopyData(packet_buffer,p_size);

	// read header
	DashHeader_chunk r_header;
	memcpy(&r_header,packet_buffer,sizeof(DashHeader_chunk));

    if (!m_downloading)
    {
	    // first packet should always been greater than chunk header
	    assert(r_packet->GetSize()>=sizeof(DashHeader_chunk));

		// check if client id is correct
		assert(r_header.client_id==m_id);

		//m_playoutBuffer[m_current_slot_d].download_begin=Simulator::Now().GetSeconds();
		assert(m_toDownload==r_header.chunk_size);
		m_toDownload-=p_size;
		assert(m_segment==r_header.chunk_number);
		m_downloading=true;

		// Set client aggressiveness
	    SetAggressiveness((double)r_header.penalty);

    }
	else if (m_toDownload>p_size)
	{
		// read packet
		m_toDownload-=p_size;

	}
	else if (m_toDownload<=p_size)
	{
	    m_downloading=false;
	    current.time=Simulator::Now().GetSeconds();
	    double download_time=current.time-current.download_time;
	    double prevBuffer = m_buffer;
	    m_buffer+=T_CHUNK-download_time;
	    if(m_buffer<T_CHUNK) {
	        m_buffer=T_CHUNK;
	    }
	    Time tNext (Seconds (0.000000001));	    
	    if(m_buffer>20) {
	        tNext=Time(Seconds(m_buffer-20));
	        m_buffer=20;
	    }
	    current.buffer=m_buffer;
	    current.download_time=download_time;
	    
	    for (int i = m_memory - 1; i > 0; i--) {
	        current.capacity.at(i) = current.capacity.at(i-1);
	    }
	    current.capacity.front() = T_CHUNK*rates.at(current.action)/download_time;
	    double prevQuality=0;
	    if(m_segment>0) {
	        prevQuality=stats.back().quality;
	    }
	    FindReward(current,prevQuality,download_time,prevBuffer);
	    std::cout << "CLIENT "<< m_id << ": DOWNLOADED SEGMENT " << m_segment << "\n";
	    std::cout<<"Reward: "<<current.reward<<"\n";
	    std::cout<<"Rebuffering: "<<current.rebuffering<<"\n";	    
	    std::cout<<"Qpenalty: "<<current.q_penalty<<"\n";
	    std::cout<<"Bpenalty: "<<current.b_penalty<<"\n";
	    std::cout<<"Capacity: "<<current.capacity.at(0)<<"\n";
	    std::cout<<"Time: "<<Simulator::Now().GetSeconds()<<"\n";
	    std::cout<<"Wait: "<<tNext.GetSeconds()<<"\n";
	    stats.push_back(current);
	    m_segment++;
	    Log();
	    Simulator::Schedule(tNext, &DdashClient::Decision, this);
	}
	return;
}

void DdashClient::Decision()
{
    if(m_segment>=m_segments){
        StopApplication();
        return;
    }
    std::cout<<"Segment: "<<m_segment<<"\n";
    std::vector<double> capacity;
    if(m_segment>0) {
	    segment prev=stats.back();
        current.capacity=prev.capacity;
    }
    current.download_time = Simulator::Now().GetSeconds();
    current.complexity=m_complexity.at(m_segment);
    std::cout << "CLIENT "<< m_id << ": REQUESTED SEGMENT " << m_segment << "\n";
	std::cout<<"Buffer: "<<m_buffer<<"\n";
    current.action=ChooseAction(current);
    std::cout<<"Action: "<<current.action<<"\n";
    current.quality=qualities.at(current.complexity).at(current.action);
    std::cout<<"Quality: "<<current.quality<<"\n";
    SendRequest(current.action);
}

int DdashClient::ChooseAction(segment last)
{
    int action = 0;
    std::string action_str;
    float state[m_memory + 4];
    for (int i = 0; i < m_memory; i++) {
        state[i] = (float) last.capacity.at(i);
    }
    state[m_memory] = (float)last.buffer / 20;
    state[m_memory + 1] = (float)last.complexity / 5.0;
    state[m_memory + 2] = (float)last.quality;
    state[m_memory + 3] = (float)m_aggr;
    
    req_file.open(m_name + "_req");
    for(float f : state)
        req_file << std::to_string(f) << std::endl;
    req_file.close();
    
	// Print state for debug
    std::cout << "ns3 - " << m_name << " STATE: [";
    for ( int i = 0; i < m_memory + 4; i++ ) {
        std::cout << state[i] << " ";
    }
    std::cout << "]" << std::endl;
	
	// Create request file flag
	std::ofstream f;
	f.open(m_name + "_req_flag");
	f.close();

	// Wait for action
	while(! std::ifstream(m_name + "_resp_flag"));
	
	// Read response
	resp_file.open(m_name + "_resp");
	if (resp_file.is_open())
	{
		getline(resp_file, action_str);
		std::cout << "ns3 - " << m_name << " - Received action: " << action_str << std::endl;
		if (action_str.length() == 0)
		{
		    std::cout<< "ERROR - no action returned! Continuing with previous action" << std::endl;
		} else
		{
		    action = stoi( action_str );
		}
		resp_file.close();
		remove((m_name + "_resp").c_str());
		remove((m_name + "_resp_flag").c_str());
	}
    return action;
    
}

void DdashClient::FindReward(segment &last, double prevQuality, double download_time, double prevBuffer)
{
    double b_penalty = 0;
    double reward = last.quality;
    if(last.capacity.at(0)<=0) {
        last.q_penalty=0;
        last.b_penalty=0;
        last.rebuffering=0;
        last.reward=0;
    } else {
        last.q_penalty = 2*std::abs(reward - prevQuality);
        double low_buffer = 12-last.buffer;
        if(low_buffer<0) {
            low_buffer=0;
        }
        b_penalty = 0.001 * low_buffer * low_buffer;
        last.rebuffering=0;
        if (download_time>prevBuffer) {
            last.rebuffering = download_time-prevBuffer;
        }
        b_penalty+= 50 * last.rebuffering;
        reward += -(b_penalty + last.q_penalty);
        reward -= m_aggr * (rates.at(last.action) / rates.at(0));
        if(reward<0) {
            reward=0;
        }
        last.b_penalty=b_penalty;
        last.reward=reward;
    }
}

void DdashClient::Log()
{
	
	*(m_log) << "{\"capacity\": ";
	*(m_log) << current.capacity.at(0);
	*(m_log) << ", \"action\": ";
	*(m_log) << current.action;
    *(m_log) << ", \"quality\": ";
	*(m_log) << current.quality;
    *(m_log) << ", \"buffer\": ";
	*(m_log) << current.buffer;
    *(m_log) << ", \"rebuffering_time\": ";
	*(m_log) << current.rebuffering;
    *(m_log) << ", \"timestamp\": ";
	*(m_log) << current.time;
    *(m_log) << ", \"state\": [";
    for (int i = 0;i < m_memory; i++) {
        (*m_log) << current.capacity.at(i) / 20 << ", ";
    }
    *(m_log) << current.buffer/20 << ", ";
    *(m_log) << current.complexity/5.0 << ", ";
    *(m_log) << current.quality << ", ";
    *(m_log) << m_aggr << "]}\n" << std::flush;
	return;
}

} /* namespace ns3 */
