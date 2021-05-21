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

#ifndef RRC_STATE_TRACE_H
#define RRC_STATE_TRACE_H

#include "ns3/uinteger.h"
#include "ns3/object.h"
#include <string>
#include <fstream>
#include <ns3/lte-enb-rrc.h>

namespace ns3
{

class RrcStateTrace : public Object
{
public:
  /**
   * Class constructor
   */
  RrcStateTrace ();

  /**
   * Class destructor
   */
  virtual
  ~RrcStateTrace ();

  // Inherited from ns3::Object
  /**
   *  Register this type.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);
  void DoDispose ();

  std::string GetRrcStateTraceFilename (void);
  
  void SetRrcStateTraceFilename (std::string outputFilename);

  void
  RegisterRrcEvent (uint64_t imsi, uint16_t cellId, uint16_t rnti, UeManager::State oldState, UeManager::State newState);
  
private:
  /**
   * Name of the file where the downlink PDCP statistics will be saved
   */
  std::string m_rrcStateTraceOutputFilename;

  std::ofstream m_rrcStateTraceFile;
};

} // namespace ns3

#endif /* RRC_STATE_TRACE_H */
