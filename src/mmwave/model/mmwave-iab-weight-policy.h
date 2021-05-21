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

#ifndef SRC_MMWAVE_IAB_WEIGHT_POLICY_H_
#define SRC_MMWAVE_IAB_WEIGHT_POLICY_H_

#include <ns3/object.h>
#include "mmwave-iab-controller.h"

namespace ns3 {

	class MmWaveIabController;

	typedef std::map<uint64_t, std::map<uint64_t, double>> SrcDstImsiNodeInfoMap;
	typedef std::pair<uint64_t, std::vector<uint64_t>> EdgeListPair;

   /** 
	* This class provides a generic template for any class providing a specific edge policy 
	* to bu used for the Maximum Weighted Matching algorithm.
	*/
	class MmWaveIabWeightPolicy : public Object
	{
	public:
	
		MmWaveIabWeightPolicy ();

		virtual ~MmWaveIabWeightPolicy ();

		// inherited from Object
		static TypeId GetTypeId (void);
		void DoDispose ();

	   /** 
		* Virtual template, to be overloaded by any class inheriting MmWaveIabWeightPolicy
		*/
		virtual SrcDstImsiNodeInfoMap ComputeWeights(const SrcDstImsiNodeInfoMap& bsrMap, const SrcDstImsiNodeInfoMap& cqiMap, 
													 const SrcDstImsiNodeInfoMap& countersMap, const std::vector<EdgeListPair>& m_edgeList) = 0;

	   /** 
		* Sets the minimum, default capacity value
		*
		* Such value will be used if no CQI info is available, anc corresponds to a CQI index of 1.
		*/
		void SetMinCapacity (double defaultCapacity);

	   /** 
		* Sets the param mu_{threshold}
		*
		* Used only in the MRBA policy.
		*/
		void SetMuThreshold (double muThreshold);

	   /** 
		* Sets the param k
		*
		* Used only in the MRBA policy.
		*/
		void SetK (double k);

	   /** 
		* Sets the param eta
		*
		* Used only in the MRBA policy.
		*/
		void SetEta (double eta);

	   /** 
		* Returns the minimum, default capacity value
		*
		* Such value will be used if no CQI info is available, anc corresponds to a CQI index of 1.
		*/
		double GetMinCapacity ();

	   /** 
		* Returns the param mu_{threshold}
		*
		* Used only in the MRBA policy.
		*/
		double GetMuThreshold ();

	   /** 
		* Returns the param k
		*
		* Used only in the MRBA policy.
		*/
		double GetK ();

	   /** 
		* Returns the param eta
		*
		* Used only in the MRBA policy.
		*/
		double GetEta ();

	private:

		double m_defaultCapacity;	// The minimum, default capacity value	
		double m_muThreshold;
		double m_k;
		double m_eta;
	};

   /** 
	* This class implements the max-sum capacity edge policy.
	*/
	class MaxSumCapacityPolicy : public MmWaveIabWeightPolicy
	{
	public:
	   /**
		* Constructor
		*/
		MaxSumCapacityPolicy ();

	   /**
		* Destructor
		*/
		virtual ~MaxSumCapacityPolicy ();

		// inherited from Object
		static TypeId GetTypeId (void);

	   /** 
		* Computes the edge weights for the IAB network.
		* 
		* Since the objective function implemented by this class is the max-sum capacity,
		* the weight of any given edge of the network is simply an estimate of the data which can 
		* be transmitted over such edge in a whole subframe.
		*/
		virtual SrcDstImsiNodeInfoMap ComputeWeights(const SrcDstImsiNodeInfoMap& bsrMap, const SrcDstImsiNodeInfoMap& cqiMap,
													 const SrcDstImsiNodeInfoMap& countersMap, const std::vector<EdgeListPair>& edgeList);
	};

   /** 
	* This class implements a min-max buffer size policy.
	*/
	class MinMaxBufferPolicy : public MmWaveIabWeightPolicy
	{
	public:
	   /**
		* Constructor
		*/
		MinMaxBufferPolicy ();

	   /**
		* Destructor
		*/
		virtual ~MinMaxBufferPolicy ();

		// inherited from Object
		static TypeId GetTypeId (void);

	   /** 
		* Computes the edge weights for the IAB network.
		* 
		* Since the objective function implemented by this class is min-max buffer size policy,
		* the weight of any given edge of the network is simply the size of its buffer.
		*/
		virtual SrcDstImsiNodeInfoMap ComputeWeights(const SrcDstImsiNodeInfoMap& bsrMap, const SrcDstImsiNodeInfoMap& cqiMap,
													 const SrcDstImsiNodeInfoMap& countersMap, const std::vector<EdgeListPair>& edgeList);
	};

   /** 
	* This class implements a min-max buffer size policy.
	*/
	class MRBAPolicy : public MmWaveIabWeightPolicy
	{
	public:
	   /**
		* Constructor
		*/
		MRBAPolicy ();

	   /**
		* Destructor
		*/
		virtual ~MRBAPolicy ();

		// inherited from Object
		static TypeId GetTypeId (void);

	   /** 
		* Computes the edge weights for the IAB network.
		* 
		* Since the objective function implemented by this class is min-max buffer size policy,
		* the weight of any given edge of the network is simply the size of its buffer.
		*/
		virtual SrcDstImsiNodeInfoMap ComputeWeights(const SrcDstImsiNodeInfoMap& bsrMap, const SrcDstImsiNodeInfoMap& cqiMap,
													 const SrcDstImsiNodeInfoMap& countersMap, const std::vector<EdgeListPair>& edgeList);
	};

}
#endif /* SRC_MMWAVE_IAB_WEIGHT_POLICY_H_ */
