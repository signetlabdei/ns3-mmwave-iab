/*
 * mmwave-rr-mac-scheduler.cc
 *
 *  Created on: Jan 11, 2015
 *      Author: sourjya
 */

#include <ns3/log.h>
#include <ns3/abort.h>
#include "mmwave-flex-tti-pf-mac-scheduler.h"
#include <ns3/lte-common.h>
#include <ns3/boolean.h>
#include <stdlib.h>     /* abs */
#include "mmwave-mac-pdu-header.h"
#include "mmwave-mac-pdu-tag.h"
#include "mmwave-spectrum-value-helper.h"
#include <cmath>
#include <algorithm>
#include <ns3/eps-bearer.h>
#include <sstream>      // std::stringstream

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmWaveFlexTtiPfMacScheduler");

NS_OBJECT_ENSURE_REGISTERED (MmWaveFlexTtiPfMacScheduler);

class MmWaveFlexTtiPfMacCschedSapProvider : public MmWaveMacCschedSapProvider
{
public:
  MmWaveFlexTtiPfMacCschedSapProvider (MmWaveFlexTtiPfMacScheduler* scheduler);

  // inherited from MmWaveMacCschedSapProvider
  virtual void CschedCellConfigReq (const struct MmWaveMacCschedSapProvider::CschedCellConfigReqParameters& params);
  virtual void CschedUeConfigReq (const struct MmWaveMacCschedSapProvider::CschedUeConfigReqParameters& params);
  virtual void CschedLcConfigReq (const struct MmWaveMacCschedSapProvider::CschedLcConfigReqParameters& params);
  virtual void CschedLcReleaseReq (const struct MmWaveMacCschedSapProvider::CschedLcReleaseReqParameters& params);
  virtual void CschedUeReleaseReq (const struct MmWaveMacCschedSapProvider::CschedUeReleaseReqParameters& params);

private:
  MmWaveFlexTtiPfMacCschedSapProvider ();
  MmWaveFlexTtiPfMacScheduler* m_scheduler;
};

MmWaveFlexTtiPfMacCschedSapProvider::MmWaveFlexTtiPfMacCschedSapProvider ()
{
}

MmWaveFlexTtiPfMacCschedSapProvider::MmWaveFlexTtiPfMacCschedSapProvider (MmWaveFlexTtiPfMacScheduler* scheduler)
	: m_scheduler (scheduler)
{
}

void
MmWaveFlexTtiPfMacCschedSapProvider::CschedCellConfigReq (const struct MmWaveMacCschedSapProvider::CschedCellConfigReqParameters& params)
{
  m_scheduler->DoCschedCellConfigReq (params);
}

void
MmWaveFlexTtiPfMacCschedSapProvider::CschedUeConfigReq (const struct MmWaveMacCschedSapProvider::CschedUeConfigReqParameters& params)
{
  m_scheduler->DoCschedUeConfigReq (params);
}


void
MmWaveFlexTtiPfMacCschedSapProvider::CschedLcConfigReq (const struct MmWaveMacCschedSapProvider::CschedLcConfigReqParameters& params)
{
  m_scheduler->DoCschedLcConfigReq (params);
}

void
MmWaveFlexTtiPfMacCschedSapProvider::CschedLcReleaseReq (const struct MmWaveMacCschedSapProvider::CschedLcReleaseReqParameters& params)
{
  m_scheduler->DoCschedLcReleaseReq (params);
}

void
MmWaveFlexTtiPfMacCschedSapProvider::CschedUeReleaseReq (const struct MmWaveMacCschedSapProvider::CschedUeReleaseReqParameters& params)
{
  m_scheduler->DoCschedUeReleaseReq (params);
}

class MmWaveFlexTtiPfMacSchedSapProvider : public MmWaveMacSchedSapProvider
{
public:
	MmWaveFlexTtiPfMacSchedSapProvider (MmWaveFlexTtiPfMacScheduler* sched);

	virtual void SchedDlRlcBufferReq (const struct MmWaveMacSchedSapProvider::SchedDlRlcBufferReqParameters& params);
	virtual void SchedTriggerReq (const struct MmWaveMacSchedSapProvider::SchedTriggerReqParameters& params);
	virtual void SchedDlCqiInfoReq (const struct MmWaveMacSchedSapProvider::SchedDlCqiInfoReqParameters& params);
	virtual void SchedUlCqiInfoReq (const struct MmWaveMacSchedSapProvider::SchedUlCqiInfoReqParameters& params);
	virtual void SchedUlMacCtrlInfoReq (const struct MmWaveMacSchedSapProvider::SchedUlMacCtrlInfoReqParameters& params);
	virtual void SchedSetMcs (int mcs);
private:
  MmWaveFlexTtiPfMacSchedSapProvider ();
	MmWaveFlexTtiPfMacScheduler* m_scheduler;
};

MmWaveFlexTtiPfMacSchedSapProvider::MmWaveFlexTtiPfMacSchedSapProvider ()
{
}

MmWaveFlexTtiPfMacSchedSapProvider::MmWaveFlexTtiPfMacSchedSapProvider (MmWaveFlexTtiPfMacScheduler* sched)
	:m_scheduler(sched)
{
}

void
MmWaveFlexTtiPfMacSchedSapProvider::SchedDlRlcBufferReq (const struct MmWaveMacSchedSapProvider::SchedDlRlcBufferReqParameters& params)
{
  m_scheduler->DoSchedDlRlcBufferReq (params);
}

void
MmWaveFlexTtiPfMacSchedSapProvider::SchedTriggerReq (const struct MmWaveMacSchedSapProvider::SchedTriggerReqParameters& params)
{
	m_scheduler->DoSchedTriggerReq(params);
}

void
MmWaveFlexTtiPfMacSchedSapProvider::SchedDlCqiInfoReq (const struct MmWaveMacSchedSapProvider::SchedDlCqiInfoReqParameters& params)
{
	m_scheduler->DoSchedDlCqiInfoReq (params);
}

void
MmWaveFlexTtiPfMacSchedSapProvider::SchedUlCqiInfoReq (const struct MmWaveMacSchedSapProvider::SchedUlCqiInfoReqParameters& params)
{
  m_scheduler->DoSchedUlCqiInfoReq (params);
}

void
MmWaveFlexTtiPfMacSchedSapProvider::SchedUlMacCtrlInfoReq (const struct MmWaveMacSchedSapProvider::SchedUlMacCtrlInfoReqParameters& params)
{
  m_scheduler->DoSchedUlMacCtrlInfoReq (params);
}

void
MmWaveFlexTtiPfMacSchedSapProvider::SchedSetMcs (int mcs)
{
	m_scheduler->DoSchedSetMcs (mcs);
}
const unsigned MmWaveFlexTtiPfMacScheduler::m_macHdrSize = 0;
const unsigned MmWaveFlexTtiPfMacScheduler::m_subHdrSize = 4;
const unsigned MmWaveFlexTtiPfMacScheduler::m_rlcHdrSize = 3;


MmWaveFlexTtiPfMacScheduler::MmWaveFlexTtiPfMacScheduler ()
: m_nextRnti (0),
  m_subframeNo (0),
  m_tbUid (0),
  m_macSchedSapUser (0),
	m_macCschedSapUser (0),
	m_timeWindow(99.0),
	m_maxSchedulingDelay (1),
	m_iabScheduler (false),
	m_split (true)
{
	NS_LOG_FUNCTION (this);
	m_macSchedSapProvider = new MmWaveFlexTtiPfMacSchedSapProvider (this);
	m_macCschedSapProvider = new MmWaveFlexTtiPfMacCschedSapProvider (this);

	m_iabBackahulSapProvider = new MemberMmWaveUeMacCschedSapProvider<MmWaveFlexTtiPfMacScheduler> (this);

	m_iabBusySubframeAllocation.clear();
}

MmWaveFlexTtiPfMacScheduler::~MmWaveFlexTtiPfMacScheduler ()
{
	NS_LOG_FUNCTION (this);
}

void
MmWaveFlexTtiPfMacScheduler::DoDispose (void)
{
	NS_LOG_FUNCTION (this);
	m_wbCqiRxed.clear();
  m_dlHarqProcessesDciInfoMap.clear ();
  m_dlHarqProcessesTimer.clear ();
  m_dlHarqProcessesRlcPduMap.clear ();
  m_dlHarqInfoList.clear ();
  m_ulHarqCurrentProcessId.clear ();
  m_ulHarqProcessesStatus.clear ();
  m_ulHarqProcessesTimer.clear ();
  m_ulHarqProcessesDciInfoMap.clear ();
  m_iabBusySubframeAllocation.clear();
  delete m_macCschedSapProvider;
  delete m_macSchedSapProvider;
}

TypeId
MmWaveFlexTtiPfMacScheduler::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::MmWaveFlexTtiPfMacScheduler")
	    .SetParent<MmWaveMacScheduler> ()
		.AddConstructor<MmWaveFlexTtiPfMacScheduler> ()
    .AddAttribute ("CqiTimerThreshold",
                   "The number of TTIs a CQI is valid (default 1000 - 1 sec.)",
                   UintegerValue (100),
                   MakeUintegerAccessor (&MmWaveFlexTtiPfMacScheduler::m_cqiTimersThreshold),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("HarqEnabled",
                   "Activate/Deactivate the HARQ [by default is active].",
                   BooleanValue (true),
                   MakeBooleanAccessor (&MmWaveFlexTtiPfMacScheduler::m_harqOn),
                   MakeBooleanChecker ())
	 .AddAttribute ("FixedMcsDl",
									"Fix MCS to value set in McsDlDefault",
									BooleanValue (false),
									MakeBooleanAccessor (&MmWaveFlexTtiPfMacScheduler::m_fixedMcsDl),
									MakeBooleanChecker ())
	.AddAttribute ("McsDefaultDl",
								 "Fixed DL MCS",
								 UintegerValue (1),
								 MakeUintegerAccessor (&MmWaveFlexTtiPfMacScheduler::m_mcsDefaultDl),
								 MakeUintegerChecker<uint8_t> ())
	 .AddAttribute ("FixedMcsUl",
									"Fix MCS to value set in McsUlDefault (for testing)",
									BooleanValue (false),
									MakeBooleanAccessor (&MmWaveFlexTtiPfMacScheduler::m_fixedMcsUl),
									MakeBooleanChecker ())
	.AddAttribute ("McsDefaultUl",
								 "Fixed UL MCS (for testing)",
								 UintegerValue (1),
								 MakeUintegerAccessor (&MmWaveFlexTtiPfMacScheduler::m_mcsDefaultUl),
								 MakeUintegerChecker<uint8_t> ())
	 .AddAttribute ("DlSchedOnly",
									"Only schedule downlink traffic (for testing)",
									BooleanValue (false),
									MakeBooleanAccessor (&MmWaveFlexTtiPfMacScheduler::m_dlOnly),
									MakeBooleanChecker ())
	 .AddAttribute ("UlSchedOnly",
									"Only schedule uplink traffic (for testing)",
									BooleanValue (false),
									MakeBooleanAccessor (&MmWaveFlexTtiPfMacScheduler::m_ulOnly),
									MakeBooleanChecker ())
	 .AddAttribute ("FixedTti",
									"Fix slot size",
									BooleanValue (false),
									MakeBooleanAccessor (&MmWaveFlexTtiPfMacScheduler::m_fixedTti),
									MakeBooleanChecker ())
	.AddAttribute ("SymPerSlot",
								 "Number of symbols per slot in Fixed TTI mode",
								 UintegerValue (6),
								 MakeUintegerAccessor (&MmWaveFlexTtiPfMacScheduler::m_symPerSlot),
								 MakeUintegerChecker<uint8_t> ())
		;

	return tid;
}


// IAB methods
void
MmWaveFlexTtiPfMacScheduler::SetMmWaveUeMacCschedSapProvider(MmWaveUeMacCschedSapProvider* sap)
{
	m_iabBackahulSapProvider = sap;
}

MmWaveUeMacCschedSapProvider*
MmWaveFlexTtiPfMacScheduler::GetMmWaveUeMacCschedSapProvider()
{
	return m_iabBackahulSapProvider;
}

std::string
MmWaveFlexTtiPfMacScheduler::PrintSubframeAllocationMask(std::vector<bool> mask)
{
	std::stringstream strStream;
	for(auto bit : mask)
		strStream << bit << " ";
	return strStream.str();
}

// IAB methods
void
MmWaveFlexTtiPfMacScheduler::DoIabBackhaulSchedNotify(const struct MmWaveUeMacCschedSapProvider::IabBackhaulSchedInfo& info)
{
	NS_LOG_FUNCTION(this << info.m_dciInfoElementTdma.m_rnti);

	NS_ASSERT_MSG(m_iabScheduler, "Received DCI info for backhaul on a non IAB scheduler");

	NS_LOG_DEBUG("MmWaveFlexTtiPfMacScheduler received DCIs for the backhaul link");

	// signal that the resources are busy
	NS_LOG_DEBUG("Frame " << info.m_sfnSf.m_frameNum << " subframe " << (uint16_t)info.m_sfnSf.m_sfNum <<  " slot/symbol " << (uint16_t)info.m_sfnSf.m_slotNum);

	uint32_t subframe = info.m_sfnSf.m_sfNum;
	uint32_t frame = info.m_sfnSf.m_frameNum;

	// get the SfIabAllocInfo for this subframe
	SfIabAllocInfo currentInfo = m_iabBusySubframeAllocation.at(subframe);
	SfIabAllocInfo newInfo(m_phyMacConfig->GetSymbolsPerSubframe ());

	NS_LOG_DEBUG("currentInfo frame " << currentInfo.m_sfnSf.m_frameNum << " subframe " << (uint16_t)currentInfo.m_sfnSf.m_sfNum);

	if(currentInfo.m_sfnSf.m_frameNum == frame)
	{
		// another DCI has already been registered for this subframe
		newInfo = currentInfo;
		NS_LOG_DEBUG("This frame/subframe had already a DCI stored with mask " << PrintSubframeAllocationMask(newInfo.m_symAllocationMask));
			// TODOIAB plot relevant info
			// , with m_dlSymStart " << 
			// newInfo.m_dlSymStart << " m_dlNumSymAlloc " << newInfo.m_dlNumSymAlloc << " m_ulSymStart " <<
			// newInfo.m_ulSymStart << " m_ulSymStart " << newInfo.m_ulNumSymAlloc);
	}
	else
	{
		newInfo.m_sfnSf = info.m_sfnSf;
	}

	uint32_t firstAllocatedIdx = info.m_dciInfoElementTdma.m_symStart;
	uint32_t nextFreeIdx = firstAllocatedIdx + info.m_dciInfoElementTdma.m_numSym;

	// check if it overlaps with already busy regions
	for(uint32_t index = firstAllocatedIdx; index < nextFreeIdx; index++)
	{
		NS_ASSERT_MSG(newInfo.m_symAllocationMask.at(index) == 0, "DCI signals that a symbol is scheduled for IAB, but it was already scheduled");
		newInfo.m_symAllocationMask.at(index) = 1;
	}	
	
	NS_LOG_DEBUG("Mask " << PrintSubframeAllocationMask(newInfo.m_symAllocationMask));


	// if(info.m_dciInfoElementTdma.m_format == DciInfoElementTdma::DL_dci)
	// {
	// 	// downlink DCI
	// 	if(newInfo.m_dlSymStart != 0 || newInfo.m_dlNumSymAlloc > 0)
	// 	{
	// 		NS_FATAL_ERROR("Trying to overwrite DL information on a SfIabAllocInfo");
	// 	}
	// 	newInfo.m_dlSymStart = info.m_dciInfoElementTdma.m_symStart;
	// 	newInfo.m_dlNumSymAlloc = info.m_dciInfoElementTdma.m_numSym;
	// 	NS_LOG_DEBUG("DL Num symbols used " << (uint16_t)info.m_dciInfoElementTdma.m_numSym << " from symbol "
	// 				<< (uint16_t)info.m_dciInfoElementTdma.m_symStart);
	// }
	// else
	// {
	// 	// uplink DCI
	// 	if(newInfo.m_ulSymStart != 0 || newInfo.m_ulNumSymAlloc > 0)
	// 	{
	// 		NS_FATAL_ERROR("Trying to overwrite UL information on a SfIabAllocInfo");
	// 	}
	// 	newInfo.m_ulSymStart = info.m_dciInfoElementTdma.m_symStart;
	// 	newInfo.m_ulNumSymAlloc = info.m_dciInfoElementTdma.m_numSym;
	// 	NS_LOG_DEBUG("UL Num symbols used " << (uint16_t)info.m_dciInfoElementTdma.m_numSym << " from symbol "
	// 				<< (uint16_t)info.m_dciInfoElementTdma.m_symStart);	
	// }

	m_iabBusySubframeAllocation.at(subframe) = newInfo;
}

