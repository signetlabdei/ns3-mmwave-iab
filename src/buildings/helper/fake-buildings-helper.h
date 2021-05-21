/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 CTTC
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
 */

#ifndef FAKE_BUILDINGS_HELPER_H
#define FAKE_BUILDINGS_HELPER_H

#include <string>
#include <ns3/attribute.h>
#include <ns3/object-factory.h>
#include <ns3/node-container.h>
#include <ns3/ptr.h>


namespace ns3 {

class MobilityModel;
class Building;

class FakeBuildingsHelper
{
public:
  /**
  * Install the MobilityBuildingInfo to a node
  *
  * \param node the mobility model of the node to be updated
  */
  static void Install (Ptr<Node> node);     // for any nodes
  /**
  * Install the MobilityBuildingInfo to the set of nodes in a NodeContainer
  *
  * \param c the NodeContainer including the nodes to be updated
  */
  static void Install (NodeContainer c);     // for any nodes
  /**
  * This method goes through the whole NodeList and, for each node in
  * the list, calls FakeBuildingsHelper::MakeConsistent() passing to it
  * the MobilityModel of that node.
  *
  * \param enbAndIabNodes the NodeContainer including the BS nodes
  * \param ueNodes the NodeContainer including the UE nodes
  * \param numberOfIndoorUes the the number of indoor UEs
  * \param numberOfLowLossUes the the number of UEs inside residential buildings
  */
  static void MakeMobilityModelConsistent (NodeContainer enbAndIabNodes, NodeContainer ueNodes, uint8_t numberOfIndoorUes, uint8_t numberOfLowLossUes);
  /**
  * Make the given mobility model consistent updating the MobilityBuildingInfo
  * object aggregated with the MobilityModel.
  * It defines numberOfIndoorUes UE as indoor (numberOfLowLossUes of them are inside
  * residential buildings, the other numberOfIndoorUes-numberOfLowLossUes are inside
  * commercial buildings) and the others as outdoor.
  *
  * \param bmm the mobility model to be made consistent
  * \param isIndoor true if the node is an indoor UE
  * \param isLowLoss true if the node is inside a residential building
  */
  static void MakeConsistent (Ptr<MobilityModel> bmm, bool isIndoor, bool isLowLoss);

};


}

#endif /* FAKE_BUILDINGS_HELPER_H */
