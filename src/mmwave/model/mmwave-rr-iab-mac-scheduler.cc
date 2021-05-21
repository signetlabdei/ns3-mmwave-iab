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



#include <ns3/log.h>
#include <ns3/abort.h>
#include "mmwave-rr-iab-mac-scheduler.h"
#include <ns3/lte-common.h>
#include <ns3/boolean.h>
#include <ns3/double.h>
#include <stdlib.h>     /* abs */
#include "mmwave-mac-pdu-header.h"
#include "mmwave-mac-pdu-tag.h"
#include "mmwave-spectrum-value-helper.h"
#include <cmath>
#include <sstream>      // std::stringstream
#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmWaveRrIabMacScheduler");

NS_OBJECT_ENSURE_REGISTERED (MmWaveRrIabMacScheduler);

class MmWaveRrIabMacCschedSapProvider : public MmWaveMacCschedSapProvider
{
public:
  MmWaveRrIabMacCschedSapProvider (MmWaveRrIabMacScheduler* scheduler);

  // inherited from MmWaveMacCschedSapProvider
  virtual void CschedCellConfigReq (const struct MmWaveMacCschedSapProvider::CschedCellConfigReqParameters& params);
  virtual void CschedUeConfigReq (const struct MmWaveMacCschedSapProvider::CschedUeConfigReqParameters& params);
  virtual void CschedLcConfigReq (const struct MmWaveMacCschedSapProvider::CschedLcConfigReqParameters& params);
  virtual void CschedLcReleaseReq (const struct MmWaveMacCschedSapProvider::CschedLcReleaseReqParameters& params);
  virtual void CschedUeReleaseReq (const struct MmWaveMacCschedSapProvider::CschedUeReleaseReqParameters& params);

private:
  MmWaveRrIabMacCschedSapProvider ();
  MmWaveRrIabMacScheduler* m_scheduler;
};

MmWaveRrIabMacCschedSapProvider::MmWaveRrIabMacCschedSapProvider ()
{
}

MmWaveRrIabMacCschedSapProvider::MmWaveRrIabMacCschedSapProvider (MmWaveRrIabMacScheduler* scheduler)
	: m_scheduler (scheduler)
{
}

void
MmWaveRrIabMacCschedSapProvider::CschedCellConfigReq (const struct MmWaveMacCschedSapProvider::CschedCellConfigReqParameters& params)
{
  m_scheduler->DoCschedCellConfigReq (params);
}

void
MmWaveRrIabMacCschedSapProvider::CschedUeConfigReq (const struct MmWaveMacCschedSapProvider::CschedUeConfigReqParameters& params)
{
  m_scheduler->DoCschedUeConfigReq (params);
}


void
MmWaveRrIabMacCschedSapProvider::CschedLcConfigReq (const struct MmWaveMacCschedSapProvider::CschedLcConfigReqParameters& params)
{
  m_scheduler->DoCschedLcConfigReq (params);
}

void
MmWaveRrIabMacCschedSapProvider::CschedLcReleaseReq (const struct MmWaveMacCschedSapProvider::CschedLcReleaseReqParameters& params)
{
  m_scheduler->DoCschedLcReleaseReq (params);
}

void
MmWaveRrIabMacCschedSapProvider::CschedUeReleaseReq (const struct MmWaveMacCschedSapProvider::CschedUeReleaseReqParameters& params)
{
  m_scheduler->DoCschedUeReleaseReq (params);
}

class MmWaveRrIabMacSchedSapProvider : public MmWaveMacSchedSapProvider
{
public:
	MmWaveRrIabMacSchedSapProvider (MmWaveRrIabMacScheduler* sched);

	virtual void SchedDlRlcBufferReq (const struct MmWaveMacSchedSapProvider::SchedDlRlcBufferReqParameters& params);
	virtual void SchedTriggerReq (const struct MmWaveMacSchedSapProvider::SchedTriggerReqParameters& params);
	virtual void SchedDlCqiInfoReq (const struct MmWaveMacSchedSapProvider::SchedDlCqiInfoReqParameters& params);
	virtual void SchedUlCqiInfoReq (const struct MmWaveMacSchedSapProvider::SchedUlCqiInfoReqParameters& params);
	virtual void SchedUlMacCtrlInfoReq (const struct MmWaveMacSchedSapProvider::SchedUlMacCtrlInfoReqParameters& params);
	virtual void SchedSetMcs (int mcs);
private:
  MmWaveRrIabMacSchedSapProvider ();
	MmWaveRrIabMacScheduler* m_scheduler;
};

MmWaveRrIabMacSchedSapProvider::MmWaveRrIabMacSchedSapProvider ()
{
}

MmWaveRrIabMacSchedSapProvider::MmWaveRrIabMacSchedSapProvider (MmWaveRrIabMacScheduler* sched)
	: m_scheduler(sched)
{
}

void
MmWaveRrIabMacSchedSapProvider::SchedDlRlcBufferReq (const struct MmWaveMacSchedSapProvider::SchedDlRlcBufferReqParameters& params)
{
  	m_scheduler->DoSchedDlRlcBufferReq (params);
}

void
MmWaveRrIabMacSchedSapProvider::SchedTriggerReq (const struct MmWaveMacSchedSapProvider::SchedTriggerReqParameters& params)
{
	m_scheduler->DoSchedTriggerReq(params);
}

void
MmWaveRrIabMacSchedSapProvider::SchedDlCqiInfoReq (const struct MmWaveMacSchedSapProvider::SchedDlCqiInfoReqParameters& params)
{
	m_scheduler->DoSchedDlCqiInfoReq (params);
}

void
MmWaveRrIabMacSchedSapProvider::SchedUlCqiInfoReq (const struct MmWaveMacSchedSapProvider::SchedUlCqiInfoReqParameters& params)
{
	m_scheduler->DoSchedUlCqiInfoReq (params);
}

void
MmWaveRrIabMacSchedSapProvider::SchedUlMacCtrlInfoReq (const struct MmWaveMacSchedSapProvider::SchedUlMacCtrlInfoReqParameters& params)
{
	m_scheduler->DoSchedUlMacCtrlInfoReq (params);
}

void
MmWaveRrIabMacSchedSapProvider::SchedSetMcs (int mcs)
{
	m_scheduler->DoSchedSetMcs (mcs);
}

const unsigned MmWaveRrIabMacScheduler::m_macHdrSize = 0;
const unsigned MmWaveRrIabMacScheduler::m_subHdrSize = 4;
const unsigned MmWaveRrIabMacScheduler::m_rlcHdrSize = 3;

const double MmWaveRrIabMacScheduler::m_berDl = 0.001;

MmWaveRrIabMacScheduler::MmWaveRrIabMacScheduler ()
	: m_nextRnti (0),
	m_subframeNo (0),
	m_tbUid (0),
	m_macSchedSapUser (0),
	m_macCschedSapUser (0),
	m_maxSchedulingDelay (1),
	m_iabScheduler (false),
	m_iabDonorScheduler (false),
	m_split (true),
	m_etaIab (0),
	m_suggestedImsi (0),
	m_representativeUeImsi (0xFFFFFFFF)
{
	NS_LOG_FUNCTION (this);
	m_macSchedSapProvider = new MmWaveRrIabMacSchedSapProvider (this);
	m_macCschedSapProvider = new MmWaveRrIabMacCschedSapProvider (this);
	m_iabBackahulSapProvider = new MemberMmWaveUeMacCschedSapProvider<MmWaveRrIabMacScheduler> (this);
	m_schedInfoRcvCallback = MakeCallback (&MmWaveRrIabMacScheduler::ReceiveIabSchedIndication, this);
	m_iabBusySubframeAllocation.clear();	
}

MmWaveRrIabMacScheduler::~MmWaveRrIabMacScheduler ()
{
	NS_LOG_FUNCTION (this);
}

void
MmWaveRrIabMacScheduler::DoDispose (void)
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
  	m_iabBusySubframeAllocation.clear ();
  	m_bsrMap.clear ();
  	delete m_macCschedSapProvider;
  	delete m_macSchedSapProvider;
}

TypeId
MmWaveRrIabMacScheduler::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::MmWaveRrIabMacScheduler")
	.SetParent<MmWaveMacScheduler> ()
	.AddConstructor<MmWaveRrIabMacScheduler> ()
    .AddAttribute ("CqiTimerThreshold",
                   "The number of TTIs a CQI is valid (default 1000 - 1 sec.)",
                   UintegerValue (100),
                   MakeUintegerAccessor (&MmWaveRrIabMacScheduler::m_cqiTimersThreshold),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("HarqEnabled",
                   "Activate/Deactivate the HARQ [by default is active].",
                   BooleanValue (true),
                   MakeBooleanAccessor (&MmWaveRrIabMacScheduler::m_harqOn),
                   MakeBooleanChecker ())
	 .AddAttribute ("FixedMcsDl",
					"Fix MCS to value set in McsDlDefault (for testing)",
					BooleanValue (false),
					MakeBooleanAccessor (&MmWaveRrIabMacScheduler::m_fixedMcsDl),
					MakeBooleanChecker ())
	.AddAttribute ("McsDefaultDl",
					"Fixed DL MCS (for testing)",
					UintegerValue (1),
					MakeUintegerAccessor (&MmWaveRrIabMacScheduler::m_mcsDefaultDl),
					MakeUintegerChecker<uint8_t> ())
	 .AddAttribute ("FixedMcsUl",
					"Fix MCS to value set in McsUlDefault (for testing)",
					BooleanValue (false),
					MakeBooleanAccessor (&MmWaveRrIabMacScheduler::m_fixedMcsUl),
					MakeBooleanChecker ())
	.AddAttribute ("McsDefaultUl",
					"Fixed UL MCS (for testing)",
					UintegerValue (1),
					MakeUintegerAccessor (&MmWaveRrIabMacScheduler::m_mcsDefaultUl),
					MakeUintegerChecker<uint8_t> ())
	 .AddAttribute ("DlSchedOnly",
					"Only schedule downlink traffic (for testing)",
					BooleanValue (false),
					MakeBooleanAccessor (&MmWaveRrIabMacScheduler::m_dlOnly),
					MakeBooleanChecker ())
	 .AddAttribute ("UlSchedOnly",
					"Only schedule uplink traffic (for testing)",
					BooleanValue (false),
					MakeBooleanAccessor (&MmWaveRrIabMacScheduler::m_ulOnly),
					MakeBooleanChecker ())
	 .AddAttribute ("FixedTti",
					"Fix slot size",
					BooleanValue (false),
					MakeBooleanAccessor (&MmWaveRrIabMacScheduler::m_fixedTti),
					MakeBooleanChecker ())
	.AddAttribute ("SymPerSlot",
					"Number of symbols per slot in Fixed TTI mode",
					UintegerValue (6),
					MakeUintegerAccessor (&MmWaveRrIabMacScheduler::m_symPerSlot),
					MakeUintegerChecker<uint8_t> ())
	.AddAttribute ("EtaIab",
					"Balancing factor to assign more resources to IAB nodes",
					DoubleValue(0.0),
					MakeDoubleAccessor (&MmWaveRrIabMacScheduler::m_etaIab),
					MakeDoubleChecker<double> (0.0, 1.0)
					)
		;

	return tid;
}

void
MmWaveRrIabMacScheduler::SetMacSchedSapUser (MmWaveMacSchedSapUser* sap)
{
	m_macSchedSapUser = sap;
}

void
MmWaveRrIabMacScheduler::SetMacCschedSapUser (MmWaveMacCschedSapUser* sap)
{
	m_macCschedSapUser = sap;
}

void
MmWaveRrIabMacScheduler::SetIabScheduler(bool iabScheduler)
{
	m_iabScheduler = iabScheduler;
}

void
MmWaveRrIabMacScheduler::SetIabDonorScheduler(bool iabDonorScheduler)
{
	m_iabDonorScheduler = iabDonorScheduler;
}

void 
MmWaveRrIabMacScheduler::SetIabBsrMapReportCallback(BsrReportCallback infoSendCallback)
{
	NS_ASSERT (!infoSendCallback.IsNull ());
	m_bsrSendCallback = infoSendCallback;
}

void 
MmWaveRrIabMacScheduler::SetIabCqiMapReportCallback(CqiReportCallback infoSendCallback)
{
	NS_ASSERT (!infoSendCallback.IsNull ());
	m_cqiSendCallback = infoSendCallback;
}

void 
MmWaveRrIabMacScheduler::SetEnbApplication(Ptr<EpcEnbApplication> enbApp)
{
	m_enbApp = enbApp;
}

void 
MmWaveRrIabMacScheduler::SetIabApplication(Ptr<EpcIabApplication> iabApp)
{
	m_iabApp = iabApp;
	m_representativeUeImsi = 0xFFFFFFFF;
	if (!m_iabDonorScheduler)
	{
		m_representativeUeImsi = m_representativeUeImsi - m_iabApp->GetImsi ();
	}
}

MmWaveMacSchedSapProvider*
MmWaveRrIabMacScheduler::GetMacSchedSapProvider ()
{
	return m_macSchedSapProvider;
}

MmWaveMacCschedSapProvider*
MmWaveRrIabMacScheduler::GetMacCschedSapProvider ()
{
	return m_macCschedSapProvider;
}

// IAB methods
void
MmWaveRrIabMacScheduler::SetMmWaveUeMacCschedSapProvider(MmWaveUeMacCschedSapProvider* sap)
{
	m_iabBackahulSapProvider = sap;
}

MmWaveUeMacCschedSapProvider*
MmWaveRrIabMacScheduler::GetMmWaveUeMacCschedSapProvider()
{
	return m_iabBackahulSapProvider;
}

std::string
MmWaveRrIabMacScheduler::PrintSubframeAllocationMask(std::vector<bool> mask)
{
	std::stringstream strStream;
	for(auto bit : mask)
		strStream << bit << " ";
	return strStream.str();
}