void
MmWaveFlexTtiPfMacScheduler::SetMacSchedSapUser (MmWaveMacSchedSapUser* sap)
{
	m_macSchedSapUser = sap;
}

void
MmWaveFlexTtiPfMacScheduler::SetMacCschedSapUser (MmWaveMacCschedSapUser* sap)
{
	m_macCschedSapUser = sap;
}


MmWaveMacSchedSapProvider*
MmWaveFlexTtiPfMacScheduler::GetMacSchedSapProvider ()
{
	return m_macSchedSapProvider;
}

MmWaveMacCschedSapProvider*
MmWaveFlexTtiPfMacScheduler::GetMacCschedSapProvider ()
{
	return m_macCschedSapProvider;
}

void
MmWaveFlexTtiPfMacScheduler::SetIabScheduler(bool iabScheduler)
{
	m_iabScheduler = iabScheduler;
}

void
MmWaveFlexTtiPfMacScheduler::ConfigureCommonParameters (Ptr<MmWavePhyMacCommon> config)
{
	m_phyMacConfig = config;
	m_amc = CreateObject <MmWaveAmc> (m_phyMacConfig);
	m_numRbg = m_phyMacConfig->GetNumRb () / m_phyMacConfig->GetNumRbPerRbg ();
	m_numHarqProcess = m_phyMacConfig->GetNumHarqProcess ();
	m_harqTimeout = m_phyMacConfig->GetHarqTimeout ();
	m_numDataSymbols = m_phyMacConfig->GetSymbolsPerSubframe () -
			m_phyMacConfig->GetDlCtrlSymbols () - m_phyMacConfig->GetUlCtrlSymbols ();
	NS_ASSERT_MSG (m_phyMacConfig->GetNumRb () == 1, \
	               "System must be configured with numRb=1 for TDMA mode");

	for (unsigned i = 0; i < m_phyMacConfig->GetUlSchedDelay(); i++)
	{
		m_ulSfAllocInfo.push_back (SfAllocInfo (SfnSf (0, i, 0)));
	}

	for (unsigned i = 0; i < m_phyMacConfig->GetSubframesPerFrame(); i++)
	{
		m_iabBusySubframeAllocation.push_back(SfIabAllocInfo(SfnSf (0, i, 0), false, m_phyMacConfig->GetSymbolsPerSubframe ()));
	}
}

void
MmWaveFlexTtiPfMacScheduler::DoSchedDlRlcBufferReq (const struct MmWaveMacSchedSapProvider::SchedDlRlcBufferReqParameters& params)
{
  NS_LOG_FUNCTION (this << params.m_rnti << (uint32_t) params.m_logicalChannelIdentity);
  std::map <uint16_t, struct UeSchedInfo>::iterator itUe = m_ueSchedInfoMap.find (params.m_rnti);
  if (itUe == m_ueSchedInfoMap.end ())
  {
  	NS_LOG_ERROR ("UE entry not found in sched info map");
  }
  else
  {
  	uint8_t lcid = params.m_logicalChannelIdentity;
  	if((unsigned)lcid >= itUe->second.m_flowStatsDl.size ())
  	{
  		NS_LOG_ERROR ("LC not registered");
  	}
  	else
  	{
  		NS_LOG_DEBUG("params m_rlcTransmissionQueueSize " << params.m_rlcTransmissionQueueSize <<
  			" m_rlcRetransmissionQueueSize " << params.m_rlcRetransmissionQueueSize <<
  			" m_rlcStatusPduSize " << params.m_rlcStatusPduSize <<
  			" m_txPacketSizes " << params.m_txPacketSizes.size());
  		if (params.m_txPacketSizes.size () > 0) 

  		{
  			NS_LOG_DEBUG("Add stats to flows for UE " << params.m_rnti << " num packets " << params.m_txPacketSizes.size ());
  			itUe->second.m_flowStatsDl[lcid].m_txPacketSizes.clear ();
  			itUe->second.m_flowStatsDl[lcid].m_txPacketDelays.clear ();
  			// add the new DL PDCP packet sizes and their delays
  			NS_LOG_DEBUG("m_totalSchedSize " << itUe->second.m_flowStatsDl[lcid].m_totalSchedSize);
  			uint32_t totalSize = 0;
  			double maxDelay = 0.0;
  			std::list<uint32_t>::const_iterator itSize = params.m_txPacketSizes.begin ();
  			std::list<double>::const_iterator itDelay = params.m_txPacketDelays.begin ();
  			while (itSize != params.m_txPacketSizes.end() && itDelay != params.m_txPacketDelays.end ())
  			{
  				totalSize += *itSize;
  				if (totalSize > itUe->second.m_flowStatsDl[lcid].m_totalSchedSize)
  				{
//  					uint32_t diff = totalSize - itUe->second.m_flowStatsDl[lcid].m_totalSchedSize;
//  					if (diff > *itSize)
//  					{
//  						itUe->second.m_flowStatsDl[lcid].m_txPacketSizes.push_back (*itSize);
//  						itUe->second.m_flowStatsDl[lcid].m_totalBufSize += *itSize;
//  					}
//  					else
//  					{
//  						itUe->second.m_flowStatsDl[lcid].m_txPacketSizes.push_back (diff);
//  						itUe->second.m_flowStatsDl[lcid].m_totalBufSize += diff;
//  					}
  		  			itUe->second.m_flowStatsDl[lcid].m_totalBufSize = params.m_rlcTransmissionQueueSize;
  					itUe->second.m_flowStatsDl[lcid].m_txPacketSizes.push_back (*itSize);
  					itUe->second.m_flowStatsDl[lcid].m_txPacketDelays.push_back (*itDelay);
  					if (*itDelay > maxDelay)
  					{
  						maxDelay = *itDelay;
  					}
  				}
  				itSize++;
  				itDelay++;
  			}
			itUe->second.m_flowStatsDl[lcid].m_txQueueHolDelay = maxDelay;
  		}
  		else if (params.m_rlcTransmissionQueueSize > 0)  // case for RlcSm
  		{
  			itUe->second.m_flowStatsDl[lcid].m_totalBufSize = params.m_rlcTransmissionQueueSize;
  		}

  		if(params.m_rlcStatusPduSize > 0)
  		{
	  		NS_LOG_DEBUG("Status PDU to be sent");
			itUe->second.m_flowStatsDl[lcid].m_rlcStatusPduSize = params.m_rlcStatusPduSize;
  		}
  		
  		if(params.m_rlcRetransmissionQueueSize > 0)
  		{
  			itUe->second.m_flowStatsDl[lcid].m_totalBufSize += params.m_rlcRetransmissionQueueSize;
  		}
  	}
  }
}

// IAB functionality
uint16_t
MmWaveFlexTtiPfMacScheduler::GetNumIabRnti()
{
	uint16_t numIabDevs = 0;
	// cycle through the list of BSRs, and check which RNTIs are for IAB devs
	for(auto itRlcBuf : m_rlcBufferReq)
	{
		uint16_t rnti = itRlcBuf.m_rnti;
		NS_LOG_INFO(this << " count rnti " << rnti);
		// get IAB info
		auto iabInfoIt = m_rntiIabInfoMap.find(rnti);
		if(iabInfoIt == m_rntiIabInfoMap.end() || !iabInfoIt->second.first)
		{
			// do nothing
		}
		else
		{
			if(((itRlcBuf.m_rlcTransmissionQueueSize > 0)
					|| (itRlcBuf.m_rlcRetransmissionQueueSize > 0)
					|| (itRlcBuf.m_rlcStatusPduSize > 0)))
			{
				numIabDevs++;
			}
		}
		NS_LOG_INFO(this << " numIabDevs " << numIabDevs);
	}

	return numIabDevs;
}


void
MmWaveFlexTtiPfMacScheduler::DoSchedUlMacCtrlInfoReq (const struct MmWaveMacSchedSapProvider::SchedUlMacCtrlInfoReqParameters& params)
{
	NS_LOG_FUNCTION (this);

	std::map <uint16_t,uint32_t>::iterator it;

	for (unsigned int i = 0; i < params.m_macCeList.size (); i++)
	{
		if ( params.m_macCeList.at (i).m_macCeType == MacCeElement::BSR )
		{
			// buffer status report
			// note that this scheduler does not differentiate the
			// allocation according to which LCGs have more/less bytes
			// to send.
			// Hence the BSR of different LCGs are just summed up to get
			// a total queue size that is used for allocation purposes.
			uint16_t rnti = params.m_macCeList.at (i).m_rnti;
  			std::map <uint16_t, struct UeSchedInfo>::iterator itUe = m_ueSchedInfoMap.find (rnti);

			uint32_t buffer = 0;
			for (uint8_t lcg = 1; lcg <= 3; ++lcg)
			{
				uint8_t bsrId = params.m_macCeList.at (i).m_macCeValue.m_bufferStatus.at (lcg-1);
				uint32_t bufSize = BsrId2BufferSize (bsrId);
				NS_LOG_DEBUG("UL BSR for lcid " << (uint16_t)lcg << " with size " << bufSize);
				if (bufSize > 0)
				{
					buffer += bufSize;

					if (itUe == m_ueSchedInfoMap.end ())
					{
						NS_LOG_ERROR ("UE entry not found in sched info map");
					}
					else
					{
						int diff = bufSize - (itUe->second.m_flowStatsUl[lcg].m_totalBufSize + itUe->second.m_flowStatsUl[lcg].m_totalSchedSize);
						NS_LOG_DEBUG("diff " << diff);
						if (diff > 0)
						{	// estimate additional packet sizes
							itUe->second.m_flowStatsUl[lcg].m_totalBufSize += diff;
							itUe->second.m_flowStatsUl[lcg].m_txPacketSizes.push_back (diff);
							// since we expect the BSR to be generated following a packet arrival and sent at least by the end of the prev. subframe,
							// the maximum delay is one SF (in microseconds)
							itUe->second.m_flowStatsUl[lcg].m_txPacketDelays.push_back (m_phyMacConfig->GetSubframePeriod ());
							if (itUe->second.m_flowStatsUl[lcg].m_txQueueHolDelay == 0)
							{
								itUe->second.m_flowStatsUl[lcg].m_txQueueHolDelay = m_phyMacConfig->GetSubframePeriod ();
							}
							NS_LOG_DEBUG("itUe->second.m_flowStatsUl[lcg] " << itUe->second.m_flowStatsUl[lcg].m_totalBufSize);
						}
					}
				}
			}
		}
	}

	return;
}

void
MmWaveFlexTtiPfMacScheduler::DoSchedDlCqiInfoReq (const struct MmWaveMacSchedSapProvider::SchedDlCqiInfoReqParameters& params)
{
  NS_LOG_FUNCTION (this);

  std::map <uint16_t,uint8_t>::iterator it;
  for (unsigned int i = 0; i < params.m_cqiList.size (); i++)
    {
      if ( params.m_cqiList.at (i).m_cqiType == DlCqiInfo::WB )
        {
          // wideband CQI reporting
          std::map <uint16_t,uint8_t>::iterator it;
          uint16_t rnti = params.m_cqiList.at (i).m_rnti;
          it = m_wbCqiRxed.find (rnti);
          if (it == m_wbCqiRxed.end ())
            {
              // create the new entry
              m_wbCqiRxed.insert ( std::pair<uint16_t, uint8_t > (rnti, params.m_cqiList.at (i).m_wbCqi) ); // only codeword 0 at this stage (SISO)
              // generate correspondent timer
              m_wbCqiTimers.insert ( std::pair<uint16_t, uint32_t > (rnti, m_cqiTimersThreshold));
            }
          else
            {
              // update the CQI value
              (*it).second = params.m_cqiList.at (i).m_wbCqi;
              // update correspondent timer
              std::map <uint16_t,uint32_t>::iterator itTimers;
              itTimers = m_wbCqiTimers.find (rnti);
              (*itTimers).second = m_cqiTimersThreshold;
            }
        }
      else if ( params.m_cqiList.at (i).m_cqiType == DlCqiInfo::SB )
        {
          // subband CQI reporting high layer configured
          // Not used by RR Scheduler
        }
      else
        {
          NS_LOG_ERROR (this << " CQI type unknown");
        }
    }
}


void
MmWaveFlexTtiPfMacScheduler::DoSchedUlCqiInfoReq (const struct MmWaveMacSchedSapProvider::SchedUlCqiInfoReqParameters& params)
{
  NS_LOG_FUNCTION (this);

	unsigned frameNum = params.m_sfnSf.m_frameNum;
	unsigned subframeNum =  params.m_sfnSf.m_sfNum;
	unsigned startSymIdx =  params.m_sfnSf.m_slotNum;

	switch (params.m_ulCqi.m_type)
	{
		case UlCqiInfo::PUSCH:
		{
			std::map <uint32_t, struct AllocMapElem>::iterator itMap;
			std::map <uint16_t, struct UlCqiMapElem>::iterator itCqi;
			itMap = m_ulAllocationMap.find (params.m_sfnSf.Encode ());
			if (itMap == m_ulAllocationMap.end ())
			{
				NS_LOG_INFO (this << " Does not find info on allocation, size : " << m_ulAllocationMap.size ());
				return;
			}
			NS_ASSERT_MSG (itMap->second.m_rntiPerChunk.size () == m_phyMacConfig->GetTotalNumChunk (), "SINR chunk map must cover full BW in TDMA mode");
			for (unsigned i = 0; i < itMap->second.m_rntiPerChunk.size (); i++)
			{
				// convert from fixed point notation Sxxxxxxxxxxx.xxx to double
				//double sinr = LteFfConverter::fpS11dot3toDouble (params.m_ulCqi.m_sinr.at (i));
				itCqi = m_ueUlCqi.find (itMap->second.m_rntiPerChunk.at (i));
				if (itCqi == m_ueUlCqi.end ())
				{
					// create a new entry
					std::vector <double> newCqi;
					for (unsigned j = 0; j < m_phyMacConfig->GetTotalNumChunk (); j++)
					{
						unsigned chunkInd = i;
						if (chunkInd == j)
						{
							newCqi.push_back (params.m_ulCqi.m_sinr.at (i));
							NS_LOG_INFO ("UL CQI report for RNTI " << itMap->second.m_rntiPerChunk.at (i) << " chunk " << i << " SINR " << params.m_ulCqi.m_sinr.at (i) << \
							             " frame " << frameNum << " subframe " << subframeNum << " startSym " << startSymIdx);
						}
						else
						{
							// initialize with NO_SINR value.
							newCqi.push_back (30.0);
						}
					}
					m_ueUlCqi.insert (std::pair <uint16_t, struct UlCqiMapElem> (itMap->second.m_rntiPerChunk.at (i),
					                                                             UlCqiMapElem (newCqi, itMap->second.m_numSym, itMap->second.m_tbSize)) );
					// generate correspondent timer
					m_ueCqiTimers.insert (std::pair <uint16_t, uint32_t > (itMap->second.m_rntiPerChunk.at (i), m_cqiTimersThreshold));
				}
				else
				{
					// update the value
					(*itCqi).second.m_ueUlCqi.at (i) = params.m_ulCqi.m_sinr.at (i);
					(*itCqi).second.m_numSym = itMap->second.m_numSym;
					(*itCqi).second.m_tbSize = itMap->second.m_tbSize;
					// update correspondent timer
					std::map <uint16_t, uint32_t>::iterator itTimers;
					itTimers = m_ueCqiTimers.find (itMap->second.m_rntiPerChunk.at (i));
					(*itTimers).second = m_cqiTimersThreshold;

					NS_LOG_INFO ("UL CQI report for RNTI " << itMap->second.m_rntiPerChunk.at (i) << " chunk " << i << " SINR " << params.m_ulCqi.m_sinr.at (i) << \
					             " frame " << frameNum << " subframe " << subframeNum << " startSym " << startSymIdx);

				}

			}
			// remove obsolete info on allocation
			m_ulAllocationMap.erase (itMap);
		}
		break;
		default:
			NS_FATAL_ERROR ("Unknown type of UL-CQI");
	}
	return;
}

void
MmWaveFlexTtiPfMacScheduler::DoSchedSetMcs (int mcs)
{
	if (mcs >= 0 && mcs <= 28)
	{
		m_mcsDefaultDl = mcs;
		m_mcsDefaultUl = mcs;
	}
}

