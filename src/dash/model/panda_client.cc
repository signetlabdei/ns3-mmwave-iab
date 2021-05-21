
#include "panda_client.h"

namespace ns3 {

Panda::Panda()
:
 	m_client_running(false)
{
}

Panda::~Panda()
{
}

void Panda::Setup (Ipv4Address server_ip,
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
	m_safety = 0.25;
	
	m_x = 0;
    m_y = 0;
    m_gamma = 0;
    m_r = 0;
	
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
	
	m_const = 1;	
	m_buffer=0;
	m_toDownload=0;
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
	
	m_alpha = 0.2;
	m_k = 0.14;
	m_w = 0.6;
	m_epsilon=0.15;
	m_targetBuf = 18;
	m_beta = 0.2;

	m_segment=0;
}

void Panda::StartApplication (void)
{
	m_client_running=true;

	m_socket->Connect (InetSocketAddress(m_server_IP, m_server_port));

	m_socket->SetConnectCallback (MakeCallback (&Panda::ConnectionComplete, this),
								  MakeCallback (&Panda::ConnectionFailed, this));

	std::cout << "CLIENT " << m_id << " (FESTIVE) STARTED\n";

	return;
}

void Panda::StopApplication (void)
{
    m_client_running=false;
	m_log->flush();
	m_log->close();
	m_client_running=false;

	return;
}

void Panda::ConnectionComplete (Ptr<Socket> socket)
{
	socket->SetRecvCallback (MakeCallback (&Panda::RecvChunk, this));
	m_accepted=true;

	std::cout << "CLIENT "<< m_id << ": CONNECTION ACCEPTED\n";

    //schedule request
    current.time=Simulator::Now().GetSeconds();
	Decision();

	return;
}

void Panda::ConnectionFailed (Ptr<Socket> socket)
{
	std::cout << "CLIENT " << m_id << ": CONNECTION FAILED\n";

	m_client_running = false;
}

std::vector<int> Panda::GenerateComplexity(int mean_scene) {
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

void Panda::SetQualityLevels(std::vector<int> complexity) {
    m_complexity = complexity;
}

void Panda::SendRequest (int action)
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
    std::cout << "CLIENT "<< m_id << ": SENT REQUEST FOR SEGMENT " << m_segment << "\n";
	return;
}

void Panda::RecvChunk(Ptr<Socket> socket)
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
		if (r_header.chunk_number != m_segment) {
	        std::cout << "CLIENT "<< m_id << ": received packet " << r_header.chunk_number << " while at segment" << m_segment <<"\n" << std::flush;
	        std::cout << "action " << current.action << "\n";	    
	        std::cout << "size " << rates.at(current.action) << "\n";	    
	        std::cout << "chunksize " << m_toDownload << "\n";
		    std::cout<<"chunk size server "<<r_header.chunk_size<<"\n";
	        return;
	    }

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
	    std::cout << "CLIENT "<< m_id << ": DOWNLOADED SEGMENT " << m_segment << "\n" << std::flush;
	    m_downloading=false;
	    current.time=Simulator::Now().GetSeconds();
	    double download_time=current.time-current.download_time;
	    double prevBuffer = m_buffer;
	    m_buffer+=T_CHUNK-download_time;
	    if(m_buffer<T_CHUNK) {
	        m_buffer=T_CHUNK;
	    }
	    Time tNext (Seconds (0.000000001));	    
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
	    std::cout<<"Reward: "<<current.reward<<"\n";
	    std::cout<<"Rebuffering: "<<current.rebuffering<<"\n";	    
	    std::cout<<"Qpenalty: "<<current.q_penalty<<"\n";
	    std::cout<<"Bpenalty: "<<current.b_penalty<<"\n";
	    std::cout<<"Capacity: "<<current.capacity.at(0)<<"\n";
	    std::cout<<"Time: "<<Simulator::Now().GetSeconds()<<"\n"<<std::flush;
        std::cout<<"Segment: "<<m_segment<< " " << stats.size()<<"\n";
	    stats.push_back(current);
	    m_segment++;
	    Log();
	    Simulator::Schedule(tNext, &Panda::Decision, this);
	}
	return;
}

void Panda::Decision()
{
    if(m_segment>=m_segments){
        StopApplication();
        return;
    }   
    std::cout<<"Segment: "<<m_segment<<"\n";
    segment prev = current;
    if(m_segment>0) {
	    segment prev=stats.back();
        current.capacity=prev.capacity;
    }
    current.download_time = Simulator::Now().GetSeconds();    
    current.complexity=m_complexity.at(m_segment);   
    std::cout << "CLIENT "<< m_id << ": REQUESTED SEGMENT " << m_segment << "\n";
	std::cout<<"Buffer: "<<m_buffer<<"\n";
    current.action=ChooseAction(current);
    if (current.action == prev.action) {
        m_const++;
    }
    else {
        m_const = 1;
    }
    std::cout<<"Action: "<<current.action<<"\n";
    current.quality=qualities.at(current.complexity).at(current.action);
    std::cout<<"Quality: "<<current.quality<<"\n";
    std::cout<<"Wait: "<<m_wait<<"\n";
    Time tNext(Seconds(m_wait));
    Simulator::Schedule(tNext, &Panda::SendRequest, this, current.action);
    //SendRequest(current.action);
}

