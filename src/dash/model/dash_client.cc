
#include "dash_client.h"

namespace ns3 {


DashClient::DashClient()
:
 	m_client_running(false)
{
}

DashClient::~DashClient()
{
}

void DashClient::Setup (Ipv4Address server_ip,
                        uint16_t server_port,
                        std::string log_file,
                        std::string qmatrix,
                        uint16_t segments,
                        uint32_t id)
{
	// Setup socket and callback
	m_server_port=server_port;
	m_server_IP=server_ip;
	m_socket = Socket::CreateSocket (GetNode (),TcpSocketFactory::GetTypeId ());
	assert(m_socket->Bind()==0);
	m_alpha=0.05;
	m_lambda=0.9;
	m_tau=0;
	int mean_scene=5;
	

	m_accepted=false;
	
	q_file=qmatrix;
	
	rates.push_back(10);
	rates.push_back(6);
	rates.push_back(4);
	rates.push_back(3);
	rates.push_back(2);
	rates.push_back(1);
	rates.push_back(0.5);
	rates.push_back(0.25);
	
	m_buffer=0;
	m_toDownload=0;
	m_state=5000;
	m_segments=segments;
	m_log=new std::ofstream;
	m_log->open(log_file.c_str());
	
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
	bool train = false;
	if(!train){
	    ReadQMatrix(q_file);
	} else {
	    for (int i = 0; i < 501; i++) {
	        std::vector<double> v;
	        q_matrix.push_back(v);
		    for (int j = 0; j < 8; j++) {
		        double qvalue = 9;
			    q_matrix.at(i).push_back(qvalue);
		    }
	    }
	}
}

void DashClient::StartApplication (void)
{
	m_client_running=true;

	m_socket->Connect (InetSocketAddress(m_server_IP, m_server_port));

	m_socket->SetConnectCallback (MakeCallback (&DashClient::ConnectionComplete, this),
								  MakeCallback (&DashClient::ConnectionFailed, this));

	std::cout << "dash client: " << m_id << " started\n";

	return;
}

void DashClient::StopApplication (void)
{
	m_client_running=false;

	return;
}

void DashClient::ConnectionComplete (Ptr<Socket> socket)
{
	socket->SetRecvCallback (MakeCallback (&DashClient::RecvChunk, this));
	m_accepted=true;

	std::cout << "client connection accepted\n";

    //schedule request
    current.time=Simulator::Now().GetSeconds();
	Decision();

	return;
}

void DashClient::ConnectionFailed (Ptr<Socket> socket)
{
	std::cout << "Client " << m_id << "failed to open connection.\n";

	m_client_running = false;
}

void DashClient::ReadQMatrix(std::string filename)
{
    std::ifstream qmat;
    qmat.open(filename.c_str());
    for (int i = 0; i < 501; i++) {
        std::vector<double> v;
	    q_matrix.push_back(v);
		for (int j = 0; j < 8; j++) {
		    double qvalue = 0;
			qmat >> qvalue;
			q_matrix.at(i).push_back(qvalue);
		}
	}
	qmat.close();
}

void DashClient::WriteQMatrix(std::string filename)
{
    std::ofstream qmat;
    qmat.open(filename.c_str());
    for (int i = 0; i < 501; i++) {
		for (int j = 0; j < 8; j++) {
			qmat << q_matrix.at(i).at(j);
			if (j<7) {
			    qmat << " ";
			}
		}
		qmat << "\n";
	}
}

std::vector<int> DashClient::GenerateComplexity(int mean_scene) {
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

void DashClient::SetQualityLevels(std::vector<int> complexity) {
    m_complexity = complexity;
}

void DashClient::SendRequest (int action)
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

void DashClient::RecvChunk(Ptr<Socket> socket)
{

	Ptr<Packet> r_packet;
	Address r_address;

	//get the packet
	r_packet=socket->RecvFrom(r_address);
	assert(r_packet);
	if (!m_client_running)
	{
		std::cout << "packet received, but sender is not running\n";
		return;
	}
    uint32_t p_size=r_packet->GetSize();
    if (!m_downloading)
    {
	    // first packet should always been greater than chunk header
	    assert(r_packet->GetSize()>=sizeof(DashHeader_chunk));

		// read packet
		uint8_t *packet_buffer;
		packet_buffer=(uint8_t *)malloc(p_size);
		r_packet->CopyData(packet_buffer,p_size);

		// get the feedback header
		DashHeader_chunk r_header;
		memcpy(&r_header,packet_buffer,sizeof(DashHeader_chunk));

		// check if client id is correct
		assert(r_header.client_id==m_id);

		//m_playoutBuffer[m_current_slot_d].download_begin=Simulator::Now().GetSeconds();
		assert(m_toDownload==r_header.chunk_size);
		m_toDownload-=p_size;
		assert(m_segment==r_header.chunk_number);
		m_downloading=true;
    }
	else if (m_toDownload>p_size)
	{
		// read packet
		m_toDownload-=p_size;
	}
	else if (m_toDownload<=p_size)
	{
	    m_downloading=false;
		std::cout<<"Still "<<m_toDownload<<"\n";
	    current.time=Simulator::Now().GetSeconds();
	    double downloadTime=current.time-current.downloadTime;
	    double prevBuffer = m_buffer;
	    m_buffer+=T_CHUNK-downloadTime;
	    if(m_buffer<T_CHUNK) {
	        m_buffer=T_CHUNK;
	    }
	    Time tNext (Seconds (0.001));
	    if(m_buffer>20) {
	        tNext=Time(Seconds(m_buffer-20));
	        m_buffer=20;
	    }
	    current.buffer=m_buffer;
	    current.downloadTime=downloadTime;
	    current.capacity=T_CHUNK*rates.at(current.action)/downloadTime;
	    double prevQuality=0;
	    if(m_segment>0) {
	        prevQuality=stats.back().quality;
	    }
	    FindReward(current,prevQuality,downloadTime,prevBuffer);
	    std::cout<<"Reward: "<<current.reward<<"\n";
	    std::cout<<"Rebuffering: "<<current.rebuffering<<"\n";	    
	    std::cout<<"Qpenalty: "<<current.q_penalty<<"\n";
	    std::cout<<"Bpenalty: "<<current.b_penalty<<"\n";
	    stats.push_back(current);
	    m_segment++;
	    log();
	    m_buffer -= tNext.GetSeconds ();
	    Simulator::Schedule(tNext, &DashClient::Decision, this);
	}
	return;
}

void DashClient::Decision()
{
    std::cout<<"Segment: "<<m_segment<<"\n";
    m_state=5000;
    double prevQuality = 0;
    if(m_segment>=m_segments){
        WriteQMatrix(q_file.c_str());
        return;
    }
    if(m_segment>0) {
	    segment prev=stats.back();
        m_state=FindState(prev.capacity, prev.buffer, m_complexity.at(m_segment), prev.quality);
        std::cout<<"Capacity: "<<prev.capacity<<"\n";
        if(m_state!=5000) {
            ParallelUpdate(prev,m_state, prev.buffer-T_CHUNK+prev.downloadTime-prev.rebuffering);
        }
        prevQuality = prev.quality;
    }
    current.downloadTime = Simulator::Now().GetSeconds();
    current.complexity=m_complexity.at(m_segment);
	current.state=m_state;
	std::cout<<"Buffer: "<<m_buffer<<"\n";
	std::cout<<"State: "<<m_state<<"\n";

    current.action=Softmax(m_state,m_tau,prevQuality);
    std::cout<<"Action: "<<current.action<<"\n";
    current.exploration=(current.action==Softmax(m_state,0,prevQuality));
    current.quality=qualities.at(current.complexity).at(current.action);
    std::cout<<"Quality: "<<current.quality<<"\n";
    SendRequest(current.action);
}

int DashClient::FindState(double capacity, double buffer, int complexity, double quality)
{
    if (m_segment==0) {
        m_state=5000;
        return m_state;
    }
    m_state = GetStateFromCapacity(capacity) + complexity * 10 + GetStateFromBuffer(buffer) * 50 + GetStateFromQuality(quality) * 500; 
    return m_state;   
}

int DashClient::Softmax(int state, double tau, double prevQuality)
{
    if (state==5000) {
        return 7;
    }
    std::vector<double> action_ps;
    int pds = FindPds(m_state);
    if(tau==0) {
        double best_q=0;
        int best_action=0;
        for(int action=0;action<8;action++) {
            double value=q_matrix.at(pds).at(action) + qualities.at(m_complexity.at(m_segment)).at(action) - std::abs(qualities.at(m_complexity.at(m_segment)).at(action) - prevQuality);
            if(value>best_q) {
                best_q=value;
                best_action=action;
            }
        }
        return best_action;
    }
    
    
    for (int action=0;action<8;action++) {
        double exp_reward = 0;
        exp_reward += q_matrix.at(pds).at(action) - 10 + qualities.at(m_complexity.at(m_segment)).at(action) - std::abs(qualities.at(m_complexity.at(m_segment)).at(action) - prevQuality);
        action_ps.push_back(exp(-exp_reward/tau));
    }
    double totp = 0;
    for (int action=0;action<8;action++) {
        totp += action_ps.at(action);
    }
    
    Ptr<UniformRandomVariable> rng = CreateObject<UniformRandomVariable> ();
    double draw=rng->GetValue(0,totp);
    double drawn_p=0;
    int action=0;
    while (action<8){
        drawn_p += action_ps.at(action);
        if (draw<drawn_p) {
            return action;
        }
        action++;
    }
    return 7;
}

int DashClient::FindPds(int state)
{
    int pds = state%500;
    if(state==5000) {
        pds=500;
    }
    return pds;
}

void DashClient::FindReward(segment &last, double prevQuality, double downloadTime, double prevBuffer)
{
    double b_penalty = 0;
    double reward = last.quality;
    if(last.state==5000) {
        last.q_penalty=0;
        last.b_penalty=0;
        last.rebuffering=0;
        last.reward=0;
    } else {
        last.q_penalty = std::abs(reward - prevQuality);
        double low_buffer = 10-last.buffer;
        if(low_buffer<0) {
            low_buffer=0;
        }
        b_penalty = 0.001 * low_buffer * low_buffer;
        last.rebuffering=0;
        if (downloadTime>prevBuffer) {
            last.rebuffering = downloadTime-prevBuffer;
        }
        b_penalty+= 50 * last.rebuffering;
        reward += -(b_penalty + last.q_penalty);
        if(reward<0) {
            reward=0;
        }
        last.b_penalty=b_penalty;
        last.reward=reward;
    }
}

void DashClient::ParallelUpdate(segment prev, int next_state, double prevBuffer)
{
    SingleUpdate(prev,next_state, prevBuffer);
    std::vector<double> buffer_states;
    
    buffer_states.push_back(2);
    buffer_states.push_back(3);
    buffer_states.push_back(4);
    buffer_states.push_back(5);
    buffer_states.push_back(6);
    buffer_states.push_back(8);
    buffer_states.push_back(10);
    buffer_states.push_back(12);
    buffer_states.push_back(15);
    buffer_states.push_back(18);
    
    for (int i=0;i<10;i++) {
        segment simulated=prev;
        if(GetStateFromBuffer(prev.state)==i) {
            continue;
        }
        double prevBuffer = buffer_states.at(i);
        simulated.state = GetCapacityFromState(simulated.state) + 10 * GetComplexityFromState(simulated.state) + 50 * GetStateFromBuffer(prevBuffer);
        simulated.buffer=prevBuffer-simulated.downloadTime+T_CHUNK;
        if(simulated.buffer<T_CHUNK) {
            simulated.buffer=T_CHUNK;
        }
        FindReward(simulated,0,simulated.downloadTime, prevBuffer);
        int simulated_state=FindState(simulated.capacity,simulated.buffer,GetComplexityFromState(next_state),simulated.quality);
        SingleUpdate(simulated, simulated_state, prevBuffer);
    }
    std::cout<<"\n";
}

void DashClient::SingleUpdate(segment prev, int next_state, double prevBuffer)
{
    int pds = FindPds(prev.state);
    std::cout<<"PDS"<<pds<<" ";
    segment next = prev;
    double reward = -prev.b_penalty;
    next.action=Softmax(next_state,0,prev.quality);
    next.quality = qualities.at(GetComplexityFromState(next_state)).at(next.action); 
    FindReward(next, prev.quality, 0, prevBuffer);
    reward += (next.quality - next.q_penalty) * m_lambda;
    reward += q_matrix.at(FindPds(next_state)).at(next.action) * m_lambda;
    q_matrix.at(pds).at(prev.action) = (1-m_alpha) * q_matrix.at(pds).at(prev.action) + m_alpha * reward;
}

int DashClient::GetComplexityFromState(int state)
{
    return (state % 50) / 10;
}

int DashClient::GetBufferFromState(int state)
{
    return (state % 500) / 50;
}

int DashClient::GetCapacityFromState(int state) {
    return state % 10;
}

int DashClient::GetStateFromBuffer(double buffer) {
    std::vector<double> buffer_states;
    
    buffer_states.push_back(3);
    buffer_states.push_back(4);
    buffer_states.push_back(5);
    buffer_states.push_back(6);
    buffer_states.push_back(8);
    buffer_states.push_back(10);
    buffer_states.push_back(12);
    buffer_states.push_back(15);
    buffer_states.push_back(18);
    
    int i=0;
    while(i<9 && buffer > buffer_states.at(i)) {
        i++;
    }
    return i;
}

int DashClient::GetStateFromCapacity(double capacity) {
    std::vector<double> capacity_states;
    
    capacity_states.push_back(0.5);
    capacity_states.push_back(1);
    capacity_states.push_back(2);
    capacity_states.push_back(3);
    capacity_states.push_back(4);
    capacity_states.push_back(5);
    capacity_states.push_back(6.5);
    capacity_states.push_back(8);
    capacity_states.push_back(10);
    
    int i=0;
    while(i<9 && capacity > capacity_states.at(i)) {
        i++;
    }
    return i;
}

int DashClient::GetStateFromQuality(double quality) {
    std::vector<double> quality_states;
    
    quality_states.push_back(0.84);
    quality_states.push_back(0.87);
    quality_states.push_back(0.9);
    quality_states.push_back(0.92);
    quality_states.push_back(0.94);
    quality_states.push_back(0.96);
    quality_states.push_back(0.98);
    quality_states.push_back(0.99);
    quality_states.push_back(0.995);
    
    int i=0;
    while(i<9 && quality > quality_states.at(i)) {
        i++;
    }
    return i;
}

void DashClient::log()
{
	if (m_client_running==false)
	{
		return;
	}
    
    *(m_log) << current.state << " " << current.capacity << " " << current.action << " " << current.buffer << " " << current.reward << " " << current.quality << " " << current.q_penalty << " " << current.b_penalty << " " << current.rebuffering << " " << current.time << " " << current.exploration << std::endl;
	return;
}

} /* namespace ns3 */
