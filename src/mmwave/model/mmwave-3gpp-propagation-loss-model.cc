 /* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
 /*
 *   Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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



#include "mmwave-3gpp-propagation-loss-model.h"
#include <ns3/log.h>
#include "ns3/mobility-model.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include <ns3/simulator.h>
#include <ns3/mmwave-ue-net-device.h>
#include <ns3/mmwave-iab-net-device.h>
#include <ns3/mc-ue-net-device.h>
#include <ns3/node.h>
using namespace ns3;


// ------------------------------------------------------------------------- //
NS_LOG_COMPONENT_DEFINE ("MmWave3gppPropagationLossModel");

NS_OBJECT_ENSURE_REGISTERED (MmWave3gppPropagationLossModel);

TypeId
MmWave3gppPropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmWave3gppPropagationLossModel")
    .SetParent<PropagationLossModel> ()
    .AddConstructor<MmWave3gppPropagationLossModel> ()
    .AddAttribute ("MinLoss",
          "The minimum value (dB) of the total loss, used at short ranges.",
          DoubleValue (0.0),
          MakeDoubleAccessor (&MmWave3gppPropagationLossModel::SetMinLoss,
                              &MmWave3gppPropagationLossModel::GetMinLoss),
          MakeDoubleChecker<double> ())
    .AddAttribute ("ChannelCondition",
  				"'l' for LOS, 'n' for NLOS, 'a' for all",
  				StringValue ("a"),
  				MakeStringAccessor (&MmWave3gppPropagationLossModel::m_channelConditions),
  				MakeStringChecker ())
  	.AddAttribute ("ScenarioEnbEnb",
  				"The available channel scenarios are 'RMa', 'UMa', 'UMi-StreetCanyon', 'InH-OfficeMixed', 'InH-OfficeOpen', 'InH-ShoppingMall'",
  				StringValue ("RMa"),
  				MakeStringAccessor (&MmWave3gppPropagationLossModel::m_scenarioEnbEnb),
  				MakeStringChecker ())
    .AddAttribute ("ScenarioEnbUe",
  				"The available channel scenarios are 'RMa', 'UMa', 'UMi-StreetCanyon', 'InH-OfficeMixed', 'InH-OfficeOpen', 'InH-ShoppingMall'",
  				StringValue ("RMa"),
  				MakeStringAccessor (&MmWave3gppPropagationLossModel::m_scenarioEnbUe),
  				MakeStringChecker ())
    .AddAttribute ("ScenarioEnbIab",
  				"The available channel scenarios are 'RMa', 'UMa', 'UMi-StreetCanyon', 'InH-OfficeMixed', 'InH-OfficeOpen', 'InH-ShoppingMall'",
  				StringValue ("RMa"),
  				MakeStringAccessor (&MmWave3gppPropagationLossModel::m_scenarioEnbIab),
  				MakeStringChecker ())
    .AddAttribute ("ScenarioIabIab",
  				"The available channel scenarios are 'RMa', 'UMa', 'UMi-StreetCanyon', 'InH-OfficeMixed', 'InH-OfficeOpen', 'InH-ShoppingMall'",
  				StringValue ("RMa"),
  				MakeStringAccessor (&MmWave3gppPropagationLossModel::m_scenarioIabIab),
  				MakeStringChecker ())
    .AddAttribute ("ScenarioIabUe",
          "The available channel scenarios are 'RMa', 'UMa', 'UMi-StreetCanyon', 'InH-OfficeMixed', 'InH-OfficeOpen', 'InH-ShoppingMall'",
          StringValue ("RMa"),
          MakeStringAccessor (&MmWave3gppPropagationLossModel::m_scenarioIabUe),
          MakeStringChecker ())
    .AddAttribute ("ScenarioUeUe",
          "The available channel scenarios are 'RMa', 'UMa', 'UMi-StreetCanyon', 'InH-OfficeMixed', 'InH-OfficeOpen', 'InH-ShoppingMall'",
          StringValue ("RMa"),
          MakeStringAccessor (&MmWave3gppPropagationLossModel::m_scenarioUeUe),
          MakeStringChecker ())
  	.AddAttribute ("OptionalNlos",
  				"Use the optional NLoS propagation loss model",
  				BooleanValue (false),
  				MakeBooleanAccessor (&MmWave3gppPropagationLossModel::m_optionNlosEnabled),
  				MakeBooleanChecker ())
  	.AddAttribute ("Shadowing",
  				"Enable shadowing effect",
  				BooleanValue (true),
  				MakeBooleanAccessor (&MmWave3gppPropagationLossModel::m_shadowingEnabled),
  				MakeBooleanChecker ())
  	.AddAttribute ("InCar",
  				"If inside a vehicle, car penetration loss should be added to propagation loss",
  				BooleanValue (false),
  				MakeBooleanAccessor (&MmWave3gppPropagationLossModel::m_inCar),
  				MakeBooleanChecker ())
  	// for testing
  	.AddAttribute ("LossFixedDb",
  				"Fixed value for the propagation loss [dB]",
  				DoubleValue (0.0),
  				MakeDoubleAccessor (&MmWave3gppPropagationLossModel::m_lossFixedDb),
  				MakeDoubleChecker<double> ())
  	.AddAttribute ("FixedLossTst",
  				"Boolean flag to use a fixed loss value",
  				BooleanValue (false),
  				MakeBooleanAccessor (&MmWave3gppPropagationLossModel::m_fixedLossTst),
  				MakeBooleanChecker ())
  ;
  return tid;
}

MmWave3gppPropagationLossModel::MmWave3gppPropagationLossModel ()
{
  m_channelConditionMap.clear ();
  m_channelScenarioMap.clear ();
  m_norVar = CreateObject<NormalRandomVariable> ();
  m_norVar->SetAttribute ("Mean", DoubleValue (0));
  m_norVar->SetAttribute ("Variance", DoubleValue (1));

  m_uniformVar = CreateObject<UniformRandomVariable> ();
  m_uniformVar->SetAttribute ("Min", DoubleValue (0));
  m_uniformVar->SetAttribute ("Max", DoubleValue (1));

}

void
MmWave3gppPropagationLossModel::SetMinLoss (double minLoss)
{
  m_minLoss = minLoss;
}
double
MmWave3gppPropagationLossModel::GetMinLoss (void) const
{
  return m_minLoss;
}

double
MmWave3gppPropagationLossModel::GetFrequency (void) const
{
  return m_frequency;
}

void
MmWave3gppPropagationLossModel::SetLossFixedDb(double loss)
{
	m_lossFixedDb = loss;
}

double
MmWave3gppPropagationLossModel::DoCalcRxPower (double txPowerDbm,
                                          Ptr<MobilityModel> a,
                                          Ptr<MobilityModel> b) const
{
	if(!m_fixedLossTst)
	{
	  	return (txPowerDbm - GetLoss (a, b));
	}
	else
	{
		return txPowerDbm - m_lossFixedDb;
	}
}

std::tuple<Ptr<MobilityModel>, Ptr<MobilityModel>, bool, bool >
MmWave3gppPropagationLossModel::GetEnbUePair(Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
	bool skip = false;
	bool backhaulLink = false;

	Ptr<MobilityModel> ueMob=0, enbMob=0;
	if(DynamicCast<MmWaveEnbNetDevice> (a->GetObject<Node> ()->GetDevice(0)) != 0)
	{
		// for sure it is downlink
		enbMob = a;
		ueMob = b;
		if(DynamicCast<MmWaveEnbNetDevice> (b->GetObject<Node> ()->GetDevice(0)) != 0)
		{
			skip = true;
		}
		else if(DynamicCast<MmWaveIabNetDevice> (b->GetObject<Node> ()->GetDevice(0)) != 0)
		{
			backhaulLink = true;
		}
	}
	else if(DynamicCast<MmWaveEnbNetDevice> (b->GetObject<Node> ()->GetDevice(0)) != 0)
	{
		// for sure it is uplink
		ueMob = a;
		enbMob = b;
		if(DynamicCast<MmWaveIabNetDevice> (a->GetObject<Node> ()->GetDevice(0)) != 0)
		{
			backhaulLink = true;
		}
	}
	else
	{
		// uplink or IAB downlink?
		if(DynamicCast<MmWaveIabNetDevice> (a->GetObject<Node> ()->GetDevice(0)) != 0)
		{
			if(DynamicCast<MmWaveIabNetDevice> (b->GetObject<Node> ()->GetDevice(0)) == 0)
			{
				// IAB to MC UE or UE (access)
				ueMob = b;
				enbMob = a;
			}
			else
			{
				// IAB to IAB
				Ptr<MmWaveIabNetDevice> txDev = DynamicCast<MmWaveIabNetDevice> (a->GetObject<Node> ()->GetDevice(0));
				Ptr<MmWaveIabNetDevice> rxDev = DynamicCast<MmWaveIabNetDevice> (b->GetObject<Node> ()->GetDevice(0));
				if(rxDev->GetBackhaulTargetEnb()) // downlink BH
				{
					enbMob = a;
					ueMob = b;
				}
				else // uplink BH or interference
				{
					ueMob = a;
					enbMob = b;
				}
				backhaulLink = true;
			}
		}
		else if(DynamicCast<MmWaveIabNetDevice> (b->GetObject<Node> ()->GetDevice(0)) != 0)
		{
			// UE and MC UE to IAB
			ueMob = a;
			enbMob = b;
		}
		else
		{
			// UE to UE or MC UE to MC UE
			skip = true;
		}
	}

	return std::make_tuple(enbMob, ueMob, skip, backhaulLink);
}

double
MmWave3gppPropagationLossModel::GetLoss (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{

	double distance3D = a->GetDistanceFrom (b);

	if (distance3D <= 0)
	{
		return  m_minLoss;
	}
	if (distance3D < 3*m_lambda)
	{
		NS_LOG_UNCOND ("distance not within the far field region => inaccurate propagation loss value");
	}

	// TODO check cases
	Ptr<MobilityModel> ueMob, enbMob;
	// if(
	// 	   (DynamicCast<MmWaveUeNetDevice> (a->GetObject<Node> ()->GetDevice (0)) !=0)
	// 	|| (DynamicCast<McUeNetDevice> (a->GetObject<Node> ()->GetDevice (0)) !=0)
	// 	|| (DynamicCast<MmWaveIabNetDevice> (a->GetObject<Node> ()->GetDevice (0)) !=0)
	// 	)
	// {
	// 	if(DynamicCast<MmWaveEnbNetDevice> (b->GetObject<Node> ()->GetDevice (0)) !=0)
	// 	{
	// 		ueMob = a;
	// 		enbMob = b;
	// 	}
	// 	else if(DynamicCast<MmWaveIabNetDevice> (b->GetObject<Node> ()->GetDevice (0)) !=0) // TODO distinguish IAB BH DL and UL
	// 	{
	// 		ueMob = a;
	// 		enbMob = a;
	// 	}
	// 	else
	// 	{
	// 		NS_LOG_INFO("UE->UE Link, skip Pathloss computation");
	// 		return 0;
	// 	}
	// }
	// else
	// {
	// 	if(DynamicCast<MmWaveEnbNetDevice> (b->GetObject<Node> ()->GetDevice (0)) !=0)
	// 	{
	// 		NS_LOG_INFO("ENB->ENB Link, skip Pathloss computation");
	// 		return 0;
	// 	}
	// 	else
	// 	{
	// 		ueMob = b;
	// 		enbMob = a;
	// 	}
	// }

	auto returnParams = GetEnbUePair(a, b);

	if(std::get<2>(returnParams))
	{
		NS_LOG_INFO("Skip pathloss for device to device propagation");
		return 0; // skip the computation for UE to UE channel
	}

	enbMob = std::get<0>(returnParams);
	ueMob =  std::get<1>(returnParams);
	bool backhaulLink = std::get<3>(returnParams);

	Vector uePos = ueMob->GetPosition();
	Vector enbPos = enbMob->GetPosition();
	double x = uePos.x-enbPos.x;
	double y = uePos.y-enbPos.y;
	double distance2D = sqrt (x*x +y*y);
	double hBs = enbPos.z;
	double hUt = uePos.z;

	channelConditionMap_t::const_iterator it;
	it = m_channelConditionMap.find(std::make_pair(a,b));
	if (it == m_channelConditionMap.end ())
	{
		channelCondition condition;

		if (m_channelConditions.compare("l")==0 )
		{
			condition.m_channelCondition = 'l';
			NS_LOG_UNCOND (GetScenario (a, b) << " scenario, channel condition is fixed to be " << condition.m_channelCondition<<", h_BS="<<hBs<<",h_UT="<<hUt);
		}
		else if (m_channelConditions.compare("n")==0)
		{
			condition.m_channelCondition = 'n';
			NS_LOG_UNCOND (GetScenario (a, b) << " scenario, channel condition is fixed to be " << condition.m_channelCondition<<", h_BS="<<hBs<<",h_UT="<<hUt);
		}
		else if (m_channelConditions.compare("a")==0)
		{
			double PRef = m_uniformVar->GetValue();
			double probLos;
			//Note: The LOS probability is derived with assuming antenna heights of 3m for indoor, 10m for UMi, and 25m for UMa.
			if (GetScenario (a,b) == "RMa")
			{
				if(distance2D <= 10)
				{
					probLos = 1;
				}
				else
				{
					probLos = exp(-(distance2D-10)/1000);
				}
			}
			else if (GetScenario (a,b) == "UMa")
			{
				if(distance2D <= 18)
				{
					probLos = 1;
				}
				else
				{
					double C_hUt;
					if (hUt <= 13)
					{
						C_hUt = 0;
					}
					else if(hUt <=23)
					{
						C_hUt = pow((hUt-13)/10,1.5);
					}
					else
					{
						NS_FATAL_ERROR ("From Table 7.4.2-1, UMa scenario hUT cannot be larger than 23 m");
					}
					probLos = (18/distance2D+exp(-distance2D/63)*(1-18/distance2D))*(1+C_hUt*5/4*pow(distance2D/100,3)*exp(-distance2D/150));
				}
			}
			else if (GetScenario (a,b) == "UMi-StreetCanyon")
			{
				if(distance2D <= 18)
				{
					probLos = 1;
				}
				else
				{
					probLos = 18/distance2D+exp(-distance2D/36)*(1-18/distance2D);
				}
			}
			else if (GetScenario (a,b) == "InH-OfficeMixed")
			{
				if(distance2D <= 1.2)
				{
					probLos = 1;
				}
				else if (distance2D <= 6.5)
				{
					probLos = exp(-(distance2D-1.2)/4.7);
				}
				else
				{
					probLos = exp(-(distance2D-6.5)/32.6)*0.32;
				}
			}
			else if (GetScenario (a,b) == "InH-OfficeOpen")
			{
				if(distance2D <= 5)
				{
					probLos = 1;
				}
				else if (distance2D <= 49)
				{
					probLos = exp(-(distance2D-5)/70.8);
				}
				else
				{
					probLos = exp(-(distance2D-49)/211.7)*0.54;
				}
			}
			else if (GetScenario (a,b) == "InH-ShoppingMall")
			{
				probLos = 1;

			}
			else
			{
				NS_FATAL_ERROR ("Unknown scenario");
			}

			// according to TR 38.874, the parameters for the backhaul channels
			// should be chosen as those who give the best realization among three 
			// random ones
			if (backhaulLink) 
			{
				// < 2 given that the first realization is PRef
				for(uint8_t attempt = 0; attempt < 2; attempt++)
				{
					double newProb = m_uniformVar->GetValue();
					if (newProb < PRef)
					{
						// the smallest, the higher the probability of LOS
						PRef = newProb;
					}
				}
			}

			if (PRef <= probLos)
			{
				condition.m_channelCondition = 'l';
			}
			else
			{
				condition.m_channelCondition = 'n';
			}
			NS_LOG_UNCOND (GetScenario (a,b) << " scenario, 2D distance = " << distance2D <<"m, Prob_LOS = " << probLos
					<< ", Prob_REF = " << PRef << ", the channel condition is " << condition.m_channelCondition<<", h_BS="<<hBs<<",h_UT="<<hUt);

		}
		else
		{
			NS_FATAL_ERROR ("Wrong channel condition configuration");
		}
		// assign a large negative value to identify initial transmission.
		condition.m_shadowing = -1e6;
		condition.m_hE = 0;
		//condition.m_carPenetrationLoss = 9+m_norVar->GetValue()*5;
		condition.m_carPenetrationLoss = 10;
		std::pair<channelConditionMap_t::const_iterator, bool> ret;
		ret = m_channelConditionMap.insert (std::make_pair(std::make_pair (a,b), condition));
		m_channelConditionMap.insert (std::make_pair(std::make_pair (b,a), condition));
		it = ret.first;
	}

	/* Reminder.
	 * The The LOS NLOS state transition will be implemented in the future as mentioned in secction 7.6.3.3
	 * */

	double lossDb = 0;
	double freqGHz = m_frequency/1e9;

	double shadowingStd = 0;
	double shadowingCorDistance = 0;
	if (GetScenario (a,b) == "RMa")
	{
		if(distance2D < 10)
		{
			NS_LOG_WARN ("The 2D distance is smaller than 10 meters, the 3GPP RMa model may not be accurate");
		}

		if (hBs < 10 || hBs > 150 )
		{
			NS_FATAL_ERROR ("According to table 7.4.1-1, the RMa scenario need to satisfy the following condition, 10 m <= hBS <= 150 m");
		}

		if (hUt < 1 || hUt > 10 )
		{
			NS_FATAL_ERROR ("According to table 7.4.1-1, the RMa scenario need to satisfy the following condition, 1 m <= hUT <= 10 m");
		}
		//default base station antenna height is 35 m
		//hBs = 35;
		//default user antenna height is 1.5 m
		//hUt = 1.5;
		double W = 20; //average street height
		double h = 5; //average building height

		double dBP = 2*M_PI*hBs*hUt*m_frequency/3e8; //break point distance
		double PL1 = 20*log10(40*M_PI*distance3D*freqGHz/3) + std::min(0.03*pow(h,1.72),10.0)*log10(distance3D) - std::min(0.044*pow(h,1.72),14.77) + 0.002*log10(h)*distance3D;

		if(distance2D <= dBP)
		{
			lossDb = PL1;
			shadowingStd = 4;

		}
		else
		{
			//PL2
			lossDb = PL1 + 40*log10(distance3D/dBP);
			shadowingStd= 6;
		}

		switch ((*it).second.m_channelCondition)
		{
			case 'l':
			{
				shadowingCorDistance = 37;
				break;
			}
			case 'n':
			{
				shadowingCorDistance = 120;
				double PLNlos = 161.04-7.1*log10(W)+7.5*log10(h)-(24.37-3.7*pow((h/hBs),2))*log10(hBs)+(43.42-3.1*log10(hBs))*(log10(distance3D)-3)+20*log10(freqGHz)-(3.2*pow(log10(11.75*hUt),2)-4.97);
				lossDb = std::max(PLNlos, lossDb);
				shadowingStd = 8;
				break;
			}
			default:
				NS_FATAL_ERROR ("Programming Error.");
		}

	}
	else if (GetScenario (a,b) == "UMa")
	{
		if(distance2D < 10)
		{
			NS_LOG_UNCOND ("The 2D distance is smaller than 10 meters, the 3GPP UMa model may not be accurate");
		}

		//default base station value is 25 m
		//hBs = 25;

		if (hUt < 1.5 || hUt > 22.5 )
		{
			NS_FATAL_ERROR ("According to table 7.4.1-1, the UMa scenario need to satisfy the following condition, 1.5 m <= hUT <= 22.5 m");
		}
		//For UMa, the effective environment height should be computed follow Table7.4.1-1.
		if((*it).second.m_hE == 0)
		{
			channelCondition condition;
			condition = (*it).second;
			if (hUt <= 18)
			{
				condition.m_hE = 1;
			}
			else
			{
				double g_d2D = 1.25*pow(distance2D/100,3)*exp(-1*distance2D/150);
				double C_d2D_hUT = pow((hUt-13)/10,1.5)*g_d2D;
				double prob = 1/(1+C_d2D_hUT);

				if(m_uniformVar->GetValue() < prob)
				{
					condition.m_hE = 1;
				}
				else
				{
					int random = m_uniformVar->GetInteger(12, (int)(hUt-1.5));
					condition.m_hE = (double)floor(random/3)*3;
				}
			}
			UpdateConditionMap(a,b,condition);
		}
		double dBP = 4*(hBs-(*it).second.m_hE)*(hUt-(*it).second.m_hE)*m_frequency/3e8;
		if(distance2D <= dBP)
		{
			//PL1
			lossDb = 32.4+20*log10(distance3D)+20*log10(freqGHz);
		}
		else
		{
			//PL2
			lossDb = 32.4+40*log10(distance3D)+20*log10(freqGHz)-10*log10(pow(dBP,2)+pow(hBs-hUt,2));
		}


		switch ((*it).second.m_channelCondition)
		{
			case 'l':
			{
				shadowingStd = 4;
				shadowingCorDistance = 37;
				break;
			}
			case 'n':
			{
				shadowingCorDistance = 50;
				if(m_optionNlosEnabled)
				{
					//optional propagation loss
					lossDb = 32.4+20*log10(freqGHz)+30*log10(distance3D);
					shadowingStd = 7.8;
				}
				else
				{
					double PLNlos = 13.54+39.08*log10(distance3D)+20*log10(freqGHz)-0.6*(hUt-1.5);
					shadowingStd = 6;
					lossDb = std::max(PLNlos, lossDb);
				}


				break;
			}
			default:
				NS_FATAL_ERROR ("Programming Error.");
		}
	}
	else if (GetScenario (a,b) == "5GCM-UMa")
	{
		if(distance2D < 10)
		{
			NS_LOG_UNCOND ("The 2D distance is smaller than 10 meters, the 3GPP UMa model may not be accurate");
		}

		//default base station value is 25 m
		//hBs = 25;

		if (hUt < 1.5 || hUt > 22.5 )
		{
			NS_FATAL_ERROR ("According to table 7.4.1-1, the UMa scenario need to satisfy the following condition, 1.5 m <= hUT <= 22.5 m");
		}
		//For UMa, the effective environment height should be computed follow Table7.4.1-1.
		if((*it).second.m_hE == 0)
		{
			channelCondition condition;
			condition = (*it).second;
			if (hUt <= 18)
			{
				condition.m_hE = 1;
			}
			else
			{
				double g_d2D = 1.25*pow(distance2D/100,3)*exp(-1*distance2D/150);
				double C_d2D_hUT = pow((hUt-13)/10,1.5)*g_d2D;
				double prob = 1/(1+C_d2D_hUT);

				if(m_uniformVar->GetValue() < prob)
				{
					condition.m_hE = 1;
				}
				else
				{
					int random = m_uniformVar->GetInteger(12, (int)(hUt-1.5));
					condition.m_hE = (double)floor(random/3)*3;
				}
			}
			UpdateConditionMap(a,b,condition);
		}
		double dBP = 4*(hBs-(*it).second.m_hE)*(hUt-(*it).second.m_hE)*m_frequency/3e8;
		if(distance2D <= dBP)
		{
			//PL1
			lossDb = 32.4+20*log10(distance3D)+20*log10(freqGHz);
		}
		else
		{
			//PL2
			lossDb = 32.4+40*log10(distance3D)+20*log10(freqGHz)-10*log10(pow(dBP,2)+pow(hBs-hUt,2));
		}


		switch ((*it).second.m_channelCondition)
		{
			case 'l':
			{
				shadowingStd = 4.1;
				shadowingCorDistance = 37;
				break;
			}
			case 'n':
			{
				shadowingCorDistance = 50;
				if(true) // always use option NLOS pathloss
				{
					//optional propagation loss
					lossDb = 32.4+20*log10(freqGHz)+30*log10(distance3D);
					shadowingStd = 6.8;
				}
				else
				{
					double PLNlos = 13.54+39.08*log10(distance3D)+20*log10(freqGHz)-0.6*(hUt-1.5);
					shadowingStd = 6;
					lossDb = std::max(PLNlos, lossDb);
				}


				break;
			}
			default:
				NS_FATAL_ERROR ("Programming Error.");
		}
	}
	else if (GetScenario (a,b) == "UMi-StreetCanyon")
	{

		if(distance2D < 10)
		{
			NS_LOG_UNCOND ("The 2D distance is smaller than 10 meters, the 3GPP UMi-StreetCanyon model may not be accurate");
		}

		//default base station value is 10 m
		//hBs = 10;

		if (hUt < 1.5 || hUt > 22.5 )
		{
			NS_FATAL_ERROR ("According to table 7.4.1-1, the UMi-StreetCanyon scenario need to satisfy the following condition, 1.5 m <= hUT <= 22.5 m");
		}
		double dBP = 4*(hBs-1)*(hUt-1)*m_frequency/3e8;
		if(distance2D <= dBP)
		{
			//PL1
			lossDb = 32.4+21*log10(distance3D)+20*log10(freqGHz);
		}
		else
		{
			//PL2
			lossDb = 32.4+40*log10(distance3D)+20*log10(freqGHz)-9.5*log10(pow(dBP,2)+pow(hBs-hUt,2));
		}


		switch ((*it).second.m_channelCondition)
		{
			case 'l':
			{
				shadowingStd = 4;
				shadowingCorDistance = 10;
				break;
			}
			case 'n':
			{
				shadowingCorDistance = 13;
				if(m_optionNlosEnabled)
				{
					//optional propagation loss
					lossDb = 32.4+20*log10(freqGHz)+31.9*log10(distance3D);
					shadowingStd = 8.2;
				}
				else
				{
					double PLNlos = 35.3*log10(distance3D)+22.4+21.3*log10(freqGHz)-0.3*(hUt-1.5);
					shadowingStd = 7.82;
					lossDb = std::max(PLNlos, lossDb);
				}

				break;
			}
			default:
				NS_FATAL_ERROR ("Programming Error.");
		}
	}
	else if (GetScenario (a,b) == "InH-OfficeMixed" || GetScenario (a,b) == "InH-OfficeOpen")
	{
		if(distance3D < 1 || distance3D > 100)
		{
			NS_LOG_UNCOND ("The pathloss might not be accurate since 3GPP InH-Office model LoS condition is accurate only within 3D distance between 1 m and 100 m");
		}

		lossDb = 32.4+17.3*log10(distance3D)+20*log10(freqGHz);


		switch ((*it).second.m_channelCondition)
		{
			case 'l':
			{
				shadowingStd = 3;
				shadowingCorDistance = 10;
				break;
			}
			case 'n':
			{
				shadowingCorDistance = 6;
				if(distance3D > 86)
				{
					NS_LOG_UNCOND ("The pathloss might not be accurate since 3GPP InH-Office model NLoS condition only supports 3D distance between 1 m and 86 m");
				}

				if(m_optionNlosEnabled)
				{
					//optional propagation loss
					double PLNlos = 32.4+20*log10(freqGHz)+31.9*log10(distance3D);
					shadowingStd = 8.29;
					lossDb = std::max(PLNlos, lossDb);

				}
				else
				{
					double PLNlos = 38.3*log10(distance3D)+17.3+24.9*log10(freqGHz);
					shadowingStd = 8.03;
					lossDb = std::max(PLNlos, lossDb);
				}
				break;
			}
			default:
				NS_FATAL_ERROR ("Programming Error.");
		}
	}

	else if (GetScenario (a,b) == "InH-ShoppingMall")
	{
		shadowingCorDistance = 10; //I use the office correlation distance since shopping mall is not in the table.

		if(distance3D < 1 || distance3D > 150)
		{
			NS_LOG_UNCOND ("The pathloss might not be accurate since 3GPP InH-Shopping mall model only supports 3D distance between 1 m and 150 m");\
		}
		lossDb = 32.4+17.3*log10(distance3D)+20*log10(freqGHz);
		shadowingStd = 2;
	}
	else
	{
		NS_FATAL_ERROR ("Unknown channel condition");
	}

	if(m_shadowingEnabled)
	{
		channelCondition cond;
		cond = (*it).second;

		double shadowingNormal = m_norVar->GetValue();
		// according to TR 38.874, the parameters for the backhaul channels
		// should be chosen as those who give the best realization among three 
		// random ones
		if (backhaulLink) 
		{
			// < 2 given that the first realization is shadowingNormal
			for(uint8_t attempt = 0; attempt < 2; attempt++) 
			{
				double newProb = m_norVar->GetValue();
				if (newProb < shadowingNormal)
				{
					// the smallest, the smaller the pathloss
					shadowingNormal = newProb;
				}
			}
		}

		shadowingNormal *= shadowingStd;

		//The first transmission the shadowing is initialed as -1e6,
		//we perform this if check the identify first  transmission.
		if((*it).second.m_shadowing < -1e5)
		{
			cond.m_shadowing = shadowingNormal;
		}
		else
		{
			double deltaX = uePos.x-(*it).second.m_position.x;
			double deltaY = uePos.y-(*it).second.m_position.y;
			double disDiff = sqrt (deltaX*deltaX +deltaY*deltaY);
			//NS_LOG_UNCOND (shadowingStd <<"  "<<disDiff <<"  "<<shadowingCorDistance);
			double R = exp(-1*disDiff/shadowingCorDistance); // from equation 7.4-5.
			cond.m_shadowing = R*(*it).second.m_shadowing + sqrt(1-R*R)*shadowingNormal;
		}

		lossDb += cond.m_shadowing;
		cond.m_position = ueMob->GetPosition();
		UpdateConditionMap(a, b, cond);
	}


	 /*FILE* log_file;

	  char* fname = (char*)malloc(sizeof(char) * 255);

	  memset(fname, 0, sizeof(char) * 255);
	  std::string temp;
	  if(m_optionNlosEnabled)
	  {
		  temp = m_scenario+"-"+(*it).second.m_channelCondition+"-opt.txt";
	  }
	  else
	  {
		  temp = m_scenario+"-"+(*it).second.m_channelCondition+".txt";
	  }

	  log_file = fopen(temp.c_str(), "a");

	  fprintf(log_file, "%f \t  %f\n", distance3D, lossDb);

	  fflush(log_file);

	  fclose(log_file);

	  if(fname)

	  free(fname);

	  fname = 0;*/

	if(m_inCar)
	{
		lossDb += (*it).second.m_carPenetrationLoss;
	}

	return std::max (lossDb, m_minLoss);
}

