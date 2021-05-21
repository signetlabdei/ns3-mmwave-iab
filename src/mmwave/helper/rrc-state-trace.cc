/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016, University of Padova, Dep. of Information Engineering, SIGNET lab. 
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
 * Author: Michele Polese <michele.polese@gmail.com>
 */

#include "rrc-state-trace.h"
#include "ns3/string.h"
#include "ns3/nstime.h"
#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RrcStateTrace");

NS_OBJECT_ENSURE_REGISTERED ( RrcStateTrace);

/// Map each of UE Manager states to its string representation.
static const std::string g_ueManagerStateName[UeManager::NUM_STATES] =
{
  "INITIAL_RANDOM_ACCESS",
  "CONNECTION_SETUP",
  "CONNECTION_REJECTED",
  "CONNECTED_NORMALLY",
  "CONNECTION_RECONFIGURATION",
  "CONNECTION_REESTABLISHMENT",
  "HANDOVER_PREPARATION",
  "HANDOVER_JOINING",
  "HANDOVER_PATH_SWITCH",
  "HANDOVER_LEAVING",
  "PREPARE_MC_CONNECTION_RECONFIGURATION",
  "MC_CONNECTION_RECONFIGURATION"
};

/**
 * \param s The UE manager state.
 * \return The string representation of the given state.
 */
static const std::string & ToString (UeManager::State s)
{
  return g_ueManagerStateName[s];
}

RrcStateTrace::RrcStateTrace ()
  : m_rrcStateTraceOutputFilename ("RrcStateTrace.txt")
{
  NS_LOG_FUNCTION (this);
}

RrcStateTrace::~RrcStateTrace ()
{
  NS_LOG_FUNCTION (this);
  if(m_rrcStateTraceFile.is_open())
  {
    m_rrcStateTraceFile.close();
  }
}

TypeId
RrcStateTrace::GetTypeId (void)
{
  static TypeId tid =
    TypeId ("ns3::RrcStateTrace") 
    .SetParent<Object> ()
    .AddConstructor<RrcStateTrace> ()
    .SetGroupName("Lte")
    .AddAttribute ("RrcStateTraceFilename",
                   "Name of the file where the LTE Switch will be saved.",
                   StringValue ("StateTransitionTrace.txt"),
                   MakeStringAccessor (&RrcStateTrace::SetRrcStateTraceFilename),
                   MakeStringChecker ())
  ;
  return tid;
}

void
RrcStateTrace::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}

void 
RrcStateTrace::SetRrcStateTraceFilename (std::string outputFilename)
{
  m_rrcStateTraceOutputFilename = outputFilename;
  NS_LOG_UNCOND("m_rrcStateTraceOutputFilename " << m_rrcStateTraceOutputFilename);
}


std::string
RrcStateTrace::GetRrcStateTraceFilename() 
{
  return m_rrcStateTraceOutputFilename;
}

void
RrcStateTrace::RegisterRrcEvent (uint64_t imsi, uint16_t cellId, uint16_t rnti, UeManager::State oldState, UeManager::State newState)
{
  NS_LOG_FUNCTION (this << " RRC event " << cellId << imsi << rnti <<  ToString(oldState)  <<  ToString(newState) );

  if (!m_rrcStateTraceFile.is_open ())
  {
  	m_rrcStateTraceFile.open (GetRrcStateTraceFilename ().c_str ());
  }

  m_rrcStateTraceFile << Simulator::Now ().GetSeconds () << "\t" << imsi << '\t' << rnti <<
      '\t' << cellId << '\t' << ToString(oldState)  << '\t' << ToString(newState) << std::endl;
}

} // namespace ns3