void
MmWaveFlexTtiPfMacScheduler::RefreshHarqProcesses ()
{
	NS_LOG_FUNCTION (this);

	std::map <uint16_t, DlHarqProcessesTimer_t>::iterator itTimers;
	for (itTimers = m_dlHarqProcessesTimer.begin (); itTimers != m_dlHarqProcessesTimer.end (); itTimers++)
	{
		for (uint16_t i = 0; i < m_phyMacConfig->GetNumHarqProcess (); i++)
		{
			if ((*itTimers).second.at (i) == m_phyMacConfig->GetHarqTimeout ())
			{ // reset HARQ process
				NS_LOG_DEBUG (this << " Reset HARQ proc " << i << " for RNTI " << (*itTimers).first);
				std::map <uint16_t, DlHarqProcessesStatus_t>::iterator itStat = m_dlHarqProcessesStatus.find ((*itTimers).first);
				if (itStat == m_dlHarqProcessesStatus.end ())
				{
					NS_FATAL_ERROR ("No Process Id Status found for this RNTI " << (*itTimers).first);
				}
				(*itStat).second.at (i) = 0;
				(*itTimers).second.at (i) = 0;
			}
			else
			{
				(*itTimers).second.at (i)++;
			}
		}
	}

	std::map <uint16_t, UlHarqProcessesTimer_t>::iterator itTimers2;
	for (itTimers2 = m_ulHarqProcessesTimer.begin (); itTimers2 != m_ulHarqProcessesTimer.end (); itTimers2++)
	{
		for (uint16_t i = 0; i < m_phyMacConfig->GetNumHarqProcess (); i++)
		{
			if ((*itTimers2).second.at (i) == m_phyMacConfig->GetHarqTimeout ())
			{ // reset HARQ process
				NS_LOG_INFO (this << " Reset HARQ proc " << i << " for RNTI " << (*itTimers2).first);
				std::map <uint16_t, UlHarqProcessesStatus_t>::iterator itStat = m_ulHarqProcessesStatus.find ((*itTimers2).first);
				if (itStat == m_ulHarqProcessesStatus.end ())
				{
					NS_FATAL_ERROR ("No Process Id Status found for this RNTI " << (*itTimers2).first);
				}
				(*itStat).second.at (i) = 0;
				(*itTimers2).second.at (i) = 0;
			}
			else
			{
				(*itTimers2).second.at (i)++;
			}
		}
	}

}

uint8_t
MmWaveFlexTtiPfMacScheduler::UpdateDlHarqProcessId (uint16_t rnti)
{
	NS_LOG_FUNCTION (this << rnti);


	if (m_harqOn == false)
	{
		uint8_t tbUid = m_tbUid;
		m_tbUid = (m_tbUid+1) % m_phyMacConfig->GetNumHarqProcess ();
		return tbUid;
	}

//	std::map <uint16_t, uint8_t>::iterator it = m_dlHarqCurrentProcessId.find (rnti);
//	if (it == m_dlHarqCurrentProcessId.end ())
//	{
//		NS_FATAL_ERROR ("No Process Id found for this RNTI " << rnti);
//	}
	std::map <uint16_t, DlHarqProcessesStatus_t>::iterator itStat = m_dlHarqProcessesStatus.find (rnti);
	if (itStat == m_dlHarqProcessesStatus.end ())
	{
		NS_FATAL_ERROR ("No Process Id Statusfound for this RNTI " << rnti);
	}

	// search for available process ID, if none available return numHarqProcess
	uint8_t harqId = m_phyMacConfig->GetNumHarqProcess ();
	for (unsigned i = 0; i < m_phyMacConfig->GetNumHarqProcess (); i++)
	{
		if(itStat->second[i] == 0)
		{
			itStat->second[i] = 1;
			harqId = i;
			break;
		}
	}
	return harqId;

//	uint8_t i = (*it).second;
//	do
//	{
//		i = (i + 1) % m_phyMacConfig->GetNumHarqProcess ();
//	}
//	while ( ((*itStat).second.at (i) != 0)&&(i != (*it).second));
//	if ((*itStat).second.at (i) == 0)
//	{
//		(*it).second = i;
//		(*itStat).second.at (i) = 1;
//	}
//	else
//	{
//		return (m_phyMacConfig->GetNumHarqProcess () + 1); // return a not valid harq proc id
//	}
//
//	return ((*it).second);
}

uint8_t
MmWaveFlexTtiPfMacScheduler::UpdateUlHarqProcessId (uint16_t rnti)
{
	NS_LOG_FUNCTION (this << rnti);

	if (m_harqOn == false)
	{
		uint8_t tbUid = m_tbUid;
		m_tbUid = (m_tbUid+1) % m_phyMacConfig->GetNumHarqProcess ();
		return tbUid;
	}

//	std::map <uint16_t, uint8_t>::iterator it = m_ulHarqCurrentProcessId.find (rnti);
//	if (it == m_ulHarqCurrentProcessId.end ())
//	{
//		NS_FATAL_ERROR ("No Process Id found for this RNTI " << rnti);
//	}
	std::map <uint16_t, DlHarqProcessesStatus_t>::iterator itStat = m_ulHarqProcessesStatus.find (rnti);
	if (itStat == m_ulHarqProcessesStatus.end ())
	{
		NS_FATAL_ERROR ("No Process Id Statusfound for this RNTI " << rnti);
	}

	// search for available process ID, if none available return numHarqProcess+1
	uint8_t harqId = m_phyMacConfig->GetNumHarqProcess ();
	for (unsigned i = 0; i < m_phyMacConfig->GetNumHarqProcess (); i++)
	{
		if(itStat->second[i] == 0)
		{
			itStat->second[i] = 1;
			harqId = i;
			break;
		}
	}
	return harqId;
//	uint8_t i = (*it).second;
//	do
//	{
//		i = (i + 1) % m_phyMacConfig->GetNumHarqProcess ();
//	}
//	while ( ((*itStat).second.at (i) != 0)&&(i != (*it).second));
//	if ((*itStat).second.at (i) == 0)
//	{
//		(*it).second = i;
//		(*itStat).second.at (i) = 1;
//	}
//	else
//	{
//		return (m_phyMacConfig->GetNumHarqProcess () + 1); // return a not valid harq proc id
//	}
//	return ((*it).second);
}

unsigned MmWaveFlexTtiPfMacScheduler::CalcMinTbSizeNumSym (unsigned mcs, unsigned bufSize, unsigned &tbSize)
{
	// bisection line search to find minimum number of slots needed to encode entire buffer
	MmWaveMacPduHeader dummyMacHeader;
	//unsigned macHdrSize = 10; //dummyMacHeader.GetSerializedSize ();
	int numSymLow = 0;
	int numSymHigh = m_phyMacConfig->GetSymbolsPerSubframe();

	int diff = 0;
	tbSize = (m_amc->GetTbSizeFromMcsSymbols (mcs, numSymHigh) / 8); // start with max value
	while ((unsigned)tbSize > bufSize)
	{
		diff = abs(numSymHigh-numSymLow)/2;
		if (diff == 0)
		{
			tbSize = (m_amc->GetTbSizeFromMcsSymbols (mcs, numSymHigh) / 8);
			return numSymHigh;
		}
		tbSize = (m_amc->GetTbSizeFromMcsSymbols (mcs, numSymHigh - diff) / 8);
		if ((unsigned)tbSize > bufSize)
		{
			numSymHigh -= diff;
		}
		while ((unsigned)tbSize < bufSize)
		{
			diff = abs(numSymHigh-numSymLow)/2;
			if (diff == 0)
			{
				tbSize = (m_amc->GetTbSizeFromMcsSymbols (mcs, numSymHigh) / 8);
				return numSymHigh;
			}
			//tmp2 = numSym;
			tbSize = (m_amc->GetTbSizeFromMcsSymbols (mcs, numSymLow + diff) / 8);
			if ((unsigned)tbSize < bufSize)
			{
				numSymLow += diff;
			}
		}
	}

	tbSize = (m_amc->GetTbSizeFromMcsSymbols (mcs, numSymHigh) / 8);
	return (unsigned)numSymHigh;
}


int
MmWaveFlexTtiPfMacScheduler::UpdateBusySymbolsForIab(uint8_t sfNum, uint8_t symIdx, int symAvail)
{
	// get the resources which are already set as busy for this subframe
	if(m_iabScheduler && !m_split)
	{
		SfIabAllocInfo busyResources = m_iabBusySubframeAllocation.at(sfNum);
		NS_LOG_DEBUG("Before check for IAB resources: symIdx " << (uint16_t)symIdx << " symAvail " << symAvail);
		if(busyResources.m_valid)
		{
			busyResources.m_valid = false; // signal that this SfIabAllocInfo has been used for a prev slot
			
			NS_LOG_DEBUG("This subframe has a DCI stored, with " <<
			" frame " << busyResources.m_sfnSf.m_frameNum << 
			" subframe " << (uint16_t)busyResources.m_sfnSf.m_sfNum <<
			PrintSubframeAllocationMask(busyResources.m_symAllocationMask));

			// " m_dlSymStart " << busyResources.m_dlSymStart << 
			// " m_dlNumSymAlloc " << busyResources.m_dlNumSymAlloc << 
			// " m_ulSymStart " << busyResources.m_ulSymStart << 
			// " m_ulSymStart " << busyResources.m_ulNumSymAlloc);	
			
			// NS_ASSERT_MSG(busyResources.m_sfnSf.m_sfNum == sfNum && 
			// 		busyResources.m_sfnSf.m_frameNum == frameNum,
			// 		"Mismatch between scheduled resource and SfIabAllocInfo");

			// TODOIAB -> do smth better, since UL resources may be allocated after DL/UL
			// resources of other users, so some previous symbols can be used

			// get the max between DL and UL sym (usually UL if both present)
			// if(busyResources.m_dlSymStart > busyResources.m_ulSymStart)
			// {
			// 	// set the symIdx value to the first free available index
			// 	symIdx = busyResources.m_dlSymStart + busyResources.m_dlNumSymAlloc;
			// 	// subtract the DL and UL symbols from the available ones
			// 	// symAvail -= (busyResources.m_dlNumSymAlloc + busyResources.m_ulNumSymAlloc);
			// }
			// else
			// {
			// 	// set the symIdx value to the first free available index
			// 	symIdx = busyResources.m_ulSymStart + busyResources.m_ulNumSymAlloc;
			// 	// subtract the DL and UL symbols from the available ones
			// 	// symAvail -= (busyResources.m_dlNumSymAlloc + busyResources.m_ulNumSymAlloc);

			// }
			// symAvail = m_phyMacConfig->GetSymbolsPerSubframe () - 1 - symIdx;
		}
		m_iabBusySubframeAllocation.at(sfNum) = busyResources;

		NS_LOG_DEBUG("After check for IAB resources: symIdx " << (uint16_t)symIdx << " symAvail " << symAvail);
	}
	else if(m_iabScheduler && m_split)
	{
		NS_LOG_LOGIC("Before check for IAB resources: symIdx " << (uint16_t)symIdx << " symAvail " << symAvail);
		SfIabAllocInfo busyResources = m_iabBusySubframeAllocation.at(sfNum);
		if(busyResources.m_valid)
		{
			m_busyResourcesSchedSubframe = busyResources;

			// count the number of symbols which are busy
			int numBusySymbols = 0;
			for(auto symbolIter : m_busyResourcesSchedSubframe.m_symAllocationMask)
			{
				numBusySymbols += symbolIter;
			}

			symAvail -= numBusySymbols;

			NS_LOG_DEBUG("This subframe has a DCI stored, with " <<
			" frame " << busyResources.m_sfnSf.m_frameNum << 
			" subframe " << (uint16_t)busyResources.m_sfnSf.m_sfNum << " " <<
			PrintSubframeAllocationMask(busyResources.m_symAllocationMask) << 
			" total number of busy symbols " << numBusySymbols <<
			" symAvail " << symAvail);

		}
		else
		{
			m_busyResourcesSchedSubframe.m_valid = false;
		}
		m_iabBusySubframeAllocation.at(sfNum).m_valid = false;
	}

	return symAvail;
}

