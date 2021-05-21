 /* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
 /*
 *   Copyright (c) 2020, University of Padova, Dep. of Information Engineering, SIGNET lab
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
 *   Author: Matteo Pagin <mattpagg@gmail.com>
 */

#include "mmwave-iab-controller.h"
#include "ns3/trace-helper.h"
#include <ns3/string.h>
#include <ns3/uinteger.h>
#include <ns3/double.h>
#include <ns3/boolean.h>
#include <ns3/log.h>
#include <map>
#define EPSILON 1.0e-4

namespace ns3{

NS_LOG_COMPONENT_DEFINE ("MmWaveIabController");

NS_OBJECT_ENSURE_REGISTERED (MmWaveIabController);

TypeId
MmWaveIabController::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::MmWaveIabController")
	.SetParent<Object> ()
	.AddAttribute ("WeightPolicy",
                   "The link weight policy to use in the Max Weighted Matching",
                   StringValue("ns3::MaxSumCapacityPolicy"),
                   MakeStringAccessor(&MmWaveIabController::SetIabWeightPolicyType),
                   MakeStringChecker ())
	.AddAttribute ("DefaultCapacityEstimate",
				   "The Capacity estimate to use when to CQI data is available",
				   DoubleValue (1),
				   MakeDoubleAccessor (&MmWaveIabController::SetIabWeightMinCapacity),
                   MakeDoubleChecker<double> ())
	.AddAttribute ("CentralAllocationPeriod",
				   "The period, in subframes, of the centralized resource allcoation process",
				   UintegerValue (1),
				   MakeUintegerAccessor (&MmWaveIabController::SetAllocationPeriod),
                   MakeUintegerChecker<uint8_t> ())
	.AddAttribute ("InfoCollectionPeriod",
				   "The period, in subframes, of the info collection from the nodes",
				   UintegerValue (1),
				   MakeUintegerAccessor (&MmWaveIabController::SetInfoCollectionPeriod),
                   MakeUintegerChecker<uint8_t> ())
	.AddAttribute ("CentralAllocationEnabled",
				   "Whether to compute central allocation indications",
					BooleanValue (true),
				   MakeBooleanAccessor (&MmWaveIabController::ActivateCentralAlloc),
                   MakeBooleanChecker ())
    ;
	return tid;
}

MmWaveIabController::MmWaveIabController ()
	: m_elapsedSfAlloc (0),
	m_sfAllocationPeriod (1),
	m_elapsedSfCollect (0),
	m_infoCollectPeriod (1),
	m_central (true)
{
	// Initialize the various info exchange callbacks
	m_bsrMapRcvCallback = MakeCallback (&MmWaveIabController::ReceiveBsrInfo, this);
	m_cqiMapRcvCallback = MakeCallback (&MmWaveIabController::ReceiveCqiInfo, this);
	m_weightPolicyFactory = ObjectFactory ();

	AsciiTraceHelper asciiTraceHelper;
	m_bsrStatsOutStream = asciiTraceHelper.CreateFileStream ("BsrStatsTrace.txt");
	*m_bsrStatsOutStream->GetStream () << "time srcImsi targetImsi bsr depth" << std::endl; 
	m_cqiStatsOutStream = asciiTraceHelper.CreateFileStream ("CqiStatsTrace.txt");
	*m_cqiStatsOutStream->GetStream () << "time srcImsi targetImsi cqi" <<  std::endl; 
}

MmWaveIabController::~MmWaveIabController ()
{

}

void
MmWaveIabController::DoDispose ()
{
	NS_LOG_FUNCTION (this);
    m_imsiBsrsMap.clear ();
	m_imsiCqisMap.clear ();
	m_depthChildParentImsiMap.clear ();
	m_edgeList.clear ();
	m_edgeWeightsMap.clear ();
	m_infoForwardCallbacksMap.clear ();
	m_activeEdgesBuffer.clear ();
	m_edgeCountersMap.clear ();
}

BsrReportCallback
MmWaveIabController::GetBsrRcvCallback ()
{
	return m_bsrMapRcvCallback;
}