double
MmWave3gppPropagationLossModel::GetOutdoorToIndoorLoss(bool lowLossModel, std::string scenario)
{
	NS_LOG_FUNCTION(this);
	// See  equation 7.4-2 and table 7.4.3-2 in 3GPP TR 38.900 V15.0.0 and the equivalent
	// equations in Sec. 6.3 of the 5GCM white paper http://www.5gworkshops.com/5GCMSIG_White%20Paper_r2dot3.pdf

	double stdP = 0; // standard deviation of the noise term in the O2I equation, in dB
	double PL_tw = 0; // building penetration loss in 7.4-2 from 3GPP TR 38.900, in dB
	double lossIndoor = 0; // total indoor loss, in dB

	if(lowLossModel)
	{
		PL_tw = 5-10*log10(0.3*pow(10,-1*(2+0.2*m_frequency*1e-9)/10)+0.7*pow(10,-1*(5+4*m_frequency*1e-9)/10));
		if(scenario != "5GCM-UMa")
		{
			stdP = 4.4;
		}
		else
		{
			stdP = 3;
		}
	}
	else
	{
		PL_tw = 5-10*log10(0.7*pow(10,-1*(23+0.3*m_frequency*1e-9)/10)+0.3*pow(10,-1*(5+4*m_frequency*1e-9)/10));
		if(scenario != "5GCM-UMa")
		{
			stdP = 6.5;
		}
		else
		{
			stdP = 5;
		}
	}

	lossIndoor += PL_tw;

	//compute PL_in

	Ptr<UniformRandomVariable> uniRv1 = CreateObject<UniformRandomVariable> ();
	Ptr<UniformRandomVariable> uniRv2 = CreateObject<UniformRandomVariable> ();
	double dis_2D_in;
	if(scenario == "RMa")
	{
		dis_2D_in = std::min(uniRv1->GetValue(0,10), uniRv2->GetValue(0,10));
	}
	else
	{
		dis_2D_in = std::min(uniRv1->GetValue(0,25), uniRv2->GetValue(0,25));
	}
	lossIndoor += 0.5*dis_2D_in;
	//compute indoor shadowing
	Ptr<NormalRandomVariable> norRv = CreateObject<NormalRandomVariable> ();
	lossIndoor += stdP*norRv->GetValue();

	return lossIndoor;
}

