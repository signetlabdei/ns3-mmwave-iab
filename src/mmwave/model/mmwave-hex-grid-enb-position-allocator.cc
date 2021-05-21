/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 * Copyright (c) 2018 Università degli Studi di Padova
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
 * Author: Nicola Baldo <nbaldo@cttc.es>
 * Modified by: Tommaso Zugno <tommasozugno@gmail.com>
 */

#include <ns3/mmwave-hex-grid-enb-position-allocator.h>
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/log.h"
#include "ns3/mobility-model.h"

#include <cmath>

namespace ns3 {

  NS_LOG_COMPONENT_DEFINE ("MmWaveHexGridEnbPositionAllocator");

  NS_OBJECT_ENSURE_REGISTERED (MmWaveHexGridEnbPositionAllocator);

  TypeId
  MmWaveHexGridEnbPositionAllocator::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::MmWaveHexGridEnbPositionAllocator")
      .SetParent<PositionAllocator> ()
      .SetGroupName ("Mobility")
      .AddConstructor<MmWaveHexGridEnbPositionAllocator> ()
      .AddAttribute ("InterSiteDistance",
                     "The distance [m] between nearby sites",
                     DoubleValue (500),
                     MakeDoubleAccessor (&MmWaveHexGridEnbPositionAllocator::m_d),
                     MakeDoubleChecker<double> ())
      .AddAttribute ("SiteHeight",
                     "The height [m] of each site",
                     DoubleValue (30),
                     MakeDoubleAccessor (&MmWaveHexGridEnbPositionAllocator::m_siteHeight),
                     MakeDoubleChecker<double> ())
      .AddAttribute ("MinX", "The x coordinate where the hex grid starts.",
                     DoubleValue (0.0),
                     MakeDoubleAccessor (&MmWaveHexGridEnbPositionAllocator::m_xMin),
                     MakeDoubleChecker<double> ())
      .AddAttribute ("MinY", "The y coordinate where the hex grid starts.",
                     DoubleValue (0.0),
                     MakeDoubleAccessor (&MmWaveHexGridEnbPositionAllocator::m_yMin),
                     MakeDoubleChecker<double> ())
      .AddAttribute ("GridWidth", "The number of sites in the central row.",
                     UintegerValue (5),
                     MakeUintegerAccessor (&MmWaveHexGridEnbPositionAllocator::m_gridWidth),
                     MakeUintegerChecker<uint32_t> ())
    ;
    return tid;
  }

  MmWaveHexGridEnbPositionAllocator::MmWaveHexGridEnbPositionAllocator ()
  {
    m_isInitialized = false;
  }

  MmWaveHexGridEnbPositionAllocator::~MmWaveHexGridEnbPositionAllocator ()
  {
  }

  void MmWaveHexGridEnbPositionAllocator::Initialize ()
  {
    m_vIndex = - int (m_gridWidth / 2);
    NS_LOG_DEBUG ("m_vIndex " << m_vIndex << " " << m_gridWidth / 2 );
    m_hIndex = - std::ceil(double ((m_gridWidth - 1 - std::abs (m_vIndex))) / 2);
    NS_LOG_DEBUG ("m_hIndex " << m_hIndex << " " << m_gridWidth - 1 - std::abs (m_vIndex));

    // compute the maximum number of nodes that can fit in a circular grid with
    // this m_gridWidth
    m_nodeIndex = m_gridWidth;
    for (uint16_t i = 1; i <= (m_gridWidth - 1) / 2; i++)
    {
      m_nodeIndex = m_nodeIndex + (m_gridWidth - i) * 2;
    }
    NS_LOG_DEBUG ("m_nodeIndex " << m_nodeIndex);

    m_isInitialized = true;
  }

  Vector
  MmWaveHexGridEnbPositionAllocator::GetNext (void) const
  {
    NS_ASSERT_MSG (m_isInitialized, "Call Initialize () first!");

    NS_ASSERT_MSG (m_nodeIndex > 0, "No further nodes fit in the grid");

    Vector pos;
    double y;
    double x;

    if ((m_vIndex % 2) == 0) // even row
    {
      x = m_hIndex * m_d;
    }
    else // odd row
    {
      x = m_hIndex * m_d + 0.5 * m_d;
    }

    y = m_vIndex * std::sqrt (0.75) * m_d;

    NS_LOG_DEBUG ("m_hIndex " << m_hIndex << " m_vIndex " << m_vIndex);

    m_hIndex++;
    double rowLimit = std::floor(double (m_gridWidth - 1 - std::abs (m_vIndex)) / 2);
    if (m_hIndex > rowLimit)
    {
      m_vIndex++;
      m_hIndex = - std::ceil (double (m_gridWidth - 1 - std::abs (m_vIndex)) / 2);
    }

    pos.x = x;
    pos.y = y;
    pos.z = m_siteHeight;

    m_nodeIndex = m_nodeIndex - 1;
    NS_LOG_DEBUG ("m_nodeIndex " << m_nodeIndex);

    return pos;
  }

  int64_t
  MmWaveHexGridEnbPositionAllocator::AssignStreams (int64_t stream)
  {
    return 0;
  }

}