CqiReportCallback
MmWaveIabController::GetCqiRcvCallback ()
{
	return m_cqiMapRcvCallback;
}

void
MmWaveIabController::SetDepthChildParentImsiMap (std::multimap<uint32_t, IabChildParent> childParentImsiMap)
{
	NS_ASSERT (!childParentImsiMap.empty ());
	m_depthChildParentImsiMap = childParentImsiMap;
	PrintIabTopology ();
	ComputeEdgeList ();
	PrintEdgeList ();
}

void 
MmWaveIabController::SetIabWeightMinCapacity (double capacity)
{
	m_weightPolicyPtr->SetMinCapacity (capacity);
}

void
MmWaveIabController::SetIabWeightPolicyType (std::string type)
{
	NS_LOG_FUNCTION (this << type);
	// Initialize the handle for the weights computation
	m_weightPolicyFactory.SetTypeId (type);
	m_weightPolicyPtr = m_weightPolicyFactory.Create<MmWaveIabWeightPolicy> ();
	
}

void 
MmWaveIabController::SetAllocationPeriod (uint8_t sfAllocPeriod)
{
	m_sfAllocationPeriod = sfAllocPeriod;
}

void 
MmWaveIabController::SetInfoCollectionPeriod (uint8_t sfInfoCollectPeriod)
{
	m_infoCollectPeriod = sfInfoCollectPeriod;
}

void
MmWaveIabController::ActivateCentralAlloc (bool active)
{
	m_central = active;
}

void 
MmWaveIabController::AddSchedInfoRcvCallback(std::pair<uint64_t, SchedReportCallback> schedInfoRcvInfo)
{
	NS_ASSERT (!schedInfoRcvInfo.second.IsNull ());
	m_infoForwardCallbacksMap.insert (schedInfoRcvInfo);
	//NS_LOG_DEBUG ("Updated callbacks map:");
	//PrintInfoForwardingCallbackMap ();
}

void 
MmWaveIabController::ComputeEdgeList ()
{
	// Add the donor node first
	m_edgeList.clear ();
	m_edgeCountersMap.clear ();

	m_edgeList.push_back (EdgeListPair (0, std::vector<uint64_t> ()));	// 0 is the dummy IMSI for the donor
	m_edgeCountersMap.insert (std::pair<uint64_t, std::map<uint64_t, double>> ({ 0, {} }));

	for (auto iabNode : m_depthChildParentImsiMap)
	{
		// Add node to the list
		m_edgeList.push_back (EdgeListPair (iabNode.second.GetChildImsi (), std::vector<uint64_t> ()));
		m_edgeCountersMap.insert (std::pair<uint64_t, std::map<uint64_t, double>> ({ iabNode.second.GetChildImsi (), {} }));
		// Add link to parent to the edge list by updating the edges starting from the parent
		int parentIndex = GetNodeIndexFromImsi (iabNode.second.GetParentImsi ());
		NS_ASSERT (parentIndex != -1);
		m_edgeList.at(parentIndex).second.push_back (iabNode.second.GetChildImsi ());
		NS_ASSERT (m_edgeCountersMap.find (iabNode.second.GetParentImsi ()) != m_edgeCountersMap.end ());
		m_edgeCountersMap.find (iabNode.second.GetParentImsi ())->second.insert (std::pair<uint64_t, double> (iabNode.second.GetChildImsi (), 0));
	}

	//PrintEdgeList ();
	//PrintEdgeCounters ();
}