int Panda::ChooseAction(segment last)
{
    if (m_segment == 0) {
        return 0;
    }
    if (m_segment == 1) {
        m_x = last.capacity.at(0) / 2;
    }
    
    double new_x = 0;
    double new_y = 0;
    double new_gamma = 0;
    int new_r = 0;
    
    double cap = m_x - last.capacity.at(0);
    if (cap < 0) {
        cap = 0;
    }
    new_x = m_x + T_CHUNK * m_k * (m_w - cap);
    if (new_x < 0) {
        new_x = 0;
    }
    std::cout<<"old "<<m_x<<"\n";
    std::cout<<"new "<<new_x<<"\n";
    new_y = m_y + T_CHUNK * m_alpha * (new_x - m_y);
    if (new_y < 0) {
        new_y = 0;
    }
    std::cout<<"oms "<<m_y<<"\n";
    std::cout<<"smo "<<new_y<<"\n";
    double safe_rate = new_y * (1 - m_epsilon);
    int up = 0;
    int down = 0;
    for (uint32_t i=0;i<rates.size() - 1;i++) {
        if (rates.at(i) > safe_rate) {
            up++;
        }
        if (rates.at(i) > new_y) {
            down++;
        }
    }
    new_r = down;
    
    if (m_r < up) {
        new_r = up;
    }
    
    if (up <= m_r && m_r <= down) {
        new_r = up;
    }
    
    m_wait = m_beta * (last.buffer - m_targetBuf);
    if (m_wait < 0) {
        m_wait = 0;
    }
    new_gamma = m_wait;
    
    m_r = new_r;
    m_x = new_x;
    m_y = new_y;
    m_gamma = new_gamma;

    return m_r;
}

std::vector<int> Panda::GenerateActions(int actions, int steps, int action_index)
{
    std::vector<int> action_vector;
    for (int i = 0; i < steps; i++) {
        int action = action_index % (int)std::pow(actions, i+1);
        if (i > 0) {
            action = action_index % (int)std::pow(actions, i+1);
            action = (int) action / (int)std::pow(actions, i);
        }
        action_vector.push_back(action);
    }
    return action_vector;
}

double Panda::HarmonicMean(std::vector<double> data, int start, int length)
{
    double den = 0;
    for (int i = 0; i < length; i++) {
        if(data.at(start+i) >0){
            den += 1/data.at(start+i);
        }
    }
    if (den >0) {
        return length / den;
    } else {
        return 0;
    }
}


double Panda::SimulatePolicy(std::vector<int> actions, std::vector<double> capacity, segment last, int steps)
{
    double total = 0;
    segment prev = current;
    segment sim = current;
    int seg = m_segment;
    for (uint32_t i = 0; i < actions.size(); i++) {
        seg++;
        if (seg >= m_segments) {
            return total;
        }
        sim.complexity=m_complexity.at(seg);
        sim.action = actions.at(i);
        sim.quality=qualities.at(sim.complexity).at(sim.action);
        double download_time = rates.at(actions.at(i)) * T_CHUNK / capacity.at(i);
        for (int j = m_memory - 1; j > 0; j--) {
	        sim.capacity.at(j) = sim.capacity.at(j-1);
	    }
	    sim.capacity.at(0) = capacity.at(i);
        sim.buffer+=T_CHUNK-download_time;
	    if(sim.buffer<T_CHUNK) {
	        sim.buffer=T_CHUNK;
	    }
	    if(sim.buffer>20) {
	        sim.buffer=20;
	    }

        FindReward(sim, prev.quality, download_time, prev.buffer);
        total+=sim.reward;
        prev = sim;
    }
    return total;
}

void Panda::SetParams(int steps, double safety) 
{
    m_steps = steps;
    m_safety = safety;
}

void Panda::FindReward(segment &last, double prevQuality, double download_time, double prevBuffer)
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
        if(reward<0) {
            reward=0;
        }
        last.b_penalty=b_penalty;
        last.reward=reward;
    }
}

void Panda::Log()
{
	if (m_client_running==false)
	{
		return;
	}
	
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
    *(m_log) << "0]}\n" << std::flush;
	return;
}

} /* namespace ns3 */
