 /* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
 /*
 *   Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *   Copyright (c) 2015, NYU WIRELESS, Tandon School of Engineering, New York University
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation;
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *   Author: Marco Miozzo <marco.miozzo@cttc.es>
 *           Nicola Baldo  <nbaldo@cttc.es>
 *
 *   Modified by: Marco Mezzavilla < mezzavilla@nyu.edu>
 *        	 	  Sourjya Dutta <sdutta@nyu.edu>
 *        	 	  Russell Ford <russell.ford@nyu.edu>
 *        		  Menglei Zhang <menglei@nyu.edu>
 */

#include <ns3/string.h>
#include <ns3/log.h>
#include <ns3/abort.h>
#include <ns3/pointer.h>
#include <iostream>
#include <string>
#include <sstream>
#include "ns3/mmwave-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
#include "ns3/mmwave-point-to-point-epc-helper.h"
#include "simulation-config/simulation-config.h"
//#include "ns3/gtk-config-store.h"

using namespace ns3;

//NS_LOG_COMPONENT_DEFINE ("MmWaveSimpleIab");
int
main (int argc, char *argv[])
{
  bool centralSched {true}, harqOn {true};
  unsigned int weightPolicy {3}, run {0}; 
  unsigned int appRunTime {1000}, cooldownPeriod {500};
  unsigned int numIabs {5}; // 5 and 9 in the data
  double eta {50}, k {2}, muThreshold {10}; //TODO: expose and parse via cmd
  double frequency {30e9}, bandwidth {500e6}; // carrier frequency in hertz
  double uesPerBs {3}; // number of UEs per BS
  unsigned centralAllocPeriod {1};

  // Traffic model
  double udpIPI {100}, packetSize {100}; // Inter-packet interval for all UDP sources [microseconds]

  LogComponentEnableAll (LOG_PREFIX_TIME);
  LogComponentEnableAll (LOG_PREFIX_FUNC);
  LogComponentEnableAll (LOG_PREFIX_NODE);
  //LogComponentEnableAll (LOG_LEVEL_DEBUG);

  CommandLine cmd;
  cmd.AddValue ("appRunTime", "how long the applications shall run [ms]", appRunTime);
  cmd.AddValue ("centralizedSched", "whether to use a centralized scheduler or not", centralSched);
  cmd.AddValue ("cooldownPeriod", "how long the simulation shall run, after the applications have stopped [ms]", cooldownPeriod);
  cmd.AddValue ("numIabs", "the number of IAB devices to deploy", numIabs);  // Temp workaround for SEM
  cmd.AddValue ("run", "run for RNG (for generating different deterministic sequences for different drops)", run);
  cmd.AddValue ("weightPolicy", "Weight policy to use: [1] - Max capacity; [2] - Min-Max buffer size; [3] - MaxRateBacklogAvoidance", weightPolicy);
  cmd.AddValue ("harqOn", "whether to use HARQ or not", harqOn);
  cmd.AddValue ("uesPerBs", "number of UEs per BS", uesPerBs);
  cmd.AddValue ("packetSize", "the size of the UDP packets", packetSize);
  cmd.AddValue ("eta", "the parameter eta for the MRBA weight policy", eta);
  cmd.AddValue ("k", "the parameter k for the MRBA weight policy", k);
  cmd.AddValue ("muThreshold", "the parameter muThreshold for the MRBA weight policy", muThreshold);
  cmd.AddValue ("allocationPeriod", "the period of the central indication", centralAllocPeriod);
  //cmd.AddValue("am", "RLC AM if true", rlcAm);
  cmd.Parse(argc, argv);

   // Enable E2E app traces
  Config::SetDefault ("ns3::PacketSink::EnableE2EStats", BooleanValue(true));
  Config::SetDefault ("ns3::BulkSendApplication::EnableE2EStats", BooleanValue(true));

  Config::SetDefault ("ns3::MmWaveHelper::Scheduler", StringValue("ns3::MmWaveRrIabMacScheduler"));
  Config::SetDefault ("ns3::MmWaveIabController::CentralAllocationEnabled", BooleanValue(true));
  Config::SetDefault ("ns3::MmWaveIabController::CentralAllocationPeriod", UintegerValue(centralAllocPeriod));

  Config::SetDefault ("ns3::MmWaveRrIabMacScheduler::HarqEnabled", BooleanValue(harqOn));
  Config::SetDefault ("ns3::MmWaveIabController::CentralAllocationEnabled", BooleanValue(centralSched));

  if (centralSched)
  {
    if (weightPolicy == 1)
    {
      Config::SetDefault ("ns3::MmWaveIabController::WeightPolicy", StringValue("ns3::MaxSumCapacityPolicy"));
    }
    else if (weightPolicy == 2)
    {
      Config::SetDefault ("ns3::MmWaveIabController::WeightPolicy", StringValue("ns3::MinMaxBufferPolicy"));
    }
    else if (weightPolicy == 3)
    {
      Config::SetDefault ("ns3::MmWaveIabController::WeightPolicy", StringValue("ns3::MRBAPolicy"));
      Config::SetDefault ("ns3::MmWaveIabWeightPolicy::K", DoubleValue(k));
      Config::SetDefault ("ns3::MmWaveIabWeightPolicy::Eta", DoubleValue(eta));
      Config::SetDefault ("ns3::MmWaveIabWeightPolicy::MuThreshold", DoubleValue(muThreshold));
    }
  }

  Config::SetDefault ("ns3::MmWavePhyMacCommon::CenterFreq", DoubleValue (frequency));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::ChunkPerRB", UintegerValue (72 * bandwidth / 1e9));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::NumRefScPerSym", UintegerValue (864 * bandwidth / 1e9));
  Config::SetDefault ("ns3::MmWave3gppChannel::UpdatePeriod", TimeValue (MilliSeconds (0))); // TODO: update if scenario complexity allows it
  Config::SetDefault ("ns3::MmWaveHelper::ChannelModel", StringValue("ns3::MmWave3gppChannel"));
  //Config::SetDefault ("ns3::MmWaveHelper::PathlossModel", StringValue ("ns3::MmWave3gppFakeBuildingsPropagationLossModel"));
  Config::SetDefault ("ns3::MmWave3gppChannel::DirectBeam", BooleanValue(true)); // Set true to perform the beam in the exact direction of receiver node
  Config::SetDefault ("ns3::AntennaArrayModel::IsotropicAntennaElements", BooleanValue(false)); // Use the 3gpp radiation model for the antenna elements
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::ScenarioEnbEnb", StringValue ("UMi-StreetCanyon"));
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::ScenarioEnbUe", StringValue ("UMi-StreetCanyon"));
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::ScenarioEnbIab", StringValue ("UMi-StreetCanyon"));
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::ScenarioIabIab", StringValue ("UMi-StreetCanyon"));
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::ScenarioIabUe", StringValue ("UMi-StreetCanyon"));
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::ScenarioUeUe", StringValue ("UMi-StreetCanyon"));

  Config::SetDefault ("ns3::MmWavePhyMacCommon::SymbolsPerSubframe", UintegerValue(24));   //TODO: check
  Config::SetDefault ("ns3::MmWavePhyMacCommon::SubframePeriod", DoubleValue(100));    //TODO: check
  
  Config::SetDefault ("ns3::MmWavePhyMacCommon::UlSchedDelay", UintegerValue(1));
  Config::SetDefault ("ns3::LteRlcAm::MaxTxBufferSize", UintegerValue (20 * 1024 * 1024));
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (20 * 1024 * 1024));
  Config::SetDefault ("ns3::LteRlcAm::PollRetransmitTimer", TimeValue(MilliSeconds(2.0)));
  Config::SetDefault ("ns3::LteRlcAm::ReorderingTimer", TimeValue(MilliSeconds(1.0)));
  Config::SetDefault ("ns3::LteRlcAm::StatusProhibitTimer", TimeValue(MilliSeconds(1.0)));
  Config::SetDefault ("ns3::LteRlcAm::ReportBufferStatusTimer", TimeValue(MicroSeconds(500)));
  Config::SetDefault ("ns3::LteRlcUm::ReportBufferStatusTimer", TimeValue(MicroSeconds(500)));
  Config::SetDefault ("ns3::MmWaveHelper::RlcAmEnabled", BooleanValue(true)); 
  //Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Scenario", StringValue(scenario3gpp));

	RngSeedManager::SetSeed (1);
	RngSeedManager::SetRun (run);

  Ptr<MmWaveHelper> mmwaveHelper = CreateObject<MmWaveHelper> ();
  Ptr<MmWavePointToPointEpcHelper>  epcHelper = CreateObject<MmWavePointToPointEpcHelper> ();
  mmwaveHelper->SetEpcHelper (epcHelper);
  mmwaveHelper->Initialize();

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults();

  // parse again so you can override default values from the command line
  cmd.Parse(argc, argv);

  Ptr<Node> pgw = epcHelper->GetPgwNode ();

   // Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  // interface 0 is localhost, 1 is the p2p device
  //Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  NodeContainer ueNodes;
  NodeContainer enbNodes;
  NodeContainer iabNodes;

  NS_LOG_UNCOND ("Creating " << numIabs << " IAB nodes and " << (numIabs + 1)*uesPerBs << " UEs");
  enbNodes.Create(1);
  iabNodes.Create(numIabs);
  ueNodes.Create((numIabs + 1)*uesPerBs);

  // Install Mobility Model
  Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator> ();
  enbPositionAlloc->Add (Vector (0.0, 0.0, 10.0));
  MobilityHelper enbmobility;
  enbmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  enbmobility.SetPositionAllocator(enbPositionAlloc);
  enbmobility.Install (enbNodes);

  MobilityHelper iabmobility;
  Ptr<ListPositionAllocator> iabPositionAlloc = CreateObject<ListPositionAllocator> ();
  iabPositionAlloc->Add (Vector (100.0, 0.0, 10.0));
  iabPositionAlloc->Add (Vector (-100.0, 0.0, 10.0));
  iabPositionAlloc->Add (Vector (0.0, 100.0, 10.0));
  iabPositionAlloc->Add (Vector (100.0, 100.0, 10.0));
  iabPositionAlloc->Add (Vector (-100.0, 100.0, 10.0));
  //iabPositionAlloc->Add (Vector (-100.0, -100.0, 10.0));
  //iabPositionAlloc->Add (Vector (100.0, -100.0, 10.0));
  //iabPositionAlloc->Add (Vector (0.0, -100.0, 10.0));
  iabmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  iabmobility.SetPositionAllocator(iabPositionAlloc);
  iabmobility.Install (iabNodes);

  MobilityHelper uemobility;
    // UE mobility
  Config::SetDefault ("ns3::MmWaveUniformDiscUePositionAllocator::rho", DoubleValue (35.0));
  Config::SetDefault ("ns3::MmWaveUniformDiscUePositionAllocator::r", DoubleValue (10.0));  // Min distance supported by UMi-StreetCanyon
  SimulationConfig::DropUes (ueNodes, NodeContainer (enbNodes,iabNodes), uesPerBs);

  // Install mmWave Devices to the nodes
  NetDeviceContainer enbmmWaveDevs = mmwaveHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer iabmmWaveDevs = mmwaveHelper->InstallIabDevice (iabNodes);
  NetDeviceContainer uemmWaveDevs = mmwaveHelper->InstallUeDevice (ueNodes);

  // Install the IP stack on IABs (for convenience), in the same IP space of UEs
  // internet.Install (iabNodes);
  // Ipv4InterfaceContainer iabIpIface;
  // iabIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (iabmmWaveDevs));
  // // Assign IP address to UEs, and install applications
  // for (uint32_t iabIndex = 0; iabIndex < iabNodes.GetN (); ++iabIndex)
  //   {
  //     Ptr<Node> iabNode = iabNodes.Get (iabIndex);
  //     // Set the default gateway for the IAB node
  //     Ptr<Ipv4StaticRouting> iabStaticRouting = ipv4RoutingHelper.GetStaticRouting (iabNode->GetObject<Ipv4> ());
  //     iabStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
  //   }

  // Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (uemmWaveDevs));
  // Assign IP address to UEs, and install applications
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  NetDeviceContainer possibleBaseStations(enbmmWaveDevs, iabmmWaveDevs);

  NS_LOG_UNCOND("number of IAB devs " << iabmmWaveDevs.GetN());

  mmwaveHelper->ChannelInitialization (uemmWaveDevs, enbmmWaveDevs, iabmmWaveDevs);
  mmwaveHelper->AttachIabToClosestEnb (iabmmWaveDevs, enbmmWaveDevs);
  mmwaveHelper->AttachToClosestEnbWithDelay (uemmWaveDevs, possibleBaseStations, Seconds(0.6));

  // Activate the central controller
  mmwaveHelper->ActivateDonorControllerIabSetup (enbmmWaveDevs, iabmmWaveDevs, enbNodes, iabNodes);

  // Install and start applications on UEs and remote host
  uint16_t portIndex = 1234;
  // uint16_t ulPort = 2000;
  // uint16_t otherPort = 3000;
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;

  AsciiTraceHelper asciiTraceHelper;
  Ptr<OutputStreamWrapper> udpStream = asciiTraceHelper.CreateFileStream ("UdpServerRxAdress.txt"); 
  
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
  {
    {
      // DL UDP
      UdpServerHelper dlPacketSinkHelper (portIndex);
      serverApps.Add (dlPacketSinkHelper.Install (ueNodes.Get(u)));
      serverApps.Get(u)->TraceConnectWithoutContext("RxWithAddresses", 
                                  MakeBoundCallback (&CallbackSinks::RxSinkAddresses, udpStream));

      UdpClientHelper dlClient (ueIpIface.GetAddress (u), portIndex);
      dlClient.SetAttribute ("Interval", TimeValue (MicroSeconds(udpIPI)));
      dlClient.SetAttribute ("PacketSize", UintegerValue(packetSize));
      dlClient.SetAttribute ("MaxPackets", UintegerValue(0xFFFFFFFF));
      clientApps.Add (dlClient.Install (remoteHost));

      portIndex++;

      // // UL UDP
      // UdpServerHelper ulPacketSinkHelper (ulPort);
      // serverApps.Add (ulPacketSinkHelper.Install (remoteHost));

      // UdpClientHelper ulClient (remoteHostAddr, ulPort);
      // ulClient.SetAttribute ("Interval", TimeValue (MicroSeconds(100)));
      // ulClient.SetAttribute ("MaxPackets", UintegerValue(0xFFFFFFFF));
      // clientApps.Add (ulClient.Install (ueNodes.Get(u)));

      // ulPort++;
    }
  }
  

  // Shuffle nodes to make sure that different apps are evenly distributed among different BSs
  /*
  std::vector<int> indexVec;
  for (uint32_t ueIndex = 0; ueIndex < ueNodes.GetN (); ueIndex++) 
  {
    indexVec.push_back(ueIndex); 
  }
  std::random_shuffle (indexVec.begin(), indexVec.end());
  NodeContainer shuffledUeNodes {};
  for (uint32_t shuffledUeIndex = 0; shuffledUeIndex < ueNodes.GetN (); shuffledUeIndex++)
  {
    shuffledUeNodes.Add (ueNodes.Get (indexVec.at(shuffledUeIndex)));
  }
  NS_ASSERT (shuffledUeNodes.GetN () == ueNodes.GetN ());
 
  // Install UDP applications
  for (uint32_t highDLIndex = 0; highDLIndex < std::floor(ueNodes.GetN ()*highRateDLUes/100); highDLIndex++)
  {
    {
      UdpServerHelper highDlPacketSinkHelper (portIndex);
      serverApps.Add (highDlPacketSinkHelper.Install (shuffledUeNodes.Get(highDLIndex)));
      NS_LOG_UNCOND (serverApps.GetN () << " " << highDLIndex);
      serverApps.Get(highDLIndex)->TraceConnectWithoutContext("RxWithAddresses", 
                                    MakeBoundCallback (&CallbackSinks::RxSinkAddresses, highDLStream));

      UdpClientHelper dlClient (ueIpIface.GetAddress (highDLIndex), portIndex);
      dlClient.SetAttribute ("Interval", TimeValue (MicroSeconds(udpIPI)));
      dlClient.SetAttribute ("PacketSize", UintegerValue(packetSize*ratioHighLowRates));
      dlClient.SetAttribute ("MaxPackets", UintegerValue(0xFFFFFFFF));
      clientApps.Add (dlClient.Install (remoteHost));

      portIndex++;
    }
  }

  NS_LOG_DEBUG ("High rate DL users setup completed!");

  for (uint32_t lowDLIndex = std::floor(ueNodes.GetN ()*highRateDLUes/100); 
        lowDLIndex < std::floor(ueNodes.GetN ()*(highRateDLUes + lowRateDLUes)/100); lowDLIndex++)
  {
    {
      UdpServerHelper lowDlPacketSinkHelper (portIndex);
      serverApps.Add (lowDlPacketSinkHelper.Install (shuffledUeNodes.Get(lowDLIndex)));;
      serverApps.Get(lowDLIndex);
      serverApps.Get(lowDLIndex)->TraceConnectWithoutContext("RxWithAddresses", 
                                    MakeBoundCallback (&CallbackSinks::RxSinkAddresses, lowDLStream));
                    
      UdpClientHelper dlClient (ueIpIface.GetAddress (lowDLIndex), portIndex);
      dlClient.SetAttribute ("Interval", TimeValue (MicroSeconds(udpIPI)));
      dlClient.SetAttribute ("PacketSize", UintegerValue(packetSize));
      dlClient.SetAttribute ("MaxPackets", UintegerValue(0xFFFFFFFF));
      clientApps.Add (dlClient.Install (remoteHost));

      portIndex++;
    }
  }
  */
  
  serverApps.Start (MilliSeconds (790));
  clientApps.Stop (MilliSeconds (790 + appRunTime));
  clientApps.Start (MilliSeconds (800));

  mmwaveHelper->EnableTraces ();
  PrintHelper::PrintGnuplottableNodeListToFile ("NodePosition.txt");

  Simulator::Stop(MilliSeconds(790 + appRunTime + cooldownPeriod));
  Simulator::Run();

  Simulator::Destroy();
  return 0;

}