std::map<uint64_t, uint64_t>
MmWaveIabController::MaxWeightedMatching ()
{
	std::map<uint64_t, uint64_t> activeEdges;
	std::vector<IabNodeIdValue> fUtils {}, gUtils {}; // Each entry holds the IMSI of a node and its utility
	std::vector<uint64_t> maxImsiVec;
	uint64_t srcNodeImsi {};
	double edgeWeight {};

	// Initialize the utilities
	for (auto nodeIt : m_edgeList)
	{
		fUtils.push_back(IabNodeIdValue (nodeIt.first));
		gUtils.push_back(IabNodeIdValue (nodeIt.first));
		maxImsiVec.push_back (0);
	}

	// Bottom-up approach (which relies on the assumption of nodes sorted by depth)
	for (int srcNodeIdx = m_depthChildParentImsiMap.size(); srcNodeIdx >= 0; srcNodeIdx--)	// Start from non-leaves only upwards, N.B. depth map misses the donor 
	{
		double maxUtil {std::numeric_limits<double>::lowest ()}, destUtil {-1}, delta {-1};
		srcNodeImsi = m_edgeList.at (srcNodeIdx).first;

		for (auto destNodeImsi : m_edgeList.at(srcNodeIdx).second)
		{
			// Retrieve dest index from IMSI
			int destNodeIdx = GetNodeIndexFromImsi (destNodeImsi);	// The utility vectors and edge list should have same order
			// Make sure that is the case
			NS_ASSERT (gUtils.at (destNodeIdx).GetImsi () == destNodeImsi);
			NS_ASSERT (fUtils.at (destNodeIdx).GetImsi () == destNodeImsi);
			// Make sure we have the proper entries in the edge weights map
			NS_ASSERT (m_edgeWeightsMap.find (srcNodeImsi) != m_edgeWeightsMap.end ());
			NS_ASSERT (m_edgeWeightsMap.find (srcNodeImsi)->second.find (destNodeImsi) != m_edgeWeightsMap.find (srcNodeImsi)->second.end ());

			edgeWeight = m_edgeWeightsMap.find (srcNodeImsi)->second.find (destNodeImsi)->second;
			gUtils.at(srcNodeIdx).SetValue (gUtils.at(srcNodeIdx).GetValue () + 
											std::max (fUtils.at(destNodeIdx).GetValue (), gUtils.at(destNodeIdx).GetValue ()));
			delta = fUtils.at(destNodeIdx).GetValue () - gUtils.at(destNodeIdx).GetValue ();
			destUtil = edgeWeight - std::max(delta, static_cast<double>(0));
			if (destUtil > maxUtil)
			{
				maxUtil = destUtil;
				maxImsiVec.at (srcNodeIdx) = destNodeImsi;
			}
		}
		if (m_edgeList.at (srcNodeIdx).second.size () != 0)
		{
			fUtils.at(srcNodeIdx).SetValue (gUtils.at(srcNodeIdx).GetValue () + maxUtil);
		}
	}

	
	for (unsigned int index = 0; index < fUtils.size (); index++)
	{
		NS_LOG_DEBUG ("Index " << index << "has F: " << fUtils.at(index).GetValue () << " G: " 
							   << gUtils.at(index).GetValue () << " and maxIMSI: " << maxImsiVec.at (index));
	}
	

	double mwmUtil {0};
	unsigned int utilsIndex {0}, edgesAdded {0};
	while (utilsIndex <= m_depthChildParentImsiMap.size())	// Once we arrive to the leaves, no further allocations can be made
	{
		if (fUtils.at(utilsIndex).GetValue () >= gUtils.at(utilsIndex).GetValue () && m_edgeList.at(utilsIndex).second.size () != 0 )
		{
			NS_ASSERT (gUtils.at(utilsIndex).GetValue () >= 0);
			activeEdges.insert (std::pair<uint64_t, uint64_t> (fUtils.at (utilsIndex).GetImsi (), maxImsiVec.at (utilsIndex)));	// This link is activated in the MWM
			mwmUtil += m_edgeWeightsMap.find (fUtils.at (utilsIndex).GetImsi ())->second.find (maxImsiVec.at (utilsIndex))->second;
			edgesAdded++;
			// Remove the destination node from utils
			uint64_t childenIndex = GetNodeIndexFromImsi (maxImsiVec.at (utilsIndex));
			NS_LOG_LOGIC("Removing dst IMSI:" << maxImsiVec.at (utilsIndex));
			fUtils.at (childenIndex).SetValue (-1);
		}
		utilsIndex++;
	}

	// Make sure we didn't add edges which share the same parent
	NS_ASSERT (edgesAdded == activeEdges.size ());

	//PrintActiveEdges (activeEdges);
	NS_LOG_LOGIC ("The max utility achievable is: " << std::max (fUtils.at(0).GetValue(), gUtils.at(0).GetValue()) 
					<< " while the utility achieved by the MWM is: " << mwmUtil);
	if (std::fabs(std::max (fUtils.at(0).GetValue(), gUtils.at(0).GetValue()) - mwmUtil) >= EPSILON)
	{
		NS_LOG_UNCOND ("The max utility achievable is: " << std::max (fUtils.at(0).GetValue(), gUtils.at(0).GetValue()) 
					<< " while the utility achieved by the MWM is: " << mwmUtil);
		NS_LOG_UNCOND ("The difference between the utilities is: " << std::fabs(std::max (fUtils.at(0).GetValue(), gUtils.at(0).GetValue()) - mwmUtil));
	}
	NS_ASSERT (std::fabs(std::max (fUtils.at(0).GetValue(), gUtils.at(0).GetValue()) - mwmUtil) < EPSILON);

	// Make sure we didn't add edges which share the same child
	std::set<uint64_t> uniqueDest;
	for (auto activeEdge : activeEdges)
	{
		uniqueDest.insert (activeEdge.second);
	}
	NS_ASSERT (uniqueDest.size () == activeEdges.size ());
	
	return activeEdges;
}