// IAB methods
void
MmWaveRrIabMacScheduler::DoIabBackhaulSchedNotify(const struct MmWaveUeMacCschedSapProvider::IabBackhaulSchedInfo& info)
{
	NS_LOG_FUNCTION(this << info.m_dciInfoElementTdma.m_rnti);

	NS_ASSERT_MSG(m_iabScheduler, "Received DCI info for backhaul on a non IAB scheduler");

	NS_LOG_DEBUG("MmWaveRrIabMacScheduler received DCIs for the backhaul link");

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
MmWaveRrIabMacScheduler::ConfigureCommonParameters (Ptr<MmWavePhyMacCommon> config)
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
MmWaveRrIabMacScheduler::DoSchedDlRlcBufferReq (const struct MmWaveMacSchedSapProvider::SchedDlRlcBufferReqParameters& params)
{
  NS_LOG_FUNCTION (this << params.m_rnti << (uint32_t) params.m_logicalChannelIdentity);
  // API generated by RLC for updating RLC parameters on a LC (tx and retx queues)
  std::list<MmWaveMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator it = m_rlcBufferReq.begin ();
  bool newLc = true;
  while (it != m_rlcBufferReq.end ())
    {
      // remove old entries of this UE-LC
      if (((*it).m_rnti == params.m_rnti)&&((*it).m_logicalChannelIdentity == params.m_logicalChannelIdentity))
        {
          it = m_rlcBufferReq.erase (it);
          newLc = false;
        }
      else
        {
          ++it;
        }
    }
  // add the new parameters
  m_rlcBufferReq.insert (it, params);
  NS_LOG_DEBUG ("BSR for RNTI " << params.m_rnti << " LC " << (uint16_t)params.m_logicalChannelIdentity << " RLC tx size " << params.m_rlcTransmissionQueueSize << " RLC retx size " << params.m_rlcRetransmissionQueueSize << " RLC stat size " <<  params.m_rlcStatusPduSize);
  // initialize statistics of the flow in case of new flows
  if (newLc == true)
  {
  	m_wbCqiRxed.insert ( std::pair<uint16_t, uint8_t > (params.m_rnti, 1)); // only codeword 0 at this stage (SISO)
  	// initialized to 1 (i.e., the lowest value for transmitting a signal)
  	m_wbCqiTimers.insert ( std::pair<uint16_t, uint32_t > (params.m_rnti, m_cqiTimersThreshold));
  }
}

void
MmWaveRrIabMacScheduler::DoSchedDlCqiInfoReq (const struct MmWaveMacSchedSapProvider::SchedDlCqiInfoReqParameters& params)
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
MmWaveRrIabMacScheduler::DoSchedUlCqiInfoReq (const struct MmWaveMacSchedSapProvider::SchedUlCqiInfoReqParameters& params)
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
				NS_LOG_DEBUG (this << " UL CQI Does not find info on allocation, size : " << m_ulAllocationMap.size ());
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
MmWaveRrIabMacScheduler::RefreshHarqProcesses ()
{
	NS_LOG_FUNCTION (this);

	std::map <uint16_t, DlHarqProcessesTimer_t>::iterator itTimers;
	for (itTimers = m_dlHarqProcessesTimer.begin (); itTimers != m_dlHarqProcessesTimer.end (); itTimers++)
	{
		for (uint16_t i = 0; i < m_phyMacConfig->GetNumHarqProcess (); i++)
		{
			if ((*itTimers).second.at (i) == m_phyMacConfig->GetHarqTimeout ())
			{ // reset HARQ process
				NS_LOG_INFO (this << " Reset HARQ proc " << i << " for RNTI " << (*itTimers).first);
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
MmWaveRrIabMacScheduler::UpdateDlHarqProcessId (uint16_t rnti)
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
MmWaveRrIabMacScheduler::UpdateUlHarqProcessId (uint16_t rnti)
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
	std::map <uint16_t, UlHarqProcessesStatus_t>::iterator itStat = m_ulHarqProcessesStatus.find (rnti);
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
}

unsigned MmWaveRrIabMacScheduler::CalcMinTbSizeNumSym (unsigned mcs, unsigned bufSize, unsigned &tbSize)
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

// IAB functionality
uint16_t
MmWaveRrIabMacScheduler::GetNumIabRnti()
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

int
MmWaveRrIabMacScheduler::UpdateBusySymbolsForIab(uint8_t sfNum, uint8_t symIdx, int symAvail)
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
MmWaveRrIabMacScheduler::DoSchedTriggerReq (const struct MmWaveMacSchedSapProvider::SchedTriggerReqParameters& params)
{
	NS_LOG_DEBUG("m_rntiIabInfoMap size " << m_rntiIabInfoMap.size());
	if(m_iabDonorScheduler)
	{
		m_donorControllerPtr->SubframeIndication ();
	}

	// Generate BSR map and send it to the central controller
	m_bsrMap = GenCumulativeBsrMap();
	if(!m_bsrSendCallback.IsNull())
	{
		//NS_LOG_DEBUG ("CALLBACK CAN BE ACTIVATED!");
		uint64_t imsiId {};
		if (m_iabDonorScheduler)
		{
			imsiId = 0; // Dummy IMSI for the donor
		}
		else
		{
			imsiId = m_iabApp->GetImsi ();
		}
		m_bsrSendCallback(m_bsrMap, imsiId);
	}

	// Clear previous scheduling info
	m_currSchedConfig = {};
	m_ueInfo.clear ();

	// Set Sfn counters
	m_schedSfn = params.m_sfnSf;
	if ((int)(m_schedSfn.m_sfNum + m_maxSchedulingDelay) >=  (int)m_phyMacConfig->GetSubframesPerFrame ())
	{
		m_schedSfn.m_frameNum++;
	}
	m_schedSfn.m_sfNum = (uint8_t)((int)(m_schedSfn.m_sfNum + m_maxSchedulingDelay)) % m_phyMacConfig->GetSubframesPerFrame ();
	m_currSchedConfig.m_sfnSf = m_schedSfn;
	m_currSchedConfig.m_sfAllocInfo.m_sfnSf = m_currSchedConfig.m_sfnSf;

	uint16_t frameNum = m_currSchedConfig.m_sfnSf.m_frameNum;
	uint8_t	sfNum = m_currSchedConfig.m_sfnSf.m_sfNum;

	NS_LOG_DEBUG ("Scheduling frame "<< (unsigned)frameNum << " subframe " << (unsigned)sfNum);

	// TODOIAB in this version we only consider the same slot for UL & DL 

	// Add slot for DL control
	SlotAllocInfo dlCtrlSlot (0, SlotAllocInfo::DL_slotAllocInfo, SlotAllocInfo::CTRL, SlotAllocInfo::DIGITAL, 0);
	dlCtrlSlot.m_dci.m_numSym = 1;
	dlCtrlSlot.m_dci.m_symStart = 0;
	m_currSchedConfig.m_sfAllocInfo.m_slotAllocInfo.push_back (dlCtrlSlot);
	int resvCtrl = m_phyMacConfig->GetDlCtrlSymbols() + m_phyMacConfig->GetUlCtrlSymbols();
	m_symAvail = m_phyMacConfig->GetSymbolsPerSubframe () - resvCtrl;
	m_slotIdx = 1; // index used to store SlotAllocInfo
	m_symIdx = m_phyMacConfig->GetDlCtrlSymbols(); // symbols reserved for control at beginning of subframe

	// Get the resources which are already set as busy
	m_symAvail = UpdateBusySymbolsForIab(sfNum, m_symIdx, m_symAvail);
	
	// Process received CQIs
	RefreshDlCqiMaps ();
	RefreshUlCqiMaps ();

	// Generate CQI map and send it to the central controller
	m_cqiMap = GenCumulativeCqiMap ();
	if(!m_cqiSendCallback.IsNull())
	{
		uint64_t imsiId {};
		if (m_iabDonorScheduler)
		{
			imsiId = 0; // Dummy IMSI for the donor
		}
		else
		{
			imsiId = m_iabApp->GetImsi ();
		}
		m_cqiSendCallback(m_cqiMap, imsiId);
	}

	NS_LOG_DEBUG ("CQI maps sent to the controller!");
	// Process DL HARQ feedback
	RefreshHarqProcesses ();

	// TODO: insert IAB centralized operations HERE

	if ((int)m_symIdx >= (int)(m_phyMacConfig->GetSymbolsPerSubframe () - m_phyMacConfig->GetUlCtrlSymbols ()))
	{
		// Add slot for UL control
		SlotAllocInfo ulCtrlSlot (0xFF, SlotAllocInfo::UL_slotAllocInfo, SlotAllocInfo::CTRL, SlotAllocInfo::DIGITAL, 0);
		ulCtrlSlot.m_dci.m_numSym = 1;
		ulCtrlSlot.m_dci.m_symStart = m_phyMacConfig->GetSymbolsPerSubframe()-1;
		m_currSchedConfig.m_sfAllocInfo.m_slotAllocInfo.push_back (ulCtrlSlot);
		m_macSchedSapUser->SchedConfigInd (m_currSchedConfig);
		return;
	}

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

	// Split the UL and DL info into different lists, one for the user(s) indicated by the controller, one of the remaining RNTIs
	std::vector <DlHarqInfo> favoredDlHarqInfoList {}, leftoverDlHarqInfoList {}; // HARQ DL retx 
	std::vector <UlHarqInfo> favoredUlHarqInfoList {}, leftoverUlHarqInfoList {}; // HARQ UL retx
	std::list <MmWaveMacSchedSapProvider::SchedDlRlcBufferReqParameters> favoredDlInfoList  {}, leftoverDlInfoList  {};	// DL new data
	std::map <uint16_t, BufferSizeStatusSize_t> favoredUlInfoList {}, leftoverUlInfoList {};	// UL new data

	std::set<uint16_t> favoredRntis {};
	NS_LOG_DEBUG ("Suggested IMSI is: " << m_suggestedImsi);
	// Suggested IMSI = 0 means that we have not received any scheduling indication!
	if (m_suggestedImsi != 0 && m_suggestedImsi != m_representativeUeImsi)
	{
		// Favored RNTI corresponds to an IAB node
		uint16_t suggestedRnti {};
		if (m_iabDonorScheduler)
		{	
			suggestedRnti = m_enbApp->GetLocalRntiFromImsi (m_suggestedImsi);
			if (suggestedRnti != 0)
			{
				NS_LOG_DEBUG ("Found IMSI: " << m_suggestedImsi);
			}
		}
		else
		{
			suggestedRnti = m_iabApp->GetLocalRntiFromImsi (m_suggestedImsi);
			if (suggestedRnti != 0)
			{
				NS_LOG_DEBUG ("Found IMSI: " << m_suggestedImsi);
			}
		}
		favoredRntis.insert (suggestedRnti);
	}
	// We simply favor UEs, obtain their list
	else if (m_suggestedImsi != 0 && m_iabDonorScheduler) 
	{
		favoredRntis = m_enbApp->GetSetUeRntis ();
	}
	else if (m_suggestedImsi != 0)
	{
		favoredRntis = m_iabApp->GetSetUeRntis ();
	}

	// Split the various info accordingly
	for (auto dlHarqElem : m_dlHarqInfoList)
	{
		if (favoredRntis.find(dlHarqElem.m_rnti) != favoredRntis.end ())
		{
			favoredDlHarqInfoList.push_back (dlHarqElem);
		}
		else
		{
			leftoverDlHarqInfoList.push_back (dlHarqElem);
		}	
	}
	for (auto ulHarqElem : m_ulHarqInfoList)
	{
		if (favoredRntis.find(ulHarqElem.m_rnti) != favoredRntis.end ())
		{
			favoredUlHarqInfoList.push_back (ulHarqElem);
		}
		else
		{
			leftoverUlHarqInfoList.push_back (ulHarqElem);
		}
	}
	for (auto dlElem : m_rlcBufferReq)
	{
		if (favoredRntis.find(dlElem.m_rnti) != favoredRntis.end ())
		{
			favoredDlInfoList.push_back (dlElem);
		}
		else
		{
			leftoverDlInfoList.push_back (dlElem);
		}
	}
	for (auto ulElem : m_ceBsrRxed)
	{
		if (favoredRntis.find(ulElem.first) != favoredRntis.end ())
		{
			favoredUlInfoList.insert (ulElem);
		}
		else
		{
			leftoverUlInfoList.insert (ulElem);
		}
	}

	// Schedule favored first
	if (m_harqOn == false)		
	{
		// Ignore HARQ feedback
		m_dlHarqInfoList.clear ();
	}
	else
	{
		// Schedule the HARQ data
		PerformHarqScheduling (favoredDlHarqInfoList, favoredUlHarqInfoList);
		NS_LOG_DEBUG ("First HARQ scheduling completed!");
	}
	PerformNonHarqScheduling (favoredDlInfoList, favoredUlInfoList);
	NS_LOG_DEBUG ("First non-HARQ scheduling completed!");

	// Schedule the remaining users
	if (m_harqOn != false)		
	{
		// Schedule the HARQ data
		PerformHarqScheduling (leftoverDlHarqInfoList, leftoverUlHarqInfoList);
		NS_LOG_DEBUG ("Second HARQ scheduling completed!");
	}
	PerformNonHarqScheduling (leftoverDlInfoList, leftoverUlInfoList);
	NS_LOG_DEBUG ("Second non-HARQ scheduling completed!");

	
	// Update DL HARQ info
	m_dlHarqInfoList.clear ();
	m_dlHarqInfoList = m_dlInfoListUntxed;
	m_dlInfoListUntxed.clear ();

	// Update UL HARQ info
	m_ulHarqInfoList.clear ();
	m_ulHarqInfoList = m_ulInfoListUntxed;
	m_ulInfoListUntxed.clear ();

	//NS_LOG_DEBUG("After HARQ allocation symAvail " << m_symAvail << " mask " << PrintSubframeAllocationMask(m_busyResourcesSchedSubframe.m_symAllocationMask));

	// Add slot for UL control
	SlotAllocInfo ulCtrlSlot (0xFF, SlotAllocInfo::UL_slotAllocInfo, SlotAllocInfo::CTRL, SlotAllocInfo::DIGITAL, 0);
	ulCtrlSlot.m_dci.m_numSym = 1;
	ulCtrlSlot.m_dci.m_symStart = m_phyMacConfig->GetSymbolsPerSubframe()-1;
	m_currSchedConfig.m_sfAllocInfo.m_slotAllocInfo.push_back (ulCtrlSlot);

	std::sort(m_currSchedConfig.m_sfAllocInfo.m_slotAllocInfo.begin(), m_currSchedConfig.m_sfAllocInfo.m_slotAllocInfo.end(), BySymIndex());

	m_macSchedSapUser->SchedConfigInd (m_currSchedConfig);

	m_suggestedImsi = 0;
	return;
}

void
MmWaveRrIabMacScheduler::PerformHarqScheduling (std::vector <DlHarqInfo> dlHarqInfoList, std::vector <UlHarqInfo> ulHarqInfoList)
{
	// Process DL HARQ feedback and assign slots for RETX if resources available
	for (unsigned i = 0; i < dlHarqInfoList.size (); i++)
	{
		if (m_symAvail == 0)
		{
			break;	// no symbols left to allocate
		}
		uint8_t harqId = dlHarqInfoList.at (i).m_harqProcessId;
		uint16_t rnti = dlHarqInfoList.at (i).m_rnti;
		m_itUeInfo = m_ueInfo.find (rnti); // at the beginning, the ueInfo map is empty
		std::map <uint16_t, DlHarqProcessesStatus_t>::iterator itStat = m_dlHarqProcessesStatus.find (rnti);
		if (itStat == m_dlHarqProcessesStatus.end ())
		{
			NS_FATAL_ERROR ("No HARQ status info found for UE " << rnti);
		}
		std::map <uint16_t, DlHarqRlcPduList_t>::iterator itRlcPdu =  m_dlHarqProcessesRlcPduMap.find (rnti);
		if (itRlcPdu == m_dlHarqProcessesRlcPduMap.end ())
		{
			NS_FATAL_ERROR ("Unable to find RlcPdcList in HARQ buffer for RNTI " << dlHarqInfoList.at (i).m_rnti);
		}
		if(dlHarqInfoList.at (i).m_harqStatus == DlHarqInfo::ACK || itStat->second.at (harqId) == 0)
		{ 
			// Acknowledgment or process timeout, reset process
			NS_LOG_DEBUG ("UE" << rnti << " DL harqId " << (unsigned)harqId << " HARQ-ACK received");
			itStat->second.at (harqId) = 0;    // release process ID
			for (uint16_t k = 0; k < itRlcPdu->second.size (); k++)		// clear RLC buffers
			{
				itRlcPdu->second.at (harqId).clear ();
			}
			continue;
		}
		else if(dlHarqInfoList.at (i).m_harqStatus == DlHarqInfo::NACK)
		{
			std::map <uint16_t, DlHarqProcessesDciInfoList_t>::iterator itHarq = m_dlHarqProcessesDciInfoMap.find (rnti);
			if (itHarq == m_dlHarqProcessesDciInfoMap.end ())
			{
				NS_FATAL_ERROR ("No DCI/HARQ buffer entry found for UE " << rnti);
			}
			DciInfoElementTdma dciInfoReTx = itHarq->second.at (harqId);
			NS_LOG_DEBUG ("UE" << rnti << " DL harqId " << (unsigned)harqId << " HARQ-NACK received, rv " << (unsigned)dciInfoReTx.m_rv);
			NS_ASSERT (harqId == dciInfoReTx.m_harqProcess);
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

			// Allocate retx if enough symbols are available
			if (m_symAvail >= dciInfoReTx.m_numSym)
			{
				if(!CheckOverlapWithBusyResources(m_symIdx, dciInfoReTx.m_numSym)) // TODOIAB: check if it can be allocated later
				{
					m_symAvail -= dciInfoReTx.m_numSym;
					dciInfoReTx.m_symStart = m_symIdx;
					m_symIdx += dciInfoReTx.m_numSym;
					NS_ASSERT (m_symIdx <= m_phyMacConfig->GetSymbolsPerSubframe () - m_phyMacConfig->GetUlCtrlSymbols ());
					dciInfoReTx.m_rv++;
					dciInfoReTx.m_ndi = 0;
					itHarq->second.at (harqId) = dciInfoReTx;
					itStat->second.at (harqId) = itStat->second.at (harqId) + 1;
					SlotAllocInfo slotInfo (m_slotIdx++, SlotAllocInfo::DL_slotAllocInfo, SlotAllocInfo::CTRL_DATA, SlotAllocInfo::DIGITAL, m_itUeInfo->first);
					NS_LOG_DEBUG("itUeInfo->first " << m_itUeInfo->first << " dci rnti " << dciInfoReTx.m_rnti);
					slotInfo.m_dci = dciInfoReTx;
					NS_LOG_DEBUG ("UE" << dciInfoReTx.m_rnti << " gets DL slots " << (unsigned)dciInfoReTx.m_symStart << "-" << (unsigned)(dciInfoReTx.m_symStart+dciInfoReTx.m_numSym-1) <<
											" tbs " << dciInfoReTx.m_tbSize << " harqId " << (unsigned)dciInfoReTx.m_harqProcess << " harqId " << (unsigned)dciInfoReTx.m_harqProcess <<
											" rv " << (unsigned)dciInfoReTx.m_rv << " in frame " << m_currSchedConfig.m_sfnSf.m_frameNum << " subframe " << (unsigned)m_currSchedConfig.m_sfnSf.m_sfNum << " RETX");
					std::map <uint16_t, DlHarqRlcPduList_t>::iterator itRlcList =  m_dlHarqProcessesRlcPduMap.find (rnti);
					if (itRlcList == m_dlHarqProcessesRlcPduMap.end ())
					{
						NS_FATAL_ERROR ("Unable to find RlcPdcList in HARQ buffer for RNTI " << rnti);
					}
					for (uint16_t k = 0; k < (*itRlcList).second.at(dciInfoReTx.m_harqProcess).size (); k++)
					{
						slotInfo.m_rlcPduInfo.push_back ((*itRlcList).second.at (dciInfoReTx.m_harqProcess).at (k));
					}
					m_currSchedConfig.m_sfAllocInfo.m_slotAllocInfo.push_back (slotInfo);
					m_currSchedConfig.m_sfAllocInfo.m_numSymAlloc += dciInfoReTx.m_numSym;
					if (m_itUeInfo == m_ueInfo.end())
					{
						m_itUeInfo = m_ueInfo.insert (std::pair<uint16_t, struct UeSchedInfo> (rnti, UeSchedInfo () )).first;
					}
					m_itUeInfo->second.m_dlSymbolsRetx = dciInfoReTx.m_numSym;
				}
				else
				{
					// Find out whether there are other resources later, keeping in mind that we cannot split RETX
					int numSymNeeded = dciInfoReTx.m_numSym;
					int numFreeSymbols = 0;
					uint8_t tmpSymIdx = m_symIdx;

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
					if(numFreeSymbols >= numSymNeeded && (int)(tmpSymIdx + numSymNeeded - 1) < (int)(m_phyMacConfig->GetSymbolsPerSubframe () - m_phyMacConfig->GetUlCtrlSymbols ()))
					{
						NS_LOG_LOGIC("Found resources for DL HARQ RETX");
						m_symAvail -= dciInfoReTx.m_numSym;
						dciInfoReTx.m_symStart = tmpSymIdx;

						NS_LOG_LOGIC("Before updating " << PrintSubframeAllocationMask(m_busyResourcesSchedSubframe.m_symAllocationMask));

						// update resource mask
						UpdateResourceMask(tmpSymIdx, dciInfoReTx.m_numSym);

						NS_LOG_LOGIC("After updating " << PrintSubframeAllocationMask(m_busyResourcesSchedSubframe.m_symAllocationMask));

						// symIdx += dciInfoReTx.m_numSym;
						NS_ASSERT ((int)(tmpSymIdx + dciInfoReTx.m_numSym - 1) < (int)(m_phyMacConfig->GetSymbolsPerSubframe () - m_phyMacConfig->GetUlCtrlSymbols ()));
						dciInfoReTx.m_rv++;
						dciInfoReTx.m_ndi = 0;
						itHarq->second.at (harqId) = dciInfoReTx;
						itStat->second.at (harqId) = itStat->second.at (harqId) + 1;
						SlotAllocInfo slotInfo (m_slotIdx++, SlotAllocInfo::DL_slotAllocInfo, SlotAllocInfo::CTRL_DATA, SlotAllocInfo::DIGITAL, m_itUeInfo->first);
						NS_LOG_LOGIC("itUeInfo->first " << m_itUeInfo->first << " dci rnti " << dciInfoReTx.m_rnti);
						slotInfo.m_dci = dciInfoReTx;
						NS_LOG_DEBUG ("UE" << dciInfoReTx.m_rnti << " gets DL slots " << (unsigned)dciInfoReTx.m_symStart << "-" << (unsigned)(dciInfoReTx.m_symStart+dciInfoReTx.m_numSym-1) <<
												" tbs " << dciInfoReTx.m_tbSize << " harqId " << (unsigned)dciInfoReTx.m_harqProcess << " harqId " << (unsigned)dciInfoReTx.m_harqProcess <<
												" rv " << (unsigned)dciInfoReTx.m_rv << " in frame " << m_currSchedConfig.m_sfnSf.m_frameNum << " subframe " << (unsigned)m_currSchedConfig.m_sfnSf.m_sfNum << " RETX");
						std::map <uint16_t, DlHarqRlcPduList_t>::iterator itRlcList =  m_dlHarqProcessesRlcPduMap.find (rnti);
						if (itRlcList == m_dlHarqProcessesRlcPduMap.end ())
						{
							NS_FATAL_ERROR ("Unable to find RlcPdcList in HARQ buffer for RNTI " << rnti);
						}
						for (uint16_t k = 0; k < (*itRlcList).second.at(dciInfoReTx.m_harqProcess).size (); k++)
						{
							slotInfo.m_rlcPduInfo.push_back ((*itRlcList).second.at (dciInfoReTx.m_harqProcess).at (k));
						}
						m_currSchedConfig.m_sfAllocInfo.m_slotAllocInfo.push_back (slotInfo);
						m_currSchedConfig.m_sfAllocInfo.m_numSymAlloc += dciInfoReTx.m_numSym;
						if (m_itUeInfo == m_ueInfo.end())
						{
							m_itUeInfo = m_ueInfo.insert (std::pair<uint16_t, struct UeSchedInfo> (rnti, UeSchedInfo () )).first;
						}
						m_itUeInfo->second.m_dlSymbolsRetx = dciInfoReTx.m_numSym;
					}
					else
					{
						NS_LOG_DEBUG ("No resource for this retx (even later) -> buffer it");
						m_dlInfoListUntxed.push_back (dlHarqInfoList.at (i));
					}
				}
			}
			else
			{
				NS_LOG_DEBUG ("No resource for this retx -> buffer it");
				m_dlInfoListUntxed.push_back (dlHarqInfoList.at (i));
			}
		}
	}

	// Process UL HARQ feedback
	for (uint16_t i = 0; i < ulHarqInfoList.size (); i++)
	{
		if (m_symAvail == 0)
		{
			break;	// no symbols left to allocate
		}
		UlHarqInfo harqInfo = ulHarqInfoList.at (i);
		uint8_t harqId = harqInfo.m_harqProcessId;
		uint16_t rnti = harqInfo.m_rnti;
		m_itUeInfo = m_ueInfo.find (rnti);
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
			NS_LOG_DEBUG ("UE" << rnti << " UL harqId " << (unsigned)harqId << " TX not ok, rv" << (unsigned)dciInfoReTx.m_rv);
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

			NS_LOG_DEBUG("Num sym needed " << (uint16_t)dciInfoReTx.m_numSym << " symIdx " << (uint16_t)m_symIdx);
			if (m_symAvail >= dciInfoReTx.m_numSym)
			{
				if(!CheckOverlapWithBusyResources(m_symIdx, dciInfoReTx.m_numSym)) // TODOIAB: check if it can be allocated later
				{
					m_symAvail -= dciInfoReTx.m_numSym;
					dciInfoReTx.m_symStart = m_symIdx;
					m_symIdx += dciInfoReTx.m_numSym;
					NS_ASSERT (m_symIdx <= m_phyMacConfig->GetSymbolsPerSubframe () - m_phyMacConfig->GetUlCtrlSymbols ());
					dciInfoReTx.m_rv++;
					dciInfoReTx.m_ndi = 0;
					itStat->second.at (harqId) = itStat->second.at (harqId) + 1;
					itHarq->second.at (harqId) = dciInfoReTx;
					SlotAllocInfo slotInfo (m_slotIdx++, SlotAllocInfo::UL_slotAllocInfo, SlotAllocInfo::CTRL_DATA, SlotAllocInfo::DIGITAL, rnti);
					slotInfo.m_dci = dciInfoReTx;
					NS_LOG_DEBUG ("UE" << dciInfoReTx.m_rnti << " gets UL slots " << (unsigned)dciInfoReTx.m_symStart << "-" << (unsigned)(dciInfoReTx.m_symStart+dciInfoReTx.m_numSym-1) <<
												" tbs " << dciInfoReTx.m_tbSize << " harqId " << (unsigned)dciInfoReTx.m_harqProcess << " rv " << (unsigned)dciInfoReTx.m_rv << " in frame " << m_schedSfn.m_frameNum << " subframe " << (unsigned)m_schedSfn.m_sfNum <<
												" RETX");
					m_currSchedConfig.m_sfAllocInfo.m_slotAllocInfo.push_back (slotInfo);
					m_currSchedConfig.m_sfAllocInfo.m_numSymAlloc += dciInfoReTx.m_numSym;
					if (m_itUeInfo == m_ueInfo.end())
					{
						m_itUeInfo = m_ueInfo.insert (std::pair<uint16_t, struct UeSchedInfo> (rnti, UeSchedInfo () )).first;
					}
					m_itUeInfo->second.m_ulSymbolsRetx = dciInfoReTx.m_numSym;
				}
				else
				{
					// Find out whether there are other resources later, keeping in mind that we cannot split RETX
					int numSymNeeded = dciInfoReTx.m_numSym;
					int numFreeSymbols = 0;
					uint8_t tmpSymIdx = m_symIdx;

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
					if(numFreeSymbols >= numSymNeeded && (int)(tmpSymIdx + numSymNeeded - 1) < (int)(m_phyMacConfig->GetSymbolsPerSubframe () - m_phyMacConfig->GetUlCtrlSymbols ()))
					{
						m_symAvail -= dciInfoReTx.m_numSym;
						dciInfoReTx.m_symStart = tmpSymIdx;

						NS_LOG_LOGIC("Before updating " << PrintSubframeAllocationMask(m_busyResourcesSchedSubframe.m_symAllocationMask));

						// update resource mask
						UpdateResourceMask(tmpSymIdx, dciInfoReTx.m_numSym);

						NS_LOG_LOGIC("After updating " << PrintSubframeAllocationMask(m_busyResourcesSchedSubframe.m_symAllocationMask));

						// symIdx += dciInfoReTx.m_numSym;
						NS_ASSERT ((int)(tmpSymIdx + dciInfoReTx.m_numSym - 1) <= (int)(m_phyMacConfig->GetSymbolsPerSubframe () - m_phyMacConfig->GetUlCtrlSymbols ()));
						dciInfoReTx.m_rv++;
						dciInfoReTx.m_ndi = 0;
						itStat->second.at (harqId) = itStat->second.at (harqId) + 1;
						itHarq->second.at (harqId) = dciInfoReTx;
						SlotAllocInfo slotInfo (m_slotIdx++, SlotAllocInfo::UL_slotAllocInfo, SlotAllocInfo::CTRL_DATA, SlotAllocInfo::DIGITAL, rnti);
						slotInfo.m_dci = dciInfoReTx;
						NS_LOG_DEBUG ("UE" << dciInfoReTx.m_rnti << " gets UL slots " << (unsigned)dciInfoReTx.m_symStart << "-" << (unsigned)(dciInfoReTx.m_symStart+dciInfoReTx.m_numSym-1) <<
													" tbs " << dciInfoReTx.m_tbSize << " harqId " << (unsigned)dciInfoReTx.m_harqProcess << " rv " << (unsigned)dciInfoReTx.m_rv << " in frame " << m_schedSfn.m_frameNum << " subframe " << (unsigned)m_schedSfn.m_sfNum <<
													" RETX");
						m_currSchedConfig.m_sfAllocInfo.m_slotAllocInfo.push_back (slotInfo);
						m_currSchedConfig.m_sfAllocInfo.m_numSymAlloc += dciInfoReTx.m_numSym;
						if (m_itUeInfo == m_ueInfo.end())
						{
							m_itUeInfo = m_ueInfo.insert (std::pair<uint16_t, struct UeSchedInfo> (rnti, UeSchedInfo () )).first;
						}
						m_itUeInfo->second.m_ulSymbolsRetx = dciInfoReTx.m_numSym;
					}
					else
					{
						NS_LOG_DEBUG ("No resource for this UL retx (even later) -> buffer it");
						m_ulInfoListUntxed.push_back (ulHarqInfoList.at (i));
					}
				}
			}
			else
			{
				NS_LOG_DEBUG ("No resource for this retx -> buffer it");
				m_ulInfoListUntxed.push_back (ulHarqInfoList.at (i));
			}
		}
	}
}

void
MmWaveRrIabMacScheduler::PerformNonHarqScheduling (std::list <MmWaveMacSchedSapProvider::SchedDlRlcBufferReqParameters> dlInfoList,
								   				   std::map <uint16_t, BufferSizeStatusSize_t> ulInfoList)
{
	std::list<MmWaveMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator itRlcBuf;
	// Number of DL/UL flows for new transmissions (not considering the HARQ RETXs)
	int nFlowsDl = 0;
	int nFlowsUl = 0;
	int nFlowsAccessDl = 0;
	int nFlowsAccessUl = 0;
	int nFlowsBackhaulDl = 0;
	int nFlowsBackhaulUl = 0;
	std::map <uint16_t, struct UeSchedInfo> tempUeInfoMap = {};

	// get info on active DL flows
	if (m_symAvail > 0 && !m_ulOnly)  // symAvail now indicated the remaining symbols in current subframe after HARQ retx sched
	{
		for (itRlcBuf = dlInfoList.begin (); itRlcBuf != dlInfoList.end (); itRlcBuf++)
		{
			NS_LOG_DEBUG ("Analyzing DL RLC buffer for user: " << itRlcBuf->m_rnti);
			m_itUeInfo = tempUeInfoMap.find (itRlcBuf->m_rnti);
			// check if it is a relay or a UE
			bool isIab = false;
			if(m_rntiIabInfoMap.find(itRlcBuf->m_rnti) != m_rntiIabInfoMap.end())
			{
				isIab = m_rntiIabInfoMap.find(itRlcBuf->m_rnti)->second.first;
			}

			if ( (((*itRlcBuf).m_rlcTransmissionQueueSize > 0)
					|| ((*itRlcBuf).m_rlcRetransmissionQueueSize > 0)
					|| ((*itRlcBuf).m_rlcStatusPduSize > 0)) )
			{
				NS_LOG_DEBUG (this << " User " << itRlcBuf->m_rnti << " LC " << (uint16_t)itRlcBuf->m_logicalChannelIdentity << " is active, status  " << (*itRlcBuf).m_rlcStatusPduSize << " retx " << (*itRlcBuf).m_rlcRetransmissionQueueSize << " tx " << (*itRlcBuf).m_rlcTransmissionQueueSize << " iab " << isIab);
				std::map <uint16_t,uint8_t>::iterator itCqi = m_wbCqiRxed.find (itRlcBuf->m_rnti);
				uint8_t cqi = 0;
				if (itCqi != m_wbCqiRxed.end ())
				{
					cqi = itCqi->second;
				}
				else // no CQI available
				{
					NS_LOG_DEBUG (this << " UE " << itRlcBuf->m_rnti << " does not have DL-CQI");
					cqi = 1; // lowest value for trying a transmission
				}
				if (cqi != 0 || m_fixedMcsDl) 	// CQI == 0 means "out of range" (see table 7.2.3-1 of 36.213)
				{
					if (m_itUeInfo == tempUeInfoMap.end ())
					{
						m_itUeInfo = tempUeInfoMap.insert (std::pair<uint16_t, struct UeSchedInfo> (itRlcBuf->m_rnti, UeSchedInfo () )).first;

						nFlowsDl++;  // for simplicity, all RLC LCs are considered as a single flow
						if(!isIab)
						{
							nFlowsAccessDl++;
						}
						else
						{
							nFlowsBackhaulDl++;
							m_itUeInfo->second.m_iab = true;
						}
					}
					else if (m_itUeInfo->second.m_maxDlBufSize == 0)
					{
						nFlowsDl++;
						if(!isIab)
						{
							nFlowsAccessDl++;
						}
						else
						{
							nFlowsBackhaulDl++;
							m_itUeInfo->second.m_iab = true;
						}
					}
					NS_LOG_DEBUG("itUeInfo->second.m_iab " << m_itUeInfo->second.m_iab);
					if (m_fixedMcsDl)
					{
						m_itUeInfo->second.m_dlMcs = m_mcsDefaultDl;
					}
					else
					{
						m_itUeInfo->second.m_dlMcs = m_amc->GetMcsFromCqi (cqi);  // get MCS
					}

					// temporarily store the TX queue size
					if(itRlcBuf->m_rlcStatusPduSize > 0)
					{
						RlcPduInfo newRlcStatusPdu;
						newRlcStatusPdu.m_lcid = itRlcBuf->m_logicalChannelIdentity;
						newRlcStatusPdu.m_size += itRlcBuf->m_rlcStatusPduSize + m_subHdrSize;
						m_itUeInfo->second.m_rlcPduInfo.push_back (newRlcStatusPdu);
						m_itUeInfo->second.m_maxDlBufSize += newRlcStatusPdu.m_size;  // add to total DL buffer size
					}

					RlcPduInfo newRlcEl;
					newRlcEl.m_lcid = itRlcBuf->m_logicalChannelIdentity;
					if (itRlcBuf->m_rlcRetransmissionQueueSize > 0)
					{
						newRlcEl.m_size = itRlcBuf->m_rlcRetransmissionQueueSize;
					}
					else if (itRlcBuf->m_rlcTransmissionQueueSize > 0)
					{
						newRlcEl.m_size = itRlcBuf->m_rlcTransmissionQueueSize;
					}

					if (newRlcEl.m_size > 0)
					{
						if (newRlcEl.m_size < 8)
						{
							newRlcEl.m_size = 8;
						}
						newRlcEl.m_size += m_rlcHdrSize + m_subHdrSize + 10;
						m_itUeInfo->second.m_rlcPduInfo.push_back (newRlcEl);
						m_itUeInfo->second.m_maxDlBufSize += newRlcEl.m_size;  // add to total DL buffer size
					}
				}
				else
				{ // SINR out of range, don't schedule for DL
					NS_LOG_WARN ("*** RNTI " << itRlcBuf->m_rnti << " DL-CQI out of range, skipping allocation");
				}
			}
		}
	}

	// get info on active UL flows
	if (m_symAvail > 0 && !m_dlOnly)  // remaining symbols in future UL subframe after HARQ retx sched
	{
		// std::map <uint16_t,uint32_t>::iterator ceBsrIt;
		for(auto ceBsrIt : ulInfoList)
		{
			NS_LOG_DEBUG ("Analyzing UL RLC buffer for user: " << ceBsrIt.first);
		// }

		// for (ceBsrIt = m_ceBsrRxed.begin (); ceBsrIt != m_ceBsrRxed.end (); ceBsrIt++)
		// {
			if (std::get<0>(ceBsrIt.second) > 0)  // UL buffer size > 0
			{
				uint32_t bufferSize = std::get<0>(ceBsrIt.second);
				uint32_t statusPduSize = std::get<1>(ceBsrIt.second); // this is included in the bufferSize
				uint16_t rnti = ceBsrIt.first;
				std::map <uint16_t, struct UlCqiMapElem>::iterator itCqi = m_ueUlCqi.find (rnti);
				bool isIab = false;
				if(m_rntiIabInfoMap.find(rnti) != m_rntiIabInfoMap.end())
				{
					isIab = m_rntiIabInfoMap.find(rnti)->second.first;
				}
				NS_LOG_DEBUG(this << " ceBsrIt UE " << rnti << " bf size " << bufferSize << " statusPduSize " << statusPduSize << " iab " << isIab);

				int cqi = 0;
				int mcs = 0;
				if (itCqi == m_ueUlCqi.end ()) // no cqi info for this UE
				{
					NS_LOG_DEBUG (this << " UE " << rnti << " does not have UL-CQI");
					cqi = 1;
					mcs = 0;
				}
				else
				{
					cqi = 0;
					SpectrumValue specVals (MmWaveSpectrumValueHelper::GetSpectrumModel (m_phyMacConfig));
					Values::iterator specIt = specVals.ValuesBegin();
					for (unsigned ichunk = 0; ichunk < m_phyMacConfig->GetTotalNumChunk (); ichunk++)
					{
						NS_ASSERT (specIt != specVals.ValuesEnd());
						*specIt = itCqi->second.m_ueUlCqi.at (ichunk); //sinrLin;
						specIt++;
					}

					cqi = m_amc->CreateCqiFeedbackWbTdma (specVals, itCqi->second.m_numSym, itCqi->second.m_tbSize, mcs);

					if (cqi == 0 && !m_fixedMcsUl) // out of range (SINR too low)
					{
						NS_LOG_DEBUG ("*** RNTI " << ceBsrIt.first << " UL-CQI out of range, skipping allocation in UL");
						// TODOIAB use continue and not break
						continue;  // do not allocate UE in uplink
					}
				}
				m_itUeInfo = tempUeInfoMap.find (rnti);
				if (m_itUeInfo == tempUeInfoMap.end ())
				{
					m_itUeInfo = tempUeInfoMap.insert (std::pair<uint16_t, struct UeSchedInfo> (rnti, UeSchedInfo () )).first;
					
					nFlowsUl++;
					if(!isIab)
					{
						nFlowsAccessUl++;
					}
					else
					{
						nFlowsBackhaulUl++;
						m_itUeInfo->second.m_iab = true;
					}
				}
				else if (m_itUeInfo->second.m_maxUlBufSize == 0)
				{
					nFlowsUl++;
					if(!isIab)
					{
						nFlowsAccessUl++;
					}
					else
					{
						nFlowsBackhaulUl++;
						m_itUeInfo->second.m_iab = true;
					}
				}
				NS_LOG_DEBUG("itUeInfo->second.m_iab " << m_itUeInfo->second.m_iab);
				if (m_fixedMcsUl)
				{
					m_itUeInfo->second.m_ulMcs = m_mcsDefaultUl;
				}
				else
				{
					m_itUeInfo->second.m_ulMcs = mcs;//m_amc->GetMcsFromCqi (cqi);  // get MCS
				}
				m_itUeInfo->second.m_maxUlBufSize = bufferSize + m_rlcHdrSize + m_macHdrSize + 8;
				m_itUeInfo->second.m_minSizeToBeScheduled = statusPduSize; // the status PDU cannot be split
			}
		}
	}

	int nFlowsTot = nFlowsDl + nFlowsUl;

	NS_LOG_DEBUG(this << " nFlowsDl " << nFlowsDl << " nFlowsUl " << nFlowsUl 
		<< " nFlowsAccessDl " << nFlowsAccessDl << " nFlowsAccessUl " << nFlowsAccessUl
		<< " nFlowsBackhaulDl " << nFlowsBackhaulDl << " nFlowBackhaulsUl " << nFlowsBackhaulUl);

	if (tempUeInfoMap.size () == 0 )
	{
		return;
	}

	// compute requested num slots and TB size based on MCS and DL buffer size
	// final allocated slots may be less
	int totDlSymReq = 0;
	int totUlSymReq = 0;
	int totAccessDlSymReq = 0;
	int totAccessUlSymReq = 0;
	int totBackhaulDlSymReq = 0;
	int totBackhaulUlSymReq = 0;
	
	for (m_itUeInfo = tempUeInfoMap.begin (); m_itUeInfo != tempUeInfoMap.end (); m_itUeInfo++)
	{
		unsigned dlTbSize = 0;
		unsigned ulTbSize = 0;
		if (m_itUeInfo->second.m_maxDlBufSize > 0)
		{
			m_itUeInfo->second.m_maxDlSymbols = CalcMinTbSizeNumSym (m_itUeInfo->second.m_dlMcs, m_itUeInfo->second.m_maxDlBufSize, dlTbSize); // dlTbSize is passed as a parameter and not as a value, it will be updated with the tbSize
			// TODOIAB fire trace to store the m_maxDlSymbols
			m_itUeInfo->second.m_maxDlBufSize = dlTbSize;
			if (m_fixedTti)
			{
				m_itUeInfo->second.m_maxDlSymbols = ceil((double)m_itUeInfo->second.m_maxDlSymbols/(double)m_symPerSlot) * m_symPerSlot; // round up to nearest sym per TTI
			}
			totDlSymReq += m_itUeInfo->second.m_maxDlSymbols;
			NS_LOG_DEBUG("itUeInfo->second.m_iab " << m_itUeInfo->second.m_iab << " " << m_itUeInfo->first);
			if(m_itUeInfo->second.m_iab)
			{
				totBackhaulDlSymReq += m_itUeInfo->second.m_maxDlSymbols;
			}
			else
			{
				totAccessDlSymReq += m_itUeInfo->second.m_maxDlSymbols;
			}
		}
		if (m_itUeInfo->second.m_maxUlBufSize > 0)
		{
			m_itUeInfo->second.m_maxUlSymbols = CalcMinTbSizeNumSym (m_itUeInfo->second.m_ulMcs, m_itUeInfo->second.m_maxUlBufSize+10, ulTbSize);
			m_itUeInfo->second.m_maxUlBufSize = ulTbSize;
			if (m_fixedTti)
			{
				m_itUeInfo->second.m_maxUlSymbols = ceil((double)m_itUeInfo->second.m_maxUlSymbols/(double)m_symPerSlot) * m_symPerSlot; // round up to nearest sym per TTI
			}
			totUlSymReq += m_itUeInfo->second.m_maxUlSymbols;
			NS_LOG_DEBUG("itUeInfo->second.m_iab " << m_itUeInfo->second.m_iab << " " << m_itUeInfo->first);
			if(m_itUeInfo->second.m_iab)
			{
				totBackhaulUlSymReq += m_itUeInfo->second.m_maxUlSymbols;
			}
			else
			{
				totAccessUlSymReq += m_itUeInfo->second.m_maxUlSymbols;
			}
		}
	}

	NS_LOG_DEBUG(this << " totDlSymReq " << totDlSymReq << " totUlSymReq " << totUlSymReq 
		<< " totAccessDlSymReq " << totAccessDlSymReq << " totAccessUlSymReq " << totAccessUlSymReq
		<< " totBackhaulDlSymReq " << totBackhaulDlSymReq << " totBackhaulUlSymReq " << totBackhaulUlSymReq);

	// TODOIAB each IAB device cannot have more than half of the available resources
	// TODOIAB force the sum of m_maxUlSymbols and m_maxDlSymbols to be smaller than or equal to the following quantity
	/*
	int maxSymAvailableForIab = std::floor(m_symAvail/2);
	int removedSymbolsDl = 0;
	int removedSymbolsUl = 0;
	for(auto m_itUeInfo = m_ueInfo.begin(); m_itUeInfo != m_ueInfo.end(); ++m_itUeInfo)
	{
		if(m_itUeInfo->second.m_iab)
		{
			int totalSymbolsRequested = m_itUeInfo->second.m_maxUlSymbols + m_itUeInfo->second.m_maxDlSymbols;
			int oldUlSymbols = m_itUeInfo->second.m_maxUlSymbols;
			int oldDlSymbols = m_itUeInfo->second.m_maxDlSymbols;
			if(totalSymbolsRequested > maxSymAvailableForIab)
			{
				double scaleFactor = (double)maxSymAvailableForIab/(double)totalSymbolsRequested;
				NS_LOG_DEBUG("IAB with rnti " << m_itUeInfo->first << " requests too many symbols " << totalSymbolsRequested << " total available " 
					<< m_symAvail << " for IAB " << maxSymAvailableForIab << " scaleFactor " << scaleFactor);
				if(m_itUeInfo->second.m_maxUlSymbols > 0)
				{
					int newSymbols = std::max(1, (int)std::floor(scaleFactor*oldUlSymbols));
					removedSymbolsUl += (m_itUeInfo->second.m_maxUlSymbols - newSymbols);
					m_itUeInfo->second.m_maxUlSymbols = newSymbols;
				}
				if(m_itUeInfo->second.m_maxDlSymbols > 0)
				{
					int newSymbols = std::max(1, (int)std::floor(scaleFactor*oldDlSymbols));
					removedSymbolsDl += (m_itUeInfo->second.m_maxDlSymbols - newSymbols);
					m_itUeInfo->second.m_maxDlSymbols = newSymbols;
				}
				NS_LOG_DEBUG("New symbols request DL " << (uint32_t)m_itUeInfo->second.m_maxDlSymbols << 
					" UL " <<(uint32_t)m_itUeInfo->second.m_maxUlSymbols);
			}
		}
	}

	NS_LOG_DEBUG("Removed symbols DL " << removedSymbolsDl << " UL " << removedSymbolsUl);

	totDlSymReq -= removedSymbolsDl;
	totUlSymReq -= removedSymbolsUl;
	totBackhaulDlSymReq -= removedSymbolsDl;
	totBackhaulUlSymReq -= removedSymbolsUl;

	NS_LOG_DEBUG(this << " after totDlSymReq " << totDlSymReq << " totUlSymReq " << totUlSymReq 
		<< " totAccessDlSymReq " << totAccessDlSymReq << " totAccessUlSymReq " << totAccessUlSymReq
		<< " totBackhaulDlSymReq " << totBackhaulDlSymReq << " totBackhaulUlSymReq " << totBackhaulUlSymReq);
	*/

	std::map <uint16_t, struct UeSchedInfo>::iterator itUeInfoStart;
	if (m_nextRnti != 0) 	// start with RNTI at which the scheduler left off
	{
		itUeInfoStart = tempUeInfoMap.find (m_nextRnti);
		if (itUeInfoStart == tempUeInfoMap.end ())
		{
			itUeInfoStart = tempUeInfoMap.begin ();
		}
	}
	else	// start with first active RNTI
	{
		itUeInfoStart = tempUeInfoMap.begin ();
	}
	m_itUeInfo = itUeInfoStart;
	NS_LOG_DEBUG(this << " Starting allocation from user:  " << itUeInfoStart->first);

	// divide OFDM symbols evenly between active UEs, which are then evenly divided between DL and UL flows
	if (nFlowsTot > 0)
	{
		int remSym = totDlSymReq + totUlSymReq;
		if (remSym > m_symAvail)
		{
			remSym = m_symAvail;
		}

		int nSymPerFlow0 = remSym / nFlowsTot;  // initial average symbols per non-retx flow
		NS_LOG_DEBUG("nSymPerFlow0 " << nSymPerFlow0);
		if (nSymPerFlow0 == 0)	// minimum of 1
		{
			nSymPerFlow0 = 1;
		}
		if (m_fixedTti)
		{
			nSymPerFlow0 = ceil((double)nSymPerFlow0/(double)m_symPerSlot) * m_symPerSlot; // round up to nearest sym per TTI
		}
		bool allocated = true; // someone got allocated
		while (remSym > 0 && allocated && nFlowsTot > 0)
		{
			allocated = false;  // additional symbols allocated to this RNTI in this iteration
			int nRemSymPerFlow = remSym / nFlowsTot;
			NS_LOG_DEBUG("nSymPerFlow " << nRemSymPerFlow);

			if (nRemSymPerFlow == 0)
			{
				nRemSymPerFlow = 1;
			}
			if (m_fixedTti)
			{
				nRemSymPerFlow = ceil((double)nRemSymPerFlow/(double)m_symPerSlot) * m_symPerSlot; // round up to nearest sym per TTI
			}
			while (remSym > 0)
			{
				int addSym = 0;
				// deficit = difference between requested and allocated symbols
				NS_LOG_DEBUG("itUeInfo->second.m_maxDlSymbols " << (uint32_t)m_itUeInfo->second.m_maxDlSymbols);
				int deficit = m_itUeInfo->second.m_maxDlSymbols - m_itUeInfo->second.m_dlSymbols;
				NS_LOG_DEBUG("deficit " << deficit << " nSymPerFlow0 " << nSymPerFlow0 << " nRemSymPerFlow " << nRemSymPerFlow);
				NS_ASSERT (deficit >= 0);
				if (m_fixedTti)
				{
					deficit = ceil((double)deficit/(double)m_symPerSlot) * m_symPerSlot; // round up to nearest sym per TTI
				}
				if (deficit > 0 && ((m_itUeInfo->second.m_dlSymbols+m_itUeInfo->second.m_dlSymbolsRetx) <= nSymPerFlow0))
				{
					if (deficit < nRemSymPerFlow)
					{
						if (deficit > remSym)
						{
							addSym = remSym;
						}
						else
						{
							addSym = deficit;
							// add remaining symbols to average
							nFlowsTot--;
							if(nFlowsTot <= 0)
							{
								break; // TODOIAB or continue?
							}
							int extra = (nRemSymPerFlow - addSym) / nFlowsTot;
							nSymPerFlow0 += extra;  // add extra to average symbols
							nRemSymPerFlow += extra;  // add extra to average symbols
						}
					}
					else
					{
						if (nRemSymPerFlow > remSym)
						{
							addSym = remSym;
						}
						else
						{
							addSym = nRemSymPerFlow;
						}
					}
					allocated = true;
				}
				NS_LOG_DEBUG("addSym " << addSym);
				m_itUeInfo->second.m_dlSymbols += addSym;
				remSym -= addSym;
				NS_ASSERT (remSym >= 0);

				addSym = 0;
				// deficit = difference between requested and allocated symbols

				NS_LOG_DEBUG("itUeInfo->second.m_maxUlSymbols " << (uint32_t)m_itUeInfo->second.m_maxUlSymbols);
				deficit = m_itUeInfo->second.m_maxUlSymbols - m_itUeInfo->second.m_ulSymbols;
				NS_LOG_DEBUG("deficit " << deficit << " nSymPerFlow0 " << nSymPerFlow0 << " nRemSymPerFlow " << nRemSymPerFlow);
				NS_ASSERT (deficit >= 0);
				if (m_fixedTti)
				{
					deficit = ceil((double)deficit/(double)m_symPerSlot) * m_symPerSlot; // round up to nearest sym per TTI
				}
				if (m_fixedTti)
				{
					nRemSymPerFlow = ceil((double)nRemSymPerFlow/(double)m_symPerSlot) * m_symPerSlot; // round up to nearest sym per TTI
				}
				if (remSym > 0 && deficit > 0 && ((m_itUeInfo->second.m_ulSymbols+m_itUeInfo->second.m_ulSymbolsRetx) <= nSymPerFlow0))
				{
					if (deficit < nRemSymPerFlow)
					{
						// add remaining symbols to average
						if (deficit > remSym)
						{
							addSym = remSym;
						}
						else
						{
							addSym = deficit;
							// add remaining symbols to average
							nFlowsTot--;
							if(nFlowsTot <= 0)
							{
								break; // TODOIAB or continue?
							}
							int extra = (nRemSymPerFlow - addSym) / nFlowsTot;
							nSymPerFlow0 += extra;  // add extra to average symbols
							nRemSymPerFlow += extra;  // add extra to average symbols
						}
					}
					else
					{
						if (nRemSymPerFlow > remSym)
						{
							addSym = remSym;
						}
						else
						{
							addSym = nRemSymPerFlow;
						}
						allocated = true;
					}
				}
				m_itUeInfo->second.m_ulSymbols += addSym;
				remSym -= addSym;
				NS_ASSERT (remSym >= 0);

				m_itUeInfo++;
				if (m_itUeInfo == tempUeInfoMap.end ())
				{ // loop around to first RNTI in map
					m_itUeInfo = tempUeInfoMap.begin ();
				}
				if (m_itUeInfo == itUeInfoStart)
				{ // break when looped back to initial RNTI or no symbols remain
					break;
				}
			}
		}
	}

	/*
	if(m_etaIab > 0 && (nFlowsBackhaulDl > 0 || nFlowsBackhaulUl >0))
	{
		// weight more the IAB devs than the other UEs
		UpdateIabAllocation(m_ueInfo);
	}
	*/

	m_nextRnti = m_itUeInfo->first;

	NS_LOG_DEBUG(this << " m_nextRnti " << m_nextRnti);

	// create DCI elements and assign symbol indices
	// such that all DL slots are contiguous (at beginning of subframe)
	// and all UL slots are contiguous (at end of subframe)
	m_itUeInfo = itUeInfoStart;

	//ulSymIdx -= totUlSymActual; // symbols reserved for control at end of subframe before UL ctrl
	NS_ASSERT (m_symIdx > 0);
	do
	{
		UeSchedInfo &ueSchedInfo = m_itUeInfo->second;
		if (ueSchedInfo.m_dlSymbols > 0 && m_symAvail > 0)
		{
			NS_LOG_INFO("num LC before allocation " << (uint32_t)ueSchedInfo.m_rlcPduInfo.size ());
			std::vector<SlotAllocInfo> tmpSlotAllocVector;
			int numSymNeeded = ueSchedInfo.m_dlSymbols;
			int totalTbSize = 0;
			NS_LOG_DEBUG("UE " << m_itUeInfo->first << " numSymNeeded " << numSymNeeded);
			do
			{
				auto symIdxNumFreeSymbols = GetFreeSymbolsAndNextIndex(m_symIdx, numSymNeeded);
				int numFreeSymbols = std::get<1>(symIdxNumFreeSymbols);
				m_symIdx = std::get<0>(symIdxNumFreeSymbols);
				m_symAvail -= numFreeSymbols;
				numSymNeeded -= numFreeSymbols;
				NS_LOG_DEBUG("UE " << m_itUeInfo->first << " numSymNeeded (after allocation) "
					<< numSymNeeded << " numFreeSymbols " << numFreeSymbols << " symAvail " << m_symAvail);

				// create the DCI
				DciInfoElementTdma dci;
				dci.m_rnti = m_itUeInfo->first;
				dci.m_format = 0;
				dci.m_symStart = m_symIdx;
				dci.m_numSym = numFreeSymbols;
				m_symIdx = GetFirstFreeSymbol(m_symIdx, numFreeSymbols); // get the next symIdx
				NS_LOG_LOGIC("Next symIdx " << (uint16_t)m_symIdx);
				dci.m_ndi = 1;
				dci.m_mcs = ueSchedInfo.m_dlMcs;
				dci.m_tbSize = m_amc->GetTbSizeFromMcsSymbols (dci.m_mcs, dci.m_numSym) / 8;
				totalTbSize += dci.m_tbSize;
		
				NS_ASSERT (m_symIdx <= m_phyMacConfig->GetSymbolsPerSubframe () - m_phyMacConfig->GetUlCtrlSymbols ());
				dci.m_rv = 0;
				dci.m_harqProcess = UpdateDlHarqProcessId (m_itUeInfo->first);
				NS_ASSERT (dci.m_harqProcess < m_phyMacConfig->GetNumHarqProcess ());
				NS_LOG_DEBUG ("UE" << m_itUeInfo->first << " DL harqId " << (unsigned)dci.m_harqProcess << " HARQ process assigned");
				SlotAllocInfo slotInfo (m_slotIdx++, SlotAllocInfo::DL_slotAllocInfo, SlotAllocInfo::CTRL_DATA, SlotAllocInfo::DIGITAL, m_itUeInfo->first);
				slotInfo.m_dci = dci;
				NS_LOG_DEBUG (this << " UE " << dci.m_rnti << " gets DL slots " << (unsigned)dci.m_symStart << "-" << (unsigned)(dci.m_symStart+dci.m_numSym-1) <<
				             " tbs " << dci.m_tbSize << " mcs " << (unsigned)dci.m_mcs << " harqId " << (unsigned)dci.m_harqProcess << " rv " << (unsigned)dci.m_rv << " in frame " << m_currSchedConfig.m_sfnSf.m_frameNum << " subframe " << (unsigned)m_currSchedConfig.m_sfnSf.m_sfNum);

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
			unsigned numLc = ueSchedInfo.m_rlcPduInfo.size ();
			NS_ASSERT (numLc != 0);
			unsigned bytesRem = totalTbSize;
			unsigned numFulfilled = 0;
			NS_LOG_DEBUG ("BytesRem: " << bytesRem << " Num LC: " << numLc);
			uint16_t avgPduSize = bytesRem / numLc;
			NS_LOG_DEBUG("numLc " << numLc << " bytesRem " << bytesRem << " avgPduSize " << avgPduSize);
			
			// first for loop computes extra to add to average if some flows are less than average
			for (unsigned i = 0; i < ueSchedInfo.m_rlcPduInfo.size (); i++)
			{
				if (ueSchedInfo.m_rlcPduInfo[i].m_size < avgPduSize)
				{
					bytesRem -= ueSchedInfo.m_rlcPduInfo[i].m_size;
					numFulfilled++;
				}
			}

			if (numFulfilled < ueSchedInfo.m_rlcPduInfo.size ())
			{
				avgPduSize = bytesRem / (ueSchedInfo.m_rlcPduInfo.size () - numFulfilled);
			}

			NS_LOG_DEBUG("numFulfilled " << numFulfilled << " new avgPduSize " << avgPduSize);

			// fill the RlcPduInfo taking into account the possible split of resources
			for(auto slotInfo : tmpSlotAllocVector)
			{
				// distribute bytes between active RLC queues and slots allocated
				int remTbSize = slotInfo.m_dci.m_tbSize;
				NS_LOG_DEBUG("slotInfo starting from " << (uint16_t)slotInfo.m_dci.m_symStart << " with tbSize " << remTbSize << " num PDU " << ueSchedInfo.m_rlcPduInfo.size());
				
				auto pduIterator = ueSchedInfo.m_rlcPduInfo.begin();
				while (remTbSize > 0 && pduIterator != ueSchedInfo.m_rlcPduInfo.end())
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
					UpdateDlRlcBufferInfo (m_itUeInfo->first, pduIterator->m_lcid, std::min(remTbSize, (int)pduIterator->m_size) - m_subHdrSize);

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
						ueSchedInfo.m_rlcPduInfo.erase(pduIterator);
						pduIterator = ueSchedInfo.m_rlcPduInfo.begin();
					}

					NS_LOG_DEBUG("Added RLC PDU with size " << slotInfo.m_rlcPduInfo.back().m_size << " remTbSize " << remTbSize);

					if (m_harqOn == true)
					{
						// store RLC PDU list for HARQ
						std::map <uint16_t, DlHarqRlcPduList_t>::iterator itRlcPdu =  m_dlHarqProcessesRlcPduMap.find (m_itUeInfo->first);
						if (itRlcPdu == m_dlHarqProcessesRlcPduMap.end ())
						{
							NS_FATAL_ERROR ("Unable to find RlcPdcList in HARQ buffer for RNTI " << m_itUeInfo->first);
						}
						(*itRlcPdu).second.at (slotInfo.m_dci.m_harqProcess).push_back (slotInfo.m_rlcPduInfo.back());
					}
				}

				
				// reorder/reindex slots to maintain DL before UL slot order
				bool reordered = false;

				if (!reordered && slotInfo.m_rlcPduInfo.size() > 0) // TODOIAB understand why a TB may be in excess with respect to the RLC size
				{
					m_currSchedConfig.m_sfAllocInfo.m_slotAllocInfo.push_back (slotInfo);
				}
				m_currSchedConfig.m_sfAllocInfo.m_numSymAlloc += slotInfo.m_dci.m_numSym;
			} 
		}

		// UL DCI applies to subframe i+Tsched
		if (ueSchedInfo.m_ulSymbols > 0 && m_symAvail > 0)
		{
			int numSymNeeded = ueSchedInfo.m_ulSymbols;
			do
			{
				int numFreeSymbols = 0;
				uint32_t minTbSizeToBeScheduled = ueSchedInfo.m_minSizeToBeScheduled;
				int minNumSymNeeded = 1;
				bool useTmpSymIdx = false;
				bool doNotScheduleStatusPduInThisDci = false;
				uint8_t tmpSymIdx = m_symIdx;
				// if the UE has to transmit a status PDU, then this cannot be split
				// therefore check if there is the possibility of scheduling at least
				// a continuous chunk with the status PDU
				if (minTbSizeToBeScheduled > 0)
				{
					// get the minimum number of symbols needed for this tb size, with the MCS of this UE
					bool numSymFound = false;
					while(!numSymFound)
					{
						uint32_t allocableTbSize = m_amc->GetTbSizeFromMcsSymbols (ueSchedInfo.m_ulMcs, minNumSymNeeded) / 8;
						if(allocableTbSize > minTbSizeToBeScheduled)
						{
							numSymFound = true;
							break;
						}
						else
						{
							minNumSymNeeded++;
						}
					}
					NS_LOG_DEBUG("minTbSizeToBeScheduled " << minTbSizeToBeScheduled << " need " << minNumSymNeeded << " symbols, total needed " << numSymNeeded);
			
					// need to find a symbol from which at least numSymNeeded should fit
					while(numFreeSymbols == 0 && tmpSymIdx < m_phyMacConfig->GetSymbolsPerSubframe()-1) 
					{
						numFreeSymbols = GetNumFreeSymbols(tmpSymIdx, minNumSymNeeded); // this is equal to minNumSymNeeded if there is no overlap, otherwise smaller
						NS_LOG_DEBUG("numFreeSymbols " << numFreeSymbols << " at symbol " << (uint32_t)tmpSymIdx);
						if(numFreeSymbols == 0)
						{
							// tmpSymIdx is inside a busy interval, get the next free symIdx and retry
							NS_LOG_DEBUG("symIdx is in a busy interval, get the first free symIdx and retry");
							tmpSymIdx = GetFirstFreeSymbol(tmpSymIdx, 0); // get the next symIdx
							NS_LOG_DEBUG("new tmpSymIdx " << (uint16_t)tmpSymIdx);
							numFreeSymbols = GetNumFreeSymbols(tmpSymIdx, minNumSymNeeded);
						}
						uint32_t allocableTbSize = m_amc->GetTbSizeFromMcsSymbols (ueSchedInfo.m_ulMcs, numFreeSymbols) / 8;
						NS_LOG_DEBUG("UL TX with status PDU minTbSize needed " << minTbSizeToBeScheduled << " tb size available " 
							<< allocableTbSize << " tmpSymIdx " << (uint16_t)tmpSymIdx << " numFreeSymbols " << numFreeSymbols 
							<< " MCS " << (uint32_t)ueSchedInfo.m_ulMcs);
						if((int)minNumSymNeeded > (int)numFreeSymbols)
						{
							numFreeSymbols = 0;
							tmpSymIdx = GetFirstFreeSymbol(tmpSymIdx, minNumSymNeeded); // get the next symIdx
							if((int)(tmpSymIdx + minNumSymNeeded) >  (int)m_phyMacConfig->GetSymbolsPerSubframe()-1)
							{	
								NS_LOG_DEBUG("No way to fit this status PDU in the available resources");
								break;
							}
						}
					}

					NS_LOG_WARN("UL TX numSymNeeded " << numSymNeeded << " minNumSymNeeded " << minNumSymNeeded << 
								" numFreeSymbols " << numFreeSymbols << " tmpSymIdx " << (uint16_t)tmpSymIdx);

					if(numFreeSymbols >= minNumSymNeeded && 
						(int)(tmpSymIdx + minNumSymNeeded - 1) < (int)(m_phyMacConfig->GetSymbolsPerSubframe () - m_phyMacConfig->GetUlCtrlSymbols ()))
					{
						// check how many additional symbols can actually be allocated here
						if (numSymNeeded > minNumSymNeeded)
						{
							numFreeSymbols = GetNumFreeSymbols(tmpSymIdx, numSymNeeded);
						}
						NS_LOG_WARN("UL TX numSymNeeded " << numSymNeeded << " numFreeSymbols " 
							<< numFreeSymbols << " tmpSymIdx " << (uint16_t)tmpSymIdx);
						if(tmpSymIdx != m_symIdx)
						{
							useTmpSymIdx = true;
							NS_LOG_DEBUG("The status PDU can be allocated, not at symIdx " << (uint32_t)m_symIdx << " but at " << (uint32_t)tmpSymIdx);
							UpdateResourceMask(tmpSymIdx, numFreeSymbols);
							NS_LOG_DEBUG("After status PDU alloc " << PrintSubframeAllocationMask(m_busyResourcesSchedSubframe.m_symAllocationMask));
						}

					}	
					else
					{
						NS_LOG_DEBUG("The status PDU cannot be allocated");
						// we need to skip this UE... or signal that the status PDU cannot be scheduled
						doNotScheduleStatusPduInThisDci = true;
						// continue with the standard allocation
						auto symIdxNumFreeSymbols = GetFreeSymbolsAndNextIndex(m_symIdx, numSymNeeded);
						numFreeSymbols = std::get<1>(symIdxNumFreeSymbols);
						m_symIdx = std::get<0>(symIdxNumFreeSymbols);
					}
				}
				else // just find the first free spot
				{
					NS_LOG_DEBUG("No status PDU to be scheduled");
					auto symIdxNumFreeSymbols = GetFreeSymbolsAndNextIndex(m_symIdx, numSymNeeded);
					numFreeSymbols = std::get<1>(symIdxNumFreeSymbols);
					m_symIdx = std::get<0>(symIdxNumFreeSymbols);
				}
				
				m_symAvail -= numFreeSymbols;
				numSymNeeded -= numFreeSymbols;
				NS_LOG_DEBUG("UE " << m_itUeInfo->first << " numSymNeeded (after allocation) " << numSymNeeded 
					<< " numFreeSymbols " << numFreeSymbols << " symAvail " << m_symAvail);

				DciInfoElementTdma dci;
				dci.m_rnti = m_itUeInfo->first;
				dci.m_format = 1;
				dci.m_numSym = numFreeSymbols;
				dci.m_symStart = useTmpSymIdx ? tmpSymIdx : m_symIdx;
				if(!useTmpSymIdx)
				{
					m_symIdx = GetFirstFreeSymbol(m_symIdx, numFreeSymbols); // get the next free symbol
				}
				// if the allocation is done at tmpSymIdx, then this will return symIdx
				NS_LOG_DEBUG("Next symIdx " << (uint16_t)m_symIdx);
				dci.m_mcs = ueSchedInfo.m_ulMcs;
				dci.m_ndi = 1;
				dci.m_tbSize = m_amc->GetTbSizeFromMcsSymbols (dci.m_mcs, dci.m_numSym) / 8;
				dci.m_doNotScheduleStatusPduInThisDci = doNotScheduleStatusPduInThisDci;

				NS_ASSERT (m_symIdx <= m_phyMacConfig->GetSymbolsPerSubframe () - m_phyMacConfig->GetUlCtrlSymbols ());
				dci.m_harqProcess = UpdateUlHarqProcessId (m_itUeInfo->first);
				NS_LOG_DEBUG ("UE" << m_itUeInfo->first << " UL harqId " << (unsigned)dci.m_harqProcess << " HARQ process assigned");
				NS_ASSERT (dci.m_harqProcess < m_phyMacConfig->GetNumHarqProcess ());
				SlotAllocInfo slotInfo (m_slotIdx++, SlotAllocInfo::UL_slotAllocInfo, SlotAllocInfo::CTRL_DATA, SlotAllocInfo::DIGITAL, m_itUeInfo->first);
				slotInfo.m_dci = dci;
				NS_LOG_DEBUG (this << " UE" << dci.m_rnti << " gets UL slots " << (unsigned)dci.m_symStart << "-" << (unsigned)(dci.m_symStart+dci.m_numSym-1) <<
							             " tbs " << dci.m_tbSize << " mcs " << (unsigned)dci.m_mcs << " harqId " << (unsigned)dci.m_harqProcess << " rv " << (unsigned)dci.m_rv << " in frame " << m_schedSfn.m_frameNum << " subframe " << (unsigned)m_schedSfn.m_sfNum);
				UpdateUlRlcBufferInfo (m_itUeInfo->first, dci.m_tbSize - m_subHdrSize);
				m_currSchedConfig.m_sfAllocInfo.m_slotAllocInfo.push_back (slotInfo);  // add to front
				m_currSchedConfig.m_sfAllocInfo.m_numSymAlloc += dci.m_numSym;
				std::vector<uint16_t> ueChunkMap;
				for (unsigned i = 0; i < m_phyMacConfig->GetTotalNumChunk (); i++)
				{
					ueChunkMap.push_back (dci.m_rnti);
				}
				SfnSf slotSfn = m_currSchedConfig.m_sfAllocInfo.m_sfnSf;
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

		// get the next user
		m_itUeInfo++;
		if (m_itUeInfo == tempUeInfoMap.end ())
		{ // loop around to first RNTI in map
			m_itUeInfo = tempUeInfoMap.begin ();
		}
	}
	while (m_itUeInfo != itUeInfoStart); // break when looped back to initial RNTI

	// Move sched info into the actual proper map
	for (auto schedEntry : tempUeInfoMap)
	{
		m_ueInfo.insert (schedEntry);
	}
	tempUeInfoMap.clear ();
}

std::pair<uint8_t, uint32_t>
MmWaveRrIabMacScheduler::GetFreeSymbolsAndNextIndex(uint8_t symIdx, uint32_t numSymNeeded)
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
	return std::make_pair(symIdx, numFreeSymbols);
}

std::map <uint64_t, double> 
MmWaveRrIabMacScheduler::GenCumulativeBsrMap()
{
	NS_LOG_DEBUG (Now().GetMicroSeconds() << " [us] - Starting generation of IMSI-BSR map");
	NS_LOG_DEBUG ("DONOR: " << m_iabDonorScheduler);

	std::map <uint64_t, double> imsiBsrMap {};
	double ueCumulativeBufAmount {};
	uint8_t ueEntries{};
	bool isUe {};

	// Retrieve RLC DL buffers occupancies
	for (auto rlcDlIt : m_rlcBufferReq)
	{
		// Retrieve IMSI from RNTI
		uint64_t dlImsi {};
		if (m_iabDonorScheduler)
		{
			dlImsi = m_enbApp->GetImsiFromLocalRnti (rlcDlIt.m_rnti);
			isUe = !m_enbApp->IsImsiIab (dlImsi);
		}
		else
		{
			dlImsi = m_iabApp->GetImsiFromLocalRnti (rlcDlIt.m_rnti);
			isUe = !m_iabApp->IsImsiIab (dlImsi);
		}

		// Add/update corresponding entry
		auto dlMapIt = imsiBsrMap.find(dlImsi);
		if(dlMapIt != imsiBsrMap.end() && !isUe)
		{
			dlMapIt->second += rlcDlIt.m_rlcTransmissionQueueSize + rlcDlIt.m_rlcRetransmissionQueueSize;
		}
		else if (!isUe)
		{
			imsiBsrMap.insert (std::pair<uint64_t, double> (dlImsi, rlcDlIt.m_rlcTransmissionQueueSize + 
																	rlcDlIt.m_rlcRetransmissionQueueSize));
		}
		else 
		{
			// We have an UE: simply add to the cumulative UE buffer 
			ueCumulativeBufAmount += rlcDlIt.m_rlcTransmissionQueueSize + rlcDlIt.m_rlcRetransmissionQueueSize;
			ueEntries++;
		}
	}

	// Retrieve RLC UL buffers occupancies
	for (auto rlcUlIt : m_ceBsrRxed)
	{
		// Retrieve IMSI from RNTI
		uint64_t ulImsi {};
		if (m_iabDonorScheduler)
		{
			ulImsi = m_enbApp->GetImsiFromLocalRnti (rlcUlIt.first);
			isUe = !m_enbApp->IsImsiIab (ulImsi);
		}
		else
		{
			ulImsi = m_iabApp->GetImsiFromLocalRnti (rlcUlIt.first);
			isUe = !m_iabApp->IsImsiIab (ulImsi);
		}

		// Add/update corresponding entry
		auto ulMapit = imsiBsrMap.find(ulImsi);
		if(ulMapit != imsiBsrMap.end() && !isUe)
		{
			ulMapit->second += std::get<0>((rlcUlIt).second);
		}
		else if (!isUe)
		{
			imsiBsrMap.insert (std::pair<uint64_t, double > (rlcUlIt.first, std::get<0>((rlcUlIt).second)));
		}
		else
		{
			// We have an UE: simply add to the cumulative UE buffer 
			ueCumulativeBufAmount += std::get<0>((rlcUlIt).second);
			ueEntries++;
		}
	}

	// Insert if UEs have been found
	if (ueEntries++ > 0)
	{
		auto genMapIt = imsiBsrMap.find(m_representativeUeImsi);
		NS_ASSERT_MSG (genMapIt == imsiBsrMap.end (), "Such IMSI shuld not have been already inserted in the map!");
		imsiBsrMap.insert (std::pair<uint64_t, double > (m_representativeUeImsi, ueCumulativeBufAmount));
	}

	NS_LOG_DEBUG ("Completed BSR map generation!");
	return imsiBsrMap;
}

std::map <uint64_t, double> 
MmWaveRrIabMacScheduler::GenCumulativeCqiMap()
{
	NS_LOG_DEBUG (Now().GetMicroSeconds() << " [us] - Starting generation of IMSI-CQI map");
	NS_LOG_DEBUG ("DONOR: " << m_iabDonorScheduler);

	std::map <uint64_t, double> imsiCqiMap {};
	uint64_t ueCumulativeCqi {}, ueEntries {};
	double timeActive {m_phyMacConfig->GetSubframePeriod ()/m_phyMacConfig->GetSymbolPeriod ()};
	int tbSize {};
	bool isUe {};

	// Retrieve DL CQIs
	for (auto dlCqiIt : m_wbCqiRxed)
	{
		// Retrieve IMSI from RNTI
		uint64_t dlImsi {};
		if (m_iabDonorScheduler)
		{
			dlImsi = m_enbApp->GetImsiFromLocalRnti (dlCqiIt.first);
			isUe = !m_enbApp->IsImsiIab (dlImsi);
		}
		else
		{
			dlImsi = m_iabApp->GetImsiFromLocalRnti (dlCqiIt.first);
			isUe = !m_iabApp->IsImsiIab (dlImsi);
		}

		// Add/update corresponding entry
		auto dlMapIt = imsiCqiMap.find(dlImsi);
		// Obtain capacity estimate from CQI index
		tbSize = m_amc->GetTbSizeFromMcsSymbols (m_amc->GetMcsFromCqi (dlCqiIt.second), 1);
		NS_ASSERT (dlMapIt == imsiCqiMap.end());
		if (!isUe)
		{
			imsiCqiMap.insert (std::pair<uint64_t, double> (dlImsi, tbSize*timeActive)); // Microseconds -> seconds
		}
		else 
		{
			// We have an UE: simply add to the cumulative UEs CQI 
			ueCumulativeCqi += tbSize*timeActive; // Microseconds -> seconds
			ueEntries++;
		}
	}

	// Retrieve UL CQIs
	for (auto ulCqiIt : m_ueUlCqi)
	{
		// Compute overall CQI from CQI per RBG
		//uint64_t cqi {};
		int mcs {};
		SpectrumValue specVals (MmWaveSpectrumValueHelper::GetSpectrumModel (m_phyMacConfig));
		Values::iterator specIt = specVals.ValuesBegin();
		for (unsigned ichunk = 0; ichunk < m_phyMacConfig->GetTotalNumChunk (); ichunk++)
		{
			NS_ASSERT (specIt != specVals.ValuesEnd());
			*specIt = ulCqiIt.second.m_ueUlCqi.at (ichunk); //sinrLin;
			specIt++;
		}
		//cqi = m_amc->CreateCqiFeedbackWbTdma (specVals, ulCqiIt.second.m_numSym, ulCqiIt.second.m_tbSize, mcs);
		// Obtain capacity estimate from CQI index
		tbSize = m_amc->GetTbSizeFromMcsSymbols (mcs, 1);
		//NS_LOG_DEBUG ("This " << tbSize << " should be equal to: " << m_amc->GetTbSizeFromMcsSymbols (m_amc->GetMcsFromCqi (cqi), 1));	// TODO: why are they not equal??
		// Retrieve IMSI from RNTI
		uint64_t ulImsi {};
		if (m_iabDonorScheduler)
		{
			ulImsi = m_enbApp->GetImsiFromLocalRnti (ulCqiIt.first);
			isUe = !m_enbApp->IsImsiIab (ulImsi);
		}
		else
		{
			ulImsi = m_iabApp->GetImsiFromLocalRnti (ulCqiIt.first);
			isUe = !m_iabApp->IsImsiIab (ulImsi);
		}

		// Add/update corresponding entry
		auto ulMapit = imsiCqiMap.find(ulImsi);
		if(ulMapit != imsiCqiMap.end() && !isUe)
		{
			// We already have a DL entry: average the two (Round Robin scheduling among UL/DL)
			ulMapit->second = (ulMapit->second + tbSize*timeActive)/2; 
		}
		else if (!isUe)
		{
			imsiCqiMap.insert (std::pair<uint64_t, double > (ulCqiIt.first, tbSize*timeActive));
		}
		else
		{
			// We have an UE: simply add to the cumulative UEs CQI 
			ueCumulativeCqi += tbSize*timeActive;
			ueEntries++;
		}
	}

	// Insert if UEs have been found
	if (ueEntries > 0)
	{
		auto genMapIt = imsiCqiMap.find(m_representativeUeImsi);
		NS_ASSERT_MSG (genMapIt == imsiCqiMap.end (), "Such IMSI shuld not have been already inserted in the map!");
		imsiCqiMap.insert (std::pair<uint64_t, double > (m_representativeUeImsi, ueCumulativeCqi/ueEntries));
	}

	NS_LOG_DEBUG ("Completed CQI map generation!");
	return imsiCqiMap;
}

void
MmWaveRrIabMacScheduler::SetDonorControllerPtr (Ptr<MmWaveIabController> donorControllerPtr)
{
	m_donorControllerPtr = donorControllerPtr;
}

double 
MmWaveRrIabMacScheduler::GetIabMinimumCapacityEstimate ()
{
	auto minTbSize = m_amc->GetTbSizeFromMcsSymbols (m_amc->GetMcsFromCqi (1), 1);
	return minTbSize*m_phyMacConfig->GetSubframePeriod ()/m_phyMacConfig->GetSymbolPeriod ();
}

void 
MmWaveRrIabMacScheduler::ReceiveIabSchedIndication (uint64_t targetImsi, uint64_t iabNodeImsi)
{
	if (m_iabDonorScheduler)
	{
		NS_ASSERT (iabNodeImsi == 0);
	}
	else
	{
		NS_ASSERT (iabNodeImsi == m_iabApp->GetImsi ());
	}
	NS_LOG_DEBUG ("Received sched indication from central controller, which suggests to activate the link towards IMSI : " << targetImsi);

	m_suggestedImsi = targetImsi;
}

std::pair<uint64_t, SchedReportCallback>
MmWaveRrIabMacScheduler::GetSchedInfoRcvCallback ()
{
	uint64_t srcImsi {0};
	if(!m_iabDonorScheduler)
	{
		srcImsi = m_iabApp->GetImsi ();
	}
	return std::make_pair (srcImsi, m_schedInfoRcvCallback);
}

void
MmWaveRrIabMacScheduler::UpdateIabAllocation(std::map <uint16_t, struct UeSchedInfo> &ueInfo)
{
	// cycle through ueInfo and get the amount of symbols allocated to IABs or UEs
	int numIabDlSymbols = 0;
	int numIabUlSymbols = 0;
	int numIabFlowsDlAllocated = 0;
	int numIabFlowsUlAllocated = 0;
	int numUeDlSymbols = 0;
	int numUeUlSymbols = 0;
	int numUeFlowsDlAllocated = 0;
	int numUeFlowsUlAllocated = 0;
	for(auto itUeInfo = ueInfo.begin(); itUeInfo != ueInfo.end(); ++itUeInfo)
	{
		if(itUeInfo->second.m_iab)
		{
			if(itUeInfo->second.m_dlSymbols > 0)
			{
				numIabDlSymbols += itUeInfo->second.m_dlSymbols;
				numIabFlowsDlAllocated += 1;
			}
			if(itUeInfo->second.m_ulSymbols > 0)
			{
				numIabUlSymbols += itUeInfo->second.m_ulSymbols;
				numIabFlowsUlAllocated += 1;
			}

		}
		else
		{
			if(itUeInfo->second.m_dlSymbols > 0)
			{
				numUeDlSymbols += itUeInfo->second.m_dlSymbols;
				numUeFlowsDlAllocated += 1;
			}
			if(itUeInfo->second.m_ulSymbols > 0)
			{
				numUeUlSymbols += itUeInfo->second.m_ulSymbols;
				numUeFlowsUlAllocated += 1;
			}
		}
	}

	NS_LOG_DEBUG( 
		"numIabDlSymbols " << numIabDlSymbols <<
	 	" numIabUlSymbols " << numIabUlSymbols <<
	 	" numIabFlowsDlAllocated " << numIabFlowsDlAllocated <<
	 	" numIabFlowsUlAllocated " << numIabFlowsUlAllocated <<
	 	" numUeDlSymbols " << numUeDlSymbols <<
	 	" numUeUlSymbols " << numUeUlSymbols <<
	 	" numUeFlowsDlAllocated " << numUeFlowsDlAllocated <<
	 	" numUeFlowsUlAllocated " << numUeFlowsUlAllocated);

	if(numIabFlowsDlAllocated > 0 && numUeFlowsDlAllocated > 0)
	{
		// remove UE DL symbols
		int removeDlUeSymbols = std::floor(m_etaIab*numUeDlSymbols);
		int removeDlUeSymbolsPerUser = std::floor((double)removeDlUeSymbols/numUeFlowsDlAllocated);
		int totDlSymbolRemoved = numUeFlowsDlAllocated*removeDlUeSymbolsPerUser;
		int addDlIabSymbols = std::floor(totDlSymbolRemoved/numIabFlowsDlAllocated);
		int totDlSymbolsAdded = addDlIabSymbols*numIabFlowsDlAllocated;
		int extra = 0;
		if(totDlSymbolsAdded < totDlSymbolRemoved)
		{
			extra = totDlSymbolRemoved - totDlSymbolsAdded;
		}

		NS_LOG_DEBUG(
		" removeDlUeSymbols " << removeDlUeSymbols <<
		" removeDlUeSymbolsPerUser " << removeDlUeSymbolsPerUser <<
		" totDlSymbolRemoved " << totDlSymbolRemoved <<
		" addDlIabSymbols " << addDlIabSymbols <<
		" totDlSymbolsAdded " << totDlSymbolsAdded <<
		" extra " << extra
			);

		int actuallyAdded = 0;
		int actuallyRemoved = 0;
		int numIabFlowsWithSymbolIncrease = 0;
		int numUeFlowsWithSymbolIncrease = 0;
		if(removeDlUeSymbolsPerUser > 0 && addDlIabSymbols > 0)
		{
			// add the symbols to IAB nodes
			auto itUeInfo = ueInfo.begin();
			while(actuallyAdded < removeDlUeSymbols && numIabFlowsWithSymbolIncrease < numIabFlowsDlAllocated)
			{
				if(itUeInfo->second.m_iab && itUeInfo->second.m_rlcPduInfo.size() > 0)
				{
					int thisTimeAdded = 0;
					int diff = 0;
					while(actuallyAdded < removeDlUeSymbols && thisTimeAdded < addDlIabSymbols)
					{
						// add
						itUeInfo->second.m_dlSymbols += 1;
						actuallyAdded += 1;
					}
					if(thisTimeAdded > 0)
					{					
						if(itUeInfo->second.m_dlSymbols >= m_phyMacConfig->GetSymbolsPerSubframe()/2)
						{
							diff = m_phyMacConfig->GetSymbolsPerSubframe()/2 - itUeInfo->second.m_dlSymbols;
							itUeInfo->second.m_dlSymbols = m_phyMacConfig->GetSymbolsPerSubframe()/2;
							actuallyAdded -= diff;
						}
						numIabFlowsWithSymbolIncrease++;
					}

				}
				++itUeInfo;
				if(itUeInfo == ueInfo.end())
				{
					itUeInfo = ueInfo.begin();
				}
			}

			int removeDlUeSymbolsPerUserUpdated = std::ceil((double)actuallyAdded/numUeFlowsDlAllocated);
			NS_LOG_DEBUG(
				"actuallyAdded " << actuallyAdded 
				<< " toBeRemoved " << removeDlUeSymbolsPerUserUpdated
				<< " numUeFlowsDlAllocated " << numUeFlowsDlAllocated);

			while(actuallyRemoved < actuallyAdded && numUeFlowsWithSymbolIncrease < numUeFlowsDlAllocated)
			{
				//remove
				if(!itUeInfo->second.m_iab && itUeInfo->second.m_rlcPduInfo.size() > 0)
				{
					if(itUeInfo->second.m_dlSymbols < removeDlUeSymbolsPerUserUpdated)
					{
						actuallyRemoved += itUeInfo->second.m_dlSymbols;
						itUeInfo->second.m_dlSymbols = 0;
						numUeFlowsWithSymbolIncrease++;
					}
					else
					{
						actuallyRemoved += removeDlUeSymbolsPerUserUpdated;
						itUeInfo->second.m_dlSymbols -= removeDlUeSymbolsPerUserUpdated;
						numUeFlowsWithSymbolIncrease++;
					}
					
				}
				++itUeInfo;
				if(itUeInfo == ueInfo.end())
				{
					itUeInfo = ueInfo.begin();
				}
			}

			// for(auto itUeInfo = ueInfo.begin(); itUeInfo != ueInfo.end(); ++itUeInfo)
			// {
			// 	if(itUeInfo->second.m_iab && itUeInfo->second.m_rlcPduInfo.size() > 0)
			// 	{
			// 		// add
			// 		itUeInfo->second.m_dlSymbols += addDlIabSymbols;
			// 		actuallyAdded += addDlIabSymbols;
			// 		int diff = 0;
			// 		if(itUeInfo->second.m_dlSymbols >= m_phyMacConfig->GetSymbolsPerSubframe()/2)
			// 		{
			// 			diff = m_phyMacConfig->GetSymbolsPerSubframe()/2 - itUeInfo->second.m_dlSymbols;
			// 			itUeInfo->second.m_dlSymbols = m_phyMacConfig->GetSymbolsPerSubframe()/2;
			// 			actuallyAdded -= diff;
			// 		}
			// 	}
			// 	else
			// 	{
					
			// 	}
			// }

			NS_LOG_DEBUG("actuallyRemoved " << actuallyRemoved << " actuallyAdded " << actuallyAdded);

			if(actuallyRemoved < actuallyAdded)
			{
				NS_LOG_DEBUG("!!!!!!!!!!! warn");
				auto itUeInfo = ueInfo.begin();
				while(actuallyRemoved < actuallyAdded)
				{	
					if(itUeInfo->second.m_iab && itUeInfo->second.m_rlcPduInfo.size() > 0 && itUeInfo->second.m_dlSymbols > 0)
					{
						itUeInfo->second.m_dlSymbols--;
						actuallyAdded--;
					}
					itUeInfo++;
					if(itUeInfo == ueInfo.end())
					{
						itUeInfo = ueInfo.begin();
					}
				}
			}
		}
		
		NS_LOG_DEBUG("actuallyRemoved " << actuallyRemoved << " actuallyAdded " << actuallyAdded);
	}
}

int
MmWaveRrIabMacScheduler::GetNumFreeSymbols(uint8_t symIdx, int numSymNeeded)
{
	NS_LOG_LOGIC("symIdx " << (uint16_t)symIdx << " numSymNeeded " << numSymNeeded);

	if(!m_iabScheduler || !m_busyResourcesSchedSubframe.m_valid)
	{
		return std::min(numSymNeeded, (int)(m_phyMacConfig->GetSymbolsPerSubframe () - m_phyMacConfig->GetUlCtrlSymbols () - symIdx));
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

	numFreeSymbols = std::min(numFreeSymbols, (int)(m_phyMacConfig->GetSymbolsPerSubframe () - m_phyMacConfig->GetUlCtrlSymbols () - symIdx));
	NS_LOG_LOGIC("max free symbols " << (int)(m_phyMacConfig->GetSymbolsPerSubframe () - m_phyMacConfig->GetUlCtrlSymbols () - symIdx));
	return numFreeSymbols;
}

void
MmWaveRrIabMacScheduler::UpdateResourceMask(uint8_t start, int numSymbols)
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
MmWaveRrIabMacScheduler::GetFirstFreeSymbol(uint8_t symIdx, int numFreeSymbols)
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
MmWaveRrIabMacScheduler::CheckOverlapWithBusyResources(uint8_t symIdx, int numSymNeeded)
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
MmWaveRrIabMacScheduler::DoSchedUlMacCtrlInfoReq (const struct MmWaveMacSchedSapProvider::SchedUlMacCtrlInfoReqParameters& params)
{
	NS_LOG_FUNCTION (this);

	std::map <uint16_t, BufferSizeStatusSize_t>::iterator it;

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

			uint32_t buffer = 0;
			for (uint8_t lcg = 0; lcg < 4; ++lcg)
			{
				uint8_t bsrId = params.m_macCeList.at (i).m_macCeValue.m_bufferStatus.at (lcg);
				buffer += BsrId2BufferSize (bsrId);
			}

			uint32_t statusPduSize = BsrId2BufferSize(
				params.m_macCeList.at (i).m_macCeValue.m_totalPduSize);

			uint16_t rnti = params.m_macCeList.at (i).m_rnti;
			it = m_ceBsrRxed.find (rnti);
			if (it == m_ceBsrRxed.end ())
			{
				// create the new entry
				BufferSizeStatusSize_t info = std::make_pair(buffer, statusPduSize);
				m_ceBsrRxed.insert ( std::pair<uint16_t, BufferSizeStatusSize_t > (rnti, info));
				NS_LOG_DEBUG (this << " Insert RNTI " << rnti << " queue " << buffer << " status " << statusPduSize);
			}
			else
			{
				// update the buffer size value
				BufferSizeStatusSize_t info = std::make_pair(buffer, statusPduSize);
				(*it).second = info;
				NS_LOG_DEBUG (this << " Insert RNTI " << rnti << " queue " << buffer << " status " << statusPduSize);
			}
		}
	}

	return;
}

void
MmWaveRrIabMacScheduler::DoSchedSetMcs (int mcs)
{
	if (mcs >= 0 && mcs <= 28)
	{
		m_mcsDefaultDl = mcs;
		m_mcsDefaultUl = mcs;
	}
}

bool
MmWaveRrIabMacScheduler::SortRlcBufferReq (MmWaveMacSchedSapProvider::SchedDlRlcBufferReqParameters i, MmWaveMacSchedSapProvider::SchedDlRlcBufferReqParameters j)
{
  return (i.m_rnti < j.m_rnti);
}


void
MmWaveRrIabMacScheduler::RefreshDlCqiMaps (void)
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
MmWaveRrIabMacScheduler::RefreshUlCqiMaps (void)
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
          NS_LOG_DEBUG (this << " UL-CQI expired for user " << (*itUl).first);
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
MmWaveRrIabMacScheduler::UpdateDlRlcBufferInfo (uint16_t rnti, uint8_t lcid, uint16_t size)
{
  NS_LOG_FUNCTION (this);
  std::list<MmWaveMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator it;
  for (it = m_rlcBufferReq.begin (); it != m_rlcBufferReq.end (); it++)
  {
  	if (((*it).m_rnti == rnti) && ((*it).m_logicalChannelIdentity == lcid))
  	{
  		NS_LOG_INFO (this << " Update DL RLC BSR UE " << rnti << " LC " << (uint16_t)lcid << " txqueue " << (*it).m_rlcTransmissionQueueSize << " retxqueue " << (*it).m_rlcRetransmissionQueueSize << " status " << (*it).m_rlcStatusPduSize << " decrease " << size);
  		// Update queues: RLC tx order Status, ReTx, Tx
  		// Update status queue
  		if (((*it).m_rlcStatusPduSize > 0) && (size >= (*it).m_rlcStatusPduSize))
  		{
  			(*it).m_rlcStatusPduSize = 0;
  		}

  		if ((*it).m_rlcRetransmissionQueueSize > 0)
  		{
  			if ((*it).m_rlcRetransmissionQueueSize <= (unsigned)(size - (*it).m_rlcStatusPduSize))
  			{
  				(*it).m_rlcRetransmissionQueueSize = 0;
  			}
  			else
  			{
  				(*it).m_rlcRetransmissionQueueSize -= (size - (*it).m_rlcStatusPduSize);
  			}
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
  			if ((*it).m_rlcTransmissionQueueSize <= (size - rlcOverhead - (*it).m_rlcStatusPduSize))
  			{
  				(*it).m_rlcTransmissionQueueSize = 0;
  			}
  			else
  			{
  				(*it).m_rlcTransmissionQueueSize -= (size - rlcOverhead - (*it).m_rlcStatusPduSize);
  			}
  		}
  		return;
  	}
  }
}

void
MmWaveRrIabMacScheduler::UpdateUlRlcBufferInfo (uint16_t rnti, uint16_t size)
{

	size = size - 2; // remove the minimum RLC overhead
	auto it = m_ceBsrRxed.find (rnti);
	if (it != m_ceBsrRxed.end ())
	{
		uint32_t bufferSize = std::get<0>((*it).second);
		NS_LOG_INFO (this << " Update UL RLC BSR UE " << rnti << " size " 
			<< size << " BSR " << bufferSize << " statusPduSize " << std::get<1>((*it).second));
		if (bufferSize >= size)
		{
		  std::get<0>((*it).second) -= bufferSize;
		  // the status PDU will be served first
		  if (size >= std::get<1>((*it).second))
		  {
		  	std::get<1>((*it).second) = 0;
		  }
		}
		else
		{
		  std::get<0>((*it).second) = 0;
		  std::get<1>((*it).second) = 0;
		}
	}
	else
	{
	  NS_LOG_ERROR (this << " Does not find BSR report info of UE " << rnti);
	}

}


void
MmWaveRrIabMacScheduler::DoCschedCellConfigReq (const struct MmWaveMacCschedSapProvider::CschedCellConfigReqParameters& params)
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
MmWaveRrIabMacScheduler::DoCschedUeConfigReq (const struct MmWaveMacCschedSapProvider::CschedUeConfigReqParameters& params)
{
  NS_LOG_FUNCTION (this << " RNTI " << params.m_rnti << " txMode " << (uint16_t)params.m_transmissionMode);

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
MmWaveRrIabMacScheduler::DoCschedLcConfigReq (const struct MmWaveMacCschedSapProvider::CschedLcConfigReqParameters& params)
{
  NS_LOG_FUNCTION (this);
  // Not used at this stage (LCs updated by DoSchedDlRlcBufferReq)
  return;
}

void
MmWaveRrIabMacScheduler::DoCschedLcReleaseReq (const struct MmWaveMacCschedSapProvider::CschedLcReleaseReqParameters& params)
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
MmWaveRrIabMacScheduler::DoCschedUeReleaseReq (const struct MmWaveMacCschedSapProvider::CschedUeReleaseReqParameters& params)
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


