/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 * Copyright (c) 2016, University of Padova, Dep. of Information Engineering, SIGNET lab
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
 * Author: Jaume Nin <jnin@cttc.cat>
 *         Nicola Baldo <nbaldo@cttc.cat>
 *
 * Modified by Michele Polese <michele.polese@gmail.com>
 *     (support for RRC_CONNECTED->RRC_IDLE state transition + support for real S1AP link)
 */


#include "epc-enb-application.h"
#include "ns3/log.h"
#include "ns3/mac48-address.h"
#include "ns3/ipv4.h"
#include "ns3/inet-socket-address.h"
#include "ns3/uinteger.h"

#include "epc-gtpu-header.h"
#include "eps-bearer-tag.h"
#include "ns3/epc-s1ap-header.h"
#include "ns3/epc-x2-header.h"
#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("EpcEnbApplication");

EpcEnbApplication::EpsFlowId_t::EpsFlowId_t ()
{
}

EpcEnbApplication::EpsFlowId_t::EpsFlowId_t (const uint16_t a, const uint8_t b)
  : m_rnti (a),
    m_bid (b),
    m_isLocal (true)
{
}

EpcEnbApplication::EpsFlowId_t::EpsFlowId_t (const uint16_t a, const uint8_t b, const bool c)
  : m_rnti (a),
    m_bid (b),
    m_isLocal (c)
{
}

bool
operator == (const EpcEnbApplication::EpsFlowId_t &a, const EpcEnbApplication::EpsFlowId_t &b)
{
  return ( (a.m_rnti == b.m_rnti) && (a.m_bid == b.m_bid) && (a.m_isLocal == b.m_isLocal));
}

bool
operator < (const EpcEnbApplication::EpsFlowId_t& a, const EpcEnbApplication::EpsFlowId_t& b)
{
  return ( (a.m_rnti < b.m_rnti) || ( (a.m_rnti == b.m_rnti) && (a.m_bid < b.m_bid) ) );
}


TypeId
EpcEnbApplication::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::EpcEnbApplication")
    .SetParent<Object> ()
    .SetGroupName("Lte");
  return tid;
}

void
EpcEnbApplication::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_lteSocket = 0;
  m_s1uSocket = 0;
  delete m_s1SapProvider;
  delete m_s1apSapEnb;
}


EpcEnbApplication::EpcEnbApplication (Ptr<Socket> lteSocket, Ptr<Socket> s1uSocket, Ipv4Address enbS1uAddress, Ipv4Address sgwS1uAddress, uint16_t cellId)
  : m_lteSocket (lteSocket),
    m_s1uSocket (s1uSocket),    
    m_enbS1uAddress (enbS1uAddress),
    m_sgwS1uAddress (sgwS1uAddress),
    m_gtpuUdpPort (2152), // fixed by the standard
    m_s1SapUser (0),
    m_s1apSapEnbProvider (0),
    m_cellId (cellId)
{
  NS_LOG_FUNCTION (this << lteSocket << s1uSocket << sgwS1uAddress);
  m_s1uSocket->SetRecvCallback (MakeCallback (&EpcEnbApplication::RecvFromS1uSocket, this));
  m_lteSocket->SetRecvCallback (MakeCallback (&EpcEnbApplication::RecvFromLteSocket, this));
  m_s1SapProvider = new MemberEpcEnbS1SapProvider<EpcEnbApplication> (this);
  m_s1apSapEnb = new MemberEpcS1apSapEnb<EpcEnbApplication> (this);
  m_rntiMaxNumIabNodesMap.clear();
}


EpcEnbApplication::~EpcEnbApplication (void)
{
  NS_LOG_FUNCTION (this);
}


void 
EpcEnbApplication::SetS1SapUser (EpcEnbS1SapUser * s)
{
  m_s1SapUser = s;
}


EpcEnbS1SapProvider* 
EpcEnbApplication::GetS1SapProvider ()
{
  return m_s1SapProvider;
}

void 
EpcEnbApplication::SetS1apSapMme (EpcS1apSapEnbProvider * s)
{
  m_s1apSapEnbProvider = s;
}

  
EpcS1apSapEnb* 
EpcEnbApplication::GetS1apSapEnb ()
{
  return m_s1apSapEnb;
}