void
MmWaveIabController::PerformCentralizedResourceAllocation ()
{
	NS_LOG_LOGIC ("Performing the centralized resource allocation");
	// Compute the edge weights
	m_edgeWeightsMap = m_weightPolicyPtr->ComputeWeights (m_imsiBsrsMap, m_imsiCqisMap, m_edgeCountersMap, m_edgeList);
	PrintEdgeList ();
	PrintEdgeWeights ();
	// Compute the MWM
	std::map<uint64_t, uint64_t> activeEdges = MaxWeightedMatching ();
	// Update the counters
	UpdateEdgeCounters (activeEdges);
	
	PrintActiveEdges (activeEdges);
	PrintEdgeCounters ();

	// Add donor instructions to the entry in the buffer which will be received during this subframe
	if (activeEdges.find (0) != activeEdges.end ())
	{
		AddToActiveEdgesBuffer (IabChildParent(activeEdges.find (0)->second, 0), 0);
	}
	// Save active edges in the buffer
	for (uint8_t depthIndex = 1; depthIndex <= m_depthChildParentImsiMap.size (); depthIndex++)
	{
    	auto depthEntries = m_depthChildParentImsiMap.equal_range(depthIndex);
    	for (auto iabNodeIt = depthEntries.first; iabNodeIt!=depthEntries.second; ++iabNodeIt)
		{
			uint64_t iabNodeImsi = iabNodeIt->second.GetChildImsi ();
			// If such IMSI has a scheduling indication, add it to the buffer
			if (activeEdges.find (iabNodeImsi) != activeEdges.end ())
			{
				NS_LOG_LOGIC ("Adding the edge: " << iabNodeImsi << " - " << activeEdges.find (iabNodeImsi)->second
												  << " at depthIndex: " << (uint16_t)depthIndex);
				NS_ASSERT (activeEdges.find (iabNodeImsi)->first == iabNodeImsi);
				AddToActiveEdgesBuffer (IabChildParent(activeEdges.find (iabNodeImsi)->second, iabNodeImsi), depthIndex);
			}
		}
	}
	
	m_elapsedSfAlloc = 0;
}

void 
MmWaveIabController::AddToActiveEdgesBuffer (IabChildParent edge, uint8_t index)
{
	if (m_activeEdgesBuffer.size () > index)
	{
		m_activeEdgesBuffer.at (index).push_back (edge);
	}
	else
	{
		for (unsigned int emptyIndex = m_activeEdgesBuffer.size (); emptyIndex < index; emptyIndex++)
		{
			m_activeEdgesBuffer.push_back ({});
		}
		NS_ASSERT (m_activeEdgesBuffer.size () == index); // Now only 1 should be missing
		m_activeEdgesBuffer.push_back ({edge});
	}
}

