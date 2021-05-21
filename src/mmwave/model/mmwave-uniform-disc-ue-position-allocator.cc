/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Author: Tommaso Zugno <tommasozugno@gmail.com>
 */

#include <ns3/mmwave-uniform-disc-ue-position-allocator.h>
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/log.h"
#include "ns3/mobility-model.h"

#include <cmath>

namespace ns3 {

  NS_LOG_COMPONENT_DEFINE ("MmWaveUniformDiscUePositionAllocator");

  NS_OBJECT_ENSURE_REGISTERED (MmWaveUniformDiscUePositionAllocator);

  TypeId
  MmWaveUniformDiscUePositionAllocator::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::MmWaveUniformDiscUePositionAllocator")
      .SetParent<PositionAllocator> ()
      .SetGroupName ("Mobility")
      .AddConstructor<MmWaveUniformDiscUePositionAllocator> ()
      .AddAttribute ("rho",
                     "The radius of the disc",
                     DoubleValue (0.0),
                     MakeDoubleAccessor (&MmWaveUniformDiscUePositionAllocator::SetRho),
                     MakeDoubleChecker<double> ())
      .AddAttribute ("X",
                     "The x coordinate of the center of the  disc.",
                     DoubleValue (0.0),
                     MakeDoubleAccessor (&MmWaveUniformDiscUePositionAllocator::SetX),
                     MakeDoubleChecker<double> ())
      .AddAttribute ("Y",
                     "The y coordinate of the center of the  disc.",
                     DoubleValue (0.0),
                     MakeDoubleAccessor (&MmWaveUniformDiscUePositionAllocator::SetY),
                     MakeDoubleChecker<double> ())
      .AddAttribute ("Z",
                     "The z coordinate of all the positions in the disc.",
                     DoubleValue (0.0),
                     MakeDoubleAccessor (&MmWaveUniformDiscUePositionAllocator::SetZ),
                     MakeDoubleChecker<double> ())
      .AddAttribute ("r",
                     "The radius of the forbidden area around each eNB.",
                     DoubleValue (0.0),
                     MakeDoubleAccessor (&MmWaveUniformDiscUePositionAllocator::m_r),
                     MakeDoubleChecker<double> ())
    ;
    return tid;
  }

  MmWaveUniformDiscUePositionAllocator::MmWaveUniformDiscUePositionAllocator ()
  {
    m_udpa = CreateObject<UniformDiscPositionAllocator> ();
  }
  MmWaveUniformDiscUePositionAllocator::~MmWaveUniformDiscUePositionAllocator ()
  {
  }

  void
  MmWaveUniformDiscUePositionAllocator::SetRho (double rho)
  {
    m_udpa->SetRho (rho);
  }
  void
  MmWaveUniformDiscUePositionAllocator::SetX (double x)
  {
    m_udpa->SetX (x);
  }
  void
  MmWaveUniformDiscUePositionAllocator::SetY (double y)
  {
    m_udpa->SetY (y);
  }
  void
  MmWaveUniformDiscUePositionAllocator::SetZ (double z)
  {
    m_udpa->SetZ (z);
  }
  void
  MmWaveUniformDiscUePositionAllocator::SetR (double r)
  {
    m_r = r;
  }
  void
  MmWaveUniformDiscUePositionAllocator::SetEnbNodeContainer (NodeContainer enbNodes)
  {
    m_enbNodes = enbNodes;
  }
  Vector
  MmWaveUniformDiscUePositionAllocator::GetNext (void) const
  {
    Vector pos;
    bool exit;
    do
    {
      pos = m_udpa->GetNext ();
      NS_LOG_DEBUG ("Trying pos " << pos);
      exit = true;

      for (uint8_t i = 0; i < m_enbNodes.GetN (); i++)
      {
        Vector enbPos = m_enbNodes.Get (i)->GetObject<MobilityModel> ()->GetPosition ();
        NS_LOG_DEBUG ("Distance from enb " << (uint16_t)i << " :" << std::sqrt ((pos.x-enbPos.x)*(pos.x-enbPos.x)+(pos.y-enbPos.y)*(pos.y-enbPos.y)));
        if ((pos.x-enbPos.x)*(pos.x-enbPos.x)+(pos.y-enbPos.y)*(pos.y-enbPos.y) <= m_r*m_r)
        {
          NS_LOG_DEBUG ("Too close");
          exit = false;
          break;
        }
      }
    }
    while(!exit);

    NS_LOG_DEBUG ("Final position " << pos);
    return pos;
  }

  int64_t
  MmWaveUniformDiscUePositionAllocator::AssignStreams (int64_t stream)
  {
    m_udpa->AssignStreams (stream);
    return 1;
  }
}