void 
EpcEnbApplication::DoInitialUeMessage (uint64_t imsi, uint16_t rnti, bool iab)
{
  NS_LOG_FUNCTION (this);
  // side effect: create entry if not exist
  m_imsiRntiMap[imsi] = rnti;
  auto rntiIt = m_rntiLocalImsiMap.find(rnti);
  if(rntiIt != m_rntiLocalImsiMap.end())
  {
    // update
    rntiIt->second = imsi;
  }
  else
  {
    // insert
    m_rntiLocalImsiMap[rnti] = imsi;
  }

  NS_LOG_INFO("DoInitialUeMessage imsi " << imsi << " rnti " << rnti << " iab " << iab);

  if(iab)
  {
    m_rntiMaxNumIabNodesMap[rnti] = 1;
    NS_LOG_INFO("cellId " << m_cellId << 
      " the maximum depth of the IAB tree for rnti " << rnti << " in this case is " 
        << m_rntiMaxNumIabNodesMap[rnti] << ", but the max is " << m_rntiMaxNumIabNodesMap[rnti]);
  }

  // IAB hack: the first and third field
  // were the same in the original implementation
  // Change the third to be the 0, which represents wired devices!
  EpcS1apSap::InitialUeMessageParams params;
  params.iab = iab;
  params.mmeUeS1Id = imsi;
  params.enbUeS1Id = rnti;
  params.stmsi = 0; // 0 represents wired devices!
  params.ecgi = m_cellId;
  m_s1apSapEnbProvider->SendInitialUeMessage (params); // TODO if more than one MME is used, extend this call
}

void 
EpcEnbApplication::DoPathSwitchRequest (EpcEnbS1SapProvider::PathSwitchRequestParameters params)
{
  NS_LOG_FUNCTION (this);
  uint16_t enbUeS1Id = params.rnti;  
  uint64_t mmeUeS1Id = params.mmeUeS1Id;
  uint64_t imsi = mmeUeS1Id;
  // side effect: create entry if not exist
  m_imsiRntiMap[imsi] = params.rnti;

  auto rntiIt = m_rntiLocalImsiMap.find(params.rnti);
  if(rntiIt != m_rntiLocalImsiMap.end())
  {
    // update
    rntiIt->second = imsi;
  }
  else
  {
    // insert
    m_rntiLocalImsiMap[params.rnti] = imsi;
  }

  uint16_t gci = params.cellId;
  std::list<EpcS1apSapMme::ErabSwitchedInDownlinkItem> erabToBeSwitchedInDownlinkList;
  for (std::list<EpcEnbS1SapProvider::BearerToBeSwitched>::iterator bit = params.bearersToBeSwitched.begin ();
       bit != params.bearersToBeSwitched.end ();
       ++bit)
    {
      EpsFlowId_t flowId;
      flowId.m_rnti = params.rnti;
      flowId.m_bid = bit->epsBearerId;
      uint32_t teid = bit->teid;
      
      EpsFlowId_t rbid (params.rnti, bit->epsBearerId);
      // side effect: create entries if not exist
      m_rbidTeidMap[params.rnti][bit->epsBearerId] = teid;
      m_teidRbidMap[teid] = rbid;

      EpcS1apSapMme::ErabSwitchedInDownlinkItem erab;
      erab.erabId = bit->epsBearerId;
      erab.enbTransportLayerAddress = m_enbS1uAddress;
      erab.enbTeid = bit->teid;

      erabToBeSwitchedInDownlinkList.push_back (erab);
    }
  m_s1apSapEnbProvider->SendPathSwitchRequest (enbUeS1Id, mmeUeS1Id, gci, erabToBeSwitchedInDownlinkList);
}

void 
EpcEnbApplication::DoUeContextRelease (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << rnti);
  std::map<uint16_t, std::map<uint8_t, uint32_t> >::iterator rntiIt = m_rbidTeidMap.find (rnti);
  if (rntiIt != m_rbidTeidMap.end ())
    {
      for (std::map<uint8_t, uint32_t>::iterator bidIt = rntiIt->second.begin ();
           bidIt != rntiIt->second.end ();
           ++bidIt)
        {
          uint32_t teid = bidIt->second;
          m_teidRbidMap.erase (teid);
        }
      m_rbidTeidMap.erase (rntiIt);
    }
}

