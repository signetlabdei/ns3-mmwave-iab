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

#include <ns3/mmwave-heterogeneous-layout-position-allocator.h>
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/log.h"
#include "ns3/mobility-model.h"

#include <cmath>

namespace ns3 {

  NS_LOG_COMPONENT_DEFINE ("MmWaveHeterogeneousLayoutPositionAllocator");

  NS_OBJECT_ENSURE_REGISTERED (MmWaveHeterogeneousLayoutPositionAllocator);

  TypeId
  MmWaveHeterogeneousLayoutPositionAllocator::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::MmWaveHeterogeneousLayoutPositionAllocator")
      .SetParent<PositionAllocator> ()
      .SetGroupName ("Mobility")
      .AddConstructor<MmWaveHeterogeneousLayoutPositionAllocator> ()
      .AddAttribute ("rho",
                     "The radius of the disc",
                     DoubleValue (0.0),
                     MakeDoubleAccessor (&MmWaveHeterogeneousLayoutPositionAllocator::SetRho),
                     MakeDoubleChecker<double> ())
      .AddAttribute ("X",
                     "The x coordinate of the center of the  disc.",
                     DoubleValue (0.0),
                     MakeDoubleAccessor (&MmWaveHeterogeneousLayoutPositionAllocator::SetX),
                     MakeDoubleChecker<double> ())
      .AddAttribute ("Y",
                     "The y coordinate of the center of the  disc.",
                     DoubleValue (0.0),
                     MakeDoubleAccessor (&MmWaveHeterogeneousLayoutPositionAllocator::SetY),
                     MakeDoubleChecker<double> ())
      .AddAttribute ("Z",
                     "The z coordinate of all the positions in the disc.",
                     DoubleValue (0.0),
                     MakeDoubleAccessor (&MmWaveHeterogeneousLayoutPositionAllocator::SetZ),
                     MakeDoubleChecker<double> ())
      .AddAttribute ("MinDistanceMacro",
                     "Minimum distance between macro and micro bs",
                     DoubleValue (0.0),
                     MakeDoubleAccessor (&MmWaveHeterogeneousLayoutPositionAllocator::m_minDistMacro),
                     MakeDoubleChecker<double> ())
      .AddAttribute ("MinDistanceMicro",
                     "Minimum distance between micro bs",
                     DoubleValue (0.0),
                     MakeDoubleAccessor (&MmWaveHeterogeneousLayoutPositionAllocator::m_minDistMicro),
                     MakeDoubleChecker<double> ())
    ;
    return tid;
  }

  MmWaveHeterogeneousLayoutPositionAllocator::MmWaveHeterogeneousLayoutPositionAllocator ()
  {
    m_udpa = CreateObject<UniformDiscPositionAllocator> ();
  }
  MmWaveHeterogeneousLayoutPositionAllocator::~MmWaveHeterogeneousLayoutPositionAllocator ()
  {
  }

  void
  MmWaveHeterogeneousLayoutPositionAllocator::SetRho (double rho)
  {
    m_udpa->SetRho (rho);
  }
  void
  MmWaveHeterogeneousLayoutPositionAllocator::SetX (double x)
  {
    m_udpa->SetX (x);
  }
  void
  MmWaveHeterogeneousLayoutPositionAllocator::SetY (double y)
  {
    m_udpa->SetY (y);
  }
  void
  MmWaveHeterogeneousLayoutPositionAllocator::SetZ (double z)
  {
    m_udpa->SetZ (z);
  }
  void
  MmWaveHeterogeneousLayoutPositionAllocator::SetMinDistanceMacroMicro (double r)
  {
    m_minDistMacro = r;
  }
  void
  MmWaveHeterogeneousLayoutPositionAllocator::SetMinDistanceMicroMicro (double r)
  {
    m_minDistMicro = r;
  }
  void
  MmWaveHeterogeneousLayoutPositionAllocator::SetMacroNodesContainer (NodeContainer macroNodes)
  {
    m_macroNodes = macroNodes;
  }
  void
  MmWaveHeterogeneousLayoutPositionAllocator::SetMicroNodesContainer (NodeContainer microNodes)
  {
    m_microNodes = microNodes;
  }
  Vector
  MmWaveHeterogeneousLayoutPositionAllocator::GetNext (void) const
  {
    Vector pos;
    do
    {
      pos = m_udpa->GetNext ();
      NS_LOG_DEBUG ("Trying pos " << pos);
    }
    while(CheckForSpecialConditions (pos));

    NS_LOG_DEBUG ("Final position " << pos);

    // add the position of this micro node in the list of the allocated
    // positions
    return pos;
  }

  bool
  MmWaveHeterogeneousLayoutPositionAllocator::CheckForSpecialConditions (Vector pos) const
  {
    // check if the position is far enough from the macro nodes
    for (uint8_t i = 0; i < m_macroNodes.GetN (); i++)
    {
      Vector macroPos = m_macroNodes.Get (i)->GetObject<MobilityModel> ()->GetPosition ();
      NS_LOG_DEBUG ("Distance from macro node " << (uint16_t)i << " :" << std::sqrt ((pos.x-macroPos.x)*(pos.x-macroPos.x)+(pos.y-macroPos.y)*(pos.y-macroPos.y)));
      if ((pos.x-macroPos.x)*(pos.x-macroPos.x)+(pos.y-macroPos.y)*(pos.y-macroPos.y) <= m_minDistMacro*m_minDistMacro)
      {
        NS_LOG_DEBUG ("Too close");
        return true;
      }
    }

    // check if the position if far enough from micro bs sites
    for (uint8_t i = 0; i < m_microNodes.GetN (); i++)
    {
      if (m_microNodes.Get (i)->GetObject<MobilityModel> ())
      {
        Vector microPos = m_microNodes.Get (i)->GetObject<MobilityModel> ()->GetPosition ();

        NS_LOG_DEBUG ("Distance from micro node in other site " << (uint16_t)i << " :" << std::sqrt ((pos.x-microPos.x)*(pos.x-microPos.x)+(pos.y-microPos.y)*(pos.y-microPos.y)));
        if ((pos.x-microPos.x)*(pos.x-microPos.x)+(pos.y-microPos.y)*(pos.y-microPos.y) <= m_minDistMicro*m_minDistMicro)
        {
          NS_LOG_DEBUG ("Too close");
          return true;
        }
      }
    }

    // the position is far enough from other macro and micro nodes
    return false;
  }

  int64_t
  MmWaveHeterogeneousLayoutPositionAllocator::AssignStreams (int64_t stream)
  {
    m_udpa->AssignStreams (stream);
    return 1;
  }
}
