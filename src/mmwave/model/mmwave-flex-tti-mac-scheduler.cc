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
#include "mmwave-flex-tti-mac-scheduler.h"
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

NS_LOG_COMPONENT_DEFINE ("MmWaveFlexTtiMacScheduler");

NS_OBJECT_ENSURE_REGISTERED (MmWaveFlexTtiMacScheduler);

class MmWaveFlexTtiMacCschedSapProvider : public MmWaveMacCschedSapProvider
{
public:
  MmWaveFlexTtiMacCschedSapProvider (MmWaveFlexTtiMacScheduler* scheduler);

  // inherited from MmWaveMacCschedSapProvider
  virtual void CschedCellConfigReq (const struct MmWaveMacCschedSapProvider::CschedCellConfigReqParameters& params);
  virtual void CschedUeConfigReq (const struct MmWaveMacCschedSapProvider::CschedUeConfigReqParameters& params);
  virtual void CschedLcConfigReq (const struct MmWaveMacCschedSapProvider::CschedLcConfigReqParameters& params);
  virtual void CschedLcReleaseReq (const struct MmWaveMacCschedSapProvider::CschedLcReleaseReqParameters& params);
  virtual void CschedUeReleaseReq (const struct MmWaveMacCschedSapProvider::CschedUeReleaseReqParameters& params);

private:
  MmWaveFlexTtiMacCschedSapProvider ();
  MmWaveFlexTtiMacScheduler* m_scheduler;
};

MmWaveFlexTtiMacCschedSapProvider::MmWaveFlexTtiMacCschedSapProvider ()
{
}

MmWaveFlexTtiMacCschedSapProvider::MmWaveFlexTtiMacCschedSapProvider (MmWaveFlexTtiMacScheduler* scheduler)
	: m_scheduler (scheduler)
{
}

void
MmWaveFlexTtiMacCschedSapProvider::CschedCellConfigReq (const struct MmWaveMacCschedSapProvider::CschedCellConfigReqParameters& params)
{
  m_scheduler->DoCschedCellConfigReq (params);
}

void
MmWaveFlexTtiMacCschedSapProvider::CschedUeConfigReq (const struct MmWaveMacCschedSapProvider::CschedUeConfigReqParameters& params)
{
  m_scheduler->DoCschedUeConfigReq (params);
}


void
MmWaveFlexTtiMacCschedSapProvider::CschedLcConfigReq (const struct MmWaveMacCschedSapProvider::CschedLcConfigReqParameters& params)
{
  m_scheduler->DoCschedLcConfigReq (params);
}

void
MmWaveFlexTtiMacCschedSapProvider::CschedLcReleaseReq (const struct MmWaveMacCschedSapProvider::CschedLcReleaseReqParameters& params)
{
  m_scheduler->DoCschedLcReleaseReq (params);
}

void
MmWaveFlexTtiMacCschedSapProvider::CschedUeReleaseReq (const struct MmWaveMacCschedSapProvider::CschedUeReleaseReqParameters& params)
{
  m_scheduler->DoCschedUeReleaseReq (params);
}

class MmWaveFlexTtiMacSchedSapProvider : public MmWaveMacSchedSapProvider
{
public:
	MmWaveFlexTtiMacSchedSapProvider (MmWaveFlexTtiMacScheduler* sched);

	virtual void SchedDlRlcBufferReq (const struct MmWaveMacSchedSapProvider::SchedDlRlcBufferReqParameters& params);
	virtual void SchedTriggerReq (const struct MmWaveMacSchedSapProvider::SchedTriggerReqParameters& params);
	virtual void SchedDlCqiInfoReq (const struct MmWaveMacSchedSapProvider::SchedDlCqiInfoReqParameters& params);
	virtual void SchedUlCqiInfoReq (const struct MmWaveMacSchedSapProvider::SchedUlCqiInfoReqParameters& params);
	virtual void SchedUlMacCtrlInfoReq (const struct MmWaveMacSchedSapProvider::SchedUlMacCtrlInfoReqParameters& params);
	virtual void SchedSetMcs (int mcs);
private:
  MmWaveFlexTtiMacSchedSapProvider ();
	MmWaveFlexTtiMacScheduler* m_scheduler;
};

MmWaveFlexTtiMacSchedSapProvider::MmWaveFlexTtiMacSchedSapProvider ()
{
}

MmWaveFlexTtiMacSchedSapProvider::MmWaveFlexTtiMacSchedSapProvider (MmWaveFlexTtiMacScheduler* sched)
	:m_scheduler(sched)
{
}

void
MmWaveFlexTtiMacSchedSapProvider::SchedDlRlcBufferReq (const struct MmWaveMacSchedSapProvider::SchedDlRlcBufferReqParameters& params)
{
  m_scheduler->DoSchedDlRlcBufferReq (params);
}

void
MmWaveFlexTtiMacSchedSapProvider::SchedTriggerReq (const struct MmWaveMacSchedSapProvider::SchedTriggerReqParameters& params)
{
	m_scheduler->DoSchedTriggerReq(params);
}

void
MmWaveFlexTtiMacSchedSapProvider::SchedDlCqiInfoReq (const struct MmWaveMacSchedSapProvider::SchedDlCqiInfoReqParameters& params)
{
	m_scheduler->DoSchedDlCqiInfoReq (params);
}

void
MmWaveFlexTtiMacSchedSapProvider::SchedUlCqiInfoReq (const struct MmWaveMacSchedSapProvider::SchedUlCqiInfoReqParameters& params)
{
  m_scheduler->DoSchedUlCqiInfoReq (params);
}

void
MmWaveFlexTtiMacSchedSapProvider::SchedUlMacCtrlInfoReq (const struct MmWaveMacSchedSapProvider::SchedUlMacCtrlInfoReqParameters& params)
{
  m_scheduler->DoSchedUlMacCtrlInfoReq (params);
}

void
MmWaveFlexTtiMacSchedSapProvider::SchedSetMcs (int mcs)
{
	m_scheduler->DoSchedSetMcs (mcs);
}

const unsigned MmWaveFlexTtiMacScheduler::m_macHdrSize = 0;
const unsigned MmWaveFlexTtiMacScheduler::m_subHdrSize = 4;
const unsigned MmWaveFlexTtiMacScheduler::m_rlcHdrSize = 3;

const double MmWaveFlexTtiMacScheduler::m_berDl = 0.001;

MmWaveFlexTtiMacScheduler::MmWaveFlexTtiMacScheduler ()
: m_nextRnti (0),
  m_subframeNo (0),
  m_tbUid (0),
  m_macSchedSapUser (0),
	m_macCschedSapUser (0),
	m_maxSchedulingDelay (1),
	m_iabScheduler (false),
	m_split (true),
	m_etaIab (0)
{
	NS_LOG_FUNCTION (this);
	m_macSchedSapProvider = new MmWaveFlexTtiMacSchedSapProvider (this);
	m_macCschedSapProvider = new MmWaveFlexTtiMacCschedSapProvider (this);
	m_iabBackahulSapProvider = new MemberMmWaveUeMacCschedSapProvider<MmWaveFlexTtiMacScheduler> (this);

	m_iabBusySubframeAllocation.clear();
}