void 
EpcEnbApplication::DoInitialContextSetupRequest (uint64_t mmeUeS1Id, uint16_t enbUeS1Id, std::list<EpcS1apSapEnb::ErabToBeSetupItem> erabToBeSetupList, bool iab)
{
  NS_LOG_FUNCTION (this);
  uint64_t imsi = mmeUeS1Id;

  std::map<uint64_t, uint16_t>::iterator imsiIt = m_imsiRntiMap.find (imsi);
  
  auto imsiIabIt = m_imsiIabMap.find(imsi);
  if(imsiIabIt == m_imsiIabMap.end())
  {
    m_imsiIabMap[imsi] = iab; // set it this imsi is an IAB dev or not
  }

  NS_LOG_INFO("EpcEnbApplication DoInitialContextSetupRequest for imsi " << imsi << " IAB " << iab);

  if(imsiIt != m_imsiRntiMap.end ())
  {
      NS_LOG_INFO("In EnpEnbApplication DoInitialContextSetupRequest size of the erabToBeSetupList is " << erabToBeSetupList.size());

      for (std::list<EpcS1apSapEnb::ErabToBeSetupItem>::iterator erabIt = erabToBeSetupList.begin ();
           erabIt != erabToBeSetupList.end ();
           ++erabIt)
        {
          // request the RRC to setup a radio bearer          
          uint16_t rnti = imsiIt->second;
          uint64_t numChildren = 0;
          auto rntiChildrenIter = m_rntiImsiChildrenMap.find(rnti);
          if(rntiChildrenIter != m_rntiImsiChildrenMap.end())
          {
            numChildren = rntiChildrenIter->second.size();
          }

          auto rntiRbidTeidIter = m_rbidTeidMap.find(rnti);
          if(rntiRbidTeidIter != m_rbidTeidMap.end())
          {
            auto bidTeidIter = rntiRbidTeidIter->second.find(erabIt->erabId);
            if(bidTeidIter != rntiRbidTeidIter->second.end())
            {
              NS_LOG_INFO("cellId " << m_cellId << " rnti " << rnti << " imsi " << imsi << " bearer already created (duplicated request)");
              break;
            }
          }

          NS_LOG_INFO(this << "Setup Radio Bearer for imsi " << imsi << " rnti " << 
              rnti << " with " << numChildren << " children");
          
          struct EpcEnbS1SapUser::DataRadioBearerSetupRequestParameters params;
          params.rnti = rnti;
          params.bearer = erabIt->erabLevelQosParameters;
          params.bearerId = erabIt->erabId;
          params.gtpTeid = erabIt->sgwTeid;
          // TODOIAB what for handovers
          params.iab = iab;
          m_s1SapUser->DataRadioBearerSetupRequest (params);

          EpsFlowId_t rbid (rnti, erabIt->erabId);
          // side effect: create entries if not exist
          m_rbidTeidMap[rnti][erabIt->erabId] = params.gtpTeid;
          m_teidRbidMap[params.gtpTeid] = rbid;

        }
  }
  else
  {
    NS_FATAL_ERROR("Unrecognized imsi");
  }
}

void 
EpcEnbApplication::DoPathSwitchRequestAcknowledge (uint64_t enbUeS1Id, uint64_t mmeUeS1Id, uint16_t gci, std::list<EpcS1apSapEnb::ErabSwitchedInUplinkItem> erabToBeSwitchedInUplinkList)
{
  NS_LOG_FUNCTION (this);

  uint64_t imsi = mmeUeS1Id;
  std::map<uint64_t, uint16_t>::iterator imsiIt = m_imsiRntiMap.find (imsi);
  NS_ASSERT_MSG (imsiIt != m_imsiRntiMap.end (), "unknown IMSI");
  uint16_t rnti = imsiIt->second;
  EpcEnbS1SapUser::PathSwitchRequestAcknowledgeParameters params;
  params.rnti = rnti;
  m_s1SapUser->PathSwitchRequestAcknowledge (params);
}

