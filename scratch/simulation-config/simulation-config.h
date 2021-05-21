#include <ns3/core-module.h>
#include <ns3/internet-module.h>
#include <ns3/point-to-point-helper.h>
#include <ns3/mobility-module.h>
#include <ns3/applications-module.h>
#include <ns3/buildings-module.h>
#include <ns3/node-list.h>
#include <ns3/lte-module.h>
#include <ns3/mmwave-module.h>
#include <ns3/trace-source-accessor.h>
#include "ns3/dash_server.h"
#include "ns3/rate_based.h"
#include "ns3/mpc_client.h"
#include "ns3/file-transfer-application.h"
#include "ns3/e2e-stats-header.h"

NS_LOG_COMPONENT_DEFINE ("SimulationConfig");

namespace ns3{

  class SimulationConfig
  {
    public:
      static std::pair<Ptr<Node>, Ipv4Address> CreateInternet (Ptr<MmWavePointToPointEpcHelper> epcHelper);
      static Ipv4InterfaceContainer InstallUeInternet (Ptr<MmWavePointToPointEpcHelper> epcHelper, NodeContainer ueNodes, NetDeviceContainer ueNetDevices);
      static void SetConstantPositionMobility (NodeContainer nodes, Vector position);
      static void SetConstantVelocityMobility (Ptr<Node> node, Vector position, Vector velocity);
      static void SetHexGridLayout (NodeContainer nodes, uint16_t gridWidth);
      static void SetHomogeneousHexGridLayout (NodeContainer iabDonors, NodeContainer iabNodes);
      static void SetHeterogeneousLayout (NodeContainer iabDonors, NodeContainer iabNodes);
      static void SetHomogeneousManhattanGrid (NodeContainer iabDonors, NodeContainer iabNodes, double bsAntennaHeight);
      static void DropMicroBs (NodeContainer iabDonors, NodeContainer iabNodes);
      static void DropUes (NodeContainer ueNodes, NodeContainer bsNodes, uint32_t numOfUesPerBs);
      static void DropUes (NodeContainer ueNodes, NodeContainer macroNodes, NodeContainer microNodes, uint32_t numOfUesPerSite);
      static void SetupUdpApplication (Ptr<Node> node, Ipv4Address address, uint16_t port, uint16_t interPacketInterval, double startTime, double endTime);
      static void SetupFtpModel3Application (Ptr<Node> clientNode, Ptr<Node> serverNode, Ipv4Address address, uint16_t port, double lambda, Time startTime, Time endTime, Ptr<OutputStreamWrapper> stream);
      static void SetupUdpPacketSink (Ptr<Node> node, uint16_t port, double startTime, double endTime, Ptr<OutputStreamWrapper> stream);
      static void SetTracesPath (std::string filePath);

    private:
      static void StartFileTransfer (Ptr<FileTransferApplication> ftpApp);
      static void ScheduleFileTransfer (Ptr<ExponentialRandomVariable> interArrivalTime, Time endTime, Ptr<FileTransferApplication> ftpApp);
      static void SetHomogeneousHexGrid3SitesLayout (NodeContainer iabDonors, NodeContainer iabNodes);
      static void SetHomogeneousHexGrid7SitesLayout (NodeContainer iabDonors, NodeContainer iabNodes);
      static void SetHomogeneousHexGrid19SitesLayout (NodeContainer iabDonors, NodeContainer iabNodes);
      
  };

  class CallbackSinks
  {
    public:
      static void RxSink (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> packet, const Address &from = Address ());
      static void RxSinkAddresses (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> packet, const Address &from = Address (), 
                                    const Address &to = Address ());
      static void RxSinkE2EStat (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> packet, const Address &from = Address (), 
                                    const Address &to = Address (), const E2eStatsHeader &e2estats = E2eStatsHeader ());
      static void TxSink (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> packet, const Address &from = Address ());
  };

  class RandomBuildings
  {
    public:
      static void CreateRandomBuildings (double streetWidth, double blockSize, double maxXAxis, double maxYAxis, uint32_t numBlocks);

    private:
      static std::pair<Box, std::list<Box>> GenerateBuildingBounds(double xMin, double xMax, double yMin, double yMax, double maxBuildSize, std::list<Box> m_previousBlocks );
      static bool AreOverlapping(Box a, Box b);
      static bool OverlapWithAnyPrevious(Box box, std::list<Box> m_previousBlocks);

  };

  class PrintHelper
  {
    public:
      static void PrintGnuplottableBuildingListToFile (std::string filename);
      static void PrintGnuplottableNodeListToFile (std::string filename);
      static void UpdateGnuplottableNodeListToFile (std::string filename, Ptr<Node> node);
  };

