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

#ifndef MMWAVE_HEX_GRID_ENB_POSITION_ALLOCATOR_H
#define MMWAVE_HEX_GRID_ENB_POSITION_ALLOCATOR_H

#include <ns3/position-allocator.h>
#include <ns3/node-container.h>

namespace ns3 {

/**
 * \ingroup mmwave
 *
 * This helper class allows to easily create a topology with eNBs
 * grouped in three-sector sites layed out on an hexagonal grid. The
 * layout is done row-wise. The code is adapted from lte-hex-grid-enb-topology-helper
 *
 */
class MmWaveHexGridEnbPositionAllocator : public PositionAllocator
{
public:
  /**
   * Register this type with the TypeId system.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  MmWaveHexGridEnbPositionAllocator ();
  virtual ~MmWaveHexGridEnbPositionAllocator ();

  virtual Vector GetNext (void) const;
  virtual int64_t AssignStreams (int64_t);

  /**
   * Set the initial values of m_hIndex and m_vIndex
   */
  void Initialize ();

private:
  /**
   * Node index. It is equal to the number of nodes that can fit in the circular
   * grid minus the number of nodes that have been alreay deployed
   */
  mutable uint32_t m_nodeIndex;
  
  /**
   * The current position in the horizontal axis
   */
  mutable int m_hIndex;

  /**
   * The current position in the vertical axis
   */
  mutable int m_vIndex;

  /**
   * The distance [m] between nearby sites
   */
  double m_d;

  /**
   * The x coordinate where the hex grid starts
   */
  double m_xMin;

  /**
   * The y coordinate where the hex grid starts
   */
  double m_yMin;

  /**
   * The number of sites in the central row
   */
  uint32_t m_gridWidth;

  /**
   * The height [m] of each site
   */
  uint32_t m_siteHeight;

  /**
   * True if Initialize is called
   */
  bool m_isInitialized;
};

} // namespace ns3

#endif /* MMWAVE_HEX_GRID_ENB_POSITION_ALLOCATOR_H */
