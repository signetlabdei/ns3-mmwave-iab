/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
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
*
*   Author: Marco Mezzavilla < mezzavilla@nyu.edu>
*        	 Sourjya Dutta <sdutta@nyu.edu>
*        	 Russell Ford <russell.ford@nyu.edu>
*        	 Menglei Zhang <menglei@nyu.edu>
*/

#include "mmwave-3gpp-fake-buildings-propagation-loss-model.h"

#include "mmwave-3gpp-propagation-loss-model.h"
#include "ns3/log.h"
#include "ns3/mobility-model.h"
#include "ns3/double.h"
#include <ns3/mobility-building-info.h>
#include <ns3/building-list.h>
#include <ns3/angles.h>
#include "ns3/config-store.h"
#include "ns3/string.h"
#include <ns3/mmwave-ue-net-device.h>
#include <ns3/mc-ue-net-device.h>
#include <ns3/mmwave-enb-net-device.h>
#include <ns3/mmwave-iab-net-device.h>
#include <ns3/node.h>
#include "ns3/boolean.h"

NS_LOG_COMPONENT_DEFINE ("MmWave3gppFakeBuildingsPropagationLossModel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (MmWave3gppFakeBuildingsPropagationLossModel);

MmWave3gppFakeBuildingsPropagationLossModel::MmWave3gppFakeBuildingsPropagationLossModel ()
{
 m_3gppPropagationLoss = CreateObject<MmWave3gppPropagationLossModel> ();
 m_3gppPropagationLoss->SetAttribute("ChannelCondition", StringValue ("a"));
}

MmWave3gppFakeBuildingsPropagationLossModel::~MmWave3gppFakeBuildingsPropagationLossModel ()
{
}

TypeId
MmWave3gppFakeBuildingsPropagationLossModel::GetTypeId (void)
{
 static TypeId tid = TypeId ("ns3::MmWave3gppFakeBuildingsPropagationLossModel")
   .SetParent<BuildingsPropagationLossModel> ()
   .AddConstructor<MmWave3gppFakeBuildingsPropagationLossModel> ()
 ;
 return tid;
}


double
MmWave3gppFakeBuildingsPropagationLossModel::DoCalcRxPower (double txPowerDbm, Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
 return txPowerDbm - GetLoss (a, b);
}

double
MmWave3gppFakeBuildingsPropagationLossModel::GetLoss (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
 NS_ASSERT_MSG ((a->GetPosition ().z >= 0) && (b->GetPosition ().z >= 0),
     "MmWave3gppFakeBuildingsPropagationLossModel does not support underground nodes (placed at z < 0)");

 // get the MobilityBuildingInfo pointers
 Ptr<MobilityBuildingInfo> a1 = a->GetObject<MobilityBuildingInfo> ();
 Ptr<MobilityBuildingInfo> b1 = b->GetObject<MobilityBuildingInfo> ();
 NS_ASSERT_MSG ((a1 != 0) && (b1 != 0), "MmWave3gppFakeBuildingsPropagationLossModel only works with MobilityBuildingInfo");

 double loss = m_3gppPropagationLoss->GetLoss (a, b);
 std::string channelScenario;

 if (b1->IsOutdoor () && a1->IsIndoor ())
 {
   if (a1->GetBuilding()->GetBuildingType () == Building::Residential)
   {
     NS_LOG_INFO ("a inside a Residential building -> 02I low loss, scenario " << channelScenario);
     loss += m_3gppPropagationLoss->GetOutdoorToIndoorLoss (true, channelScenario);
   }
   else
   {
     NS_LOG_INFO ("a inside a NON-Residential building -> 02I high loss, scenario " << channelScenario);
     loss += m_3gppPropagationLoss->GetOutdoorToIndoorLoss (false, channelScenario);
   }
 }
 else if (a1->IsOutdoor () && b1->IsIndoor ())
 {
   if (b1->GetBuilding()->GetBuildingType () == Building::Residential)
   {
     NS_LOG_INFO ("b is inside a Residential building -> 02I low loss, scenario " << channelScenario);
     loss += m_3gppPropagationLoss->GetOutdoorToIndoorLoss (true, channelScenario);
   }
   else
   {
     NS_LOG_INFO ("b inside a NON-Residential building -> 02I high loss, scenario " << channelScenario);
     loss += m_3gppPropagationLoss->GetOutdoorToIndoorLoss (false, channelScenario);
   }
 }

 return loss;
}

std::string
MmWave3gppFakeBuildingsPropagationLossModel::GetScenario (Ptr<MobilityModel> a, Ptr<MobilityModel> b)
{
 return m_3gppPropagationLoss->GetScenario(a, b);
}

char
MmWave3gppFakeBuildingsPropagationLossModel::GetChannelCondition(Ptr<MobilityModel> a, Ptr<MobilityModel> b)
{
 return m_3gppPropagationLoss->GetChannelCondition (a,b);
}

void
MmWave3gppFakeBuildingsPropagationLossModel::SetConfigurationParameters (Ptr<MmWavePhyMacCommon> ptrConfig)
{
 m_3gppPropagationLoss->SetConfigurationParameters(ptrConfig);
}

} // namespace ns3
