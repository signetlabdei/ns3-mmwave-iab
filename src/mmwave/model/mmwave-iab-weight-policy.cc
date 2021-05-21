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


#include "mmwave-iab-weight-policy.h"
#include <ns3/double.h>
#include <ns3/log.h>
#include <math.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmWaveIabWeightPolicy");

NS_OBJECT_ENSURE_REGISTERED (MmWaveIabWeightPolicy);

MmWaveIabWeightPolicy::MmWaveIabWeightPolicy ()
{
	NS_LOG_FUNCTION (this);
}

MmWaveIabWeightPolicy::~MmWaveIabWeightPolicy ()
{
	NS_LOG_FUNCTION (this);
}

void
MmWaveIabWeightPolicy::DoDispose (void)
{
	NS_LOG_FUNCTION (this);
}

TypeId
MmWaveIabWeightPolicy::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::MmWaveIabWeightPolicy")
    .SetParent<Object> ()
	.AddAttribute ("K",
                   "The parameter k used in the MRBA weight policy",
                   DoubleValue (2),
                   MakeDoubleAccessor (&MmWaveIabWeightPolicy::SetK),
                   MakeDoubleChecker<double> ())
	.AddAttribute ("Eta",
                   "The parameter eta used in the MRBA weight policy",
                   DoubleValue (50),
                   MakeDoubleAccessor (&MmWaveIabWeightPolicy::SetEta),
                   MakeDoubleChecker<double> ())
	.AddAttribute ("MuThreshold",
                   "The parameter muThreshold used in the MRBA weight policy",
                   DoubleValue (10),
                   MakeDoubleAccessor (&MmWaveIabWeightPolicy::SetMuThreshold),
                   MakeDoubleChecker<double> ())
	;

  return tid;
}

void
MmWaveIabWeightPolicy::SetMinCapacity (double defaultCapacity)
{
	m_defaultCapacity = defaultCapacity;
}

double 
MmWaveIabWeightPolicy::GetMinCapacity ()
{
	return m_defaultCapacity;
}

void
MmWaveIabWeightPolicy::SetMuThreshold (double muThreshold)
{
	m_muThreshold = muThreshold;
}

double
MmWaveIabWeightPolicy::GetMuThreshold ()
{
	return m_muThreshold;
}

void
MmWaveIabWeightPolicy::SetK (double k)
{
	m_k = k;
}

double
MmWaveIabWeightPolicy::GetK ()
{
	return m_k;
}

void
MmWaveIabWeightPolicy::SetEta (double eta)
{
	m_eta = eta;
}

double
MmWaveIabWeightPolicy::GetEta ()
{
	return m_eta;
}


NS_OBJECT_ENSURE_REGISTERED (MaxSumCapacityPolicy);

MaxSumCapacityPolicy::MaxSumCapacityPolicy ()
{
	NS_LOG_FUNCTION (this);
}

MaxSumCapacityPolicy::~MaxSumCapacityPolicy ()
{
	NS_LOG_FUNCTION (this);
}

TypeId
MaxSumCapacityPolicy::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::MaxSumCapacityPolicy")
    .SetParent<MmWaveIabWeightPolicy> ()
	.AddConstructor<MaxSumCapacityPolicy> ()
	;

  return tid;
}

SrcDstImsiNodeInfoMap
MaxSumCapacityPolicy::ComputeWeights (const SrcDstImsiNodeInfoMap& bsrMap, const SrcDstImsiNodeInfoMap& cqiMap,
									  const SrcDstImsiNodeInfoMap& countersMap, const std::vector<EdgeListPair>& edgeList)
{
	// Here edge weight is simply the link capacity
	SrcDstImsiNodeInfoMap edgeWeightsMap {};

	// Scan through the list of edges
	for (auto srcListEntry : edgeList)
	{
		uint64_t srcNode {srcListEntry.first};
		for (auto dstNode : srcListEntry.second)
		{
			double edgeCapacity {GetMinCapacity ()};	// Initialize to min, default capacity estimate
			auto edgeCapIt = cqiMap.find (srcNode)->second.find (dstNode);	// Src should always be found, no need to check that!
			auto srcWeightMap = edgeWeightsMap.find (srcNode);
			if (edgeCapIt != cqiMap.find (srcNode)->second.end ()) 
			{
				// We have a capacity estimate available for this link! 
				edgeCapacity = edgeCapIt->second;
			}
			// Check if we already have an entry for such source node
			if (srcWeightMap != edgeWeightsMap.end ())
			{
				srcWeightMap->second.insert (std::pair<uint64_t, double> (dstNode, edgeCapacity));
			}
			else
			{
				edgeWeightsMap.insert (std::pair<uint64_t, std::map<uint64_t, double>> ({ srcNode, {{ dstNode, edgeCapacity }} }));
			}	
		} 	
	}
	
	return edgeWeightsMap;
}

NS_OBJECT_ENSURE_REGISTERED (MinMaxBufferPolicy);