int64_t
MmWave3gppPropagationLossModel::DoAssignStreams (int64_t stream)
{
  return 0;
}

void
MmWave3gppPropagationLossModel::UpdateConditionMap (Ptr<MobilityModel> a, Ptr<MobilityModel> b, channelCondition cond) const
{
	m_channelConditionMap[std::make_pair (a,b)] = cond;
	m_channelConditionMap[std::make_pair (b,a)] = cond;

}

char
MmWave3gppPropagationLossModel::GetChannelCondition(Ptr<MobilityModel> a, Ptr<MobilityModel> b)
{
	channelConditionMap_t::const_iterator it;
	it = m_channelConditionMap.find(std::make_pair(a,b));
	if (it == m_channelConditionMap.end ())
	{
		NS_FATAL_ERROR ("Cannot find the link in the map");
	}
	return (*it).second.m_channelCondition;

}

std::string
MmWave3gppPropagationLossModel::GetScenario (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  NS_LOG_FUNCTION (this);

  std::string scenario;
  std::pair <Ptr<MobilityModel>, Ptr<MobilityModel>> key = std::make_pair (a, b);
  std::pair <Ptr<MobilityModel>, Ptr<MobilityModel>> key_reverse = std::make_pair (b, a);

  // Look for the channel scenario associated to this link in the m_channelScenarioMap
  // If not found, retrieve the scenario store it the the map
  if (m_channelScenarioMap.find (key) != m_channelScenarioMap.end ())
  {
    scenario = m_channelScenarioMap.at (key);
  }
  else if (m_channelScenarioMap.find (key_reverse) != m_channelScenarioMap.end ())
  {
    scenario = m_channelScenarioMap.at (key_reverse);
  }
  else
  {
    if ((DynamicCast<MmWaveEnbNetDevice> (a->GetObject<Node> ()->GetDevice(0))) != 0)
    {
      if ((DynamicCast<MmWaveEnbNetDevice> (b->GetObject<Node> ()->GetDevice(0))) != 0)
      {
        // eNB to eNB
        scenario = m_scenarioEnbEnb;
      }
      else if ((DynamicCast<MmWaveUeNetDevice> (b->GetObject<Node> ()->GetDevice(0))) != 0)
      {
        // eNB to UE
        scenario = m_scenarioEnbUe;
      }
      else if ((DynamicCast<MmWaveIabNetDevice> (b->GetObject<Node> ()->GetDevice(0))) != 0)
      {
        // eNB to IAB
        scenario = m_scenarioEnbIab;
      }
      else
      {
        NS_FATAL_ERROR ("Unknown case");
      }
    }
    else if ((DynamicCast<MmWaveUeNetDevice> (a->GetObject<Node> ()->GetDevice(0))) != 0)
    {
      if ((DynamicCast<MmWaveEnbNetDevice> (b->GetObject<Node> ()->GetDevice(0))) != 0)
      {
        // UE to eNB
        scenario = m_scenarioEnbUe;
      }
      else if ((DynamicCast<MmWaveUeNetDevice> (b->GetObject<Node> ()->GetDevice(0))) != 0)
      {
        // UE to UE
        scenario = m_scenarioUeUe;
      }
      else if ((DynamicCast<MmWaveIabNetDevice> (b->GetObject<Node> ()->GetDevice(0))) != 0)
      {
        // UE to IAB
        scenario = m_scenarioIabUe;
      }
      else
      {
        NS_FATAL_ERROR ("Unknown case");
      }
    }
    else if ((DynamicCast<MmWaveIabNetDevice> (a->GetObject<Node> ()->GetDevice(0))) != 0)
    {
      if ((DynamicCast<MmWaveEnbNetDevice> (b->GetObject<Node> ()->GetDevice(0))) != 0)
      {
        // IAB to eNB
        scenario = m_scenarioEnbIab;
      }
      else if ((DynamicCast<MmWaveUeNetDevice> (b->GetObject<Node> ()->GetDevice(0))) != 0)
      {
        // IAB to UE
        scenario = m_scenarioIabUe;
      }
      else if ((DynamicCast<MmWaveIabNetDevice> (b->GetObject<Node> ()->GetDevice(0))) != 0)
      {
        // IAB to IAB
        scenario = m_scenarioIabIab;
      }
      else
      {
        NS_FATAL_ERROR ("Unknown case");
      }
    }
    else
    {
      NS_FATAL_ERROR ("Unknown case");
    }
    m_channelScenarioMap.insert (std::make_pair (key, scenario));
  }

	return scenario;
}

void
MmWave3gppPropagationLossModel::SetConfigurationParameters (Ptr<MmWavePhyMacCommon> ptrConfig)
{
	m_phyMacConfig = ptrConfig;
	m_frequency = m_phyMacConfig->GetCenterFrequency();
    static const double C = 299792458.0; // speed of light in vacuum
    m_lambda = C / m_frequency;

    NS_LOG_INFO("Frequency " << m_frequency);
}
