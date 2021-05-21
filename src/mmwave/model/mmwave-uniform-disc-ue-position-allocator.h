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

#ifndef MMWAVE_UNIFORM_DISC_UE_POSITION_ALLOCATOR_H
#define MMWAVE_UNIFORM_DISC_UE_POSITION_ALLOCATOR_H

#include <ns3/position-allocator.h>
#include <ns3/node-container.h>

namespace ns3 {
/**
 * \ingroup mobility
 * \brief Allocate the positions uniformely (with constant density) randomly within a disc
 *        and outside the eNBs forbidden areas.
 *
 * MmWaveUniformDiscUePositionAllocator allocates the positions randomly within a disc \f$ D \f$ lying on the
 * plane \f$ z\f$ and having center at coordinates \f$ (x,y,z) \f$
 * and radius \f$ \rho \f$. The random positions are chosen such that,
 * for any subset \f$ S \subset D \f$, the expected value of the
 * fraction of points which fall into \f$ S \subset D \f$ corresponds
 * to \f$ \frac{|S|}{|D|} \f$, i.e., to the ratio of the area of the
 * subset to the area of the whole disc. Moreover, the positions are chosen outside
 * the forbidden areas associated with each eNB in \f$ \enbNodes \f$
 *
 */
class MmWaveUniformDiscUePositionAllocator : public PositionAllocator
{
public:
  /**
   * Register this type with the TypeId system.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  MmWaveUniformDiscUePositionAllocator ();
  virtual ~MmWaveUniformDiscUePositionAllocator ();

  /**
   * \param rho the value of the radius of the disc
   */
  void SetRho (double rho);

  /**
   * \param x  the X coordinate of the center of the disc
   */
  void SetX (double x);

  /**
   * \param y   the Y coordinate of the center of the disc
   */
  void SetY (double y);

  /**
   * \param z   the Z coordinate of all the positions allocated
   */
  void SetZ (double z);

  /**
   * \param enbNodes   the list of the enb nodes
   */
  void SetEnbNodeContainer (NodeContainer enbNodes);

  /**
   * \param r   the radius of the forbidden area around each enb
   */
  void SetR (double r);

  virtual Vector GetNext (void) const;
  virtual int64_t AssignStreams (int64_t stream);
private:
  Ptr<UniformDiscPositionAllocator> m_udpa;  //!< pointer to uniform disc position allocator
  NodeContainer m_enbNodes; //!< list of eNB nodes
  double m_r;  //!< radius of the forbidden area around each eNB
};

} // namespace ns3

#endif /* MMWAVE_UNIFORM_DISC_UE_POSITION_ALLOCATOR_H */