void 
EpcEnbApplication::RecvFromLteSocket (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this);  
  NS_ASSERT (socket == m_lteSocket);
  Ptr<Packet> packet = socket->Recv ();

  /// \internal
  /// Workaround for \bugid{231}
  //SocketAddressTag satag;
  //packet->RemovePacketTag (satag);

  // filter packets
  EpsBearerTag tag;
  bool found = packet->RemovePacketTag (tag);
  NS_ASSERT (found);
  uint16_t rnti = tag.GetRnti ();
  uint8_t bid = tag.GetBid ();
  EpsFlowId_t rbid (rnti, bid);
  NS_LOG_LOGIC ("received packet with RNTI=" << (uint32_t) rnti << ", BID=" << (uint32_t)  bid);

  // find imsi to check if the packet was received by an IAB node
  auto imsiLocalIt = m_rntiLocalImsiMap.find(rnti); // notice that this IMSI corresponds to that of the UE/IAB connected to this gNB, not that of a remote UE/IAB
  bool iabRelatedMessage = false;

  if(imsiLocalIt == m_rntiLocalImsiMap.end())
  {
    NS_FATAL_ERROR("Unknown IMSI/RNTI association");
  }
  else
  {
    uint64_t imsiLocal = imsiLocalIt->second;
    // find if the IMSI associated to this RNTI is that of an IAB device
    auto iabIt = m_imsiIabMap.find(imsiLocal);

    iabRelatedMessage = iabIt->second; // if this is true, then the message came from an IAB dev. It can be data or control!
  }

  uint32_t teid = 0;
  uint8_t gtpMessageType = 0;

  if(iabRelatedMessage)
  {
    GtpuHeader gtpHeader;
    if(packet->RemoveHeader(gtpHeader))
    {
      NS_LOG_INFO("Removed GTP header " << gtpHeader);
      teid = gtpHeader.GetTeid();
      gtpMessageType = gtpHeader.GetMessageType();
    }
    else
    {
      NS_FATAL_ERROR("GTP header not found");
    }

    // for IAB, check if this is an S1ap or an X2 packet!
    EpcS1APHeader s1apHeader;
    if(gtpMessageType == GtpuHeader::S1AP && packet->RemoveHeader (s1apHeader))
    {
      NS_LOG_INFO("EpcEnbApplication recognizes this s1apHeader " << s1apHeader);

      EpcS1APPathSwitchRequestHeader psReqHeader;
      EpcS1APInitialContextSetupResponseHeader setupRespHeader;
      EpcS1APErabReleaseIndicationHeader erabReleaseHeader;
      EpcS1APInitialUeMessageHeader initialMessageHeader;

      uint64_t imsi; // this is the IMSI of the UE related to the control

      if(packet->RemoveHeader(initialMessageHeader))
      {
        imsi = initialMessageHeader.GetMmeUeS1Id();
        bool iab = initialMessageHeader.GetIab();

        // get all the imsis which are the children of this rnti
        std::list<uint64_t> imsiList = initialMessageHeader.GetParentImsiList();
        auto rntiIt = m_rntiImsiChildrenMap.find(rnti);

        if(rntiIt == m_rntiImsiChildrenMap.end())
        {
          m_rntiImsiChildrenMap.insert(std::make_pair(rnti, std::vector<uint64_t>()));
          rntiIt = m_rntiImsiChildrenMap.find(rnti);
        }

        for(auto imsiChildIter : imsiList)
        {
          auto imsiInRntiListIter = std::find(rntiIt->second.begin(), rntiIt->second.end(), imsiChildIter);
          if(imsiInRntiListIter == rntiIt->second.end())
          {
            // add the imsi to the list
            // NS_LOG_INFO("add the imsi to the list");
            rntiIt->second.push_back(imsiChildIter);
          }
        }

        if(iab)
        {
          auto rntiMaxIt = m_rntiMaxNumIabNodesMap.find(rnti);
          uint32_t depthTmp = imsiList.size() + 1;
          if (rntiMaxIt == m_rntiMaxNumIabNodesMap.end())
          {
            m_rntiMaxNumIabNodesMap.insert(std::make_pair(rnti, depthTmp));
            rntiMaxIt = m_rntiMaxNumIabNodesMap.find(rnti);
          }
          else
          {
            uint32_t oldMax = rntiMaxIt->second;
            if (depthTmp > oldMax)
            {
              rntiMaxIt->second = depthTmp;
            }
          }
          NS_LOG_INFO("cellId " << m_cellId << 
            " the maximum depth of the IAB tree for rnti " << rnti << " in this case is " 
            << depthTmp << ", but the max is " << rntiMaxIt->second);
        }

        // scan the list
        NS_LOG_INFO("EpcIabApplication RecvFromLteSocket RNTI " << rnti << 
          " wired eNB -- children list has " << rntiIt->second.size() << " entries");

        // add the imsi of this device to the list
        initialMessageHeader.AddParent(0); // use 0 for wired
        // NS_LOG_LOGIC("Number of parents " << initialMessageHeader.GetParentImsiList().size());

        packet->AddHeader(initialMessageHeader);
      }
      else if(packet->PeekHeader(setupRespHeader))
      {
        imsi = setupRespHeader.GetMmeUeS1Id();
      }
      else if(packet->PeekHeader(erabReleaseHeader))
      {
        imsi = erabReleaseHeader.GetMmeUeS1Id();
      }
      else if(packet->PeekHeader(psReqHeader))
      {
        imsi = psReqHeader.GetMmeUeS1Id();
      }
      else
      {
        NS_FATAL_ERROR("S1-AP packet with unrecognized header");
      }

      rbid.m_isLocal = false;
      m_imsiRntiMap[imsi] = rnti;
      m_imsiLocalRbidMap[imsi] = rbid;
      m_rbidRemoteImsiMap[rbid] = imsi;

      packet->AddHeader(s1apHeader);

      NS_LOG_INFO("This is an S1AP packet related to imsi " << imsi << ", forward to MME");

      m_s1apSapEnbProvider->ForwardIabS1apMessage(packet, imsi);

      return;
    }

    EpcX2Header x2Header;
    if(gtpMessageType == GtpuHeader::X2 && packet->PeekHeader(x2Header))
    {
      NS_FATAL_ERROR("TODO");
      return;
    }

    SendToS1uSocket(packet, teid);
    return;
  }
  

  std::map<uint16_t, std::map<uint8_t, uint32_t> >::iterator rntiIt = m_rbidTeidMap.find (rnti);
  if (rntiIt == m_rbidTeidMap.end ())
    {
      NS_LOG_WARN ("UE context not found, discarding packet when receiving from lteSocket");
    }
  else
    {
      std::map<uint8_t, uint32_t>::iterator bidIt = rntiIt->second.find (bid);
      NS_ASSERT (bidIt != rntiIt->second.end ());
      teid = bidIt->second; // the m_rbidTeidMap contiains only LOCAL teids
      SendToS1uSocket (packet, teid);
    }
}