void 
MmWaveIabController::SubframeIndication ()
{
	NS_LOG_LOGIC ("Starting a new subframe");
	// Update counters
	m_elapsedSfAlloc++;
	m_elapsedSfCollect = (m_elapsedSfCollect + 1) % m_infoCollectPeriod;
	if (m_elapsedSfAlloc == m_sfAllocationPeriod && m_central)
	{
		PerformCentralizedResourceAllocation ();
		//PrintBsrMaps ();
		//PrintActiveEdgesBuffer ();
		// Send the scheduling indications to the IAB nodes
		try {
			for (auto activeEdge : m_activeEdgesBuffer.at (0))
			{
				uint64_t parentImsi = activeEdge.GetParentImsi ();
				NS_ASSERT (m_infoForwardCallbacksMap.find (parentImsi) != m_infoForwardCallbacksMap.end ());	// TODO: check also whether this is a valid edge
				
				SchedReportCallback parentCallback = m_infoForwardCallbacksMap.find (parentImsi)->second;
				parentCallback (activeEdge.GetChildImsi (), parentImsi);
			}
		}
		catch (const std::out_of_range& err) {
			NS_LOG_WARN ( "No scheduling indications to be sent!");
		}
		// Remove first element in the queue
		m_activeEdgesBuffer.erase (m_activeEdgesBuffer.begin ());
	}
	
}


int
MmWaveIabController::GetNodeIndexFromImsi (uint64_t imsi)
{
	for (auto iabNode = m_edgeList.begin (); iabNode != m_edgeList.end (); iabNode++)
	{
		if (iabNode->first == imsi)
		{
			return iabNode - m_edgeList.begin ();
		}
	}

	return -1;
}

void
MmWaveIabController::ReceiveBsrInfo (std::map<uint64_t, double> incomingBsrInfo, uint64_t srcNodeImsi)
{
	NS_LOG_LOGIC ("IAB controller: BSR map received!");
	if (m_elapsedSfCollect != 0)
	{
		NS_LOG_LOGIC ("Info collection period not elapsed yet, skipping BSR info update");
		return;
	}

	auto imsiBsrsIt = m_imsiBsrsMap.find(srcNodeImsi);
	if (imsiBsrsIt != m_imsiBsrsMap.end ())
	{
		imsiBsrsIt->second = incomingBsrInfo;
	}
	else
	{
		m_imsiBsrsMap.insert (std::pair<uint64_t, std::map<uint64_t, double>> (srcNodeImsi, incomingBsrInfo));
	}
	NS_LOG_LOGIC ("IAB controller: BSR maps updated!");
	PrintBsrMaps ();
}

void 
MmWaveIabController::ReceiveCqiInfo (std::map<uint64_t, double> incomingCqiInfo, uint64_t srcNodeImsi)
{
	// UEs are not part of the initial edge list and they connect later on in the sim: check whether new UEs have connected!
	UpdateNetworkTopologyInfo (incomingCqiInfo, srcNodeImsi);

	NS_LOG_LOGIC ("IAB controller: CQI map received!");
	if (m_elapsedSfCollect != 0)
	{
		NS_LOG_LOGIC ("Info collection period not elapsed yet, skipping BSR info update");
		return;
	}

	auto imsiCqisIt = m_imsiCqisMap.find(srcNodeImsi);
	if (imsiCqisIt != m_imsiCqisMap.end ())
	{
		imsiCqisIt->second = incomingCqiInfo;
	}
	else
	{
		m_imsiCqisMap.insert (std::pair<uint64_t, std::map<uint64_t, double>> (srcNodeImsi, incomingCqiInfo));
	}

	NS_LOG_LOGIC ("IAB controller: CQI maps updated!");
	PrintCqiMaps ();
}