MinMaxBufferPolicy::MinMaxBufferPolicy ()
{
	NS_LOG_FUNCTION (this);
}

MinMaxBufferPolicy::~MinMaxBufferPolicy ()
{
	NS_LOG_FUNCTION (this);
}

TypeId
MinMaxBufferPolicy::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::MinMaxBufferPolicy")
    .SetParent<MmWaveIabWeightPolicy> ()
	.AddConstructor<MinMaxBufferPolicy> ()
	;

  return tid;
}

SrcDstImsiNodeInfoMap
MinMaxBufferPolicy::ComputeWeights (const SrcDstImsiNodeInfoMap& bsrMap, const SrcDstImsiNodeInfoMap& cqiMap,
									const SrcDstImsiNodeInfoMap& countersMap, const std::vector<EdgeListPair>& edgeList)
{
	// Here edge weight is simply the link capacity
	SrcDstImsiNodeInfoMap edgeWeightsMap {};

	// Scan through the list of edges
	for (auto srcListEntry : edgeList)
	{
		uint64_t srcNode {srcListEntry.first};
		for (auto dstNode : srcListEntry.second)
		{
			double edgeBufferSize {0};	// Initialize to empty buffer, default estimate
			auto edgeBufIt = bsrMap.find (srcNode)->second.find (dstNode);	// Src should always be found, no need to check that!
			auto srcWeightMap = edgeWeightsMap.find (srcNode);
			if (edgeBufIt != bsrMap.find (srcNode)->second.end ()) 
			{
				// We have a capacity estimate available for this link! 
				edgeBufferSize = edgeBufIt->second;
			}
			// Check if we already have an entry for such source node
			if (srcWeightMap != edgeWeightsMap.end ())
			{
				srcWeightMap->second.insert (std::pair<uint64_t, double> (dstNode, edgeBufferSize));
			}
			else
			{
				edgeWeightsMap.insert (std::pair<uint64_t, std::map<uint64_t, double>> ({ srcNode, {{ dstNode, edgeBufferSize }} }));
			}	
		} 	
	}
	
	return edgeWeightsMap;
}

NS_OBJECT_ENSURE_REGISTERED (MRBAPolicy);

MRBAPolicy::MRBAPolicy ()
{
	NS_LOG_FUNCTION (this);
}

MRBAPolicy::~MRBAPolicy ()
{
	NS_LOG_FUNCTION (this);
}

TypeId
MRBAPolicy::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::MRBAPolicy")
    .SetParent<MmWaveIabWeightPolicy> ()
	.AddConstructor<MRBAPolicy> ()
	;

  return tid;
}

SrcDstImsiNodeInfoMap
MRBAPolicy::ComputeWeights (const SrcDstImsiNodeInfoMap& bsrMap, const SrcDstImsiNodeInfoMap& cqiMap,
							const SrcDstImsiNodeInfoMap& countersMap, const std::vector<EdgeListPair>& edgeList)
{
	// Here edge weight is simply the link capacity
	SrcDstImsiNodeInfoMap edgeWeightsMap {};

	// Scan through the list of edges
	for (auto srcListEntry : edgeList)
	{
		uint64_t srcNode {srcListEntry.first};
		for (auto dstNode : srcListEntry.second)
		{
			double edgeBufferSize {0};	// Initialize to empty buffer, default estimate
			double edgeCapacity {GetMinCapacity ()};	// Initialize to min, default capacity estimate

			auto edgeBufIt = bsrMap.find (srcNode)->second.find (dstNode);	// Src should always be found, no need to check that!
			auto edgeCapIt = cqiMap.find (srcNode)->second.find (dstNode);	// Src should always be found, no need to check that!
			auto srcWeightMap = edgeWeightsMap.find (srcNode);

			// Obtain rate and buffer size info
			if (edgeBufIt != bsrMap.find (srcNode)->second.end ()) 
			{
				// We have a capacity estimate available for this link! 
				edgeBufferSize = edgeBufIt->second;
			}
			if (edgeCapIt != cqiMap.find (srcNode)->second.end ()) 
			{
				// We have a capacity estimate available for this link! 
				edgeCapacity = edgeCapIt->second;
			}

			double edgeCounter = countersMap.find (srcNode)->second.find (dstNode)->second;
			double edgeWeight {edgeCapacity + GetEta ()*edgeBufferSize*pow(edgeCounter/GetMuThreshold (), GetK ())};

			// Check if we already have an entry for such source node
			if (srcWeightMap != edgeWeightsMap.end ())
			{
				srcWeightMap->second.insert (std::pair<uint64_t, double> (dstNode, edgeWeight));
			}
			else
			{
				edgeWeightsMap.insert (std::pair<uint64_t, std::map<uint64_t, double>> ({ srcNode, {{ dstNode, edgeWeight }} }));
			}	
		} 	
	}
	
	return edgeWeightsMap;
}

}


