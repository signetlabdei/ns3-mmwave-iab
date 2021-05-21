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

#ifndef MMWAVE_3GPP_FAKE_BUILDINGS_OBSTACLE_PROPAGATION_LOSS_MODEL_H_
#define MMWAVE_3GPP_FAKE_BUILDINGS_OBSTACLE_PROPAGATION_LOSS_MODEL_H_

#include "ns3/mmwave-3gpp-propagation-loss-model.h"
#include <ns3/buildings-propagation-loss-model.h>
#include <ns3/simulator.h>
#include "mmwave-phy-mac-common.h"
#include <fstream>

namespace ns3 {


class MmWave3gppFakeBuildingsPropagationLossModel : public BuildingsPropagationLossModel
{
public:
 static TypeId GetTypeId (void);
 MmWave3gppFakeBuildingsPropagationLossModel ();
 ~MmWave3gppFakeBuildingsPropagationLossModel ();
 virtual double GetLoss (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;
   // inherited from PropagationLossModel
 virtual double DoCalcRxPower (double txPowerDbm, Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

 void SetConfigurationParameters (Ptr<MmWavePhyMacCommon> ptrConfig);
 std::string GetScenario(Ptr<MobilityModel> a, Ptr<MobilityModel> b);
 char GetChannelCondition(Ptr<MobilityModel> a, Ptr<MobilityModel> b);

private:
 Ptr<MmWave3gppPropagationLossModel> m_3gppPropagationLoss;
};

}

#endif /* MMWAVE_3GPP_FAKE_BUILDINGS_OBSTACLE_PROPAGATION_LOSS_MODEL_H_ */