void
MmWaveFlexTtiPfMacScheduler::DoSchedTriggerReq (const struct MmWaveMacSchedSapProvider::SchedTriggerReqParameters& params)
{
	//uint8_t slotNum = params.m_sfnSf.m_slotNum;
	NS_LOG_DEBUG("m_rntiIabInfoMap size " << m_rntiIabInfoMap.size());

	MmWaveMacSchedSapUser::SchedConfigIndParameters ret;
	SfnSf dlSfn = params.m_sfnSf;
	SfnSf ulSfn = params.m_sfnSf;

	// downlink
	if ((int)(dlSfn.m_sfNum + m_maxSchedulingDelay) >=  (int)m_phyMacConfig->GetSubframesPerFrame ())
	{
		dlSfn.m_frameNum++;
	}
	dlSfn.m_sfNum = (uint8_t)((int)(dlSfn.m_sfNum + m_maxSchedulingDelay)) % m_phyMacConfig->GetSubframesPerFrame ();
	ret.m_sfnSf = dlSfn;
	ret.m_sfAllocInfo.m_sfnSf = ret.m_sfnSf;

	// uplink
	if (ulSfn.m_sfNum + m_maxSchedulingDelay >=  m_phyMacConfig->GetSubframesPerFrame ())
	{
		ulSfn.m_frameNum++;
	}
	ulSfn.m_sfNum = (ulSfn.m_sfNum + m_maxSchedulingDelay) % m_phyMacConfig->GetSubframesPerFrame ();

	uint16_t frameNum = ret.m_sfnSf.m_frameNum;
	uint8_t	sfNum = ret.m_sfnSf.m_sfNum;
	//uint8_t slotNum = params.m_sfnSf.m_slotNum;

	NS_LOG_DEBUG ("Scheduling DL frame "<< (unsigned)frameNum << " subframe " << (unsigned)sfNum
								<< " UL frame " << (unsigned)ulSfn.m_frameNum << " subframe " << (unsigned)ulSfn.m_sfNum);
	//ret.m_ulSfAllocInfo = SfAllocInfo (ulSfn);

	// IAB in this version we only consider the same slot for UL & DL
	NS_ASSERT_MSG(sfNum == ulSfn.m_sfNum, "different subframes for DL and UL"); 

	// IAB find the number of IAB devs that have requested resources via BSRs
	// uint16_t numIabDevs = GetNumIabRnti();
	// NS_LOG_DEBUG(this << " numIabDevs " << numIabDevs);

	// add slot for DL control
	SlotAllocInfo dlCtrlSlot (0, SlotAllocInfo::DL_slotAllocInfo, SlotAllocInfo::CTRL, SlotAllocInfo::DIGITAL, 0);
	dlCtrlSlot.m_dci.m_numSym = 1;
	dlCtrlSlot.m_dci.m_symStart = 0;
	ret.m_sfAllocInfo.m_slotAllocInfo.push_back (dlCtrlSlot);

	int symAvail = m_phyMacConfig->GetSymbolsPerSubframe () - m_phyMacConfig->GetDlCtrlSymbols() - m_phyMacConfig->GetUlCtrlSymbols();
	uint8_t slotIdx = 1; // index used to store SlotAllocInfo
	uint8_t symIdx = m_phyMacConfig->GetDlCtrlSymbols(); // symbols reserved for control at beginning of subframe

	// get the resources which are already set as busy
	symAvail = UpdateBusySymbolsForIab(sfNum, symIdx, symAvail);

	// process received CQIs
	RefreshDlCqiMaps ();
	RefreshUlCqiMaps ();

	// Process DL HARQ feedback
	RefreshHarqProcesses ();

	if ((int)symIdx >= (int)(m_phyMacConfig->GetSymbolsPerSubframe () - m_phyMacConfig->GetUlCtrlSymbols ()))
	{
		// add slot for UL control
		SlotAllocInfo ulCtrlSlot (0xFF, SlotAllocInfo::UL_slotAllocInfo, SlotAllocInfo::CTRL, SlotAllocInfo::DIGITAL, 0);
		ulCtrlSlot.m_dci.m_numSym = 1;
		ulCtrlSlot.m_dci.m_symStart = m_phyMacConfig->GetSymbolsPerSubframe()-1;
		ret.m_sfAllocInfo.m_slotAllocInfo.push_back (ulCtrlSlot);
		m_macSchedSapUser->SchedConfigInd (ret);
		return;
	}

	//m_rlcBufferReq.sort (SortRlcBufferReq); 	// sort list by RNTI
	// number of DL/UL flows for new transmissions (not HARQ RETX)
	std::map <uint16_t, UeSchedInfo*> ueAllocMap;		// map of allocated users for this SF
	std::map <uint16_t, UeSchedInfo*>::iterator itUeAllocMap;
	std::map <uint16_t, UeSchedInfo>::iterator itUeSchedInfoMap;
	std::map <uint8_t, FlowStats>::iterator itFlow;
	std::list<MmWaveMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator itRlcBuf;

	// retrieve past HARQ retx buffered
	if (m_dlHarqInfoList.size () > 0 && params.m_dlHarqInfoList.size () > 0)
	{
		m_dlHarqInfoList.insert (m_dlHarqInfoList.end (), params.m_dlHarqInfoList.begin (), params.m_dlHarqInfoList.end ());
	}
	else if (params.m_dlHarqInfoList.size () > 0)
	{
		m_dlHarqInfoList = params.m_dlHarqInfoList;
	}
	if (m_ulHarqInfoList.size () > 0 && params.m_ulHarqInfoList.size () > 0)
	{
		m_ulHarqInfoList.insert (m_ulHarqInfoList.end (), params.m_ulHarqInfoList.begin (), params.m_ulHarqInfoList.end ());
	}
	else if (params.m_ulHarqInfoList.size () > 0)
	{
		m_ulHarqInfoList = params.m_ulHarqInfoList;
	}

	if (m_harqOn == false)		// Ignore HARQ feedback
	{
		m_dlHarqInfoList.clear ();
	}
	else
	{
		// Process DL HARQ feedback and assign slots for RETX if resources available
		std::vector <struct DlHarqInfo> dlInfoListUntxed;  // TBs not able to be retransmitted in this sf
		std::vector <struct UlHarqInfo> ulInfoListUntxed;

		for (unsigned i = 0; i < m_dlHarqInfoList.size (); i++)
		{
			if (symAvail == 0)
			{
				break;	// no symbols left to allocate
			}
			uint8_t harqId = m_dlHarqInfoList.at (i).m_harqProcessId;
			uint16_t rnti = m_dlHarqInfoList.at (i).m_rnti;
			itUeSchedInfoMap = m_ueSchedInfoMap.find (rnti);
			NS_ASSERT (itUeSchedInfoMap != m_ueSchedInfoMap.end ());
			std::map <uint16_t, UlHarqProcessesStatus_t>::iterator itStat = m_dlHarqProcessesStatus.find (rnti);
			if (itStat == m_dlHarqProcessesStatus.end ())
			{
				NS_FATAL_ERROR ("No HARQ status info found for UE " << rnti);
			}
			std::map <uint16_t, DlHarqRlcPduList_t>::iterator itRlcPdu =  m_dlHarqProcessesRlcPduMap.find (rnti);
			if (itRlcPdu == m_dlHarqProcessesRlcPduMap.end ())
			{
				NS_FATAL_ERROR ("Unable to find RlcPdcList in HARQ buffer for RNTI " << m_dlHarqInfoList.at (i).m_rnti);
			}
			if(m_dlHarqInfoList.at (i).m_harqStatus == DlHarqInfo::ACK || itStat->second.at (harqId) == 0)
			{ // acknowledgment or process timeout, reset process
				//NS_LOG_DEBUG ("UE" << rnti << " DL harqId " << (unsigned)harqId << " HARQ-ACK received");
				itStat->second.at (harqId) = 0;    // release process ID
				for (uint16_t k = 0; k < itRlcPdu->second.size (); k++)		// clear RLC buffers
				{
					itRlcPdu->second.at (harqId).clear ();
				}
				continue;
			}
			else if(m_dlHarqInfoList.at (i).m_harqStatus == DlHarqInfo::NACK)
			{
				std::map <uint16_t, DlHarqProcessesDciInfoList_t>::iterator itHarq = m_dlHarqProcessesDciInfoMap.find (rnti);
				if (itHarq == m_dlHarqProcessesDciInfoMap.end ())
				{
					NS_FATAL_ERROR ("No DCI/HARQ buffer entry found for UE " << rnti);
				}
				DciInfoElementTdma dciInfoReTx = itHarq->second.at (harqId);
				//NS_LOG_DEBUG ("UE" << rnti << " DL harqId " << (unsigned)harqId << " HARQ-NACK received, rv " << (unsigned)dciInfoReTx.m_rv);
				NS_ASSERT (harqId == dciInfoReTx.m_harqProcess);
				//NS_ASSERT(itStat->second.at (harqId) > 0);
				NS_ASSERT(itStat->second.at (harqId)-1 == dciInfoReTx.m_rv);
				if (dciInfoReTx.m_rv == 3) // maximum number of retx reached -> drop process
				{
					NS_LOG_INFO ("Max number of retransmissions reached -> drop process");
					itStat->second.at (harqId) = 0;
					for (uint16_t k = 0; k < (*itRlcPdu).second.size (); k++)
					{
						itRlcPdu->second.at (harqId).clear ();
					}
					continue;
				}
				// allocate retx if enough symbols are available
				if (symAvail >= dciInfoReTx.m_numSym)
				{
					if(!CheckOverlapWithBusyResources(symIdx, dciInfoReTx.m_numSym)) // TODOIAB: check if it can be allocated later)
					{
						symAvail -= dciInfoReTx.m_numSym;
						dciInfoReTx.m_symStart = symIdx;
						symIdx += dciInfoReTx.m_numSym;
						NS_ASSERT (symIdx <= m_phyMacConfig->GetSymbolsPerSubframe () - m_phyMacConfig->GetUlCtrlSymbols ());
						dciInfoReTx.m_rv++;
						dciInfoReTx.m_ndi = 0;
						itHarq->second.at (harqId) = dciInfoReTx;
						itStat->second.at (harqId) = itStat->second.at (harqId) + 1;
						SlotAllocInfo slotInfo (slotIdx++, SlotAllocInfo::DL_slotAllocInfo, SlotAllocInfo::CTRL_DATA, SlotAllocInfo::DIGITAL, rnti);
						slotInfo.m_dci = dciInfoReTx;
						NS_LOG_DEBUG ("UE" << dciInfoReTx.m_rnti << " gets DL slots " << (unsigned)dciInfoReTx.m_symStart << "-" << (unsigned)(dciInfoReTx.m_symStart+dciInfoReTx.m_numSym-1) <<
													" tbs " << dciInfoReTx.m_tbSize << " harqId " << (unsigned)dciInfoReTx.m_harqProcess << " harqId " << (unsigned)dciInfoReTx.m_harqProcess <<
													" rv " << (unsigned)dciInfoReTx.m_rv << " in frame " << ret.m_sfnSf.m_frameNum << " subframe " << (unsigned)ret.m_sfnSf.m_sfNum << " RETX");
						std::map <uint16_t, DlHarqRlcPduList_t>::iterator itRlcList =  m_dlHarqProcessesRlcPduMap.find (rnti);
						if (itRlcList == m_dlHarqProcessesRlcPduMap.end ())
						{
							NS_FATAL_ERROR ("Unable to find RlcPdcList in HARQ buffer for RNTI " << rnti);
						}
						for (uint16_t k = 0; k < (*itRlcList).second.at(dciInfoReTx.m_harqProcess).size (); k++)
						{
							slotInfo.m_rlcPduInfo.push_back ((*itRlcList).second.at (dciInfoReTx.m_harqProcess).at (k));
						}
						ret.m_sfAllocInfo.m_slotAllocInfo.push_back (slotInfo);
						ret.m_sfAllocInfo.m_numSymAlloc += dciInfoReTx.m_numSym;

						itUeSchedInfoMap->second.m_dlSymbolsRetx = dciInfoReTx.m_numSym;
						itUeAllocMap = ueAllocMap.find (rnti);
						if (itUeAllocMap == ueAllocMap.end())
						{
							itUeAllocMap = ueAllocMap.insert (std::pair<uint16_t, UeSchedInfo*> (rnti, &itUeSchedInfoMap->second)).first;
						}
					}
					else
					{
						// find if there are other resources later
						// we cannot split RETX!
						int numSymNeeded = dciInfoReTx.m_numSym;
						int numFreeSymbols = 0;
						uint8_t tmpSymIdx = symIdx;

						while(numFreeSymbols == 0 && tmpSymIdx < m_phyMacConfig->GetSymbolsPerSubframe()-1) 
						{
							numFreeSymbols = GetNumFreeSymbols(tmpSymIdx, numSymNeeded); // this is equal to numSymNeeded if there is no overlap, otherwise smaller
							NS_LOG_LOGIC("RETX numSymNeeded " << numSymNeeded << " numFreeSymbols " 
								<< numFreeSymbols << " tmpSymIdx " << (uint16_t)tmpSymIdx);
							if(numFreeSymbols < numSymNeeded)
							{
								numFreeSymbols = 0;
								tmpSymIdx = GetFirstFreeSymbol(tmpSymIdx, numSymNeeded); // get the next symIdx
								if((int)(tmpSymIdx + numSymNeeded) > (int)m_phyMacConfig->GetSymbolsPerSubframe()-1)
								{
									NS_LOG_DEBUG("No way to fit this retx in the available resources");
									break;
								}
							}
						}

						NS_LOG_LOGIC("RETX numSymNeeded " << numSymNeeded << " numFreeSymbols " 
							<< numFreeSymbols << " tmpSymIdx " << (uint16_t)tmpSymIdx);
						if(numFreeSymbols >= numSymNeeded && (int)(tmpSymIdx + numSymNeeded) < (int)(m_phyMacConfig->GetSymbolsPerSubframe () - m_phyMacConfig->GetUlCtrlSymbols ()))
						{
							NS_LOG_LOGIC("Found resources for DL HARQ RETX");
							symAvail -= dciInfoReTx.m_numSym;
							dciInfoReTx.m_symStart = tmpSymIdx;

							NS_LOG_LOGIC("Before updating " << PrintSubframeAllocationMask(m_busyResourcesSchedSubframe.m_symAllocationMask));

							// update resource mask
							UpdateResourceMask(tmpSymIdx, dciInfoReTx.m_numSym);

							NS_LOG_LOGIC("After updating " << PrintSubframeAllocationMask(m_busyResourcesSchedSubframe.m_symAllocationMask));

							// symIdx += dciInfoReTx.m_numSym;
							NS_ASSERT (tmpSymIdx + dciInfoReTx.m_numSym < m_phyMacConfig->GetSymbolsPerSubframe () - m_phyMacConfig->GetUlCtrlSymbols ());
							dciInfoReTx.m_rv++;
							dciInfoReTx.m_ndi = 0;
							itHarq->second.at (harqId) = dciInfoReTx;
							itStat->second.at (harqId) = itStat->second.at (harqId) + 1;
							SlotAllocInfo slotInfo (slotIdx++, SlotAllocInfo::DL_slotAllocInfo, SlotAllocInfo::CTRL_DATA, SlotAllocInfo::DIGITAL,  dciInfoReTx.m_rnti);
							slotInfo.m_dci = dciInfoReTx;
							NS_LOG_DEBUG ("UE" << dciInfoReTx.m_rnti << " gets DL slots " << (unsigned)dciInfoReTx.m_symStart << "-" << (unsigned)(dciInfoReTx.m_symStart+dciInfoReTx.m_numSym-1) <<
									             " tbs " << dciInfoReTx.m_tbSize << " harqId " << (unsigned)dciInfoReTx.m_harqProcess << " harqId " << (unsigned)dciInfoReTx.m_harqProcess <<
									             " rv " << (unsigned)dciInfoReTx.m_rv << " in frame " << ret.m_sfnSf.m_frameNum << " subframe " << (unsigned)ret.m_sfnSf.m_sfNum << " RETX");
							std::map <uint16_t, DlHarqRlcPduList_t>::iterator itRlcList =  m_dlHarqProcessesRlcPduMap.find (rnti);
							if (itRlcList == m_dlHarqProcessesRlcPduMap.end ())
							{
								NS_FATAL_ERROR ("Unable to find RlcPdcList in HARQ buffer for RNTI " << rnti);
							}
							for (uint16_t k = 0; k < (*itRlcList).second.at(dciInfoReTx.m_harqProcess).size (); k++)
							{
								slotInfo.m_rlcPduInfo.push_back ((*itRlcList).second.at (dciInfoReTx.m_harqProcess).at (k));
							}
							ret.m_sfAllocInfo.m_slotAllocInfo.push_back (slotInfo);
							ret.m_sfAllocInfo.m_numSymAlloc += dciInfoReTx.m_numSym;

							itUeSchedInfoMap->second.m_dlSymbolsRetx = dciInfoReTx.m_numSym;
							itUeAllocMap = ueAllocMap.find (rnti);
							if (itUeAllocMap == ueAllocMap.end())
							{
								itUeAllocMap = ueAllocMap.insert (std::pair<uint16_t, UeSchedInfo*> (rnti, &itUeSchedInfoMap->second)).first;
							}
						}
						else
						{
							NS_LOG_DEBUG ("No resource for this retx (even later) -> buffer it");
							dlInfoListUntxed.push_back (m_dlHarqInfoList.at (i));
						}
					}
				}
				else
				{
					NS_LOG_INFO ("No resource for this retx -> buffer it");
					dlInfoListUntxed.push_back (m_dlHarqInfoList.at (i));
				}
			}
		}

		m_dlHarqInfoList.clear ();
		m_dlHarqInfoList = dlInfoListUntxed;

		// Process UL HARQ feedback
		for (uint16_t i = 0; i < m_ulHarqInfoList.size (); i++)
		{
			if (symAvail == 0)
			{
				break;	// no symbols left to allocate
			}
			UlHarqInfo harqInfo = m_ulHarqInfoList.at (i);
			uint8_t harqId = harqInfo.m_harqProcessId;
			uint16_t rnti = harqInfo.m_rnti;
			itUeSchedInfoMap = m_ueSchedInfoMap.find (rnti);
			NS_ASSERT (itUeSchedInfoMap != m_ueSchedInfoMap.end ());
			std::map <uint16_t, UlHarqProcessesStatus_t>::iterator itStat = m_ulHarqProcessesStatus.find (rnti);
			if (itStat == m_ulHarqProcessesStatus.end ())
			{
				NS_LOG_ERROR ("No info found in HARQ buffer for UE (might have changed eNB) " << rnti);
			}
			if (harqInfo.m_receptionStatus == UlHarqInfo::Ok || itStat->second.at (harqId) == 0)
			{
				//NS_LOG_DEBUG ("UE" << rnti << " UL harqId " << (unsigned)harqInfo.m_harqProcessId << " HARQ-ACK received");
				if (itStat != m_ulHarqProcessesStatus.end ())
				{
					itStat->second.at (harqId) = 0;  // release process ID
				}
			}
			else if (harqInfo.m_receptionStatus == UlHarqInfo::NotOk)
			{
				std::map <uint16_t, UlHarqProcessesDciInfoList_t>::iterator itHarq = m_ulHarqProcessesDciInfoMap.find (rnti);
				if (itHarq == m_ulHarqProcessesDciInfoMap.end ())
				{
					NS_LOG_ERROR ("No info found in UL-HARQ buffer for UE (might have changed eNB) " << rnti);
				}
				// retx correspondent block: retrieve the UL-DCI
				DciInfoElementTdma dciInfoReTx = itHarq->second.at (harqId);
				//NS_LOG_DEBUG ("UE" << rnti << " UL harqId " << (unsigned)harqInfo.m_harqProcessId << " HARQ-NACK received, rv " << (unsigned)dciInfoReTx.m_rv);
				NS_ASSERT (harqId == dciInfoReTx.m_harqProcess);
				NS_ASSERT(itStat->second.at (harqId) > 0);
				NS_ASSERT(itStat->second.at (harqId)-1 == dciInfoReTx.m_rv);
				if (dciInfoReTx.m_rv == 3)
				{
					NS_LOG_INFO ("Max number of retransmissions reached (UL)-> drop process");
					itStat->second.at (harqId) = 0;
					continue;
				}

				if (symAvail >= dciInfoReTx.m_numSym)
				{
					if(!CheckOverlapWithBusyResources(symIdx, dciInfoReTx.m_numSym)) // TODOIAB: check if it can be allocated later
					{
						symAvail -= dciInfoReTx.m_numSym;
						dciInfoReTx.m_symStart = symIdx;
						symIdx += dciInfoReTx.m_numSym;
						NS_ASSERT (symIdx <= m_phyMacConfig->GetSymbolsPerSubframe () - m_phyMacConfig->GetUlCtrlSymbols ());
						dciInfoReTx.m_rv++;
						dciInfoReTx.m_ndi = 0;
						itStat->second.at (harqId) = itStat->second.at (harqId) + 1;
						itHarq->second.at (harqId) = dciInfoReTx;
						SlotAllocInfo slotInfo (slotIdx++, SlotAllocInfo::UL_slotAllocInfo, SlotAllocInfo::CTRL_DATA, SlotAllocInfo::DIGITAL, rnti);
						slotInfo.m_dci = dciInfoReTx;
						NS_LOG_DEBUG ("UE" << dciInfoReTx.m_rnti << " gets UL slots " << (unsigned)dciInfoReTx.m_symStart << "-" << (unsigned)(dciInfoReTx.m_symStart+dciInfoReTx.m_numSym-1) <<
													" tbs " << dciInfoReTx.m_tbSize << " harqId " << (unsigned)dciInfoReTx.m_harqProcess << " rv " << (unsigned)dciInfoReTx.m_rv << " in frame " << ulSfn.m_frameNum << " subframe " << (unsigned)ulSfn.m_sfNum <<
													" RETX");
						ret.m_sfAllocInfo.m_slotAllocInfo.push_back (slotInfo);
						ret.m_sfAllocInfo.m_numSymAlloc += dciInfoReTx.m_numSym;

						itUeSchedInfoMap->second.m_ulSymbolsRetx = dciInfoReTx.m_numSym;
						itUeAllocMap = ueAllocMap.find (rnti);
						if (itUeAllocMap == ueAllocMap.end())
						{
							itUeAllocMap = ueAllocMap.insert (std::pair<uint16_t, UeSchedInfo*> (rnti, &itUeSchedInfoMap->second)).first;
						}
					}
					else
					{
						// find if there are other resources later
						// we cannot split RETX!
						int numSymNeeded = dciInfoReTx.m_numSym;
						int numFreeSymbols = 0;
						uint8_t tmpSymIdx = symIdx;

						while(numFreeSymbols == 0 && tmpSymIdx < m_phyMacConfig->GetSymbolsPerSubframe()-1) 
						{
							numFreeSymbols = GetNumFreeSymbols(tmpSymIdx, numSymNeeded); // this is equal to numSymNeeded if there is no overlap, otherwise smaller
							NS_LOG_LOGIC("UL RETX numSymNeeded " << numSymNeeded << " numFreeSymbols " 
								<< numFreeSymbols << " tmpSymIdx " << (uint16_t)tmpSymIdx);
							if(numFreeSymbols < numSymNeeded)
							{
								numFreeSymbols = 0;
								tmpSymIdx = GetFirstFreeSymbol(tmpSymIdx, numSymNeeded); // get the next symIdx
								if((int)(tmpSymIdx + numSymNeeded) >  (int)m_phyMacConfig->GetSymbolsPerSubframe()-1)
								{	
									NS_LOG_DEBUG("No way to fit this retx in the available resources");
									break;
								}
							}
						}

						NS_LOG_LOGIC("UL RETX numSymNeeded " << numSymNeeded << " numFreeSymbols " 
							<< numFreeSymbols << " tmpSymIdx " << (uint16_t)tmpSymIdx);
						if(numFreeSymbols >= numSymNeeded && (int)(tmpSymIdx + numSymNeeded) < (int)(m_phyMacConfig->GetSymbolsPerSubframe () - m_phyMacConfig->GetUlCtrlSymbols ()))
						{
							symAvail -= dciInfoReTx.m_numSym;
							dciInfoReTx.m_symStart = tmpSymIdx;

							NS_LOG_LOGIC("Before updating " << PrintSubframeAllocationMask(m_busyResourcesSchedSubframe.m_symAllocationMask));

							// update resource mask
							UpdateResourceMask(tmpSymIdx, dciInfoReTx.m_numSym);

							NS_LOG_LOGIC("After updating " << PrintSubframeAllocationMask(m_busyResourcesSchedSubframe.m_symAllocationMask));

							// symIdx += dciInfoReTx.m_numSym;
							NS_ASSERT (tmpSymIdx + dciInfoReTx.m_numSym <= m_phyMacConfig->GetSymbolsPerSubframe () - m_phyMacConfig->GetUlCtrlSymbols ());
							dciInfoReTx.m_rv++;
							dciInfoReTx.m_ndi = 0;
							itStat->second.at (harqId) = itStat->second.at (harqId) + 1;
							itHarq->second.at (harqId) = dciInfoReTx;
							SlotAllocInfo slotInfo (slotIdx++, SlotAllocInfo::UL_slotAllocInfo, SlotAllocInfo::CTRL_DATA, SlotAllocInfo::DIGITAL, rnti);
							slotInfo.m_dci = dciInfoReTx;
							NS_LOG_DEBUG ("UE" << dciInfoReTx.m_rnti << " gets UL slots " << (unsigned)dciInfoReTx.m_symStart << "-" << (unsigned)(dciInfoReTx.m_symStart+dciInfoReTx.m_numSym-1) <<
													 " tbs " << dciInfoReTx.m_tbSize << " harqId " << (unsigned)dciInfoReTx.m_harqProcess << " rv " << (unsigned)dciInfoReTx.m_rv << " in frame " << ulSfn.m_frameNum << " subframe " << (unsigned)ulSfn.m_sfNum <<
													 " RETX");
							ret.m_sfAllocInfo.m_slotAllocInfo.push_back (slotInfo);
							ret.m_sfAllocInfo.m_numSymAlloc += dciInfoReTx.m_numSym;

							itUeSchedInfoMap->second.m_ulSymbolsRetx = dciInfoReTx.m_numSym;
							itUeAllocMap = ueAllocMap.find (rnti);
							if (itUeAllocMap == ueAllocMap.end())
							{
								itUeAllocMap = ueAllocMap.insert (std::pair<uint16_t, UeSchedInfo*> (rnti, &itUeSchedInfoMap->second)).first;
							}
						}
						else
						{
							NS_LOG_DEBUG ("No resource for this UL retx (even later) -> buffer it");
							ulInfoListUntxed.push_back (m_ulHarqInfoList.at (i));
						}
					}
				}
				else
				{
					ulInfoListUntxed.push_back (m_ulHarqInfoList.at (i));
				}
			}
		}

		m_ulHarqInfoList.clear ();
		m_ulHarqInfoList = ulInfoListUntxed;
	}

	// no further allocations
	if (symAvail == 0)
	{
		// add slot for UL control
		SlotAllocInfo ulCtrlSlot (0xFF, SlotAllocInfo::UL_slotAllocInfo, SlotAllocInfo::CTRL, SlotAllocInfo::DIGITAL, 0);
		ulCtrlSlot.m_dci.m_numSym = 1;
		ulCtrlSlot.m_dci.m_symStart = m_phyMacConfig->GetSymbolsPerSubframe()-1;
		//ret.m_ulSfAllocInfo.m_slotAllocInfo.push_back (ulCtrlSlot);
		ret.m_sfAllocInfo.m_slotAllocInfo.push_back (ulCtrlSlot);
		//m_ulSfAllocInfo.push_back (ret.m_ulSfAllocInfo); // add UL SF info for later calls to scheduler
		m_macSchedSapUser->SchedConfigInd (ret);

		// reset the alloc info for the next scheduler call
		for (itUeAllocMap = ueAllocMap.begin (); itUeAllocMap != ueAllocMap.end(); itUeAllocMap++)
		{
			itUeAllocMap->second->m_dlSymbolsRetx = 0;
			itUeAllocMap->second->m_ulSymbolsRetx = 0;
		}
		return;
	}

	// ********************* END OF HARQ SECTION, START OF NEW DATA SCHEDULING ********************* //

	// number of DL/UL flows for new transmissions (not HARQ RETX)
	int nFlowsDl = 0;
	int nFlowsUl = 0;
	int nFlowsAccessDl = 0;
	int nFlowsAccessUl = 0;
	int nFlowsBackhaulDl = 0;
	int nFlowsBackhaulUl = 0;

	// compute achievable rates in current subframe
	for (std::map<uint16_t, UeSchedInfo>::iterator ueIt = m_ueSchedInfoMap.begin(); ueIt != m_ueSchedInfoMap.end(); ueIt++)
	{
		UeSchedInfo* ueInfo = &ueIt->second;

		// check if it is a relay or a UE
		bool isIab = false;
		if(m_rntiIabInfoMap.find(ueInfo->m_rnti) != m_rntiIabInfoMap.end())
		{
			isIab = m_rntiIabInfoMap.find(ueInfo->m_rnti)->second.first;
		}

		// get DL-CQI and compute DL rate per symbol
		bool dlAdded = false;
		std::map <uint16_t,uint8_t>::iterator itCqiDl = m_wbCqiRxed.find (ueInfo->m_rnti);
		uint8_t cqi = 0;
		if (itCqiDl != m_wbCqiRxed.end ())
		{
			cqi = itCqiDl->second;
		}
		else // no CQI available
		{
			NS_LOG_INFO (this << " UE " << ueInfo->m_rnti << " does not have DL-CQI");
			cqi = 1; // lowest value for trying a transmission
		}
		if (cqi != 0)
		{
			ueInfo->m_dlMcs = m_amc->GetMcsFromCqi (cqi);  // update MCS
			// compute total DL and UL bytes buffered
			for (unsigned iflow = 0; iflow < ueInfo->m_flowStatsDl.size(); iflow++)
			{
				if(ueInfo->m_flowStatsDl[iflow].m_rlcStatusPduSize > 0)
				{
					RlcPduInfo newRlcStatusPdu;
					newRlcStatusPdu.m_lcid = ueInfo->m_flowStatsDl[iflow].m_lcid;
					newRlcStatusPdu.m_size = ueInfo->m_flowStatsDl[iflow].m_rlcStatusPduSize;
					newRlcStatusPdu.m_size += m_subHdrSize; // as in MmWaveFlexTtiMacScheduler
					ueInfo->m_rlcPduInfo.push_back (newRlcStatusPdu);
					
					ueInfo->m_totBufDl += ueInfo->m_flowStatsDl[iflow].m_rlcStatusPduSize;

					nFlowsDl++;  // for simplicity, all RLC LCs are considered as a single flow
					if(!isIab)
					{
						nFlowsAccessDl++;
					}
					else
					{
						nFlowsBackhaulDl++;
						ueInfo->m_iab = true;
					}
				}

				if (ueInfo->m_flowStatsDl[iflow].m_totalBufSize > 0)
				{
					RlcPduInfo newRlcEl;
					newRlcEl.m_lcid = ueInfo->m_flowStatsDl[iflow].m_lcid;
					newRlcEl.m_size = ueInfo->m_flowStatsDl[iflow].m_totalBufSize;
					newRlcEl.m_size += m_rlcHdrSize + m_subHdrSize + 10; // as in MmWaveFlexTtiMacScheduler
					ueInfo->m_rlcPduInfo.push_back (newRlcEl);

					ueInfo->m_totBufDl += ueInfo->m_flowStatsDl[iflow].m_totalBufSize;

					nFlowsDl++;  // for simplicity, all RLC LCs are considered as a single flow
					if(!isIab)
					{
						nFlowsAccessDl++;
					}
					else
					{
						nFlowsBackhaulDl++;
						ueInfo->m_iab = true;
					}
				}
			}
			if (ueInfo->m_totBufDl > 0)
			{
				uint32_t tbSizeMax = m_amc->GetTbSizeFromMcsSymbols (ueInfo->m_dlMcs, 1);
				ueInfo->m_currTputDl = std::min(ueInfo->m_totBufDl,tbSizeMax) / (m_phyMacConfig->GetSubframePeriod () * 1E-6);
				m_ueStatHeap.push_back(ueInfo);
				itUeAllocMap = ueAllocMap.find (ueInfo->m_rnti);
				if (itUeAllocMap == ueAllocMap.end())
				{
					ueAllocMap.insert (std::pair<uint16_t, UeSchedInfo*> (ueInfo->m_rnti, ueInfo));
				}

				dlAdded = true;
			}
		}

		// get UL-CQI and compute UL rate per symbol
		std::map <uint16_t, struct UlCqiMapElem>::iterator itCqiUl = m_ueUlCqi.find (ueInfo->m_rnti);
		int mcs = 0;
		if (itCqiUl != m_ueUlCqi.end ()) // no cqi info for this UE
		{
			// translate vector of doubles to SpectrumValue's
			SpectrumValue specVals (MmWaveSpectrumValueHelper::GetSpectrumModel (m_phyMacConfig));
			Values::iterator specIt = specVals.ValuesBegin();
			for (unsigned ichunk = 0; ichunk < m_phyMacConfig->GetTotalNumChunk (); ichunk++)
			{
				NS_ASSERT (specIt != specVals.ValuesEnd());
				*specIt = itCqiUl->second.m_ueUlCqi.at (ichunk); //sinrLin;
				specIt++;
			}
			// for UL CQI, we need to know the TB size previously allocated to accurately compute CQI/MCS
			cqi = m_amc->CreateCqiFeedbackWbTdma (specVals, itCqiUl->second.m_numSym, itCqiUl->second.m_tbSize, mcs);
		}
		else
		{
			NS_LOG_INFO (this << " UE " << ueInfo->m_rnti << " does not have UL-CQI");
			cqi = 1;
			mcs = 0;
		}
		if (cqi != 0)
		{
			ueInfo->m_ulMcs = mcs;
			for (unsigned iflow = 0; iflow < ueInfo->m_flowStatsUl.size(); iflow++)
			{
				ueInfo->m_totBufUl += ueInfo->m_flowStatsUl[iflow].m_totalBufSize;
			}

			NS_LOG_LOGIC("Tot UL buffer " << ueInfo->m_totBufUl);
			if (ueInfo->m_totBufUl > 0)
			{
				ueInfo->m_totBufUl += m_rlcHdrSize + m_macHdrSize + 8; // as in MmWaveFlexTtiMacScheduler
				uint32_t tbSizeMax = m_amc->GetTbSizeFromMcsSymbols (ueInfo->m_ulMcs, 1);
				ueInfo->m_currTputUl = std::min(ueInfo->m_totBufUl,tbSizeMax) / (m_phyMacConfig->GetSubframePeriod () * 1E-6);
				
				nFlowsUl++;  // for simplicity, all RLC LCs are considered as a single flow
				if(!isIab)
				{
					nFlowsAccessUl++;
				}
				else
				{
					nFlowsBackhaulUl++;
					ueInfo->m_iab = true;
				}

				if (!dlAdded)
				{
					m_ueStatHeap.push_back(ueInfo);
					itUeAllocMap = ueAllocMap.find (ueInfo->m_rnti);
					if (itUeAllocMap == ueAllocMap.end())
					{
						ueAllocMap.insert (std::pair<uint16_t, UeSchedInfo*> (ueInfo->m_rnti, ueInfo));
					}
				}
			}
		}
	}

//	itUeAllocMap = ueAllocMap.find (10);
//	if (itUeAllocMap != ueAllocMap.end())
//	{
//		std::cout << frameNum << " " << sfNum << " " << itUeAllocMap->second->m_rlcPduInfo.size () << std::endl;
//	}

	int nFlowsTot = nFlowsDl + nFlowsUl;

	NS_LOG_DEBUG(this << " nFlowsDl " << nFlowsDl << " nFlowsUl " << nFlowsUl 
		<< " nFlowsAccessDl " << nFlowsAccessDl << " nFlowsAccessUl " << nFlowsAccessUl
		<< " nFlowsBackhaulDl " << nFlowsBackhaulDl << " nFlowBackhaulsUl " << nFlowsBackhaulUl << " tot " << nFlowsTot);

	// TODOIAB each IAB device cannot have more than half of the available resources
	// TODOIAB force the sum of m_maxUlSymbols and m_maxDlSymbols to be smaller than or equal to the following quantity
	int maxSymAvailableForSingleIab = std::floor(symAvail/2);
	for(auto ueInfo = m_ueSchedInfoMap.begin(); ueInfo != m_ueSchedInfoMap.end(); ++ueInfo)
	{
		//UeSchedInfo* ueInfo = *itUeInfo;

		if(ueInfo->second.m_iab && (ueInfo->second.m_totBufDl > 0 || ueInfo->second.m_totBufUl > 0))
		{
			int totalDlBufferRequested = ueInfo->second.m_totBufDl;
			int totalUlBufferRequested = ueInfo->second.m_totBufUl;

			double dlRatio = (double)totalDlBufferRequested/(totalDlBufferRequested + totalUlBufferRequested);

			// TODOIAB check floor & ceil
			int maxDlSym = std::floor(dlRatio*maxSymAvailableForSingleIab);
			int maxUlSym = std::ceil((1-dlRatio)*maxSymAvailableForSingleIab);

			NS_LOG_DEBUG("IAB " << ueInfo->second.m_rnti << " request DL " <<
				totalDlBufferRequested << " UL " << totalUlBufferRequested <<
				" max DL symbols granted " << maxDlSym << 
				" max UL symbols granted " << maxUlSym << 
				" max tot " << maxDlSym + maxUlSym << 
				" maxSymAvailableForSingleIab " << maxSymAvailableForSingleIab);
			
			ueInfo->second.m_maxDlSymbols = maxDlSym;
			ueInfo->second.m_maxUlSymbols = maxUlSym;
		}
	}

	// allocate each slot to UE with highest PF metric, then update PF metrics
	// TODOIAB limit IAB allocation
	while (symAvail > 0)
	{
		std::sort(m_ueStatHeap.begin(), m_ueStatHeap.end (),
							MmWaveFlexTtiPfMacScheduler::CompareUeWeightsPf);
		// evenly distribute symbols between DL and UL flows of same UE
		bool ueAlloc = false;
		// bool skipForIab = false;
		std::vector<UeSchedInfo*>::iterator ueHeapIt = m_ueStatHeap.begin();
//		for (itUeAllocMap = ueAllocMap.begin (); itUeAllocMap != ueAllocMap.end(); itUeAllocMap++)
//		{
//			UeSchedInfo* ueInfo = itUeAllocMap->second;
//			double pfMetric = std::max(ueInfo->m_currTputDl,ueInfo->m_currTputUl) / std::max(1E-9,(ueInfo->m_avgTputDl + ueInfo->m_avgTputDl));
//			std::cout << ueInfo->m_rnti << "=" << pfMetric << " ";
//		}
//		std::cout << std::endl;

		while (!ueAlloc && ueHeapIt != m_ueStatHeap.end())
		{
			UeSchedInfo* ueInfo = *ueHeapIt;

			if (ueInfo->m_totBufDl <= 0)
			{
				ueInfo->m_dlAllocDone = true;
			}
			else
			{
				if(ueInfo->m_iab && (ueInfo->m_maxDlSymbols <= ueInfo->m_dlSymbols))
				{
					ueInfo->m_dlAllocDone = true;
					// skipForIab = true; // IAB: set to true because non IAB users may still have a chance
					NS_LOG_LOGIC("Avoid allocating additional DL resources to IAB dev " << ueInfo->m_rnti
						<< " symbols " << (uint16_t)ueInfo->m_dlSymbols << " max " << (uint16_t)ueInfo->m_maxDlSymbols);
				}
			}
			if (ueInfo->m_totBufUl <= 0)
			{
				ueInfo->m_ulAllocDone = true;
			}
			else
			{
				if(ueInfo->m_iab && (ueInfo->m_maxUlSymbols <= ueInfo->m_ulSymbols))
				{
					ueInfo->m_ulAllocDone = true;
					// skipForIab = true; // IAB: set to true because non IAB users may still have a chance
					NS_LOG_LOGIC("Avoid allocating additional UL resources to IAB dev " << ueInfo->m_rnti
						<< " symbols " << (uint16_t)ueInfo->m_ulSymbols << " max " << (uint16_t)ueInfo->m_maxUlSymbols);
				}
			}

			if ((ueInfo->m_allocUlLast || ueInfo->m_dlAllocDone) && !ueInfo->m_ulAllocDone)
			{

				ueInfo->m_ulSymbols++;
				symAvail--;
				ueInfo->m_ulTbSize = m_amc->GetTbSizeFromMcsSymbols (ueInfo->m_ulMcs, ueInfo->m_ulSymbols) / 8;
				if (ueInfo->m_ulTbSize >= ueInfo->m_totBufUl)
				{
					ueInfo->m_ulAllocDone = true;
					ueInfo->m_lastAvgTputUl = ueInfo->m_avgTputUl;
				}
				ueInfo->m_allocUlLast = true;

				uint32_t tbSize = m_amc->GetTbSizeFromMcsSymbols (ueInfo->m_ulMcs, ueInfo->m_ulSymbols);
				ueInfo->m_currTputUl = std::min(ueInfo->m_totBufUl,tbSize) / (m_phyMacConfig->GetSubframePeriod () * 1E-6);
				ueInfo->m_avgTputUl = ((1.0 - (1.0 / m_timeWindow)) * ueInfo->m_lastAvgTputUl) +
						((1.0 / m_timeWindow) * ((double)ueInfo->m_ulTbSize / (m_phyMacConfig->GetSubframePeriod () * 1E-6)));
				ueAlloc = true;

			}
			else if (!ueInfo->m_dlAllocDone)
			{

				ueInfo->m_dlSymbols++;
				symAvail--;
				ueInfo->m_dlTbSize = m_amc->GetTbSizeFromMcsSymbols (ueInfo->m_dlMcs, ueInfo->m_dlSymbols) / 8;
				if (ueInfo->m_dlTbSize >= ueInfo->m_totBufDl)
				{
					ueInfo->m_dlAllocDone = true;
					ueInfo->m_lastAvgTputDl = ueInfo->m_avgTputDl;
				}
				ueInfo->m_allocUlLast = false;

				uint32_t tbSize = m_amc->GetTbSizeFromMcsSymbols (ueInfo->m_dlMcs, ueInfo->m_dlSymbols);
				ueInfo->m_currTputDl = std::min(ueInfo->m_totBufDl,tbSize) / (m_phyMacConfig->GetSubframePeriod () * 1E-6);
				ueInfo->m_avgTputDl = ((1.0 - (1.0 / m_timeWindow)) * ueInfo->m_lastAvgTputDl) +
						((1.0 / m_timeWindow) * ((double)ueInfo->m_dlTbSize / (m_phyMacConfig->GetSubframePeriod () * 1E-6)));
				ueAlloc = true;
			}

			ueHeapIt++;
		}

		if (!ueAlloc)
		{
			break;
		}
	}

	// no further allocations
	if (ueAllocMap.size () == 0)
	{
		// add slot for UL control
		SlotAllocInfo ulCtrlSlot (0xFF, SlotAllocInfo::UL_slotAllocInfo, SlotAllocInfo::CTRL, SlotAllocInfo::DIGITAL, 0);
		ulCtrlSlot.m_dci.m_numSym = 1;
		ulCtrlSlot.m_dci.m_symStart = m_phyMacConfig->GetSymbolsPerSubframe()-1;
		//ret.m_ulSfAllocInfo.m_slotAllocInfo.push_back (ulCtrlSlot);
		ret.m_sfAllocInfo.m_slotAllocInfo.push_back (ulCtrlSlot);
		//m_ulSfAllocInfo.push_back (ret.m_ulSfAllocInfo); // add UL SF info for later calls to scheduler
		m_macSchedSapUser->SchedConfigInd (ret);

		// reset the alloc info for the next scheduler call
		for (itUeAllocMap = ueAllocMap.begin (); itUeAllocMap != ueAllocMap.end(); itUeAllocMap++)
		{
			itUeAllocMap->second->m_dlSymbols = 0;
			itUeAllocMap->second->m_ulSymbols = 0;
			itUeAllocMap->second->m_dlTbSize = 0;
			itUeAllocMap->second->m_ulTbSize = 0;
			itUeAllocMap->second->m_dlSymbolsRetx = 0;
			itUeAllocMap->second->m_ulSymbolsRetx = 0;
			itUeAllocMap->second->m_currTputDl = 0;
			itUeAllocMap->second->m_currTputUl = 0;
			itUeAllocMap->second->m_avgTputDl = 0;
			itUeAllocMap->second->m_avgTputUl = 0;
			itUeAllocMap->second->m_totBufDl = 0;
			itUeAllocMap->second->m_totBufUl = 0;
			itUeAllocMap->second->m_dlAllocDone = false;
			itUeAllocMap->second->m_ulAllocDone = false;
			itUeAllocMap->second->m_rlcPduInfo.clear ();
		}
		return;
	}

//	itUeAllocMap = ueAllocMap.find (10);
//	if (itUeAllocMap != ueAllocMap.end())
//	{
//		std::cout << frameNum << " " << sfNum << " " << itUeAllocMap->second->m_rlcPduInfo.size () << std::endl;
//	}

	// iterate through map of allocated UEs, assign TDMA symbol indices and create DCIs
	//unsigned numSymAllocPrev = ret.m_dlSfAllocInfo.m_numSymAlloc; // allocated in prev sched request
	for (itUeAllocMap = ueAllocMap.begin (); itUeAllocMap != ueAllocMap.end(); itUeAllocMap++)
	{
		UeSchedInfo *ueInfo = itUeAllocMap->second;
		if (ueInfo->m_dlSymbols > 0)
		{
			std::vector<SlotAllocInfo> tmpSlotAllocVector;
			int numSymNeeded = ueInfo->m_dlSymbols;
			int totalTbSize = 0;
			NS_LOG_DEBUG("UE " << ueInfo->m_rnti << " numSymNeeded " << numSymNeeded);
			do
			{
				// if there is overlapping, split allocation
				// get the number of symbols that could be allocated
				int numFreeSymbols = GetNumFreeSymbols(symIdx, numSymNeeded); // this is equal to numSymNeeded if there is no overlap, otherwise smaller
				if(numFreeSymbols == 0)
				{
					// symIdx is inside a busy interval, get the next free symIdx and retry
					NS_LOG_DEBUG("symIdx is in a busy interval, get the first free symIdx and retry");
					symIdx = GetFirstFreeSymbol(symIdx, 0); // get the next symIdx
					NS_LOG_DEBUG("new symIdx " << (uint16_t)symIdx);
					numFreeSymbols = GetNumFreeSymbols(symIdx, numSymNeeded);
				}
				symAvail -= numFreeSymbols;
				numSymNeeded -= numFreeSymbols;
				NS_LOG_DEBUG("UE " << ueInfo->m_rnti << " numSymNeeded " << numSymNeeded << " numFreeSymbols " << numFreeSymbols << " symAvail " << symAvail);

				DciInfoElementTdma dci;
				dci.m_rnti = ueInfo->m_rnti;
				dci.m_format = 0;
				dci.m_symStart = symIdx;
				dci.m_numSym = numFreeSymbols;
				symIdx = GetFirstFreeSymbol(symIdx, numFreeSymbols); // get the next symIdx
				NS_LOG_LOGIC("Next symIdx " << (uint16_t)symIdx);
				NS_ASSERT (symIdx <= m_phyMacConfig->GetSymbolsPerSubframe () - m_phyMacConfig->GetUlCtrlSymbols ());
				dci.m_mcs = ueInfo->m_dlMcs;
				dci.m_rv = 0;
				dci.m_ndi = 1;
				dci.m_tbSize = m_amc->GetTbSizeFromMcsSymbols (dci.m_mcs, dci.m_numSym) / 8;
				totalTbSize += dci.m_tbSize;
				dci.m_harqProcess = UpdateDlHarqProcessId (ueInfo->m_rnti);
				NS_ASSERT (dci.m_harqProcess < m_phyMacConfig->GetNumHarqProcess ());
				
				NS_LOG_DEBUG ("UE" << ueInfo->m_rnti << " DL harqId " << (unsigned)dci.m_harqProcess << " HARQ process assigned");
				SlotAllocInfo slotInfo (slotIdx++, SlotAllocInfo::DL_slotAllocInfo, SlotAllocInfo::CTRL_DATA, SlotAllocInfo::DIGITAL, ueInfo->m_rnti);
				slotInfo.m_dci = dci;
				NS_LOG_DEBUG ("UE" << dci.m_rnti << " gets DL symbols " << (unsigned)dci.m_symStart << "-" << (unsigned)(dci.m_symStart+dci.m_numSym-1) <<
										" tbs " << dci.m_tbSize << " mcs " << (unsigned)dci.m_mcs << " harqId " << (unsigned)dci.m_harqProcess << " rv " << (unsigned)dci.m_rv << " in frame " << ret.m_sfnSf.m_frameNum << " subframe " << (unsigned)ret.m_sfnSf.m_sfNum);

				if (m_harqOn == true)
				{	// store DCI for HARQ buffer
					std::map <uint16_t, DlHarqProcessesDciInfoList_t>::iterator itDciInfo = m_dlHarqProcessesDciInfoMap.find (dci.m_rnti);
					if (itDciInfo == m_dlHarqProcessesDciInfoMap.end ())
					{
						NS_FATAL_ERROR ("Unable to find RNTI entry in DCI HARQ buffer for RNTI " << dci.m_rnti);
					}
					(*itDciInfo).second.at (dci.m_harqProcess) = dci;
					// refresh timer
					std::map <uint16_t, DlHarqProcessesTimer_t>::iterator itHarqTimer =  m_dlHarqProcessesTimer.find (dci.m_rnti);
					if (itHarqTimer== m_dlHarqProcessesTimer.end ())
					{
						NS_FATAL_ERROR ("Unable to find HARQ timer for RNTI " << (uint16_t)dci.m_rnti);
					}
					(*itHarqTimer).second.at (dci.m_harqProcess) = 0;
				}

				tmpSlotAllocVector.push_back(slotInfo);
			} while(numSymNeeded > 0);

			// distribute bytes between active RLC queues
			unsigned numLc = ueInfo->m_rlcPduInfo.size ();
			unsigned bytesRem = totalTbSize;
			unsigned numFulfilled = 0;
			uint16_t avgPduSize = bytesRem / numLc;
			NS_LOG_DEBUG("numLc " << numLc << " bytesRem " << bytesRem << " avgPduSize " << avgPduSize);

			// first for loop computes extra to add to average if some flows are less than average
			for (unsigned i = 0; i < ueInfo->m_rlcPduInfo.size (); i++)
			{
				if (ueInfo->m_rlcPduInfo[i].m_size < avgPduSize)
				{
					bytesRem -= ueInfo->m_rlcPduInfo[i].m_size;
					numFulfilled++;
				}
			}

			if (numFulfilled < ueInfo->m_rlcPduInfo.size ())
			{
				avgPduSize = bytesRem / (ueInfo->m_rlcPduInfo.size () - numFulfilled);
			}

			NS_LOG_DEBUG("numFulfilled " << numFulfilled << " new avgPduSize " << avgPduSize);

			// fill the RlcPduInfo taking into account the possible split of resources
			for(auto slotInfo : tmpSlotAllocVector)
			{
				// distribute bytes between active RLC queues and slots allocated
				int remTbSize = slotInfo.m_dci.m_tbSize;
				// TODOIAB check m_rlcPduInfo
				NS_LOG_DEBUG("slotInfo starting from " << (uint16_t)slotInfo.m_dci.m_symStart << " with tbSize " << remTbSize << " num PDU " << ueInfo->m_rlcPduInfo.size());
				
				auto pduIterator = ueInfo->m_rlcPduInfo.begin();
				while (remTbSize > 0 && pduIterator != ueInfo->m_rlcPduInfo.end())
				{
					NS_LOG_DEBUG("PDU with initial size " << pduIterator->m_size);
					// allocate as many bytes for this currentTbSize					
					if (pduIterator->m_size > avgPduSize)
					{
						pduIterator->m_size = avgPduSize;
						NS_LOG_LOGIC("Clip PDU to avgPduSize");
					}
					// else tbSize equals RLC queue size
					NS_ASSERT(pduIterator->m_size > 0);

					NS_LOG_DEBUG("Update RLC buffer with size (plus hdr)" << std::min(remTbSize, (int)pduIterator->m_size));
					// update RLC buffer info with expected queue size after scheduling
					UpdateDlRlcBufferInfo (ueInfo->m_rnti, pduIterator->m_lcid, std::min(remTbSize, (int)pduIterator->m_size) - m_subHdrSize);

					if((int)pduIterator->m_size > remTbSize)
					{
						// max amount of bytes that can be allocated
						slotInfo.m_rlcPduInfo.push_back (RlcPduInfo(pduIterator->m_lcid, remTbSize));
						pduIterator->m_size -= remTbSize - m_subHdrSize - 10; // TODOIAB add the size of a subheader (+10 understand why!)
						remTbSize = 0;
					}
					else
					{
						slotInfo.m_rlcPduInfo.push_back ((*pduIterator));
						remTbSize -= pduIterator->m_size;
						
						// we need to remove this m_rlcPduInfo since it has been satisfied
						ueInfo->m_rlcPduInfo.erase(pduIterator);
						pduIterator = ueInfo->m_rlcPduInfo.begin();
					}

					NS_LOG_DEBUG("Added RLC PDU with size " << slotInfo.m_rlcPduInfo.back().m_size << " remTbSize " << remTbSize);

					if (m_harqOn == true)
					{
						// store RLC PDU list for HARQ
						std::map <uint16_t, DlHarqRlcPduList_t>::iterator itRlcPdu =  m_dlHarqProcessesRlcPduMap.find (ueInfo->m_rnti);
						if (itRlcPdu == m_dlHarqProcessesRlcPduMap.end ())
						{
							NS_FATAL_ERROR ("Unable to find RlcPdcList in HARQ buffer for RNTI " << ueInfo->m_rnti);
						}
						(*itRlcPdu).second.at (slotInfo.m_dci.m_harqProcess).push_back (slotInfo.m_rlcPduInfo.back());
					}
				}

				
				// reorder/reindex slots to maintain DL before UL slot order
				bool reordered = false;
				// std::deque <SlotAllocInfo>::iterator itSlot = ret.m_sfAllocInfo.m_slotAllocInfo.begin ();
				// for (unsigned islot = 0; islot < ret.m_sfAllocInfo.m_slotAllocInfo.size (); islot++)
				// {
				// 	if (ret.m_sfAllocInfo.m_slotAllocInfo [islot].m_tddMode == SlotAllocInfo::UL_slotAllocInfo)
				// 	{
				// 		slotInfo.m_slotIdx = ret.m_sfAllocInfo.m_slotAllocInfo [islot].m_slotIdx;
				// 		slotInfo.m_dci.m_symStart = ret.m_sfAllocInfo.m_slotAllocInfo [islot].m_dci.m_symStart;
				// 		ret.m_sfAllocInfo.m_slotAllocInfo.insert (itSlot, slotInfo);
				// 		for (unsigned jslot = islot+1; jslot < ret.m_sfAllocInfo.m_slotAllocInfo.size (); jslot++)
				// 		{
				// 			ret.m_sfAllocInfo.m_slotAllocInfo[jslot].m_slotIdx++;	// increase indices of UL slots
				// 			ret.m_sfAllocInfo.m_slotAllocInfo[jslot].m_dci.m_symStart =
				// 														ret.m_sfAllocInfo.m_slotAllocInfo[jslot-1].m_dci.m_symStart +
				// 														ret.m_sfAllocInfo.m_slotAllocInfo[jslot-1].m_dci.m_numSym;
				// 		}
				// 		reordered = true;
				// 		break;
				// 	}
				// 	itSlot++;
				// }
				if (!reordered && slotInfo.m_rlcPduInfo.size() > 0) // TODOIAB understand why a TB may be in excess with respect to the RLC size
				{
					ret.m_sfAllocInfo.m_slotAllocInfo.push_back (slotInfo);
				}
				ret.m_sfAllocInfo.m_numSymAlloc += slotInfo.m_dci.m_numSym;
			} 
		}
	}

	// slotIdx = ret.m_sfAllocInfo.m_slotAllocInfo.back ().m_slotIdx + 1;
	// symIdx = ret.m_sfAllocInfo.m_slotAllocInfo.back ().m_dci.m_symStart + ret.m_sfAllocInfo.m_slotAllocInfo.back ().m_dci.m_numSym;

	for (itUeAllocMap = ueAllocMap.begin (); itUeAllocMap != ueAllocMap.end(); itUeAllocMap++)
	{
		UeSchedInfo *ueInfo = itUeAllocMap->second;
		// Note: UL-DCI applies to subframe i+Tsched
		if (ueInfo->m_ulSymbols > 0)
		{
			int numSymNeeded = ueInfo->m_ulSymbols;
			do
			{
				// if there is overlapping, split allocation
				// get the number of symbols that could be allocated
				NS_LOG_DEBUG("UE " << ueInfo->m_rnti << " numSymNeeded " << numSymNeeded);
				int numFreeSymbols = GetNumFreeSymbols(symIdx, numSymNeeded); // this is equal to numSymNeeded if there is no overlap, otherwise smaller
				if(numFreeSymbols == 0)
				{
					// symIdx is inside a busy interval, get the next free symIdx and retry
					NS_LOG_DEBUG("symIdx is in a busy interval, get the first free symIdx and retry (UL)");
					symIdx = GetFirstFreeSymbol(symIdx, 0); // get the next symIdx
					NS_LOG_DEBUG("new symIdx " << (uint16_t)symIdx);
					numFreeSymbols = GetNumFreeSymbols(symIdx, numSymNeeded);
				}

				symAvail -= numFreeSymbols;
				numSymNeeded -= numFreeSymbols;
				NS_LOG_DEBUG("UE " << ueInfo->m_rnti << " numSymNeeded " << numSymNeeded << " numFreeSymbols " << numFreeSymbols << " symAvail " << symAvail);

				DciInfoElementTdma dci;
				dci.m_rnti = ueInfo->m_rnti;
				dci.m_format = 1;
				dci.m_symStart = symIdx;
				dci.m_numSym = numFreeSymbols;
				symIdx = GetFirstFreeSymbol(symIdx, numFreeSymbols); // get the next free symbol
				NS_LOG_LOGIC("Next symIdx " << (uint16_t)symIdx);
				NS_ASSERT (symIdx <= m_phyMacConfig->GetSymbolsPerSubframe () - m_phyMacConfig->GetUlCtrlSymbols ());
				dci.m_mcs = ueInfo->m_ulMcs;
				dci.m_ndi = 1;
				dci.m_tbSize = m_amc->GetTbSizeFromMcsSymbols (dci.m_mcs, dci.m_numSym) / 8;
				
				dci.m_harqProcess = UpdateUlHarqProcessId (ueInfo->m_rnti);
				NS_LOG_DEBUG ("UE" << ueInfo->m_rnti << " UL harqId " << (unsigned)dci.m_harqProcess << " HARQ process assigned");
				NS_ASSERT (dci.m_harqProcess < m_phyMacConfig->GetNumHarqProcess ());
				SlotAllocInfo slotInfo (slotIdx++, SlotAllocInfo::UL_slotAllocInfo, SlotAllocInfo::CTRL_DATA, SlotAllocInfo::DIGITAL, ueInfo->m_rnti);
				slotInfo.m_dci = dci;
				NS_LOG_DEBUG ("UE" << dci.m_rnti << " gets UL symbols " << (unsigned)dci.m_symStart << "-" << (unsigned)(dci.m_symStart+dci.m_numSym-1) <<
										" tbs " << dci.m_tbSize << " mcs " << (unsigned)dci.m_mcs << " harqId " << (unsigned)dci.m_harqProcess << " rv " << (unsigned)dci.m_rv << " in frame " << ret.m_sfnSf.m_frameNum << " subframe " << (unsigned)ret.m_sfnSf.m_sfNum);
				
				UpdateUlRlcBufferInfo (ueInfo->m_rnti, dci.m_tbSize - m_subHdrSize);
				ret.m_sfAllocInfo.m_slotAllocInfo.push_back (slotInfo);
				ret.m_sfAllocInfo.m_numSymAlloc += dci.m_numSym;
				std::vector<uint16_t> ueChunkMap;
				for (unsigned i = 0; i < m_phyMacConfig->GetTotalNumChunk (); i++)
				{
					ueChunkMap.push_back (dci.m_rnti);
				}
				SfnSf slotSfn = ret.m_sfAllocInfo.m_sfnSf;
				slotSfn.m_slotNum = dci.m_symStart;  // use the start symbol index of the slot because the absolute UL slot index depends on the future DL allocation
				// insert into allocation map to recall previous allocations upon receiving UL-CQI
				m_ulAllocationMap.insert ( std::pair<uint32_t, struct AllocMapElem> (slotSfn.Encode (), AllocMapElem(ueChunkMap, dci.m_numSym, dci.m_tbSize)) );

				if (m_harqOn == true)
				{
					uint8_t harqId = dci.m_harqProcess;
					std::map <uint16_t, UlHarqProcessesDciInfoList_t>::iterator itHarqTbInfo = m_ulHarqProcessesDciInfoMap.find (dci.m_rnti);
					if (itHarqTbInfo == m_ulHarqProcessesDciInfoMap.end ())
					{
						NS_FATAL_ERROR ("Unable to find RNTI entry in UL DCI HARQ buffer for RNTI " << dci.m_rnti);
					}
					(*itHarqTbInfo).second.at (harqId) = dci;
					// Update HARQ process status (RV 0)
					std::map <uint16_t, UlHarqProcessesStatus_t>::iterator itStat = m_ulHarqProcessesStatus.find (dci.m_rnti);
					NS_ASSERT (itStat->second[dci.m_harqProcess] > 0);
					// refresh timer
					std::map <uint16_t, UlHarqProcessesTimer_t>::iterator itHarqTimer =  m_ulHarqProcessesTimer.find (dci.m_rnti);
					if (itHarqTimer== m_ulHarqProcessesTimer.end ())
					{
						NS_FATAL_ERROR ("Unable to find HARQ timer for RNTI " << (uint16_t)dci.m_rnti);
					}
					(*itHarqTimer).second.at (dci.m_harqProcess) = 0;
				}
			} while(numSymNeeded > 0);
		}
	}

	// reset the alloc info for the next scheduler call
	for (itUeAllocMap = ueAllocMap.begin (); itUeAllocMap != ueAllocMap.end(); itUeAllocMap++)
	{
		itUeAllocMap->second->m_dlSymbols = 0;
		itUeAllocMap->second->m_ulSymbols = 0;
		itUeAllocMap->second->m_dlTbSize = 0;
		itUeAllocMap->second->m_ulTbSize = 0;
		itUeAllocMap->second->m_dlSymbolsRetx = 0;
		itUeAllocMap->second->m_ulSymbolsRetx = 0;
		itUeAllocMap->second->m_currTputDl = 0;
		itUeAllocMap->second->m_currTputUl = 0;
		itUeAllocMap->second->m_avgTputDl = 0;
		itUeAllocMap->second->m_avgTputUl = 0;
		itUeAllocMap->second->m_totBufDl = 0;
		itUeAllocMap->second->m_totBufUl = 0;
		itUeAllocMap->second->m_dlAllocDone = false;
		itUeAllocMap->second->m_ulAllocDone = false;
		itUeAllocMap->second->m_rlcPduInfo.clear ();
	}

	// add slot for UL control
	SlotAllocInfo ulCtrlSlot (0xFF, SlotAllocInfo::UL_slotAllocInfo, SlotAllocInfo::CTRL, SlotAllocInfo::DIGITAL, 0);
	ulCtrlSlot.m_dci.m_numSym = 1;
	ulCtrlSlot.m_dci.m_symStart = m_phyMacConfig->GetSymbolsPerSubframe()-1;
	//ret.m_ulSfAllocInfo.m_slotAllocInfo.push_back (ulCtrlSlot);
	ret.m_sfAllocInfo.m_slotAllocInfo.push_back (ulCtrlSlot);

	// we need to sort the symbols, given that HARQ may have been scheduled out of order
	std::sort(ret.m_sfAllocInfo.m_slotAllocInfo.begin(), ret.m_sfAllocInfo.m_slotAllocInfo.end(),
		MmWavePhyMacCommon::CompareSlotAllocInfo);

	m_macSchedSapUser->SchedConfigInd (ret);
	return;
}