  std::pair<Ptr<Node>, Ipv4Address>
  SimulationConfig::CreateInternet (Ptr<MmWavePointToPointEpcHelper> epcHelper)
  {
    // Create the Internet by connecting remoteHost to pgw. Setup routing too
    Ptr<Node> pgw = epcHelper->GetPgwNode ();

    // Create remotehost
    NodeContainer remoteHostContainer;
    remoteHostContainer.Create (1);
    InternetStackHelper internet;
    internet.Install (remoteHostContainer);
    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ipv4InterfaceContainer internetIpIfaces;

    Ptr<Node> remoteHost = remoteHostContainer.Get (0);
    // Create the Internet
    PointToPointHelper p2ph;
    p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
    p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
    p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.001)));

    NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);

    Ipv4AddressHelper ipv4h;
    ipv4h.SetBase ("1.0.0.0", "255.255.0.0");
    internetIpIfaces = ipv4h.Assign (internetDevices);
    // interface 0 is localhost, 1 is the p2p device
    Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

    Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
    remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.255.0.0"), 1);

    return std::pair<Ptr<Node>, Ipv4Address> (remoteHost, remoteHostAddr);
  }

  Ipv4InterfaceContainer
  SimulationConfig::InstallUeInternet (Ptr<MmWavePointToPointEpcHelper> epcHelper, NodeContainer ueNodes, NetDeviceContainer ueNetDevices)
  {
    // Install the IP stack on the UEs
    InternetStackHelper internet;
    internet.Install (ueNodes);
    Ipv4InterfaceContainer ueIpIface;
    ueIpIface = epcHelper->AssignUeIpv4Address (ueNetDevices);
    // Assign IP address to UEs, and install applications
    // Set the default gateway for the UE
    Ipv4StaticRoutingHelper ipv4RoutingHelper;

    for (uint32_t i = 0; i < ueNodes.GetN (); i++)
    {
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNodes.Get (i)->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

    return ueIpIface;
  }

  void
  SimulationConfig::SetConstantPositionMobility (NodeContainer nodes, Vector position)
  {
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
    positionAlloc->Add (position);
    MobilityHelper mobility;
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.SetPositionAllocator(positionAlloc);
    mobility.Install (nodes);
    BuildingsHelper::Install (nodes);
  }

  void
  SimulationConfig::SetConstantVelocityMobility (Ptr<Node> node, Vector position, Vector velocity)
  {
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
    MobilityHelper mobility;
    positionAlloc->Add (position);
    mobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
    mobility.SetPositionAllocator(positionAlloc);
    mobility.Install (node);
    node->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (velocity);
    BuildingsHelper::Install (node);
  }

  void
  SimulationConfig::SetHexGridLayout (NodeContainer nodes, uint16_t gridWidth)
  {
    Ptr<MmWaveHexGridEnbPositionAllocator> positionAlloc = CreateObject<MmWaveHexGridEnbPositionAllocator> ();
    positionAlloc->SetAttribute ("GridWidth", UintegerValue(gridWidth));
    positionAlloc->Initialize ();
    MobilityHelper mobility;
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.SetPositionAllocator(positionAlloc);
    mobility.Install (nodes);
  }

  void
  SimulationConfig::SetHomogeneousHexGridLayout (NodeContainer iabDonors, NodeContainer iabNodes)
  {
    uint32_t numOfNodes = iabDonors.GetN () + iabNodes.GetN ();

    if (numOfNodes == 3) // low complexity scenario
    {
      SetHomogeneousHexGrid3SitesLayout (iabDonors, iabNodes);
    }
    else if (numOfNodes == 7) // low complexity scenario
    {
      SetHomogeneousHexGrid7SitesLayout (iabDonors, iabNodes);
    }
    else if (numOfNodes == 19) // complete scenario
    {
      SetHomogeneousHexGrid19SitesLayout (iabDonors, iabNodes);
    }
    else
    {
      NS_FATAL_ERROR ("Unsupported configuration! The number of sites should be 3, 7 or 19!");
    }
  }

  void
  SimulationConfig::SetHeterogeneousLayout (NodeContainer iabDonors, NodeContainer iabNodes)
  {

    if (iabDonors.GetN () == 3 || iabDonors.GetN () == 7)
    {
      SetHexGridLayout (iabDonors, 3);
      DropMicroBs (iabDonors, iabNodes);
    }
    else
    {
      NS_FATAL_ERROR ("Unsupported configuration! The number of sites should be 3 or 7!");
    }
  }

  void
  SimulationConfig::DropMicroBs (NodeContainer iabDonors, NodeContainer iabNodes)
  {
    uint8_t numMicroPerSite = iabNodes.GetN () / iabDonors.GetN ();
    uint8_t iabNodesIndex = 0;
    for (uint8_t iabDonorIndex = 0; iabDonorIndex < iabDonors.GetN (); iabDonorIndex++)
    {
      NS_LOG_DEBUG ("Drop micro bs in site " << (uint16_t)iabDonorIndex);
      NodeContainer microNodes;

      for (uint8_t i = 0; i < numMicroPerSite; i++)
      {
        microNodes.Add (iabNodes.Get (i + iabNodesIndex));
      }
      iabNodesIndex += numMicroPerSite;

      Vector macroPos = iabDonors.Get (iabDonorIndex)->GetObject<MobilityModel> ()->GetPosition ();

      Ptr<MmWaveHeterogeneousLayoutPositionAllocator> pa = CreateObject<MmWaveHeterogeneousLayoutPositionAllocator> ();
      pa->SetX (macroPos.x);
      pa->SetY (macroPos.y);
      pa->SetMacroNodesContainer (iabDonors);
      pa->SetMicroNodesContainer (iabNodes);

      MobilityHelper mobility;
      mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
      mobility.SetPositionAllocator(pa);
      mobility.Install (microNodes);

    }
  }

  void
  SimulationConfig::SetHomogeneousHexGrid3SitesLayout (NodeContainer iabDonors, NodeContainer iabNodes)
  {
    // This method is used to setup a low complexity simulation scenario.
    NS_ASSERT_MSG (iabNodes.GetN () + iabDonors.GetN () == 3, "Unexpected number of nodes!");
    NS_LOG_INFO ("Deploy the base stations using an hex grid layout with 3 sites");

    // TODO check if the position for the iabDonors are ok
    // build a node container containing all nodes. The nodes iabDonor nodes are
    // placed in order to obtain the desired configuration of the layout.
    NS_ASSERT_MSG (iabDonors.GetN () == 1, "Unsupported configuration! The number of IAB donors should be 1!");
    NodeContainer allNodes;
    allNodes.Add (iabDonors.Get (0));
    allNodes.Add (iabNodes.Get (0));
    // this is a trick to set the correct layout (equilateral triangle) using
    // the MmWaveHexGridEnbPositionAllocator
    Ptr<Node> fakeNode = CreateObject<Node> ();
    allNodes.Add (fakeNode);
    allNodes.Add (iabNodes.Get (1));

    SetHexGridLayout (allNodes, 3);
  }

  void
  SimulationConfig::SetHomogeneousManhattanGrid (NodeContainer iabDonors, NodeContainer iabNodes, double bsAntennaHeight)
  {
    NS_ASSERT_MSG (iabDonors.GetN () == 1, "Unsupported configuration! The number of IAB donors should be 1!");
    NS_ASSERT_MSG (iabNodes.GetN () == 5, "Unsupported configuration! The number of IAB nodes should be 5!");

    // IAB donor first, located at the "origin" road intersection
    Ptr<ListPositionAllocator> donorPositionAlloc = CreateObject<ListPositionAllocator> ();
    donorPositionAlloc->Add (Vector (0.0, 0.0, 10.0));
    MobilityHelper enbmobility;
    enbmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    enbmobility.SetPositionAllocator(donorPositionAlloc);
    enbmobility.Install (iabDonors);

    // IAB nodes, located at other road intersections
    Ptr<ListPositionAllocator> nodesPositionAlloc = CreateObject<ListPositionAllocator> ();
    nodesPositionAlloc->Add (Vector (0.0, 50.0, bsAntennaHeight));
    nodesPositionAlloc->Add (Vector (0.0, -50.0, bsAntennaHeight));
    nodesPositionAlloc->Add (Vector (50, -50.0, bsAntennaHeight));
    nodesPositionAlloc->Add (Vector (50, 50.0, bsAntennaHeight));
    nodesPositionAlloc->Add (Vector (50, 0.0, bsAntennaHeight));
    MobilityHelper mobility;
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.SetPositionAllocator(nodesPositionAlloc);
    mobility.Install (iabNodes);
  }

  void
  SimulationConfig::SetHomogeneousHexGrid7SitesLayout (NodeContainer iabDonors, NodeContainer iabNodes)
  {
    // This method is used to setup a low complexity simulation scenario.
    NS_ASSERT_MSG (iabNodes.GetN () + iabDonors.GetN () == 7, "Unexpected number of nodes!");
    NS_LOG_INFO ("Deploy the base stations using an hex grid layout with 7 sites");

    // TODO check if the position for the iabDonors are ok
    // build a node container containing all nodes. The nodes iabDonor nodes are
    // placed in order to obtain the desired configuration of the layout.
    NodeContainer allNodes;
    uint8_t iabDonorIndex = 0;
    uint8_t iabNodesIndex = 0;
    if (iabDonors.GetN () == 1)
    {
      // place the iabDonor in the center of the hex grid (position 3)
      for (uint8_t i = 0; i < iabNodes.GetN () + iabDonors.GetN (); i++)
      {
        if (i == 3)
        {
          allNodes.Add (iabDonors.Get (iabDonorIndex));
          iabDonorIndex++;
        }
        else
        {
          allNodes.Add (iabNodes.Get (iabNodesIndex));
          iabNodesIndex++;
        }
      }
    }
    else if (iabDonors.GetN () == 3)
    {
      // place the iabDonors in positions 1, 2 and 6
      for (uint8_t i = 0; i < iabNodes.GetN () + iabDonors.GetN (); i++)
      {
        if (i == 1 || i == 2 || i == 6)
        {
          allNodes.Add (iabDonors.Get (iabDonorIndex));
          iabDonorIndex++;
        }
        else
        {
          allNodes.Add (iabNodes.Get (iabNodesIndex));
          iabNodesIndex++;
        }
      }
    }
    else
    {
      NS_FATAL_ERROR ("Unsupported configuration! The number of IAB donors should be 1, 3!");
    }

    SetHexGridLayout (allNodes, 3);
  }

  void
  SimulationConfig::SetHomogeneousHexGrid19SitesLayout (NodeContainer iabDonors, NodeContainer iabNodes)
  {
    // This method is used to setup the homogeneous layout.
    NS_ASSERT_MSG (iabNodes.GetN () + iabDonors.GetN () == 19, "Unexpected number of nodes!");
    NS_LOG_INFO ("Deploy the base stations using an hex grid layout with 19 sites");

    // TODO check if the position for the iabDonors are ok
    // build a node container containing all nodes. The nodes iabDonor nodes are
    // placed in order to obtain the desired configuration of the layout.
    NodeContainer allNodes;
    uint8_t iabDonorIndex = 0;
    uint8_t iabNodesIndex = 0;
    if (iabDonors.GetN () == 1)
    {
      // place the iabDonor in the center of the hex grid (position 9)
      for (uint8_t i = 0; i < iabNodes.GetN () + iabDonors.GetN (); i++)
      {
        if (i == 9)
        {
          allNodes.Add (iabDonors.Get (0));
          iabDonorIndex++;
        }
        else
        {
          allNodes.Add (iabNodes.Get (iabNodesIndex));
          iabNodesIndex++;
        }
      }
    }
    else if (iabDonors.GetN () == 3)
    {
      // place the iabDonors in positions 3, 9 and 15
      for (uint8_t i = 0; i < iabNodes.GetN () + iabDonors.GetN (); i++)
      {
        if (i == 4 || i == 10 || i == 13)
        {
          allNodes.Add (iabDonors.Get (iabDonorIndex));
          iabDonorIndex++;
        }
        else
        {
          allNodes.Add (iabNodes.Get (iabNodesIndex));
          iabNodesIndex++;
        }
      }
    }
    else if (iabDonors.GetN () == 7)
    {
      // place the iabDonors in positions 0, 3, 6, 9, 12, 15, 18
      for (uint8_t i = 0; i < iabNodes.GetN () + iabDonors.GetN (); i++)
      {
        if (i == 0 || i == 2 || i == 7 || i == 9 || i == 11 || i == 16 || i == 18)
        {
          allNodes.Add (iabDonors.Get (iabDonorIndex));
          iabDonorIndex++;
        }
        else
        {
          allNodes.Add (iabNodes.Get (iabNodesIndex));
          iabNodesIndex++;
        }
      }
    }
    else
    {
      NS_FATAL_ERROR ("Unsupported configuration! The number of IAB donors should be 1, 3 or 7!");
    }

    SetHexGridLayout (allNodes, 5);
  }

  void
  SimulationConfig::DropUes (NodeContainer ueNodes, NodeContainer bsNodes, uint32_t numOfUesPerBs)
  {
    // This method drops the UEs in the layout.
    // For each BS, numOfUesPerBs are uniformly placed inside a disc around the
    // BS using the MmWaveUniformDiscUePositionAllocator.
    // The radious of the disc and the minimum distance between the UE and the BS
    // are configured as attributes of the MmWaveUniformDiscUePositionAllocator
    // class since they are fixed.

    /*// Create the random variables used for the computation of the height of
    // each user as described in TR 36.873
    Ptr<UniformRandomVariable> N_fl = CreateObject<UniformRandomVariable>();
    N_fl->SetAttribute ("Min", UintegerValue (4));
    N_fl->SetAttribute ("Max", UintegerValue (8));

    Ptr<UniformRandomVariable> n_fl = CreateObject<UniformRandomVariable>();
    n_fl->SetAttribute ("Min", UintegerValue (1));*/

    NS_LOG_INFO ("Drop the UEs. Number of UEs: " << ueNodes.GetN ()
                  << " Number of UEs per BS: " << numOfUesPerBs);

    uint16_t ueNodesIndex = 0;
    for (uint16_t bsIndex = 0; bsIndex < bsNodes.GetN (); bsIndex++)
    {
      // Create a MmWaveUniformDiscUePositionAllocator object for each BS and
      // set the center of the disc at the BS position
      Ptr<MmWaveUniformDiscUePositionAllocator> positionAlloc = CreateObject<MmWaveUniformDiscUePositionAllocator> ();
      positionAlloc->SetEnbNodeContainer (bsNodes);
      Vector bsPos = bsNodes.Get (bsIndex)->GetObject<MobilityModel> ()->GetPosition ();
      positionAlloc->SetX (bsPos.x);
      positionAlloc->SetY (bsPos.y);

      // Create a NodeContainer containing numOfUesPerBs UE nodes
      NodeContainer uesToDrop;
      for (uint8_t i = 0; i < numOfUesPerBs; i++)
      {

        positionAlloc->SetZ (1.7);
        NS_ASSERT_MSG (ueNodesIndex < ueNodes.GetN (), "Not enough UE nodes!");
        uesToDrop.Add (ueNodes.Get (ueNodesIndex));
        ueNodesIndex++;
      }

      MobilityHelper mobility;
      mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
      mobility.SetPositionAllocator(positionAlloc);
      mobility.Install (uesToDrop);
    }
    
  }

  void
  SimulationConfig::DropUes (NodeContainer ueNodes, NodeContainer macroNodes, NodeContainer microNodes, uint32_t numOfUesPerSite)
  {
    NS_LOG_INFO ("Drop the UEs. Number of UEs: " << ueNodes.GetN ()
                  << " Number of UEs per BS: " << numOfUesPerSite);

    uint16_t ueNodesIndex = 0;
    for (uint16_t macroIndex = 0; macroIndex < macroNodes.GetN (); macroIndex++)
    {
      // Create a MmWaveUniformDiscUePositionAllocator object for each BS and
      // set the center of the disc at the BS position
      Ptr<MmWaveHeterogeneousLayoutPositionAllocator> pa = CreateObject<MmWaveHeterogeneousLayoutPositionAllocator> ();
      pa->SetMacroNodesContainer (macroNodes);
      pa->SetMicroNodesContainer (microNodes);
      Vector macroPos = macroNodes.Get (macroIndex)->GetObject<MobilityModel> ()->GetPosition ();
      pa->SetX (macroPos.x);
      pa->SetY (macroPos.y);

      // Create a NodeContainer containing numOfUesPerBs UE nodes
      NodeContainer uesToDrop;
      for (uint8_t i = 0; i < numOfUesPerSite; i++)
      {
        NS_ASSERT_MSG (ueNodesIndex < ueNodes.GetN (), "Not enough UE nodes!");
        uesToDrop.Add (ueNodes.Get (ueNodesIndex));
        ueNodesIndex++;
      }

      MobilityHelper mobility;
      mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
      mobility.SetPositionAllocator(pa);
      mobility.Install (uesToDrop);
    }
  }

  void
  SimulationConfig::SetupUdpApplication (Ptr<Node> node, Ipv4Address address, uint16_t port, uint16_t interPacketInterval, double startTime, double endTime)
  {
    ApplicationContainer app;
    UdpClientHelper client (address, port);
    client.SetAttribute ("Interval", TimeValue (MicroSeconds(interPacketInterval)));
    client.SetAttribute ("MaxPackets", UintegerValue(10000000));

    app.Add (client.Install (node));
    app.Start (Seconds (startTime));
    app.Stop (Seconds (endTime));

    NS_LOG_INFO ("Number of packets to send " << std::floor((endTime-startTime)/interPacketInterval*1000));
  }

  void
  SimulationConfig::SetupUdpPacketSink (Ptr<Node> node, uint16_t port, double startTime, double endTime, Ptr<OutputStreamWrapper> stream)
  {
    ApplicationContainer app;
    PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
    app.Add (packetSinkHelper.Install (node));
    app.Start (Seconds (startTime));
    app.Stop (Seconds (endTime));

    app.Get(0)->TraceConnectWithoutContext("Rx", MakeBoundCallback (&CallbackSinks::RxSink, stream));
  }

  void
  SimulationConfig::SetupFtpModel3Application (Ptr<Node> clientNode, Ptr<Node> serverNode, Ipv4Address address, uint16_t port, double lambda, Time startTime, Time endTime, Ptr<OutputStreamWrapper> stream)
  {
    // Install FTP application on client node
    ApplicationContainer clientApps;
    FileTransferHelper ftp ("ns3::TcpSocketFactory", InetSocketAddress (address, port));
    clientApps.Add (ftp.Install (clientNode));
    clientApps.Start (startTime);
    clientApps.Stop (endTime);

    // Install PacketSink application on server node
    ApplicationContainer serverApps;
    PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
    serverApps = packetSinkHelper.Install (serverNode);
    serverApps.Start (Seconds (0));
    // serverApps.Stop (endTime); // stop the PacketSink when the simulation ends

    serverApps.Get(0)->TraceConnectWithoutContext("Rx", MakeBoundCallback (&CallbackSinks::RxSink, stream));
    clientApps.Get(0)->TraceConnectWithoutContext("Tx", MakeBoundCallback (&CallbackSinks::TxSink, stream));

    // Trigger data transfer
    Ptr<ExponentialRandomVariable> interArrivalTime = CreateObject<ExponentialRandomVariable> ();
    interArrivalTime->SetAttribute ("Mean", DoubleValue (1/lambda));

    Ptr<FileTransferApplication> ftpApp = DynamicCast<FileTransferApplication> (clientApps.Get (0));

    // Compute the time instant of the first file transfer. If it would happen
    // after the endTime do not send any file, otherwise schedule the actual
    // transmission
    double firstSend = interArrivalTime->GetValue () + startTime.GetSeconds (); // TODO: bound
    if (firstSend < endTime.GetSeconds ())
    {
      Simulator::Schedule (Seconds (firstSend), &ScheduleFileTransfer, interArrivalTime, endTime, ftpApp);
      NS_LOG_INFO ("App " << ftpApp << " first file transmission scheduled at " << firstSend + Simulator::Now ().GetSeconds ());
    }
    else
    {
      NS_LOG_WARN ("App " << ftpApp << " not enough time for any transmission (firstSend="<< firstSend << " endTime=" << endTime.GetSeconds () << ").");
    }
  }

  void
  SimulationConfig::ScheduleFileTransfer (Ptr<ExponentialRandomVariable> interArrivalTime, Time endTime, Ptr<FileTransferApplication> ftpApp)
  {
    // This method is used by FTP Model 3

    // Send file
    SimulationConfig::StartFileTransfer (ftpApp);

    // Compute the time instant of the next file transfer. If it would happen
    // after the endTime do not send any file, otherwise schedule the actual
    // transmission.
    // NOTE The next file transmission is scheduled independently on the
    // previous file, i.e., without waiting for the end of its transmission. If
    // it is not yet completed, the next file will be queued. This is the
    // difference with FTP Model 2.
    double nextSend = interArrivalTime->GetValue ();
    if (nextSend + Simulator::Now ().GetSeconds () < endTime.GetSeconds ())
    {
      Simulator::Schedule (Seconds (nextSend), &ScheduleFileTransfer, interArrivalTime, endTime, ftpApp);
      NS_LOG_INFO ("App " << ftpApp << " next file transmission scheduled at " << nextSend + Simulator::Now ().GetSeconds ());
    }
    else
    {
      NS_LOG_INFO ("App " << ftpApp << " not enough time for further transmissions.");
    }
  }

  void
  SimulationConfig::StartFileTransfer (Ptr<FileTransferApplication> ftpApp)
  {
    NS_LOG_INFO ("App " << ftpApp << " start file transmission");
    ftpApp->SendFile ();
  }

  void
  SimulationConfig::SetTracesPath (std::string filePath)
  {
    Config::SetDefault("ns3::MmWaveBearerStatsCalculator::DlRlcOutputFilename", StringValue(filePath + "DlRlcStats.txt"));
    Config::SetDefault("ns3::MmWaveBearerStatsCalculator::UlRlcOutputFilename", StringValue(filePath + "UlRlcStats.txt"));
    Config::SetDefault("ns3::MmWaveBearerStatsCalculator::DlPdcpOutputFilename", StringValue(filePath + "DlPdcpStats.txt"));
    Config::SetDefault("ns3::MmWaveBearerStatsCalculator::UlPdcpOutputFilename", StringValue(filePath + "UlPdcpStats.txt"));
    Config::SetDefault("ns3::MmWavePhyRxTrace::OutputFilename", StringValue(filePath + "RxPacketTrace.txt"));
    Config::SetDefault("ns3::LteRlcAm::BufferSizeFilename", StringValue(filePath + "RlcAmBufferSize.txt"));
  }

  void
  RandomBuildings::CreateRandomBuildings (double streetWidth, double blockSize, double maxXAxis, double maxYAxis, uint32_t numBlocks)
  {
    /* Create the building */
  	 double maxObstacleSize = blockSize - streetWidth;

  	 std::vector<Ptr<Building> > buildingVector;
  	 std::list<Box>  m_previousBlocks;

  	 for(uint32_t buildingIndex = 0; buildingIndex < numBlocks; buildingIndex++)
  	 {
  		 Ptr < Building > building;
  		 building = Create<Building> ();
  		 /* returns a vecotr where:
  		 * position [0]: coordinates for x min
  		 * position [1]: coordinates for x max
  		 * position [2]: coordinates for y min
  		 * position [3]: coordinates for y max
  		 */

  		 std::pair<Box, std::list<Box>> pairBuildings = RandomBuildings::GenerateBuildingBounds(0, maxXAxis-maxObstacleSize, 0, maxYAxis-maxObstacleSize, maxObstacleSize, m_previousBlocks);
  		 m_previousBlocks = std::get<1>(pairBuildings);
  	 	 Box box = std::get<0>(pairBuildings);

  		 Ptr<UniformRandomVariable> randomBuildingZ = CreateObject<UniformRandomVariable>();
  		 randomBuildingZ->SetAttribute("Min",DoubleValue(1.6));
  		 randomBuildingZ->SetAttribute("Max",DoubleValue(50));
  		 double buildingHeight = randomBuildingZ->GetValue();
       NS_LOG_INFO ("Building height " << buildingHeight << "\n");

  		 building->SetBoundaries (Box(box.xMin, box.xMax,
  																	 box.yMin,  box.yMax,
  																	 0.0, buildingHeight));

  		 building->SetNRoomsX(1);
  		 building->SetNRoomsY(1);
  		 building->SetNFloors(1);
  		 buildingVector.push_back(building);
  	 }
  		/* END Create the building */
  }

  void
  CallbackSinks::RxSink (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> packet, const Address &from)
  {
    *stream->GetStream () << "Rx\t" << Simulator::Now ().GetSeconds () << "\t" << packet->GetSize() <<  std::endl;
  }

  void
  CallbackSinks::RxSinkAddresses (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> packet, const Address &from, 
                                  const Address &to)
  {
    SeqTsHeader seqTs;
    Ptr<Packet> mutablePacket = packet->Copy ();
    mutablePacket->RemoveHeader (seqTs);
    *stream->GetStream () << packet->GetSize () << " " << InetSocketAddress::ConvertFrom (to).GetPort ()
                          << " " << Simulator::Now ().GetNanoSeconds () << " " 
                          << Simulator::Now ().GetNanoSeconds () - seqTs.GetTs ().GetNanoSeconds () << "\n";
  }

  void
  CallbackSinks::RxSinkE2EStat (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> packet, const Address &from, 
                                    const Address &to, const E2eStatsHeader &e2estats)
  {
    SeqTsHeader seqTs;
    Ptr<Packet> mutablePacket = packet->Copy ();
    mutablePacket->RemoveHeader (seqTs);
    *stream->GetStream () << packet->GetSize () << " " << InetSocketAddress::ConvertFrom (to).GetPort ()
                          << " " << Simulator::Now ().GetNanoSeconds () << " " 
                          << Simulator::Now ().GetNanoSeconds () - seqTs.GetTs ().GetNanoSeconds () << "\n"; 
  }

  void
  CallbackSinks::TxSink (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> packet, const Address &from)
  {
    *stream->GetStream () << "Tx\t" << Simulator::Now ().GetSeconds () << "\t" << packet->GetSize() <<  std::endl;
  }

  std::pair<Box, std::list<Box>>
  RandomBuildings::GenerateBuildingBounds(double xMin, double xMax, double yMin, double yMax, double maxBuildSize, std::list<Box> m_previousBlocks )
  {

    Ptr<UniformRandomVariable> xMinBuilding = CreateObject<UniformRandomVariable>();
    xMinBuilding->SetAttribute("Min",DoubleValue(xMin));
    xMinBuilding->SetAttribute("Max",DoubleValue(xMax-1)); // 1 m is the minimum size

    Ptr<UniformRandomVariable> yMinBuilding = CreateObject<UniformRandomVariable>();
    yMinBuilding->SetAttribute("Min",DoubleValue(yMin));
    yMinBuilding->SetAttribute("Max",DoubleValue(yMax-1)); // 1 m is the minimum size

    Box box;
    uint32_t attempt = 0;
    do
    {
      NS_ASSERT_MSG(attempt < 100, "Too many failed attempts to position non-overlapping buildings. Maybe area too small or too many buildings?");
      box.xMin = xMinBuilding->GetValue();

      Ptr<UniformRandomVariable> xMaxBuilding = CreateObject<UniformRandomVariable>();
      xMaxBuilding->SetAttribute("Min",DoubleValue(box.xMin + 1)); // 1 m is the minimum size
      xMaxBuilding->SetAttribute("Max",DoubleValue(box.xMin + maxBuildSize));
      box.xMax = xMaxBuilding->GetValue();

      box.yMin = yMinBuilding->GetValue();

      Ptr<UniformRandomVariable> yMaxBuilding = CreateObject<UniformRandomVariable>();
      yMaxBuilding->SetAttribute("Min",DoubleValue(box.yMin + 1)); // 1 m is the minimum size
      yMaxBuilding->SetAttribute("Max",DoubleValue(box.yMin + maxBuildSize));
      box.yMax = yMaxBuilding->GetValue();

      ++attempt;
    }
    while (OverlapWithAnyPrevious (box, m_previousBlocks));


    NS_LOG_INFO("Building in coordinates (" << box.xMin << " , " << box.yMin << ") and ("  << box.xMax << " , " << box.yMax <<
      ") accepted after " << attempt << " attempts");
    m_previousBlocks.push_back(box);
    std::pair<Box, std::list<Box>> pairReturn = std::make_pair(box,m_previousBlocks);
    return pairReturn;
  }

  bool
  RandomBuildings::AreOverlapping(Box a, Box b)
  {
    return !((a.xMin > b.xMax) || (b.xMin > a.xMax) || (a.yMin > b.yMax) || (b.yMin > a.yMax) );
  }

  bool
  RandomBuildings::OverlapWithAnyPrevious(Box box, std::list<Box> m_previousBlocks)
  {
    for (std::list<Box>::iterator it = m_previousBlocks.begin(); it != m_previousBlocks.end(); ++it)
    {
      if (AreOverlapping(*it,box))
      {
        return true;
      }
    }
    return false;
  }

  void
  PrintHelper::PrintGnuplottableBuildingListToFile (std::string filename)
  {
    std::ofstream outFile;
    outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
    if (!outFile.is_open ())
      {
        NS_LOG_ERROR ("Can't open file " << filename);
        return;
      }

  	//outFile << "set xrange [0:100]" << std::endl;
  	//outFile << "set yrange [0:100]" << std::endl;
  	outFile << "unset key" << std::endl;
  	outFile << "set grid" << std::endl;

    uint32_t index = 0;
    for (BuildingList::Iterator it = BuildingList::Begin (); it != BuildingList::End (); ++it)
      {
        ++index;
        Box box = (*it)->GetBoundaries ();
        outFile << "set object " << index
                << " rect from " << box.xMin  << "," << box.yMin
                << " to "   << box.xMax  << "," << box.yMax
                //<< " height " << box.zMin << "," << box.zMax
                << " front fs empty "
                << std::endl;
      }
  }

  void
  PrintHelper::PrintGnuplottableNodeListToFile (std::string filename)
  {
    std::ofstream outFile;
    outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
    if (!outFile.is_open ())
      {
        NS_LOG_ERROR ("Can't open file " << filename);
        return;
      }
    for (NodeList::Iterator it = NodeList::Begin (); it != NodeList::End (); ++it)
      {
        Ptr<Node> node = *it;
        int nDevs = node->GetNDevices ();
        for (int j = 0; j < nDevs; j++)
          {
            Ptr<LteUeNetDevice> uedev = node->GetDevice (j)->GetObject <LteUeNetDevice> ();
            Ptr<MmWaveUeNetDevice> mmuedev = node->GetDevice (j)->GetObject <MmWaveUeNetDevice> ();
            Ptr<McUeNetDevice> mcuedev = node->GetDevice (j)->GetObject <McUeNetDevice> ();
            Ptr<LteEnbNetDevice> enbdev = node->GetDevice (j)->GetObject <LteEnbNetDevice> ();
            Ptr<MmWaveEnbNetDevice> mmenbdev = node->GetDevice (j)->GetObject <MmWaveEnbNetDevice> ();
            Ptr<MmWaveIabNetDevice> mmIabdev = node->GetDevice (j)->GetObject <MmWaveIabNetDevice> ();
            if (uedev)
              {
                Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
                outFile << "set label \"" << uedev->GetImsi ()
                        << "\" at "<< pos.x << "," << pos.y << " left font \"Helvetica,8\" textcolor rgb \"black\" front point pt 7 ps 0.3 lc rgb \"black\" offset 0,0"
                        << std::endl;

                Simulator::Schedule (Seconds (1), &PrintHelper::UpdateGnuplottableNodeListToFile, filename, node);
              }
            else if (mmuedev)
             {
                Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
                outFile << "set label \"" << mmuedev->GetImsi ()
                        << "\" at "<< pos.x << "," << pos.y << " left font \"Helvetica,8\" textcolor rgb \"black\" front point pt 7 ps 0.3 lc rgb \"black\" offset 0,0"
                        << std::endl;

                Simulator::Schedule (Seconds (1), &PrintHelper::UpdateGnuplottableNodeListToFile, filename, node);
              }
            else if (mcuedev)
             {
                Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
                outFile << "set label \"" << mcuedev->GetImsi ()
                        << "\" at "<< pos.x << "," << pos.y << " left font \"Helvetica,8\" textcolor rgb \"black\" front point pt 7 ps 0.3 lc rgb \"black\" offset 0,0"
                        << std::endl;

                Simulator::Schedule (Seconds (1), &PrintHelper::UpdateGnuplottableNodeListToFile, filename, node);
              }
            else if (enbdev)
              {
                 Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
                 outFile << "set label \"" << enbdev->GetCellId ()
                         << "\" at "<< pos.x << "," << pos.y << " left font \"Helvetica,8\" textcolor rgb \"red\" front point pt 5 ps 0.8 lc rgb \"red\" offset 0,0"
                         << std::endl;
               }
            else if (mmenbdev)
              {
                 Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
                 outFile << "set label \"" << mmenbdev->GetCellId ()
                         << "\" at "<< pos.x << "," << pos.y << " left font \"Helvetica,8\" textcolor rgb \"red\" front point pt 5 ps 0.8 lc rgb \"red\" offset 0,0"
                         << std::endl;
               }
            else if (mmIabdev)
            {
              Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
              outFile << "set label \"" << mmIabdev->GetCellId ()
                      << "\" at "<< pos.x << "," << pos.y << " left font \"Helvetica,8\" textcolor rgb \"blue\" front point pt 5 ps 0.8 lc rgb \"blue\" offset 0,0"
                      << std::endl;
            }
          }
      }
  }

  void
  PrintHelper::UpdateGnuplottableNodeListToFile (std::string filename, Ptr<Node> node)
  {
    std::ofstream outFile;
    outFile.open (filename.c_str (), std::ios_base::app);
    if (!outFile.is_open ())
      {
        NS_LOG_ERROR ("Can't open file " << filename);
        return;
      }
    Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
    outFile << "set label \""
            << "\" at "<< pos.x << "," << pos.y << " left font \"Helvetica,8\" textcolor rgb \"black\" front point pt 1 ps 0.3 lc rgb \"black\" offset 0,0"
            << std::endl;

    Simulator::Schedule (Seconds (1), &PrintHelper::UpdateGnuplottableNodeListToFile, filename, node);
  }

} // end namespace ns3
