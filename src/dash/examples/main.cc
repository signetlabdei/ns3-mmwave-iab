/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Jaume Nin <jaume.nin@cttc.cat>
 */
 
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/athstats-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
#include "ns3/position-allocator.h"
#include "dash_server.h"
#include "rate_based.h"
#include "ddash_client.h"
#include "festive_client.h"
#include "panda_client.h"
#include "mpc_client.h"
//#include "ns3/gtk-config-store.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("DashAdaptation");

int main (int argc, char *argv[])
{
  int seed = 1;
  int run = 0;

  Config::SetDefault("ns3::TcpSocket::SndBufSize", ns3::UintegerValue(20000000)); // 4 MB  
  Config::SetDefault("ns3::TcpSocket::RcvBufSize", ns3::UintegerValue(2000000)); // 4 MB
  Config::SetDefault("ns3::TcpSocket::DelAckCount", ns3::UintegerValue(1));

  uint16_t cross = 30;
  double simTime = 2000;
  int segments = 100;
  std::string logfile="client_stats";
  std::string clientType="festive";
  std::string qfile="qvalues.csv";
  double interarrival = 20;
  double aggressiveness = 0;
  double temp = 0;
  int history = 5;

  // Command line arguments
  CommandLine cmd;
  cmd.AddValue("simTime", "Total duration of the simulation [s])", simTime);
  cmd.AddValue("clients", "Maximum number of clients", cross);
  cmd.AddValue("aggressive", "Aggressiveness penalty", aggressiveness);
  cmd.AddValue("interarrival", "Interarrival time", interarrival);
  cmd.AddValue("segments", "Video length in segments", segments);
  cmd.AddValue("logFile", "Log file", logfile);
  cmd.AddValue("qFile", "Q-value matrix file", qfile);
  cmd.AddValue("type", "Client type", clientType);  
  cmd.AddValue("temp", "Exploration temperature", temp);
  cmd.AddValue("seed", "Random generator seed", seed);  
  cmd.AddValue("run", "Random generator run number", run);
  cmd.AddValue("history", "Number of previous capacities in the state", history);
  cmd.Parse(argc, argv);
  std::cout<<seed<<"\n";
  RngSeedManager::SetSeed (seed);
  ns3::RngSeedManager::SetRun (run);

  // parse again so you can override default values from the command line
  cmd.Parse(argc, argv);

  
  NodeContainer p2pNodes;
  p2pNodes.Create (2);
  
  Ptr<Node> remoteHost = p2pNodes.Get (0);
  
  
  WifiHelper wifi;
  MobilityHelper mobility;
  MobilityHelper staticAp;
  NetDeviceContainer wifiDevs;

  NodeContainer apNodes;
  NodeContainer staNodes;
  apNodes.Add (p2pNodes.Get (1));
  staNodes.Create (cross);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("100ms"));
  
  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (p2pNodes);
  
  WifiMacHelper wifiMac;
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::NakagamiPropagationLossModel");
  wifiPhy.SetChannel (wifiChannel.Create ());
  Ssid ssid = Ssid ("wifi-default");
  wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  wifi.SetRemoteStationManager ("ns3::MinstrelHtWifiManager");
  // setup stas.
  wifiMac.SetType ("ns3::StaWifiMac",
                   "Ssid", SsidValue (ssid));
  NetDeviceContainer staDevs = wifi.Install (wifiPhy, wifiMac, staNodes);
  // setup ap.
  wifiMac.SetType ("ns3::ApWifiMac",
                   "Ssid", SsidValue (ssid));
  wifiDevs = wifi.Install (wifiPhy, wifiMac, apNodes);
  wifiDevs.Add (staDevs);

  // mobility.
  
  staticAp.SetPositionAllocator ("ns3::RandomBoxPositionAllocator",
                                 "X", StringValue ("ns3::UniformRandomVariable[Min=-1.0|Max=1.0]"),
                                 "Y", StringValue ("ns3::UniformRandomVariable[Min=-1.0|Max=1.0]"));
  staticAp.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  
  ObjectFactory pos;

  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
  pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=-15.0|Max=15.0]"));
  pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=-15.0|Max=15.0]"));
  Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
  mobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel", 
                                 "Speed", StringValue ("ns3::UniformRandomVariable[Min=0.2|Max=5.0]"),
                                 "Pause", StringValue ("ns3::UniformRandomVariable[Min=0.1|Max=5.0]"),
                                 "PositionAllocator", PointerValue (taPositionAlloc));
  mobility.SetPositionAllocator (taPositionAlloc);
  
  mobility.Install (staNodes);
  staticAp.Install (apNodes); 

  InternetStackHelper stack;
  stack.Install (p2pNodes.Get (0));
  stack.Install (apNodes);
  stack.Install (staNodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer wifiInterfaces;
  wifiInterfaces = address.Assign (wifiDevs);
  
  Ipv4Address remoteHostAddr = p2pInterfaces.GetAddress (0);

  // DASH server
  uint16_t serv_port = 45010;
  Ptr<DashServer> server= CreateObject<DashServer> ();
  remoteHost->AddApplication (server);
  std::stringstream logger;
  logger << "server_log" << 100 * run << ".log";
  server->Setup(serv_port, logger.str().c_str(), false, 10, aggressiveness, 0); // TODO: the last parameter is the temperature of the server
  server->SetStartTime (Seconds (0.01));
  server->SetStopTime (Seconds (simTime));
  
  // DASH clients
  double timer = 1;
  Ptr<ExponentialRandomVariable> arrivalRng = CreateObject<ExponentialRandomVariable> ();
  for (int i = 0; i < cross; i++) {
    if (!clientType.compare("mpc"))
    {
	    std::cout << "MPC CLIENT\n";
        Ptr<MpcClient> client;
        client=CreateObject<MpcClient> ();
        std::stringstream logger;
        logger << logfile << 100 * run + i << ".log";
        staNodes.Get(i)->AddApplication (client);
        //client->Setup(remoteHostAddr,serv_port,logger.str().c_str(),segments,i + 100 * run,10);
        // add temperature for DDASH client
        client->Setup(remoteHostAddr,serv_port,logger.str().c_str(),segments,i + 100 * run, temp, history);
        client->SetStartTime (Seconds (timer));
        client->SetStopTime (Seconds(simTime));
    }
    if (!clientType.compare("rate"))
    {
	    std::cout << "RATEBASED CLIENT\n";
        Ptr<RateBased> client;
        client=CreateObject<RateBased> ();
        std::stringstream logger;
        logger << logfile << 100 * run + i << ".log";
        staNodes.Get(i)->AddApplication (client);
        //client->Setup(remoteHostAddr,serv_port,logger.str().c_str(),segments,i + 100 * run,10);
        // add temperature for DDASH client
        client->Setup(remoteHostAddr,serv_port,logger.str().c_str(),segments,i + 100 * run, temp, history);
        client->SetStartTime (Seconds (timer));
        client->SetStopTime (Seconds(simTime));
    }    
    if (!clientType.compare("festive"))
    {
	    std::cout << "FESTIVE CLIENT\n";
        Ptr<FestiveClient> client;
        client=CreateObject<FestiveClient> ();
        std::stringstream logger;
        logger << logfile << 100 * run + i << ".log";
        staNodes.Get(i)->AddApplication (client);
        //client->Setup(remoteHostAddr,serv_port,logger.str().c_str(),segments,i + 100 * run,10);
        // add temperature for DDASH client
        client->Setup(remoteHostAddr,serv_port,logger.str().c_str(),segments,i + 100 * run, temp, history);
        client->SetStartTime (Seconds (timer));
        client->SetStopTime (Seconds(simTime));
    }    
    if (!clientType.compare("panda"))
    {
	    std::cout << "PANDA CLIENT\n";
        Ptr<Panda> client;
        client=CreateObject<Panda> ();
        std::stringstream logger;
        logger << logfile << 100 * run + i << ".log";
        staNodes.Get(i)->AddApplication (client);
        //client->Setup(remoteHostAddr,serv_port,logger.str().c_str(),segments,i + 100 * run,10);
        // add temperature for DDASH client
        client->Setup(remoteHostAddr,serv_port,logger.str().c_str(),segments,i + 100 * run, temp, history);
        client->SetStartTime (Seconds (timer));
        client->SetStopTime (Seconds(simTime));
    }
    if (!clientType.compare("ddash"))
    {
	    std::cout << "DDASH CLIENT\n";
        Ptr<DdashClient> client;
        client=CreateObject<DdashClient> ();
        std::stringstream logger;
        logger << logfile << 100 * run + i << ".log";
        staNodes.Get(i)->AddApplication (client);
        client->Setup(remoteHostAddr,serv_port,logger.str().c_str(),segments,i + 100 * run, temp, history);
        client->SetStartTime (Seconds (timer));
        client->SetStopTime (Seconds(simTime));
    }
    timer += arrivalRng->GetValue(interarrival, 200);
  }


  // Uncomment to enable PCAP tracing
  //csma.EnablePcapAll("dash-traces");
  //pointToPoint.EnablePcapAll("server-side");

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop(Seconds(simTime));
  Simulator::Run();

  /*GtkConfigStore config;
  config.ConfigureAttributes();*/

  Simulator::Destroy();
  return 0;

}