bool
MmWaveFlexTtiPfMacScheduler::SortRlcBufferReq (MmWaveMacSchedSapProvider::SchedDlRlcBufferReqParameters i, MmWaveMacSchedSapProvider::SchedDlRlcBufferReqParameters j)
{
  return (i.m_rnti < j.m_rnti);
}


int
MmWaveFlexTtiPfMacScheduler::GetNumFreeSymbols(uint8_t symIdx, int numSymNeeded)
{
	NS_LOG_LOGIC("symIdx " << (uint16_t)symIdx << " numSymNeeded " << numSymNeeded);

	if(!m_iabScheduler || !m_busyResourcesSchedSubframe.m_valid)
	{
		return numSymNeeded;
	}

	int numFreeSymbols = 0;
	// get the possible overlapping region
	uint32_t loopEnd = std::min(symIdx + numSymNeeded, (int)m_busyResourcesSchedSubframe.m_symAllocationMask.size());
	NS_LOG_LOGIC("loopEnd " << loopEnd);
	for(uint32_t index = symIdx; index < loopEnd; ++index)
	{
		if(m_busyResourcesSchedSubframe.m_symAllocationMask.at(index) == 0)
		{
			numFreeSymbols++;
		}
		else
		{
			NS_LOG_LOGIC("Symbol busy, allocated " << numFreeSymbols << " out of " << numSymNeeded);
			break;
		}
	}
	return numFreeSymbols;
}