MmWaveFlexTtiMacScheduler::~MmWaveFlexTtiMacScheduler ()
{
	NS_LOG_FUNCTION (this);
}

void
MmWaveFlexTtiMacScheduler::DoDispose (void)
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
MmWaveFlexTtiMacScheduler::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::MmWaveFlexTtiMacScheduler")
	    .SetParent<MmWaveMacScheduler> ()
		.AddConstructor<MmWaveFlexTtiMacScheduler> ()
    .AddAttribute ("CqiTimerThreshold",
                   "The number of TTIs a CQI is valid (default 1000 - 1 sec.)",
                   UintegerValue (100),
                   MakeUintegerAccessor (&MmWaveFlexTtiMacScheduler::m_cqiTimersThreshold),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("HarqEnabled",
                   "Activate/Deactivate the HARQ [by default is active].",
                   BooleanValue (true),
                   MakeBooleanAccessor (&MmWaveFlexTtiMacScheduler::m_harqOn),
                   MakeBooleanChecker ())
	 .AddAttribute ("FixedMcsDl",
									"Fix MCS to value set in McsDlDefault (for testing)",
									BooleanValue (false),
									MakeBooleanAccessor (&MmWaveFlexTtiMacScheduler::m_fixedMcsDl),
									MakeBooleanChecker ())
	.AddAttribute ("McsDefaultDl",
								 "Fixed DL MCS (for testing)",
								 UintegerValue (1),
								 MakeUintegerAccessor (&MmWaveFlexTtiMacScheduler::m_mcsDefaultDl),
								 MakeUintegerChecker<uint8_t> ())
	 .AddAttribute ("FixedMcsUl",
									"Fix MCS to value set in McsUlDefault (for testing)",
									BooleanValue (false),
									MakeBooleanAccessor (&MmWaveFlexTtiMacScheduler::m_fixedMcsUl),
									MakeBooleanChecker ())
	.AddAttribute ("McsDefaultUl",
								 "Fixed UL MCS (for testing)",
								 UintegerValue (1),
								 MakeUintegerAccessor (&MmWaveFlexTtiMacScheduler::m_mcsDefaultUl),
								 MakeUintegerChecker<uint8_t> ())
	 .AddAttribute ("DlSchedOnly",
									"Only schedule downlink traffic (for testing)",
									BooleanValue (false),
									MakeBooleanAccessor (&MmWaveFlexTtiMacScheduler::m_dlOnly),
									MakeBooleanChecker ())
	 .AddAttribute ("UlSchedOnly",
									"Only schedule uplink traffic (for testing)",
									BooleanValue (false),
									MakeBooleanAccessor (&MmWaveFlexTtiMacScheduler::m_ulOnly),
									MakeBooleanChecker ())
	 .AddAttribute ("FixedTti",
									"Fix slot size",
									BooleanValue (false),
									MakeBooleanAccessor (&MmWaveFlexTtiMacScheduler::m_fixedTti),
									MakeBooleanChecker ())
	.AddAttribute ("SymPerSlot",
								 "Number of symbols per slot in Fixed TTI mode",
								 UintegerValue (6),
								 MakeUintegerAccessor (&MmWaveFlexTtiMacScheduler::m_symPerSlot),
								 MakeUintegerChecker<uint8_t> ())
	.AddAttribute ("EtaIab",
								"Balancing factor to assign more resources to IAB nodes",
								DoubleValue(0.0),
								MakeDoubleAccessor (&MmWaveFlexTtiMacScheduler::m_etaIab),
								MakeDoubleChecker<double> (0.0, 1.0)
								)
		;

	return tid;
}

void
MmWaveFlexTtiMacScheduler::SetMacSchedSapUser (MmWaveMacSchedSapUser* sap)
{
	m_macSchedSapUser = sap;
}

void
MmWaveFlexTtiMacScheduler::SetMacCschedSapUser (MmWaveMacCschedSapUser* sap)
{
	m_macCschedSapUser = sap;
}

void
MmWaveFlexTtiMacScheduler::SetIabScheduler(bool iabScheduler)
{
	m_iabScheduler = iabScheduler;
}

void
MmWaveFlexTtiMacScheduler::SetIabBsrMapReportCallback(BsrReportCallback infoSendCallback)
{
	NS_FATAL_ERROR ("This scheduler does not support central IAB controller yet!");
}

void 
MmWaveFlexTtiMacScheduler::SetIabCqiMapReportCallback(CqiReportCallback infoSendCallback)
{
	NS_FATAL_ERROR ("This scheduler does not support central IAB controller yet!");
}


MmWaveMacSchedSapProvider*
MmWaveFlexTtiMacScheduler::GetMacSchedSapProvider ()
{
	return m_macSchedSapProvider;
}

MmWaveMacCschedSapProvider*
MmWaveFlexTtiMacScheduler::GetMacCschedSapProvider ()
{
	return m_macCschedSapProvider;
}

// IAB methods
void
MmWaveFlexTtiMacScheduler::SetMmWaveUeMacCschedSapProvider(MmWaveUeMacCschedSapProvider* sap)
{
	m_iabBackahulSapProvider = sap;
}

MmWaveUeMacCschedSapProvider*
MmWaveFlexTtiMacScheduler::GetMmWaveUeMacCschedSapProvider()
{
	return m_iabBackahulSapProvider;
}

std::string
MmWaveFlexTtiMacScheduler::PrintSubframeAllocationMask(std::vector<bool> mask)
{
	std::stringstream strStream;
	for(auto bit : mask)
		strStream << bit << " ";
	return strStream.str();
}

