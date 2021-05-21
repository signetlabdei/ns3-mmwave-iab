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

#include "fake-buildings-helper.h"

#include <ns3/node-list.h>
#include <ns3/building.h>
#include <ns3/building-list.h>
#include <ns3/mobility-model.h>
#include <ns3/mobility-building-info.h>
#include <ns3/abort.h>
#include <ns3/log.h>
#include "ns3/random-variable-stream.h"
#include "ns3/double.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("FakeBuildingsHelper");

void
FakeBuildingsHelper::Install (NodeContainer c)
{
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
  {
    Install (*i);
  }
}


void
FakeBuildingsHelper::Install (Ptr<Node> node)
{
  Ptr<Object> object = node;
  Ptr<MobilityModel> model = object->GetObject<MobilityModel> ();
  if (model == 0)
  {
    NS_ABORT_MSG_UNLESS (0 != model, "node " << node->GetId () << " does not have a MobilityModel");

  }
  Ptr<MobilityBuildingInfo> buildingInfo = CreateObject<MobilityBuildingInfo> ();
  model->AggregateObject (buildingInfo);
}


void
FakeBuildingsHelper::MakeMobilityModelConsistent (NodeContainer enbAndIabNodes, NodeContainer ueNodes, uint8_t numberOfIndoorUes, uint8_t numberOfLowLossUes)
{
  NS_LOG_FUNCTION_NOARGS ();

  NS_ASSERT_MSG (numberOfIndoorUes <= ueNodes.GetN (), "numberOfIndoorUes > total number of UEs");
  NS_ASSERT_MSG (numberOfIndoorUes >= numberOfLowLossUes, "numberOfIndoorUes < numberOfLowLossUes");

  uint32_t indoorCount = 0;
  uint32_t lowLossCount = 0;

  // Create the random variables used for the computation of the height of
  // each user as described in TR 36.873
  Ptr<UniformRandomVariable> N_fl = CreateObject<UniformRandomVariable>();
  N_fl->SetAttribute ("Min", DoubleValue (4.0));
  N_fl->SetAttribute ("Max", DoubleValue (8.0));

  Ptr<UniformRandomVariable> n_fl = CreateObject<UniformRandomVariable>();
  n_fl->SetAttribute ("Min", DoubleValue (1.0));

  for (NodeContainer::Iterator nit = ueNodes.Begin (); nit != ueNodes.End (); ++nit)
  {
    Ptr<MobilityModel> mm = (*nit)->GetObject<MobilityModel> ();
    bool isIndoor = false;
    bool isLowLoss = false;
    if (mm != 0)
    {
      if (indoorCount < numberOfIndoorUes)
      {
        isIndoor = true;
        indoorCount++;

        if (lowLossCount < numberOfLowLossUes)
        {
          isLowLoss = true;
          lowLossCount++;
        }
      }

      MakeConsistent (mm, isIndoor, isLowLoss);

      // set the height of the UE according to TR 36.873
      if (isIndoor)
      {
        // h_UT = 3(n_fl-1)+1.5
        n_fl->SetAttribute ("Max", DoubleValue (N_fl->GetValue ())); // TODO check if cast to int is needed
        Vector pos = mm->GetPosition ();
        NS_LOG_DEBUG ("Old position " << mm->GetPosition ());
        pos.z = 3 * (n_fl->GetValue () - 1) + 1.5; // TODO check if cast to int is needed
        mm->SetPosition (pos);
        NS_LOG_DEBUG ("New position (INDOOR)" << mm->GetPosition ());
      }
      else
      {
        // h_UT = 1.5
        Vector pos = mm->GetPosition ();
        NS_LOG_DEBUG ("Old position " << mm->GetPosition ());
        pos.z = 1.5;
        mm->SetPosition (pos);
        NS_LOG_DEBUG ("New position (OUTDOOR)" << mm->GetPosition ());
      }

      Ptr<MobilityBuildingInfo> bmm = mm->GetObject<MobilityBuildingInfo> ();
      NS_ABORT_MSG_UNLESS (0 != bmm, "node " << (*nit)->GetId () << " has a MobilityModel that does not have a MobilityBuildingInfo");
    }
  }

  for (NodeContainer::Iterator nit = enbAndIabNodes.Begin (); nit != enbAndIabNodes.End (); ++nit)
  {
    Ptr<MobilityModel> mm = (*nit)->GetObject<MobilityModel> ();
    if (mm != 0)
    {
      // BS are always outdoor
      MakeConsistent (mm, false, false);
    }

    Ptr<MobilityBuildingInfo> bmm = mm->GetObject<MobilityBuildingInfo> ();
    NS_ABORT_MSG_UNLESS (0 != bmm, "node " << (*nit)->GetId () << " has a MobilityModel that does not have a MobilityBuildingInfo");
  }
}


void
FakeBuildingsHelper::MakeConsistent (Ptr<MobilityModel> mm, bool isIndoor, bool isLowLoss)
{
  Ptr<MobilityBuildingInfo> bmm = mm->GetObject<MobilityBuildingInfo> ();
  NS_ABORT_MSG_UNLESS (0 != bmm, "This MobilityModel does not have a MobilityBuildingInfo, call Install () first");

  if (isIndoor)
  {
    Ptr < Building > building;
		building = CreateObject<Building> ();

    if (isLowLoss)
    {
      NS_LOG_LOGIC ("MobilityBuildingInfo " << bmm << " pos " << mm->GetPosition () << " is inside a residential building (low loss O2I)");
      building->SetBuildingType (Building::Residential);
    }
    else
    {
      NS_LOG_LOGIC ("MobilityBuildingInfo " << bmm << " pos " << mm->GetPosition () << " is inside a commercial building (high loss O2I)");
      building->SetBuildingType (Building::Commercial);
    }
    bmm->SetIndoor (building, 1, 1, 1);
  }
  else
  {
    NS_LOG_LOGIC ("MobilityBuildingInfo " << bmm << " pos " << mm->GetPosition ()  << " is outdoor");
    bmm->SetOutdoor ();
  }
}

} // namespace ns3