void
MmWaveFlexTtiPfMacScheduler::UpdateResourceMask(uint8_t start, int numSymbols)
{
	if(!m_iabScheduler)
	{
		NS_FATAL_ERROR("Try to update the mask for a non IAB scheduler");
	}

	for(uint32_t index = start; index < (uint32_t)(start + numSymbols); ++index)
	{
		m_busyResourcesSchedSubframe.m_symAllocationMask.at(index) = 1;
	}
		
}

uint8_t
MmWaveFlexTtiPfMacScheduler::GetFirstFreeSymbol(uint8_t symIdx, int numFreeSymbols)
{
	NS_LOG_LOGIC("symIdx " << (uint16_t)symIdx << " numFreeSymbols " << numFreeSymbols);

	if(!m_iabScheduler || !m_busyResourcesSchedSubframe.m_valid)
	{
		return symIdx + numFreeSymbols;
	}

	int index;
	for(index = symIdx + numFreeSymbols; index < (int)m_busyResourcesSchedSubframe.m_symAllocationMask.size(); ++index)
	{
		if(m_busyResourcesSchedSubframe.m_symAllocationMask.at(index) == 0)
		{
			NS_LOG_LOGIC("Free symbol found at " << index);
			break;
		}
	}
	return static_cast<uint8_t>(index);
}