// IAB methods
void
MmWaveFlexTtiMacScheduler::DoIabBackhaulSchedNotify(const struct MmWaveUeMacCschedSapProvider::IabBackhaulSchedInfo& info)
{
	NS_LOG_FUNCTION(this << info.m_dciInfoElementTdma.m_rnti);

	NS_ASSERT_MSG(m_iabScheduler, "Received DCI info for backhaul on a non IAB scheduler");

	NS_LOG_DEBUG("MmWaveFlexTtiMacScheduler received DCIs for the backhaul link");

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
		NS_LOG_DEBUG("This frame/subframe had already a DCI stored with mask " << PrintSubframeAllocationMask(currentInfo.m_symAllocationMask));
		NS_LOG_DEBUG("The new mask starts from " << (int)info.m_dciInfoElementTdma.m_symStart << " to " 
						<< (int) (info.m_dciInfoElementTdma.m_symStart + info.m_dciInfoElementTdma.m_numSym));
		newInfo = currentInfo;
		//TODOIAB Check whether the two DCIs are different perhaps?
			// TODOIAB plot relevant info
			// , with m_dlSymStart " << 
			// newInfo.m_dlSymStart << " m_dlNumSymAlloc " << newInfo.m_dlNumSymAlloc << " m_ulSymStart " <<
			// newInfo.m_ulSymStart << " m_ulSymStart " << newInfo.m_ulNumSymAlloc);
	}
	else
	{
		newInfo.m_sfnSf = info.m_sfnSf;
		uint32_t firstAllocatedIdx = info.m_dciInfoElementTdma.m_symStart;
		uint32_t nextFreeIdx = firstAllocatedIdx + info.m_dciInfoElementTdma.m_numSym;

		// check if it overlaps with already busy regions
		for(uint32_t index = firstAllocatedIdx; index < nextFreeIdx; index++)
		{
			NS_ASSERT_MSG(newInfo.m_symAllocationMask.at(index) == 0, "DCI signals that a symbol is scheduled for IAB, but it was already scheduled");
			newInfo.m_symAllocationMask.at(index) = 1;
		}	
	}
	
	NS_LOG_DEBUG("Mask " << PrintSubframeAllocationMask(newInfo.m_symAllocationMask));

	m_iabBusySubframeAllocation.at(subframe) = newInfo;
}

void
MmWaveFlexTtiMacScheduler::ConfigureCommonParameters (Ptr<MmWavePhyMacCommon> config)
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
MmWaveFlexTtiMacScheduler::DoSchedDlRlcBufferReq (const struct MmWaveMacSchedSapProvider::SchedDlRlcBufferReqParameters& params)
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
MmWaveFlexTtiMacScheduler::DoSchedDlCqiInfoReq (const struct MmWaveMacSchedSapProvider::SchedDlCqiInfoReqParameters& params)
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
MmWaveFlexTtiMacScheduler::DoSchedUlCqiInfoReq (const struct MmWaveMacSchedSapProvider::SchedUlCqiInfoReqParameters& params)
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
MmWaveFlexTtiMacScheduler::RefreshHarqProcesses ()
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
MmWaveFlexTtiMacScheduler::UpdateDlHarqProcessId (uint16_t rnti)
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
MmWaveFlexTtiMacScheduler::UpdateUlHarqProcessId (uint16_t rnti)
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