void
EpcEnbApplication::DoForwardIabS1apReply (Ptr<Packet> packet)
{
  EpcS1APHeader s1apHeader;
  packet->RemoveHeader (s1apHeader);

  NS_LOG_LOGIC ("S1ap header: " << s1apHeader);

  uint8_t procedureCode = s1apHeader.GetProcedureCode ();

  EpsFlowId_t localRbid;

  if (procedureCode == EpcS1APHeader::InitialContextSetupRequest)
  {
    // this message carries the setup information for the bearers of the remote UE
    // use this to update the map with the remote TEIDs and the local RNTI/BID
    NS_LOG_LOGIC ("Forward INITIAL CONTEXT SETUP REQUEST");
    EpcS1APInitialContextSetupRequestHeader reqHeader;
    packet->RemoveHeader(reqHeader);

    uint64_t mmeUeS1apId = reqHeader.GetMmeUeS1Id(); // this is the IMSI of the remote user
    // uint16_t enbUeS1apId = reqHeader.GetEnbUeS1Id(); // this is the remote RNTI
    bool iab = reqHeader.GetIab();
    std::list<EpcS1apSap::ErabToBeSetupItem> erabToBeSetupList = reqHeader.GetErabToBeSetupItem ();

    NS_LOG_INFO ("EpcEnbApplication::DoForwardIabS1apReply S1ap Initial Context Setup Request " << reqHeader << " imsi " << mmeUeS1apId << " iab " << iab);

    // find the LOCAL rnti associated to this imsi
    auto localRntiIt = m_imsiRntiMap.find(mmeUeS1apId);
    auto localRbidIt = m_imsiLocalRbidMap.find(mmeUeS1apId);
    NS_ASSERT_MSG(localRntiIt != m_imsiRntiMap.end() && localRbidIt != m_imsiLocalRbidMap.end(), "IMSI not found");

    localRbid = localRbidIt->second;

    m_imsiIabMap[mmeUeS1apId] = iab;

    // prune the non-IAB node from the list of children of this local RNTI
    auto rntiChildrenIter = m_rntiImsiChildrenMap.find(localRntiIt->second);
    NS_ASSERT_MSG(rntiChildrenIter != m_rntiImsiChildrenMap.end(), "RNTI not found");
    if(!iab)
    {
      auto imsiChildEntry = std::find(rntiChildrenIter->second.begin(), rntiChildrenIter->second.end(), mmeUeS1apId);
      if(imsiChildEntry != rntiChildrenIter->second.end())
      {
        rntiChildrenIter->second.erase(imsiChildEntry);
      }
    }
    NS_LOG_INFO("cellID " << m_cellId << " EpcEnbApplication RNTI " << localRntiIt->second << " has " << rntiChildrenIter->second.size() 
      << " IAB children");
    
    // let's find the maximum depth of the IAB tree from this node
    auto rntiMaxIt = m_rntiMaxNumIabNodesMap.find(localRntiIt->second);
    uint32_t maxDepth = 0;
    if (rntiMaxIt != m_rntiMaxNumIabNodesMap.end())
    {
      maxDepth = rntiMaxIt->second;
      NS_LOG_INFO("cellId " << m_cellId << " rnti " << localRntiIt->second
        << " has maxDepth " << maxDepth);
    }
    else
    {
      NS_FATAL_ERROR("rnti " << localRntiIt->second << " has no max depth associated");
    }

    // notify the scheduler
    EpcEnbS1SapUser::NotifyNumIabPerRntiParameters params;
    params.rnti = localRntiIt->second;
    params.numIab = maxDepth;
    params.iab = true;
    m_s1SapUser->NotifyNumIabPerRnti (params);

    for (std::list<EpcS1apSapEnb::ErabToBeSetupItem>::iterator erabIt = erabToBeSetupList.begin ();
           erabIt != erabToBeSetupList.end ();
           ++erabIt)
      {
        // store the TEID information
        m_teidRbidMap[erabIt->sgwTeid] = localRbid;
        m_teidRemoteMap[erabIt->sgwTeid] = true;
      }
      packet->AddHeader(reqHeader);
  }
  else if (procedureCode == EpcS1APHeader::PathSwitchRequestAck)
  {
    NS_FATAL_ERROR("Not implemented");
  }
  else
  {
    NS_ASSERT_MSG (false, "ProcedureCode NOT SUPPORTED!!!");
  }

  packet->AddHeader(s1apHeader);

  // send to the LTE socket to the RNTI associated to this imsi
  // add a GTP header to carry the correct TEID
  // it is needed because the RecvFromS1uSocket on the other end will remove this header
  GtpuHeader gtpHeader;
  gtpHeader.SetTeid(0xFFFFFFFF);
  gtpHeader.SetMessageType(GtpuHeader::S1AP);
  packet->AddHeader(gtpHeader);
  SendToLteSocket(packet, localRbid.m_rnti, localRbid.m_bid);
}