void
MmWaveIabController::UpdateNetworkTopologyInfo (const std::map<uint64_t, double>& incomingCqiInfo, uint64_t srcNodeImsi)
{
	auto srcIndex = GetNodeIndexFromImsi (srcNodeImsi);
	NS_ASSERT (m_edgeList.at (srcIndex).first == srcNodeImsi);
	NS_ASSERT (m_edgeCountersMap.find (srcNodeImsi) != m_edgeCountersMap.end ());
	for (auto incomingDstIt : incomingCqiInfo)
	{
		bool alreadyThere {false};
		auto oldListIt = m_edgeList.at (srcIndex).second.begin ();
		while (oldListIt != m_edgeList.at (srcIndex).second.end () && !alreadyThere)
		{
			alreadyThere = (*oldListIt == incomingDstIt.first);
			oldListIt++;
		}
		if (!alreadyThere)
		{	
			// Check if IAB
			if (GetNodeIndexFromImsi (incomingDstIt.first) != -1)
			{
				// TODO: why does it happen?
				NS_LOG_DEBUG ("Supposedly new link towards IAB with IMSI: " << incomingDstIt.first);
				continue;
			}
			NS_LOG_LOGIC ("The network topology has changed, new UE with IMSI: " << incomingDstIt.first);
			// Add this node to the network topology info
			m_edgeList.at (srcIndex).second.push_back (incomingDstIt.first); // Add the edge
			m_edgeList.push_back (EdgeListPair (incomingDstIt.first, std::vector<uint64_t> ())); // add the node

			// Insert such entries also to the counters map
			NS_ASSERT (m_edgeCountersMap.find (srcNodeImsi) != m_edgeCountersMap.end ());
			m_edgeCountersMap.find (srcNodeImsi)->second.insert (std::pair<uint64_t, double> (incomingDstIt.first, 0));
			m_edgeCountersMap.insert (std::pair<uint64_t, std::map<uint64_t, double>> ({ incomingDstIt.first, {} }));

			PrintEdgeList ();
			PrintEdgeCounters ();
		}
	}
}

void
MmWaveIabController::UpdateEdgeCounters (std::map<uint64_t, uint64_t> activeEdges)
{
	NS_LOG_DEBUG ("Updating edge counters!");
	// Increment all counters
	for (auto srcNodeIt = m_edgeCountersMap.begin (); srcNodeIt != m_edgeCountersMap.end (); srcNodeIt++)
	{
		for (auto dstNodeIt = srcNodeIt->second.begin (); dstNodeIt != srcNodeIt->second.end (); dstNodeIt++)
		{
			dstNodeIt->second++;
		}	
	}
	// Reset the active edges
	for (auto activeEdge : activeEdges)
	{
		m_edgeCountersMap.find (activeEdge.first)->second.find (activeEdge.second)->second = 0;
	}
}

void
MmWaveIabController::PrintBsrMaps ()
{
	for (auto imsiBsrIt : m_imsiBsrsMap)
	{
		NS_LOG_DEBUG ("BSR info for source IMSI: " << imsiBsrIt.first);
		for (auto currImsiBsrIt : imsiBsrIt.second)
		{
			uint32_t nodeDepth {};
			// Find corresponding depth in the IAB network
			for (auto iabNode : m_depthChildParentImsiMap)
			{
				if (iabNode.second.GetChildImsi () == imsiBsrIt.first)
				{
					nodeDepth = iabNode.first;
				}
			} 

			NS_LOG_DEBUG ("\t Target IMSI : " << currImsiBsrIt.first << " - Buffer status: " << currImsiBsrIt.second);
			*m_bsrStatsOutStream->GetStream () << Simulator::Now ().GetNanoSeconds () << " " <<  imsiBsrIt.first << " "  
											   << currImsiBsrIt.first << " " << currImsiBsrIt.second << " " << nodeDepth << std::endl;
			//NS_LOG_UNCOND ("BSR " << currImsiBsrIt.second);
		}
	}
}

void 
MmWaveIabController::PrintCqiMaps ()
{
	for (auto imsiCqiIt : m_imsiCqisMap)
	{
		NS_LOG_DEBUG ("CQI info for source IMSI: " << imsiCqiIt.first);
		for (auto currImsiCqiIt : imsiCqiIt.second)
		{
			NS_LOG_DEBUG ("\t Target IMSI : " << currImsiCqiIt.first << " - capacity estimate: " 
											  << (double)currImsiCqiIt.second);
			//*m_cqiStatsOutStream->GetStream () << Simulator::Now ().GetNanoSeconds () << " " << imsiCqiIt.first << " " 
			//								   << currImsiCqiIt.first << " " << (double)currImsiCqiIt.second << std::endl;
			//NS_LOG_UNCOND ("CQI "<< (double)currImsiBsrIt.second);
		}
	}	
}