bool
MmWaveFlexTtiPfMacScheduler::CheckOverlapWithBusyResources(uint8_t symIdx, int numSymNeeded)
{
	NS_LOG_LOGIC("symIdx " << (uint16_t)symIdx << " numSymNeeded " << numSymNeeded);

	if(!m_iabScheduler || !m_busyResourcesSchedSubframe.m_valid)
	{
		return false;
	}

	uint32_t loopEnd = std::min(symIdx + numSymNeeded, (int)m_busyResourcesSchedSubframe.m_symAllocationMask.size());
	NS_LOG_LOGIC("loopEnd " << loopEnd);
	for(uint32_t index = symIdx; index < loopEnd; ++index)
	{
		if(m_busyResourcesSchedSubframe.m_symAllocationMask.at(index) == 1)
		{
			NS_LOG_LOGIC("overlapping from symbol " << index);
			return true;
		}
	}
	return false;
}


void
MmWaveFlexTtiPfMacScheduler::RefreshDlCqiMaps (void)
{
  NS_LOG_FUNCTION (this << m_wbCqiTimers.size ());
  // refresh DL CQI P01 Map
  std::map <uint16_t,uint32_t>::iterator itP10 = m_wbCqiTimers.begin ();
  while (itP10 != m_wbCqiTimers.end ())
    {
      NS_LOG_INFO (this << " P10-CQI for user " << (*itP10).first << " is " << (uint32_t)(*itP10).second << " thr " << (uint32_t)m_cqiTimersThreshold);
      if ((*itP10).second == 0)
        {
          // delete correspondent entries
          std::map <uint16_t,uint8_t>::iterator itMap = m_wbCqiRxed.find ((*itP10).first);
          NS_ASSERT_MSG (itMap != m_wbCqiRxed.end (), " Does not find CQI report for user " << (*itP10).first);
          NS_LOG_INFO (this << " P10-CQI exired for user " << (*itP10).first);
          m_wbCqiRxed.erase (itMap);
          std::map <uint16_t,uint32_t>::iterator temp = itP10;
          itP10++;
          m_wbCqiTimers.erase (temp);
        }
      else
        {
          (*itP10).second--;
          itP10++;
        }
    }

  return;
}


void
MmWaveFlexTtiPfMacScheduler::RefreshUlCqiMaps (void)
{
  // refresh UL CQI  Map
  std::map <uint16_t,uint32_t>::iterator itUl = m_ueCqiTimers.begin ();
  while (itUl != m_ueCqiTimers.end ())
    {
      NS_LOG_INFO (this << " UL-CQI for user " << (*itUl).first << " is " << (uint32_t)(*itUl).second << " thr " << (uint32_t)m_cqiTimersThreshold);
      if ((*itUl).second == 0)
        {
          // delete correspondent entries
          std::map <uint16_t, struct UlCqiMapElem>::iterator itMap = m_ueUlCqi.find ((*itUl).first);
          NS_ASSERT_MSG (itMap != m_ueUlCqi.end (), " Does not find CQI report for user " << (*itUl).first);
          NS_LOG_INFO (this << " UL-CQI expired for user " << (*itUl).first);
          itMap->second.m_ueUlCqi.clear ();
          m_ueUlCqi.erase (itMap);
          std::map <uint16_t,uint32_t>::iterator temp = itUl;
          itUl++;
          m_ueCqiTimers.erase (temp);
        }
      else
        {
          (*itUl).second--;
          itUl++;
        }
    }

  return;
}