void 
EpcEnbApplication::RecvFromS1uSocket (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);  
  NS_ASSERT (socket == m_s1uSocket);
  Ptr<Packet> packet = socket->Recv ();
  GtpuHeader gtpu;
  packet->RemoveHeader (gtpu);
  uint32_t teid = gtpu.GetTeid ();

  if(m_teidRemoteMap.find(teid) != m_teidRemoteMap.end())
  {
    // this is for a remote UE! Re-add the GtpuHeader
    packet->AddHeader(gtpu);
  }

  /// \internal
  /// Workaround for \bugid{231}
  //SocketAddressTag tag;
  //packet->RemovePacketTag (tag);

  std::map<uint32_t, EpsFlowId_t>::iterator it = m_teidRbidMap.find (teid);
  if (it != m_teidRbidMap.end ())
    {

      // uint16_t rnti = it->second.m_rnti;
      // auto rntiChildrenIter = m_rntiImsiChildrenMap.find(rnti);
      // uint16_t numChildren = 0;
      // if(rntiChildrenIter != m_rntiImsiChildrenMap.end())
      // {
      //   numChildren = rntiChildrenIter->second.size();
      // }

      // NS_LOG_INFO(this << " RecvFromS1uSocket rnti " << 
      //         rnti << " with " << numChildren << " children");


      SendToLteSocket (packet, it->second.m_rnti, it->second.m_bid);
    }
  else
    {
      packet = 0;
      NS_LOG_DEBUG("UE context not found, discarding packet when receiving from s1uSocket");
    }  
}