void 
MmWaveIabController::PrintIabTopology ()
{
	for (auto iabNode : m_depthChildParentImsiMap)
	{
		NS_LOG_DEBUG ("IAB node: " << iabNode.second.GetChildImsi () << " sits at depth: " << iabNode.first 
									<< " and is attached to: " << iabNode.second.GetParentImsi ());
	}	
}

void 
MmWaveIabController::PrintEdgeWeights ()
{
	NS_LOG_DEBUG ("---------------------------------");
	NS_LOG_DEBUG ("Current edge weights: ");
	for (auto srcNodeIt : m_edgeWeightsMap)
	{
		NS_LOG_DEBUG ("Edge between SRC node: " << srcNodeIt.first << " and:");
		for (auto dstNodeIt : srcNodeIt.second)
		{
			NS_LOG_DEBUG ("\tDST node: " << dstNodeIt.first << " has weight: " << dstNodeIt.second);
		}
	}	
	NS_LOG_DEBUG ("---------------------------------");
}

void
MmWaveIabController::PrintEdgeCounters ()
{
	NS_LOG_DEBUG ("---------------------------------");
	NS_LOG_DEBUG ("Current edge counters: ");
	for (auto srcNodeIt : m_edgeCountersMap)
	{
		NS_LOG_DEBUG ("Edge between SRC node: " << srcNodeIt.first << " and:");
		for (auto dstNodeIt : srcNodeIt.second)
		{
			NS_LOG_DEBUG ("\tDST node: " << dstNodeIt.first << " has not been favored for : " << dstNodeIt.second << " subframes");
		}
	}	
	NS_LOG_DEBUG ("---------------------------------");
}

void 
MmWaveIabController::PrintEdgeList ()
{
	for (auto iabNode : m_edgeList)
	{
		NS_LOG_DEBUG ("IAB node: " << iabNode.first << " has a link towards: ");
		if (iabNode.second.size () == 0)
		{
			NS_LOG_DEBUG ("\t ..actually no one!");
			continue;
		}
		for (auto link : iabNode.second)
		{
			NS_LOG_DEBUG ("\t IAB node " << link);
		}
	}	
}


void 
MmWaveIabController::PrintActiveEdges (const std::map<uint64_t, uint64_t>& activeEdges)
{
	NS_LOG_DEBUG ("Printing a list of active edges");
	double edgeWeight {};
	for (auto activeEntry : activeEdges)
	{
		NS_LOG_DEBUG ("SRC node: " << activeEntry.first << " to DST node: " << activeEntry.second);
		NS_ASSERT (m_edgeWeightsMap.find (activeEntry.first) != m_edgeWeightsMap.end());
		NS_ASSERT (m_edgeWeightsMap.find (activeEntry.first)->second.find (activeEntry.second) != 
				   m_edgeWeightsMap.find (activeEntry.first)->second.end());

		edgeWeight = m_edgeWeightsMap.find (activeEntry.first)->second.find (activeEntry.second)->second;
		NS_LOG_DEBUG ("\tand weight: " << edgeWeight);
	}	
}

void
MmWaveIabController::PrintInfoForwardingCallbackMap ()
{
	for (auto callbackPair : m_infoForwardCallbacksMap)
	{
		NS_LOG_DEBUG ("Sched info callback available for IMSI: " << callbackPair.first);
	}
}

 void
 MmWaveIabController::PrintActiveEdgesBuffer ()
 {
	NS_LOG_DEBUG ("---------------------------------");
	for (unsigned int depthIndex = 0; depthIndex < m_activeEdgesBuffer.size (); depthIndex++)
	{
		for (auto activePair : m_activeEdgesBuffer.at(depthIndex))
		{
			NS_LOG_DEBUG ("In " << depthIndex << " subframes the edge from: " << activePair.GetParentImsi () 
								<< " to: " << activePair.GetChildImsi () << " will be activated");
		}
	}
	NS_LOG_DEBUG ("---------------------------------");
 }

}// namespace ns3