void
MmWaveFlexTtiPfMacScheduler::UpdateDlRlcBufferInfo (uint16_t rnti, uint8_t lcid, uint16_t size)
{
  NS_LOG_FUNCTION (this);

  std::map <uint16_t, struct UeSchedInfo>::iterator itUe = m_ueSchedInfoMap.find (rnti);
  if (itUe == m_ueSchedInfoMap.end ())
  {
  	NS_LOG_ERROR ("UE entry not found in sched info map");
  }
  else
  {
	NS_LOG_DEBUG (this << " UE " << rnti << " LC " << (uint16_t)lcid << " txqueue " << itUe->second.m_flowStatsDl[lcid].m_totalBufSize << " decrease " << size);
  	
  	// status PDU 
	if(itUe->second.m_flowStatsDl[lcid].m_rlcStatusPduSize > 0)
	{
		if(itUe->second.m_flowStatsDl[lcid].m_rlcStatusPduSize < size)
		{
			size -= itUe->second.m_flowStatsDl[lcid].m_rlcStatusPduSize;
			itUe->second.m_flowStatsDl[lcid].m_rlcStatusPduSize = 0;
		}
		else
		{
			itUe->second.m_flowStatsDl[lcid].m_rlcStatusPduSize	-= size;
			size = 0;
		}
	}

  	if(itUe->second.m_flowStatsDl[lcid].m_totalBufSize > 0)
  	{
  		uint32_t rlcOverhead;
        if (lcid == 1)
          {
            // for SRB1 (using RLC AM) it's better to
            // overestimate RLC overhead rather than
            // underestimate it and risk unneeded
            // segmentation which increases delay
            rlcOverhead = 4;
          }
        else
          {
            // minimum RLC overhead due to header
            rlcOverhead = 2;
          }
        // update transmission queue
        if (itUe->second.m_flowStatsDl[lcid].m_totalBufSize <= size - rlcOverhead)
          {
            itUe->second.m_flowStatsDl[lcid].m_totalBufSize = 0;
          }
        else
          {
            itUe->second.m_flowStatsDl[lcid].m_totalBufSize -= size - rlcOverhead;
          }
  	}
  }

  // the following code does not do anything..
  std::list<MmWaveMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator it;
  NS_LOG_DEBUG("m_rlcBufferReq size " << m_rlcBufferReq.size());
  for (it = m_rlcBufferReq.begin (); it != m_rlcBufferReq.end (); it++)
    {
      if (((*it).m_rnti == rnti) && ((*it).m_logicalChannelIdentity == lcid))
        {
          NS_LOG_DEBUG (this << " UE " << rnti << " LC " << (uint16_t)lcid << " txqueue " << (*it).m_rlcTransmissionQueueSize << " retxqueue " << (*it).m_rlcRetransmissionQueueSize << " status " << (*it).m_rlcStatusPduSize << " decrease " << size);
          // Update queues: RLC tx order Status, ReTx, Tx
          // Update status queue
           if (((*it).m_rlcStatusPduSize > 0) && (size >= (*it).m_rlcStatusPduSize))
              {
                (*it).m_rlcStatusPduSize = 0;
              }
            else if (((*it).m_rlcRetransmissionQueueSize > 0) && (size >= (*it).m_rlcRetransmissionQueueSize))
              {
                (*it).m_rlcRetransmissionQueueSize = 0;
              }
            else if ((*it).m_rlcTransmissionQueueSize > 0)
              {
                uint32_t rlcOverhead;
                if (lcid == 1)
                  {
                    // for SRB1 (using RLC AM) it's better to
                    // overestimate RLC overhead rather than
                    // underestimate it and risk unneeded
                    // segmentation which increases delay
                    rlcOverhead = 4;
                  }
                else
                  {
                    // minimum RLC overhead due to header
                    rlcOverhead = 2;
                  }
                // update transmission queue
                if ((*it).m_rlcTransmissionQueueSize <= size - rlcOverhead)
                  {
                    (*it).m_rlcTransmissionQueueSize = 0;
                  }
                else
                  {
                    (*it).m_rlcTransmissionQueueSize -= size - rlcOverhead;
                  }
              }
          return;
        }
    }
}

void
MmWaveFlexTtiPfMacScheduler::UpdateUlRlcBufferInfo (uint16_t rnti, uint16_t size)
{
	size = size - 2; // remove the minimum RLC overhead
	std::map <uint16_t, struct UeSchedInfo>::iterator itUe = m_ueSchedInfoMap.find (rnti);
	uint32_t rlcOverhead = 2;
	size -= rlcOverhead;
	if (itUe == m_ueSchedInfoMap.end ())
	{
		NS_LOG_ERROR ("UE entry not found in sched info map");
	}
	else
	{
		NS_LOG_DEBUG (this << " UE " << rnti << " txqueue " << itUe->second.m_totBufUl << " decrease " << size);

		// the allocation is done in LCID order
		uint8_t lcid = 1;
		while(lcid <= 3 && size > 0)
		{
			if(itUe->second.m_flowStatsUl[lcid].m_totalBufSize > 0)
			{
				NS_LOG_DEBUG("lcid " << (uint16_t)lcid << " decrease " << size << " from " << itUe->second.m_flowStatsUl[lcid].m_totalBufSize);
				if(itUe->second.m_flowStatsUl[lcid].m_totalBufSize < size)
				{
					size -= itUe->second.m_flowStatsUl[lcid].m_totalBufSize;
					itUe->second.m_flowStatsUl[lcid].m_totalBufSize = 0; 
				}
				else
				{
					itUe->second.m_flowStatsUl[lcid].m_totalBufSize -= size;
					size = 0;
				}
			}
			++lcid;
		}
	}
}


void
MmWaveFlexTtiPfMacScheduler::DoCschedCellConfigReq (const struct MmWaveMacCschedSapProvider::CschedCellConfigReqParameters& params)
{
  NS_LOG_FUNCTION (this);
  // Read the subset of parameters used
  m_cschedCellConfig = params;
  //m_rachAllocationMap.resize (m_cschedCellConfig.m_ulBandwidth, 0);
  MmWaveMacCschedSapUser::CschedUeConfigCnfParameters cnf;
  cnf.m_result = SUCCESS;
  m_macCschedSapUser->CschedUeConfigCnf (cnf);
  return;
}

void
MmWaveFlexTtiPfMacScheduler::DoCschedUeConfigReq (const struct MmWaveMacCschedSapProvider::CschedUeConfigReqParameters& params)
{
  NS_LOG_FUNCTION (this << " RNTI " << params.m_rnti << " txMode " << (uint16_t)params.m_transmissionMode);

  NS_LOG_DEBUG("rnti " << params.m_rnti);

  std::map <uint16_t, struct UeSchedInfo>::iterator itUe = m_ueSchedInfoMap.find (params.m_rnti);
  if (itUe == m_ueSchedInfoMap.end ())
  {
  	itUe = m_ueSchedInfoMap.insert (std::pair <uint16_t, struct UeSchedInfo> (params.m_rnti, UeSchedInfo (params.m_rnti))).first;
  	for (unsigned i = 0; i <= 3; i++)
  	{
  		itUe->second.m_flowStatsDl.push_back (FlowStats (false, &(itUe->second), i));
  		itUe->second.m_flowStatsUl.push_back (FlowStats (true, &(itUe->second), i));
  	}
  }

  if (m_dlHarqProcessesStatus.find (params.m_rnti) == m_dlHarqProcessesStatus.end ())
  {
  	//m_dlHarqCurrentProcessId.insert (std::pair <uint16_t,uint8_t > (params.m_rnti, 0));
  	DlHarqProcessesStatus_t dlHarqPrcStatus;
  	dlHarqPrcStatus.resize (m_phyMacConfig->GetNumHarqProcess (), 0);
  	m_dlHarqProcessesStatus.insert (std::pair <uint16_t, DlHarqProcessesStatus_t> (params.m_rnti, dlHarqPrcStatus));
  	DlHarqProcessesTimer_t dlHarqProcessesTimer;
  	dlHarqProcessesTimer.resize (m_phyMacConfig->GetNumHarqProcess (),0);
  	m_dlHarqProcessesTimer.insert (std::pair <uint16_t, DlHarqProcessesTimer_t> (params.m_rnti, dlHarqProcessesTimer));
  	DlHarqProcessesDciInfoList_t dlHarqTbInfoList;
  	dlHarqTbInfoList.resize (m_phyMacConfig->GetNumHarqProcess ());
  	m_dlHarqProcessesDciInfoMap.insert (std::pair <uint16_t, DlHarqProcessesDciInfoList_t> (params.m_rnti, dlHarqTbInfoList));
  	DlHarqRlcPduList_t dlHarqRlcPduList;
  	dlHarqRlcPduList.resize (m_phyMacConfig->GetNumHarqProcess ());
  	m_dlHarqProcessesRlcPduMap.insert (std::pair <uint16_t, DlHarqRlcPduList_t> (params.m_rnti, dlHarqRlcPduList));
  }

  if (m_ulHarqProcessesStatus.find (params.m_rnti) == m_ulHarqProcessesStatus.end ())
  {
  	//				m_ulHarqCurrentProcessId.insert (std::pair <uint16_t,uint8_t > (rnti, 0));
  	UlHarqProcessesStatus_t ulHarqPrcStatus;
  	ulHarqPrcStatus.resize (m_phyMacConfig->GetNumHarqProcess (), 0);
  	m_ulHarqProcessesStatus.insert (std::pair <uint16_t, UlHarqProcessesStatus_t> (params.m_rnti, ulHarqPrcStatus));
  	UlHarqProcessesTimer_t ulHarqProcessesTimer;
  	ulHarqProcessesTimer.resize (m_phyMacConfig->GetNumHarqProcess (),0);
  	m_ulHarqProcessesTimer.insert (std::pair <uint16_t, UlHarqProcessesTimer_t> (params.m_rnti, ulHarqProcessesTimer));
  	UlHarqProcessesDciInfoList_t ulHarqTbInfoList;
  	ulHarqTbInfoList.resize (m_phyMacConfig->GetNumHarqProcess ());
  	m_ulHarqProcessesDciInfoMap.insert (std::pair <uint16_t, UlHarqProcessesDciInfoList_t> (params.m_rnti, ulHarqTbInfoList));
  }

  // IAB configure if this rnti is an IAB dev or not!
  if(params.m_reconfigureFlag)
  {
  	uint32_t prevNumDevs = 0;
  	auto iterIab = m_rntiIabInfoMap.find(params.m_rnti);
  	if(iterIab != m_rntiIabInfoMap.end())
  	{
  		iterIab->second.first = params.m_ueCapabilities.m_iab;
  		prevNumDevs = iterIab->second.second;
  		iterIab->second.second = params.m_ueCapabilities.m_numIabDevsPerRnti;
  	}
  	else
  	{
  		auto pair = std::make_pair(params.m_rnti, std::make_pair(params.m_ueCapabilities.m_iab, params.m_ueCapabilities.m_numIabDevsPerRnti));
  		m_rntiIabInfoMap.insert(pair);
  	}
  	NS_LOG_DEBUG(this << " Reconfiguration of UE " << params.m_rnti << " iab " << params.m_ueCapabilities.m_iab
  		<< " numDevs " << params.m_ueCapabilities.m_numIabDevsPerRnti << " prevNumDevs " << prevNumDevs);

  	if(params.m_ueCapabilities.m_numIabDevsPerRnti + 1 > (int)m_maxSchedulingDelay) // + 1 since we need at least 1 TTI without no downstream IAB nodes
  	{
  		NS_LOG_WARN(this << " !!! --- !!! Updating m_maxSchedulingDelay from " 
  			<< m_maxSchedulingDelay << " to " << params.m_ueCapabilities.m_numIabDevsPerRnti + 1);
  		m_maxSchedulingDelay = params.m_ueCapabilities.m_numIabDevsPerRnti + 1;
  	}
  }
}

void
MmWaveFlexTtiPfMacScheduler::DoCschedLcConfigReq (const struct MmWaveMacCschedSapProvider::CschedLcConfigReqParameters& params)
{
	NS_LOG_FUNCTION (this);
  std::map <uint16_t, struct UeSchedInfo>::iterator itUe = m_ueSchedInfoMap.find (params.m_rnti);
  if (itUe != m_ueSchedInfoMap.end ())
  {
  	for (uint16_t i = 0; i < params.m_logicalChannelConfigList.size (); i++)
  	{
  		if (params.m_logicalChannelConfigList[i].m_direction ==
  				LogicalChannelConfigListElement_s::DIR_DL)
  		{
    		uint8_t lcid = params.m_logicalChannelConfigList[i].m_logicalChannelIdentity;
    		for (unsigned j = itUe->second.m_flowStatsDl.size (); j <= lcid; j++)
    		{
    			itUe->second.m_flowStatsDl.push_back (FlowStats (false, &(itUe->second), j));
    		}
  			itUe->second.m_flowStatsDl[lcid].m_qci = params.m_logicalChannelConfigList[i].m_qci;
  			if (params.m_logicalChannelConfigList[i].m_qci == EpsBearer::GBR_ULTRA_LOW_LAT)
  			{
  				EpsBearer lowLatBearer (EpsBearer::GBR_ULTRA_LOW_LAT);
  				itUe->second.m_flowStatsDl[lcid].m_deadlineUs = lowLatBearer.GetPacketDelayBudgetMs () * 1000;
  			}
  			m_flowHeap.push_back ( &(itUe->second.m_flowStatsDl[lcid]) );
  		}
  		else if (params.m_logicalChannelConfigList[i].m_direction ==
  				LogicalChannelConfigListElement_s::DIR_UL)
  		{
    		uint8_t lcid = params.m_logicalChannelConfigList[i].m_logicalChannelGroup; // use LCG ID instead of LCID
    		for (unsigned j = itUe->second.m_flowStatsUl.size (); j <= lcid; j++)
    		{
    			itUe->second.m_flowStatsUl.push_back (FlowStats (true, &(itUe->second), j));
    		}
  			itUe->second.m_flowStatsUl[lcid].m_isUplink = true;
  			itUe->second.m_flowStatsUl[lcid].m_qci = params.m_logicalChannelConfigList[i].m_qci;
  			if (params.m_logicalChannelConfigList[i].m_qci == EpsBearer::GBR_ULTRA_LOW_LAT)
  			{
  				EpsBearer lowLatBearer (EpsBearer::GBR_ULTRA_LOW_LAT);
  				itUe->second.m_flowStatsUl[lcid].m_deadlineUs = lowLatBearer.GetPacketDelayBudgetMs () * 1000;
  			}
  			m_flowHeap.push_back ( &(itUe->second.m_flowStatsUl[lcid]) );
  		}
  		else if (params.m_logicalChannelConfigList[i].m_direction ==
  				LogicalChannelConfigListElement_s::DIR_BOTH)
  		{
  			uint8_t lcid = params.m_logicalChannelConfigList[i].m_logicalChannelIdentity;
  			for (unsigned j = itUe->second.m_flowStatsDl.size (); j <= lcid; j++)
  			{
  				itUe->second.m_flowStatsDl.push_back (FlowStats (false, &(itUe->second), j));
  				itUe->second.m_flowStatsUl.push_back (FlowStats (true, &(itUe->second), j));
  			}
  			itUe->second.m_flowStatsDl[lcid].m_qci = params.m_logicalChannelConfigList[i].m_qci;
  			itUe->second.m_flowStatsUl[lcid].m_qci = params.m_logicalChannelConfigList[i].m_qci;

  			if (1 || params.m_logicalChannelConfigList[i].m_qci == EpsBearer::GBR_ULTRA_LOW_LAT)
  			{
  				EpsBearer lowLatBearer (EpsBearer::GBR_ULTRA_LOW_LAT);
  				itUe->second.m_flowStatsDl[lcid].m_deadlineUs = lowLatBearer.GetPacketDelayBudgetMs () * 1000;
  				itUe->second.m_flowStatsUl[lcid].m_deadlineUs = lowLatBearer.GetPacketDelayBudgetMs () * 1000;
  			}

  			m_flowHeap.push_back ( &(itUe->second.m_flowStatsDl[lcid]) );
  			m_flowHeap.push_back ( &(itUe->second.m_flowStatsUl[lcid]) );
  		}
  	}
  }
  else
  {
  	NS_LOG_ERROR ("Cannot find UE info entry");
  }
  return;
}

void
MmWaveFlexTtiPfMacScheduler::DoCschedLcReleaseReq (const struct MmWaveMacCschedSapProvider::CschedLcReleaseReqParameters& params)
{
  NS_LOG_FUNCTION (this);
    for (uint16_t i = 0; i < params.m_logicalChannelIdentity.size (); i++)
    {
     std::list<MmWaveMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator it = m_rlcBufferReq.begin ();
      while (it!=m_rlcBufferReq.end ())
        {
          if (((*it).m_rnti == params.m_rnti)&&((*it).m_logicalChannelIdentity == params.m_logicalChannelIdentity.at (i)))
            {
              it = m_rlcBufferReq.erase (it);
            }
          else
            {
              it++;
            }
        }
    }
  return;
}

void
MmWaveFlexTtiPfMacScheduler::DoCschedUeReleaseReq (const struct MmWaveMacCschedSapProvider::CschedUeReleaseReqParameters& params)
{
  NS_LOG_FUNCTION (this << " Release RNTI " << params.m_rnti);

  //m_dlHarqCurrentProcessId.erase (params.m_rnti);
  m_dlHarqProcessesStatus.erase  (params.m_rnti);
  m_dlHarqProcessesTimer.erase (params.m_rnti);
  m_dlHarqProcessesDciInfoMap.erase  (params.m_rnti);
  m_dlHarqProcessesRlcPduMap.erase  (params.m_rnti);
  //m_ulHarqCurrentProcessId.erase  (params.m_rnti);
  m_ulHarqProcessesTimer.erase (params.m_rnti);
  m_ulHarqProcessesStatus.erase  (params.m_rnti);
  m_ulHarqProcessesDciInfoMap.erase  (params.m_rnti);
  m_ceBsrRxed.erase (params.m_rnti);
  std::list<MmWaveMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator it = m_rlcBufferReq.begin ();
  while (it != m_rlcBufferReq.end ())
    {
      if ((*it).m_rnti == params.m_rnti)
        {
          NS_LOG_INFO (this << " Erase RNTI " << (*it).m_rnti << " LC " << (uint16_t)(*it).m_logicalChannelIdentity);
          it = m_rlcBufferReq.erase (it);
        }
      else
        {
          it++;
        }
    }
  if (m_nextRntiUl == params.m_rnti)
    {
      m_nextRntiUl = 0;
    }

  if (m_nextRntiDl == params.m_rnti)
    {
      m_nextRntiDl = 0;
    }

  return;
}


}