unsigned MmWaveFlexTtiMacScheduler::CalcMinTbSizeNumSym (unsigned mcs, unsigned bufSize, unsigned &tbSize)
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
MmWaveFlexTtiMacScheduler::GetNumIabRnti()
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
MmWaveFlexTtiMacScheduler::UpdateBusySymbolsForIab(uint8_t sfNum, uint8_t symIdx, int symAvail)
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
MmWaveFlexTtiMacScheduler::DoSchedTriggerReq (const struct MmWaveMacSchedSapProvider::SchedTriggerReqParameters& params)
{
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

	// TODOIAB in this version we only consider the same slot for UL & DL
	NS_ASSERT_MSG(sfNum == ulSfn.m_sfNum, "different subframes for DL and UL " << (uint32_t)sfNum << " " << (uint32_t)ulSfn.m_sfNum); 

	// IAB find the number of IAB devs that have requested resources via BSRs
	// uint16_t numIabDevs = GetNumIabRnti();
	// NS_LOG_DEBUG(this << " numIabDevs " << numIabDevs);

	// add slot for DL control
	SlotAllocInfo dlCtrlSlot (0, SlotAllocInfo::DL_slotAllocInfo, SlotAllocInfo::CTRL, SlotAllocInfo::DIGITAL, 0);
	dlCtrlSlot.m_dci.m_numSym = 1;
	dlCtrlSlot.m_dci.m_symStart = 0;
	ret.m_sfAllocInfo.m_slotAllocInfo.push_back (dlCtrlSlot);
	int resvCtrl = m_phyMacConfig->GetDlCtrlSymbols() + m_phyMacConfig->GetUlCtrlSymbols();
	int symAvail = m_phyMacConfig->GetSymbolsPerSubframe () - resvCtrl;
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
	std::map <uint16_t, struct UeSchedInfo> ueInfo;
	std::map <uint16_t, struct UeSchedInfo>::iterator itUeInfo;
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
			itUeInfo = ueInfo.find (rnti); // at the beginning, the ueInfo map is empty
			std::map <uint16_t, DlHarqProcessesStatus_t>::iterator itStat = m_dlHarqProcessesStatus.find (rnti);
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
				NS_LOG_DEBUG ("UE" << rnti << " DL harqId " << (unsigned)harqId << " HARQ-ACK received");
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
				NS_LOG_DEBUG ("UE" << rnti << " DL harqId " << (unsigned)harqId << " HARQ-NACK received, rv " << (unsigned)dciInfoReTx.m_rv);
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

				// (1) Check if the CQI has decreased. If no updated value available, use the same MCS minus 1.
				// 			If CQI is below min threshold, drop process.
				// (2) Calculate new number of symbols it will take to encode at lower MCS.
				//			If this exceeds the total number of symbols, reTX with original parameters.
				// 			If exceeds remaining symbols available in this subframe (but not total symbols in SF),
				//			update DCI info and try scheduling in next SF.

				/*std::map <uint16_t,uint8_t>::iterator itCqi = m_wbCqiRxed.find (itRlcBuf->m_rnti);
				int cqi;
				int mcsNew;
				if (itCqi != m_wbCqiRxed.end ())
				{
					cqi = itCqi->second;
					if (cqi == 0)
					{
						NS_LOG_INFO ("CQI for reTX is below threshhold. Drop process");
						itStat->second.at (harqId) = 0;
						for (uint16_t k = 0; k < (*itRlcPdu).second.size (); k++)
						{
							itRlcPdu->second.at (harqId).clear ();
						}
						continue;
					}
					else
					{
						mcsNew = m_amc->GetMcsFromCqi (cqi);  // get MCS
					}
				}
				else
				{
					if(dciInfoReTx.m_mcs > 0)
					{
						mcsNew = dciInfoReTx.m_mcs - 1;
					}
					else
					{
						mcsNew = dciInfoReTx.m_mcs;
					}
				}
				// compute number of symbols required
				unsigned numSymReq;
				if (mcsNew < dciInfoReTx.m_mcs)
				{
					numSymReq = m_amc->GetNumSymbolsFromTbsMcs (dciInfoReTx.m_tbSize, mcsNew);
					while (numSymReq < symAvail && mcsNew < dciInfoReTx.m_mcs);
					{
						mcsNew++;
						numSymReq = m_amc->GetNumSymbolsFromTbsMcs (dciInfoReTx.m_tbSize, mcsNew);
					}
					mcsNew--;
					numSymReq = m_amc->GetNumSymbolsFromTbsMcs (dciInfoReTx.m_tbSize, mcsNew);
				}
				if (numSymReq <= (m_phyMacConfig->GetSymbolsPerSubframe () - resvCtrl))
				{	// not enough symbols to encode TB at required MCS, attempt in later SF
					dlInfoListUntxed.push_back (m_dlHarqInfoList.at (i));
					continue;
				}*/

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
						SlotAllocInfo slotInfo (slotIdx++, SlotAllocInfo::DL_slotAllocInfo, SlotAllocInfo::CTRL_DATA, SlotAllocInfo::DIGITAL, itUeInfo->first);
						NS_LOG_DEBUG("itUeInfo->first " << itUeInfo->first << " dci rnti " << dciInfoReTx.m_rnti);
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
						if (itUeInfo == ueInfo.end())
						{
							itUeInfo = ueInfo.insert (std::pair<uint16_t, struct UeSchedInfo> (rnti, UeSchedInfo () )).first;
						}
						itUeInfo->second.m_dlSymbolsRetx = dciInfoReTx.m_numSym;
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
						if(numFreeSymbols >= numSymNeeded && (int)(tmpSymIdx + numSymNeeded - 1) < (int)(m_phyMacConfig->GetSymbolsPerSubframe () - m_phyMacConfig->GetUlCtrlSymbols ()))
						{
							NS_LOG_LOGIC("Found resources for DL HARQ RETX");
							symAvail -= dciInfoReTx.m_numSym;
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
							SlotAllocInfo slotInfo (slotIdx++, SlotAllocInfo::DL_slotAllocInfo, SlotAllocInfo::CTRL_DATA, SlotAllocInfo::DIGITAL, itUeInfo->first);
							NS_LOG_LOGIC("itUeInfo->first " << itUeInfo->first << " dci rnti " << dciInfoReTx.m_rnti);
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
							if (itUeInfo == ueInfo.end())
							{
								itUeInfo = ueInfo.insert (std::pair<uint16_t, struct UeSchedInfo> (rnti, UeSchedInfo () )).first;
							}
							itUeInfo->second.m_dlSymbolsRetx = dciInfoReTx.m_numSym;
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
					NS_LOG_DEBUG ("No resource for this retx -> buffer it");
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
			itUeInfo = ueInfo.find (rnti);
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

				NS_LOG_DEBUG("Retx num sym needed " << dciInfoReTx.m_numSym << " symIdx " << (uint16_t)symIdx);
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
						if (itUeInfo == ueInfo.end())
						{
							itUeInfo = ueInfo.insert (std::pair<uint16_t, struct UeSchedInfo> (rnti, UeSchedInfo () )).first;
						}
						itUeInfo->second.m_ulSymbolsRetx = dciInfoReTx.m_numSym;
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
						if(numFreeSymbols >= numSymNeeded && (int)(tmpSymIdx + numSymNeeded - 1) < (int)(m_phyMacConfig->GetSymbolsPerSubframe () - m_phyMacConfig->GetUlCtrlSymbols ()))
						{
							symAvail -= dciInfoReTx.m_numSym;
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
							SlotAllocInfo slotInfo (slotIdx++, SlotAllocInfo::UL_slotAllocInfo, SlotAllocInfo::CTRL_DATA, SlotAllocInfo::DIGITAL, rnti);
							slotInfo.m_dci = dciInfoReTx;
							NS_LOG_DEBUG ("UE" << dciInfoReTx.m_rnti << " gets UL slots " << (unsigned)dciInfoReTx.m_symStart << "-" << (unsigned)(dciInfoReTx.m_symStart+dciInfoReTx.m_numSym-1) <<
													 " tbs " << dciInfoReTx.m_tbSize << " harqId " << (unsigned)dciInfoReTx.m_harqProcess << " rv " << (unsigned)dciInfoReTx.m_rv << " in frame " << ulSfn.m_frameNum << " subframe " << (unsigned)ulSfn.m_sfNum <<
													 " RETX");
							ret.m_sfAllocInfo.m_slotAllocInfo.push_back (slotInfo);
							ret.m_sfAllocInfo.m_numSymAlloc += dciInfoReTx.m_numSym;
							if (itUeInfo == ueInfo.end())
							{
								itUeInfo = ueInfo.insert (std::pair<uint16_t, struct UeSchedInfo> (rnti, UeSchedInfo () )).first;
							}
							itUeInfo->second.m_ulSymbolsRetx = dciInfoReTx.m_numSym;
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
					NS_LOG_DEBUG ("No resource for this retx -> buffer it");
					ulInfoListUntxed.push_back (m_ulHarqInfoList.at (i));
				}
			}
		}

		m_ulHarqInfoList.clear ();
		m_ulHarqInfoList = ulInfoListUntxed;
	}

	NS_LOG_DEBUG("After HARQ allocation symAvail " << symAvail << " mask " << PrintSubframeAllocationMask(m_busyResourcesSchedSubframe.m_symAllocationMask));

	// ********************* END OF HARQ SECTION, START OF NEW DATA SCHEDULING ********************* //

	// number of DL/UL flows for new transmissions (not HARQ RETX)
	int nFlowsDl = 0;
	int nFlowsUl = 0;
	int nFlowsAccessDl = 0;
	int nFlowsAccessUl = 0;
	int nFlowsBackhaulDl = 0;
	int nFlowsBackhaulUl = 0;

	// get info on active DL flows
	if (symAvail > 0 && !m_ulOnly)  // remaining symbols in current subframe after HARQ retx sched
	{
		for (itRlcBuf = m_rlcBufferReq.begin (); itRlcBuf != m_rlcBufferReq.end (); itRlcBuf++)
		{
			itUeInfo = ueInfo.find (itRlcBuf->m_rnti);
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
					if (itUeInfo == ueInfo.end ())
					{
						itUeInfo = ueInfo.insert (std::pair<uint16_t, struct UeSchedInfo> (itRlcBuf->m_rnti, UeSchedInfo () )).first;

						nFlowsDl++;  // for simplicity, all RLC LCs are considered as a single flow
						if(!isIab)
						{
							nFlowsAccessDl++;
						}
						else
						{
							nFlowsBackhaulDl++;
							itUeInfo->second.m_iab = true;
						}
					}
					else if (itUeInfo->second.m_maxDlBufSize == 0)
					{
						nFlowsDl++;
						if(!isIab)
						{
							nFlowsAccessDl++;
						}
						else
						{
							nFlowsBackhaulDl++;
							itUeInfo->second.m_iab = true;
						}
					}
					NS_LOG_DEBUG("itUeInfo->second.m_iab " << itUeInfo->second.m_iab);
					if (m_fixedMcsDl)
					{
						itUeInfo->second.m_dlMcs = m_mcsDefaultDl;
					}
					else
					{
						itUeInfo->second.m_dlMcs = m_amc->GetMcsFromCqi (cqi);  // get MCS
					}

					// temporarily store the TX queue size
					if(itRlcBuf->m_rlcStatusPduSize > 0)
					{
						RlcPduInfo newRlcStatusPdu;
						newRlcStatusPdu.m_lcid = itRlcBuf->m_logicalChannelIdentity;
						newRlcStatusPdu.m_size += itRlcBuf->m_rlcStatusPduSize + m_subHdrSize;
						itUeInfo->second.m_rlcPduInfo.push_back (newRlcStatusPdu);
						itUeInfo->second.m_maxDlBufSize += newRlcStatusPdu.m_size;  // add to total DL buffer size
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
						itUeInfo->second.m_rlcPduInfo.push_back (newRlcEl);
						itUeInfo->second.m_maxDlBufSize += newRlcEl.m_size;  // add to total DL buffer size
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
	if (symAvail > 0 && !m_dlOnly)  // remaining symbols in future UL subframe after HARQ retx sched
	{
		// std::map <uint16_t,uint32_t>::iterator ceBsrIt;
		for(auto ceBsrIt : m_ceBsrRxed)
		{
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
						//double sinrLin = std::pow (10, itCqi->second.m_ueUlCqi.at (ichunk) / 10);
//						double se1 = log2 ( 1 + (std::pow (10, sinrLin / 10 )  /
//								( (-std::log (5.0 * m_berDl )) / 1.5) ));
//						cqi += m_amc->GetCqiFromSpectralEfficiency (se1);
						NS_ASSERT (specIt != specVals.ValuesEnd());
						*specIt = itCqi->second.m_ueUlCqi.at (ichunk); //sinrLin;
						specIt++;
					}

					cqi = m_amc->CreateCqiFeedbackWbTdma (specVals, itCqi->second.m_numSym, itCqi->second.m_tbSize, mcs);
//					for (unsigned i = 0; i < chunkCqi.size(); i++)
//					{
//						cqi += chunkCqi[i];
//					}
//					cqi = cqi / m_phyMacConfig->GetTotalNumChunk ();

					// take the lowest CQI value (worst chunk)
					//				double minSinr = itCqi->second.at (0);
					//				double sinrLinAvg = std::pow (10, itCqi->second.at (0) / 10);
					//				for (unsigned ichunk = 1; ichunk < m_phyMacConfig->GetTotalNumChunk (); ichunk++)
					//				{
					//					if (itCqi->second.at (ichunk) < minSinr)
					//					{
					//						minSinr = itCqi->second.at (ichunk);
					//					}
					//					sinrLinAvg += std::pow (10, itCqi->second.at (ichunk) / 10);
					//				}
					//				// TODO: verify SE calculation
					//				sinrLinAvg /= m_phyMacConfig->GetTotalNumChunk ();
					////				double se = log2 ( 1 + sinrLinAvg );
					//				double se = log2 ( 1 + (std::pow (10, minSinr / 10 )  /
					//						( (-std::log (5.0 * 0.00005 )) / 1.5) ));
					//				cqi = m_amc->GetCqiFromSpectralEfficiency (se);
					if (cqi == 0 && !m_fixedMcsUl) // out of range (SINR too low)
					{
						NS_LOG_DEBUG ("*** RNTI " << ceBsrIt.first << " UL-CQI out of range, skipping allocation in UL");
						// TODOIAB use continue and not break
						continue;  // do not allocate UE in uplink
					}
				}
				itUeInfo = ueInfo.find (rnti);
				if (itUeInfo == ueInfo.end ())
				{
					itUeInfo = ueInfo.insert (std::pair<uint16_t, struct UeSchedInfo> (rnti, UeSchedInfo () )).first;
					
					nFlowsUl++;
					if(!isIab)
					{
						nFlowsAccessUl++;
					}
					else
					{
						nFlowsBackhaulUl++;
						itUeInfo->second.m_iab = true;
					}
				}
				else if (itUeInfo->second.m_maxUlBufSize == 0)
				{
					nFlowsUl++;
					if(!isIab)
					{
						nFlowsAccessUl++;
					}
					else
					{
						nFlowsBackhaulUl++;
						itUeInfo->second.m_iab = true;
					}
				}
				NS_LOG_DEBUG("itUeInfo->second.m_iab " << itUeInfo->second.m_iab);
				if (m_fixedMcsUl)
				{
					itUeInfo->second.m_ulMcs = m_mcsDefaultUl;
				}
				else
				{
					itUeInfo->second.m_ulMcs = mcs;//m_amc->GetMcsFromCqi (cqi);  // get MCS
				}
				itUeInfo->second.m_maxUlBufSize = bufferSize + m_rlcHdrSize + m_macHdrSize + 8;
				itUeInfo->second.m_minSizeToBeScheduled = statusPduSize; // the status PDU cannot be split
			}
		}
	}

	int nFlowsTot = nFlowsDl + nFlowsUl;

	NS_LOG_DEBUG(this << " nFlowsDl " << nFlowsDl << " nFlowsUl " << nFlowsUl 
		<< " nFlowsAccessDl " << nFlowsAccessDl << " nFlowsAccessUl " << nFlowsAccessUl
		<< " nFlowsBackhaulDl " << nFlowsBackhaulDl << " nFlowBackhaulsUl " << nFlowsBackhaulUl);

	if (ueInfo.size () == 0)
	{
		// add slot for UL control
		SlotAllocInfo ulCtrlSlot (0xFF, SlotAllocInfo::UL_slotAllocInfo, SlotAllocInfo::CTRL, SlotAllocInfo::DIGITAL, 0);
		ulCtrlSlot.m_dci.m_numSym = 1;
		ulCtrlSlot.m_dci.m_symStart = m_phyMacConfig->GetSymbolsPerSubframe()-1;
		ret.m_sfAllocInfo.m_slotAllocInfo.push_back (ulCtrlSlot);
		m_macSchedSapUser->SchedConfigInd (ret);
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
	
	for (itUeInfo = ueInfo.begin (); itUeInfo != ueInfo.end (); itUeInfo++)
	{
		unsigned dlTbSize = 0;
		unsigned ulTbSize = 0;
		if (itUeInfo->second.m_maxDlBufSize > 0)
		{
			itUeInfo->second.m_maxDlSymbols = CalcMinTbSizeNumSym (itUeInfo->second.m_dlMcs, itUeInfo->second.m_maxDlBufSize, dlTbSize); // dlTbSize is passed as a parameter and not as a value, it will be updated with the tbSize
			// TODOIAB fire trace to store the m_maxDlSymbols
			itUeInfo->second.m_maxDlBufSize = dlTbSize;
			if (m_fixedTti)
			{
				itUeInfo->second.m_maxDlSymbols = ceil((double)itUeInfo->second.m_maxDlSymbols/(double)m_symPerSlot) * m_symPerSlot; // round up to nearest sym per TTI
			}
			totDlSymReq += itUeInfo->second.m_maxDlSymbols;
			NS_LOG_DEBUG("itUeInfo->second.m_iab " << itUeInfo->second.m_iab << " " << itUeInfo->first);
			if(itUeInfo->second.m_iab)
			{
				totBackhaulDlSymReq += itUeInfo->second.m_maxDlSymbols;
			}
			else
			{
				totAccessDlSymReq += itUeInfo->second.m_maxDlSymbols;
			}
		}
		if (itUeInfo->second.m_maxUlBufSize > 0)
		{
			itUeInfo->second.m_maxUlSymbols = CalcMinTbSizeNumSym (itUeInfo->second.m_ulMcs, itUeInfo->second.m_maxUlBufSize+10, ulTbSize);
			itUeInfo->second.m_maxUlBufSize = ulTbSize;
			if (m_fixedTti)
			{
				itUeInfo->second.m_maxUlSymbols = ceil((double)itUeInfo->second.m_maxUlSymbols/(double)m_symPerSlot) * m_symPerSlot; // round up to nearest sym per TTI
			}
			totUlSymReq += itUeInfo->second.m_maxUlSymbols;
			NS_LOG_DEBUG("itUeInfo->second.m_iab " << itUeInfo->second.m_iab << " " << itUeInfo->first);
			if(itUeInfo->second.m_iab)
			{
				totBackhaulUlSymReq += itUeInfo->second.m_maxUlSymbols;
			}
			else
			{
				totAccessUlSymReq += itUeInfo->second.m_maxUlSymbols;
			}
		}
	}

	NS_LOG_DEBUG(this << " totDlSymReq " << totDlSymReq << " totUlSymReq " << totUlSymReq 
		<< " totAccessDlSymReq " << totAccessDlSymReq << " totAccessUlSymReq " << totAccessUlSymReq
		<< " totBackhaulDlSymReq " << totBackhaulDlSymReq << " totBackhaulUlSymReq " << totBackhaulUlSymReq);

	// TODOIAB each IAB device cannot have more than half of the available resources
	// TODOIAB force the sum of m_maxUlSymbols and m_maxDlSymbols to be smaller than or equal to the following quantity
	int maxSymAvailableForIab = std::floor(symAvail/2);
	int removedSymbolsDl = 0;
	int removedSymbolsUl = 0;
	for(auto itUeInfo = ueInfo.begin(); itUeInfo != ueInfo.end(); ++itUeInfo)
	{
		if(itUeInfo->second.m_iab)
		{
			int totalSymbolsRequested = itUeInfo->second.m_maxUlSymbols + itUeInfo->second.m_maxDlSymbols;
			int oldUlSymbols = itUeInfo->second.m_maxUlSymbols;
			int oldDlSymbols = itUeInfo->second.m_maxDlSymbols;
			if(totalSymbolsRequested > maxSymAvailableForIab)
			{
				double scaleFactor = (double)maxSymAvailableForIab/(double)totalSymbolsRequested;
				NS_LOG_DEBUG("IAB with rnti " << itUeInfo->first << " requests too many symbols " << totalSymbolsRequested << " total available " 
					<< symAvail << " for IAB " << maxSymAvailableForIab << " scaleFactor " << scaleFactor);
				if(itUeInfo->second.m_maxUlSymbols > 0)
				{
					int newSymbols = std::max(1, (int)std::floor(scaleFactor*oldUlSymbols));
					removedSymbolsUl += (itUeInfo->second.m_maxUlSymbols - newSymbols);
					itUeInfo->second.m_maxUlSymbols = newSymbols;
				}
				if(itUeInfo->second.m_maxDlSymbols > 0)
				{
					int newSymbols = std::max(1, (int)std::floor(scaleFactor*oldDlSymbols));
					removedSymbolsDl += (itUeInfo->second.m_maxDlSymbols - newSymbols);
					itUeInfo->second.m_maxDlSymbols = newSymbols;
				}
				NS_LOG_DEBUG("New symbols request DL " << (uint32_t)itUeInfo->second.m_maxDlSymbols << 
					" UL " <<(uint32_t)itUeInfo->second.m_maxUlSymbols);
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

	std::map <uint16_t, struct UeSchedInfo>::iterator itUeInfoStart;
	if (m_nextRnti != 0) 	// start with RNTI at which the scheduler left off
	{
		itUeInfoStart = ueInfo.find (m_nextRnti);
		if (itUeInfoStart == ueInfo.end ())
		{
			itUeInfoStart = ueInfo.begin ();
		}
	}
	else	// start with first active RNTI
	{
		itUeInfoStart = ueInfo.begin ();
	}
	itUeInfo = itUeInfoStart;

	// divide OFDM symbols evenly between active UEs, which are then evenly divided between DL and UL flows
	if (nFlowsTot > 0)
	{
		int remSym = totDlSymReq + totUlSymReq;
		if (remSym > symAvail)
		{
			remSym = symAvail;
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
				NS_LOG_DEBUG("itUeInfo->second.m_maxDlSymbols " << (uint32_t)itUeInfo->second.m_maxDlSymbols);
				int deficit = itUeInfo->second.m_maxDlSymbols - itUeInfo->second.m_dlSymbols;
				NS_LOG_DEBUG("deficit " << deficit << " nSymPerFlow0 " << nSymPerFlow0 << " nRemSymPerFlow " << nRemSymPerFlow);
				NS_ASSERT (deficit >= 0);
				if (m_fixedTti)
				{
					deficit = ceil((double)deficit/(double)m_symPerSlot) * m_symPerSlot; // round up to nearest sym per TTI
				}
				if (deficit > 0 && ((itUeInfo->second.m_dlSymbols+itUeInfo->second.m_dlSymbolsRetx) <= nSymPerFlow0))
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
				itUeInfo->second.m_dlSymbols += addSym;
				remSym -= addSym;
				NS_ASSERT (remSym >= 0);

				addSym = 0;
				// deficit = difference between requested and allocated symbols

				NS_LOG_DEBUG("itUeInfo->second.m_maxUlSymbols " << (uint32_t)itUeInfo->second.m_maxUlSymbols);
				deficit = itUeInfo->second.m_maxUlSymbols - itUeInfo->second.m_ulSymbols;
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
				if (remSym > 0 && deficit > 0 && ((itUeInfo->second.m_ulSymbols+itUeInfo->second.m_ulSymbolsRetx) <= nSymPerFlow0))
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
				itUeInfo->second.m_ulSymbols += addSym;
				remSym -= addSym;
				NS_ASSERT (remSym >= 0);

				itUeInfo++;
				if (itUeInfo == ueInfo.end ())
				{ // loop around to first RNTI in map
					itUeInfo = ueInfo.begin ();
				}
				if (itUeInfo == itUeInfoStart)
				{ // break when looped back to initial RNTI or no symbols remain
					break;
				}
			}
		}
	}

	if(m_etaIab > 0 && (nFlowsBackhaulDl > 0 || nFlowsBackhaulUl >0))
	{
		// weight more the IAB devs than the other UEs
		UpdateIabAllocation(ueInfo);
	}

	m_nextRnti = itUeInfo->first;

	NS_LOG_DEBUG(this << " m_nextRnti " << m_nextRnti);

	// create DCI elements and assign symbol indices
	// such that all DL slots are contiguous (at beginning of subframe)
	// and all UL slots are contiguous (at end of subframe)
	itUeInfo = itUeInfoStart;

	//ulSymIdx -= totUlSymActual; // symbols reserved for control at end of subframe before UL ctrl
	NS_ASSERT (symIdx > 0);
	do
	{
		UeSchedInfo &ueSchedInfo = itUeInfo->second;
		if (ueSchedInfo.m_dlSymbols > 0 && symAvail > 0)
		{
			NS_LOG_INFO("num LC before allocation " << (uint32_t)ueSchedInfo.m_rlcPduInfo.size ());
			std::vector<SlotAllocInfo> tmpSlotAllocVector;
			int numSymNeeded = ueSchedInfo.m_dlSymbols;
			int totalTbSize = 0;
			NS_LOG_DEBUG("UE " << itUeInfo->first << " numSymNeeded " << numSymNeeded);
			do
			{
				auto symIdxNumFreeSymbols = GetFreeSymbolsAndNextIndex(symIdx, numSymNeeded);
				int numFreeSymbols = std::get<1>(symIdxNumFreeSymbols);
				symIdx = std::get<0>(symIdxNumFreeSymbols);
				symAvail -= numFreeSymbols;
				numSymNeeded -= numFreeSymbols;
				NS_LOG_DEBUG("UE " << itUeInfo->first << " numSymNeeded (after allocation) "
					<< numSymNeeded << " numFreeSymbols " << numFreeSymbols << " symAvail " << symAvail);

				// create the DCI
				DciInfoElementTdma dci;
				dci.m_rnti = itUeInfo->first;
				dci.m_format = 0;
				dci.m_symStart = symIdx;
				dci.m_numSym = numFreeSymbols;
				symIdx = GetFirstFreeSymbol(symIdx, numFreeSymbols); // get the next symIdx
				NS_LOG_LOGIC("Next symIdx " << (uint16_t)symIdx);
				dci.m_ndi = 1;
				dci.m_mcs = ueSchedInfo.m_dlMcs;
				dci.m_tbSize = m_amc->GetTbSizeFromMcsSymbols (dci.m_mcs, dci.m_numSym) / 8;
				totalTbSize += dci.m_tbSize;
		
				NS_ASSERT (symIdx <= m_phyMacConfig->GetSymbolsPerSubframe () - m_phyMacConfig->GetUlCtrlSymbols ());
				dci.m_rv = 0;
				dci.m_harqProcess = UpdateDlHarqProcessId (itUeInfo->first);
				NS_ASSERT (dci.m_harqProcess < m_phyMacConfig->GetNumHarqProcess ());
				NS_LOG_DEBUG ("UE" << itUeInfo->first << " DL harqId " << (unsigned)dci.m_harqProcess << " HARQ process assigned");
				SlotAllocInfo slotInfo (slotIdx++, SlotAllocInfo::DL_slotAllocInfo, SlotAllocInfo::CTRL_DATA, SlotAllocInfo::DIGITAL, itUeInfo->first);
				slotInfo.m_dci = dci;
				NS_LOG_DEBUG (this << " UE " << dci.m_rnti << " gets DL slots " << (unsigned)dci.m_symStart << "-" << (unsigned)(dci.m_symStart+dci.m_numSym-1) <<
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
			unsigned numLc = ueSchedInfo.m_rlcPduInfo.size ();
			unsigned bytesRem = totalTbSize;
			unsigned numFulfilled = 0;
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
					UpdateDlRlcBufferInfo (itUeInfo->first, pduIterator->m_lcid, std::min(remTbSize, (int)pduIterator->m_size) - m_subHdrSize);

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
						std::map <uint16_t, DlHarqRlcPduList_t>::iterator itRlcPdu =  m_dlHarqProcessesRlcPduMap.find (itUeInfo->first);
						if (itRlcPdu == m_dlHarqProcessesRlcPduMap.end ())
						{
							NS_FATAL_ERROR ("Unable to find RlcPdcList in HARQ buffer for RNTI " << itUeInfo->first);
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

		// UL DCI applies to subframe i+Tsched
		if (ueSchedInfo.m_ulSymbols > 0 && symAvail > 0)
		{
			int numSymNeeded = ueSchedInfo.m_ulSymbols;
			do
			{
				int numFreeSymbols = 0;
				uint32_t minTbSizeToBeScheduled = ueSchedInfo.m_minSizeToBeScheduled;
				int minNumSymNeeded = 1;
				bool useTmpSymIdx = false;
				bool doNotScheduleStatusPduInThisDci = false;
				uint8_t tmpSymIdx = symIdx;
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

					if(numFreeSymbols >= minNumSymNeeded && 
						(int)(tmpSymIdx + minNumSymNeeded - 1) < (int)(m_phyMacConfig->GetSymbolsPerSubframe () - m_phyMacConfig->GetUlCtrlSymbols ()))
					{
						// check how many additional symbols can actually be allocated here
						if (numSymNeeded > minNumSymNeeded)
						{
							numFreeSymbols = GetNumFreeSymbols(tmpSymIdx, numSymNeeded);
						}
						NS_LOG_DEBUG("UL TX numSymNeeded " << numSymNeeded << " numFreeSymbols " 
							<< numFreeSymbols << " tmpSymIdx " << (uint16_t)tmpSymIdx);
						if(tmpSymIdx != symIdx)
						{
							useTmpSymIdx = true;
							NS_LOG_DEBUG("The status PDU can be allocated, not at symIdx " << (uint32_t)symIdx << " but at " << (uint32_t)tmpSymIdx);
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
						auto symIdxNumFreeSymbols = GetFreeSymbolsAndNextIndex(symIdx, numSymNeeded);
						numFreeSymbols = std::get<1>(symIdxNumFreeSymbols);
						symIdx = std::get<0>(symIdxNumFreeSymbols);
					}
				}
				else // just find the first free spot
				{
					NS_LOG_DEBUG("No status PDU to be scheduled");
					auto symIdxNumFreeSymbols = GetFreeSymbolsAndNextIndex(symIdx, numSymNeeded);
					numFreeSymbols = std::get<1>(symIdxNumFreeSymbols);
					symIdx = std::get<0>(symIdxNumFreeSymbols);
				}
				
				symAvail -= numFreeSymbols;
				numSymNeeded -= numFreeSymbols;
				NS_LOG_DEBUG("UE " << itUeInfo->first << " numSymNeeded (after allocation) " << numSymNeeded 
					<< " numFreeSymbols " << numFreeSymbols << " symAvail " << symAvail);

				DciInfoElementTdma dci;
				dci.m_rnti = itUeInfo->first;
				dci.m_format = 1;
				dci.m_numSym = numFreeSymbols;
				dci.m_symStart = useTmpSymIdx ? tmpSymIdx : symIdx;
				if(!useTmpSymIdx)
				{
					symIdx = GetFirstFreeSymbol(symIdx, numFreeSymbols); // get the next free symbol
				}
				// if the allocation is done at tmpSymIdx, then this will return symIdx
				NS_LOG_DEBUG("Next symIdx " << (uint16_t)symIdx);
				dci.m_mcs = ueSchedInfo.m_ulMcs;
				dci.m_ndi = 1;
				dci.m_tbSize = m_amc->GetTbSizeFromMcsSymbols (dci.m_mcs, dci.m_numSym) / 8;
				dci.m_doNotScheduleStatusPduInThisDci = doNotScheduleStatusPduInThisDci;

				NS_ASSERT (symIdx <= m_phyMacConfig->GetSymbolsPerSubframe () - m_phyMacConfig->GetUlCtrlSymbols ());
				dci.m_harqProcess = UpdateUlHarqProcessId (itUeInfo->first);
				NS_LOG_DEBUG ("UE" << itUeInfo->first << " UL harqId " << (unsigned)dci.m_harqProcess << " HARQ process assigned");
				NS_ASSERT (dci.m_harqProcess < m_phyMacConfig->GetNumHarqProcess ());
				SlotAllocInfo slotInfo (slotIdx++, SlotAllocInfo::UL_slotAllocInfo, SlotAllocInfo::CTRL_DATA, SlotAllocInfo::DIGITAL, itUeInfo->first);
				slotInfo.m_dci = dci;
				NS_LOG_DEBUG (this << " UE" << dci.m_rnti << " gets UL slots " << (unsigned)dci.m_symStart << "-" << (unsigned)(dci.m_symStart+dci.m_numSym-1) <<
							             " tbs " << dci.m_tbSize << " mcs " << (unsigned)dci.m_mcs << " harqId " << (unsigned)dci.m_harqProcess << " rv " << (unsigned)dci.m_rv << " in frame " << ulSfn.m_frameNum << " subframe " << (unsigned)ulSfn.m_sfNum);
				UpdateUlRlcBufferInfo (itUeInfo->first, dci.m_tbSize - m_subHdrSize);
				ret.m_sfAllocInfo.m_slotAllocInfo.push_back (slotInfo);  // add to front
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

		// get the next user
		itUeInfo++;
		if (itUeInfo == ueInfo.end ())
		{ // loop around to first RNTI in map
			itUeInfo = ueInfo.begin ();
		}
	}
	while (itUeInfo != itUeInfoStart); // break when looped back to initial RNTI

	// add slot for UL control
	SlotAllocInfo ulCtrlSlot (0xFF, SlotAllocInfo::UL_slotAllocInfo, SlotAllocInfo::CTRL, SlotAllocInfo::DIGITAL, 0);
	ulCtrlSlot.m_dci.m_numSym = 1;
	ulCtrlSlot.m_dci.m_symStart = m_phyMacConfig->GetSymbolsPerSubframe()-1;
	ret.m_sfAllocInfo.m_slotAllocInfo.push_back (ulCtrlSlot);

	std::sort(ret.m_sfAllocInfo.m_slotAllocInfo.begin(), ret.m_sfAllocInfo.m_slotAllocInfo.end(), BySymIndex());

	m_macSchedSapUser->SchedConfigInd (ret);
	return;
}

std::pair<uint8_t, uint32_t>
MmWaveFlexTtiMacScheduler::GetFreeSymbolsAndNextIndex(uint8_t symIdx, uint32_t numSymNeeded)
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

void
MmWaveFlexTtiMacScheduler::UpdateIabAllocation(std::map <uint16_t, struct UeSchedInfo> &ueInfo)
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
MmWaveFlexTtiMacScheduler::GetNumFreeSymbols(uint8_t symIdx, int numSymNeeded)
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
	return std::min(numSymNeeded, (int)(m_phyMacConfig->GetSymbolsPerSubframe () - m_phyMacConfig->GetUlCtrlSymbols () - symIdx));
}

void
MmWaveFlexTtiMacScheduler::UpdateResourceMask(uint8_t start, int numSymbols)
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
MmWaveFlexTtiMacScheduler::GetFirstFreeSymbol(uint8_t symIdx, int numFreeSymbols)
{
	NS_LOG_LOGIC("symIdx " << (uint16_t)symIdx << " numFreeSymbols " << numFreeSymbols);

	if(!m_iabScheduler || !m_busyResourcesSchedSubframe.m_valid)
	{
		return std::min(symIdx + numFreeSymbols, (int)(m_phyMacConfig->GetSymbolsPerSubframe () - m_phyMacConfig->GetUlCtrlSymbols () - 1));
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
MmWaveFlexTtiMacScheduler::CheckOverlapWithBusyResources(uint8_t symIdx, int numSymNeeded)
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
MmWaveFlexTtiMacScheduler::DoSchedUlMacCtrlInfoReq (const struct MmWaveMacSchedSapProvider::SchedUlMacCtrlInfoReqParameters& params)
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
MmWaveFlexTtiMacScheduler::DoSchedSetMcs (int mcs)
{
	if (mcs >= 0 && mcs <= 28)
	{
		m_mcsDefaultDl = mcs;
		m_mcsDefaultUl = mcs;
	}
}

bool
MmWaveFlexTtiMacScheduler::SortRlcBufferReq (MmWaveMacSchedSapProvider::SchedDlRlcBufferReqParameters i, MmWaveMacSchedSapProvider::SchedDlRlcBufferReqParameters j)
{
  return (i.m_rnti < j.m_rnti);
}


void
MmWaveFlexTtiMacScheduler::RefreshDlCqiMaps (void)
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
MmWaveFlexTtiMacScheduler::RefreshUlCqiMaps (void)
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
MmWaveFlexTtiMacScheduler::UpdateDlRlcBufferInfo (uint16_t rnti, uint8_t lcid, uint16_t size)
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
MmWaveFlexTtiMacScheduler::UpdateUlRlcBufferInfo (uint16_t rnti, uint16_t size)
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
MmWaveFlexTtiMacScheduler::DoCschedCellConfigReq (const struct MmWaveMacCschedSapProvider::CschedCellConfigReqParameters& params)
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
MmWaveFlexTtiMacScheduler::DoCschedUeConfigReq (const struct MmWaveMacCschedSapProvider::CschedUeConfigReqParameters& params)
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
MmWaveFlexTtiMacScheduler::DoCschedLcConfigReq (const struct MmWaveMacCschedSapProvider::CschedLcConfigReqParameters& params)
{
  NS_LOG_FUNCTION (this);
  // Not used at this stage (LCs updated by DoSchedDlRlcBufferReq)
  return;
}

void
MmWaveFlexTtiMacScheduler::DoCschedLcReleaseReq (const struct MmWaveMacCschedSapProvider::CschedLcReleaseReqParameters& params)
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
MmWaveFlexTtiMacScheduler::DoCschedUeReleaseReq (const struct MmWaveMacCschedSapProvider::CschedUeReleaseReqParameters& params)
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