void 
EpcEnbApplication::SendToLteSocket (Ptr<Packet> packet, uint16_t rnti, uint8_t bid)
{
  NS_LOG_FUNCTION (this << packet << rnti << (uint16_t) bid << packet->GetSize ()); 

  EpsBearerTag tag (rnti, bid);
  packet->AddPacketTag (tag);
  int sentBytes = m_lteSocket->Send (packet);
  NS_ASSERT (sentBytes > 0);
}


void 
EpcEnbApplication::SendToS1uSocket (Ptr<Packet> packet, uint32_t teid)
{
  NS_LOG_FUNCTION (this << packet << teid <<  packet->GetSize ());  
  GtpuHeader gtpu;
  gtpu.SetTeid (teid);
  // From 3GPP TS 29.281 v10.0.0 Section 5.1
  // Length of the payload + the non obligatory GTP-U header
  gtpu.SetLength (packet->GetSize () + gtpu.GetSerializedSize () - 8);  
  packet->AddHeader (gtpu);
  uint32_t flags = 0;
  m_s1uSocket->SendTo (packet, flags, InetSocketAddress (m_sgwS1uAddress, m_gtpuUdpPort));
}

uint64_t
EpcEnbApplication::GetImsiFromLocalRnti (uint16_t rnti)
{
  auto mapIt = m_rntiLocalImsiMap.find(rnti);
  if(mapIt == m_rntiLocalImsiMap.end())
  {
    NS_ABORT_MSG ("RNTI: " << rnti << " NOT FOUND!");
  }
  return mapIt->second;
}

uint16_t
EpcEnbApplication::GetLocalRntiFromImsi (uint64_t imsi)
{
  auto mapIt = m_imsiRntiMap.find(imsi);
  if(mapIt == m_imsiRntiMap.end())
  {
    NS_LOG_UNCOND (Now().GetMicroSeconds() << " IMSI: " << imsi << " NOT FOUND FROM DONOR" );
    return 0;
  }
  return mapIt->second;
}

std::set<uint16_t> 
EpcEnbApplication::GetSetUeRntis ()
{
  std::set<uint16_t> ueRntiSet {};
  for (auto localDevice : m_rntiLocalImsiMap)
  {
    if (!IsImsiIab(localDevice.second))
    {
      ueRntiSet.insert (localDevice.first);
    }
  }
  return ueRntiSet;
}

uint64_t
EpcEnbApplication::GetCellId ()
{
  return m_cellId;
}

bool
EpcEnbApplication::IsImsiIab (uint64_t imsi)
{
  auto imsiIabIt = m_imsiIabMap.find (imsi);
  if(imsiIabIt == m_imsiIabMap.end ())
  {
    // UE/IAB node not RRC Connected yet
    return false;
  }
  return imsiIabIt->second;
}

void
EpcEnbApplication::DoReleaseIndication (uint64_t imsi, uint16_t rnti, uint8_t bearerId)
{
  NS_LOG_FUNCTION (this << bearerId );
  std::list<EpcS1apSapMme::ErabToBeReleasedIndication> erabToBeReleaseIndication;
  EpcS1apSapMme::ErabToBeReleasedIndication erab;
  erab.erabId = bearerId;
  erabToBeReleaseIndication.push_back (erab);
  //From 3GPP TS 23401-950 Section 5.4.4.2, enB sends EPS bearer Identity in Bearer Release Indication message to MME
  m_s1apSapEnbProvider->SendErabReleaseIndication (imsi, rnti, erabToBeReleaseIndication);
}

}  // namespace ns3
