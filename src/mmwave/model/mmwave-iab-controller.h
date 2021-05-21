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


#ifndef SRC_MMWAVE_IAB_CONTROLLER_H_
#define SRC_MMWAVE_IAB_CONTROLLER_H_

#include <map>
#include <ns3/simulator.h>
#include "ns3/callback.h"
#include "ns3/mmwave-iab-weight-policy.h"
#include "ns3/output-stream-wrapper.h"

namespace ns3{

	class MmWaveIabWeightPolicy;	// Avoid circular dependencies issues

	typedef Callback<void, std::map<uint64_t, double>, uint64_t> BsrReportCallback;
	typedef Callback<void, std::map<uint64_t, double>, uint64_t> CqiReportCallback; 
	typedef Callback<void, uint64_t, uint64_t> SchedReportCallback;	// TODO: temp
	typedef std::pair<uint64_t, std::vector<uint64_t>> EdgeListPair;
	typedef std::map<uint64_t, std::map<uint64_t, double>> SrcDstImsiNodeInfoMap;

   /** 
	* Struct holding a pair of IMSIs, one related to a parent node and one to the child node.
	* Its main purpose is to aid code readability and to avoid mistaking one such IMSI for the other.
	*/
	struct IabChildParent
	{
	   /** 
		* Default constructor, initializes all IMSIs to 0
		*/
		IabChildParent () : m_childImsi (0), m_parentImsi (0)
		{
		}

	   /** 
		* Additional constructor, allows to specify the child and parent IMSIs
		*
		* \param childImsi the IMSI of the child node
		* \param parentImsi the IMSI of the parent node
		*/
		IabChildParent (uint64_t childImsi, uint64_t parentImsi)
			: m_childImsi (childImsi), m_parentImsi (parentImsi)
		{
		}

	   /** 
		* Returns the IMSI of the child node
		*/
		uint64_t GetChildImsi ()
		{
			return m_childImsi;
		}

	   /** 
		* Returns the IMSI of the parent node
		*/
		uint64_t GetParentImsi ()
		{
			return m_parentImsi;
		}

		uint64_t m_childImsi;	// The IMSI of the child node
		uint64_t m_parentImsi;	// The IMSI of the parent node
	};

   /** 
	* Struct holding the node's IMSI and a value related to it (utility, weight ecc.).
	* Its main purpose it to aid code readability and avoid mistaking one of such quantities for the other.
	*/
	struct IabNodeIdValue
	{
	   /** 
		* Default constructor, initializes all values to 0
		*/
		IabNodeIdValue () : m_imsi (0), m_value (0)
		{
		}

	   /** 
		* Additional constructor, allows to specify the node's IMSI and sets the default utility to 0 
		*
		* \param imsi the node's IMSI
		*/
		IabNodeIdValue (uint64_t imsi)
			: m_imsi (imsi), m_value (0)
		{
		}

	   /** 
		* Additional constructor, allows to specify both the node's IMSI and the related value
		*
		* \param imsi the node's IMSI
		* \param value the node's info
		*/
		IabNodeIdValue (uint64_t imsi, double value)
			: m_imsi (imsi), m_value (value)
		{
		}

	   /** 
		* Returns the node's IMSI 
		*/
		uint64_t GetImsi ()
		{
			return m_imsi;
		}

	   /** 
		* Returns the node's info 
		*/
		double GetValue ()
		{
			return m_value;
		}

	   /** 
		* Sets the node's info 
		*
		* \param util the value related to such node
		*/
		void SetValue (double value)
		{
			m_value = value;
		}

		uint64_t m_imsi;	// The node's IMSI.
		double m_value;	// The node's info.
	};


class MmWaveIabController : public Object
{
public:

	MmWaveIabController ();

	virtual ~MmWaveIabController ();

	// Inherited from Object
	static TypeId GetTypeId (void);
	void DoDispose ();

   /**
    * Returns the callback in charge of receiving the IMSI-BSR map from the scheduler instances
    */
	BsrReportCallback GetBsrRcvCallback ();

   /**
    * Returns the callback in charge of receiving the IMSI-CQI map from the scheduler instances
    */
	CqiReportCallback GetCqiRcvCallback ();

   /**
    * Receive the depth of IAB's node to IMSI and parent's IMSI map
	* 
	* \param childParentImsiMap  the incoming depth to IAB's node IMSI and parent's IMSI map
    */
	void SetDepthChildParentImsiMap (std::multimap<uint32_t, IabChildParent> childParentImsiMap);

   /**
    * Sets a default, min capacity estimate which will be used when no CQI info is available
	* 
	* \param capacity  the minimum capacity estimate (corresponding to CQI index 1)
    */
	void SetIabWeightMinCapacity (double capacity);

   /**
    * Sets the weight computation policy that shall be used for the Max Weighted Matching algorithm
	* 
	* \param type the type to be used, must be a an instance of #MmWaveIabWeightPolicy
    */
	void SetIabWeightPolicyType (std::string type);

   /**
    * Sets the period, in number of subframes, of the centralized allocation process
	* 
	* \param sfAllocPeriod The centralized allocation process period, in number of subframes
    */
	void SetAllocationPeriod (uint8_t sfAllocPeriod);

   /**
    * Sets the period, in number of subframes, of the info collection process
	* 
	* \param sfInfoCollectPeriod The info collection process period, in number of subframes
    */
	void SetInfoCollectionPeriod (uint8_t sfInfoCollectPeriod);

   /**
    * Sets the period, in number of subframes, of the info collection process
	* 
	* \param sfInfoCollectPeriod The info collection process period, in number of subframes
    */
	void ActivateCentralAlloc (bool active);

   /**
    * Inserts an info forwarding callback, associated with its target IMSI, to the related map #m_infoForwardCallbacksMap
	* 
	* \param schedInfoRcvInfo The IMSI of the target node and its forwarding callback
    */
	void AddSchedInfoRcvCallback(std::pair<uint64_t, SchedReportCallback> schedInfoRcvInfo);

   /**
    * Notifies the donor controller of the beginning of a new subframe
	* 
	* Based on the number of subframes which have elapsed since the last allocation, calls #PerformCentralizedResourceAllocation
    */
	void SubframeIndication ();


private:
   /**
    * Computes an edge list for the network, starting from the info provided by #m_depthChildParentImsiMap 
    */
	void ComputeEdgeList ();

   /**
    * Computes the maximal weighted matching for the IAB network
    */
	std::map<uint64_t, uint64_t> MaxWeightedMatching ();

   /** 
	* This function is in charge of performing the centralized resource allocation
	* and of dispacthing such info to the various IAB nodes in the network.
	*/ 
	void PerformCentralizedResourceAllocation ();

   /** 
	* Adds a child-parent IMSI pair to #m_activeEdgesBuffer
	*
	* \param edge the edge to be added, identified by child and parent IMSIs
	* \param index effectively, the depth in the network of the parent node 
	*/ 
	void AddToActiveEdgesBuffer (IabChildParent edge, uint8_t index);

   /**
    * Returns the index in the edge list of a given node, identified by its IMSI
	* 
	* \param imsi  the imsi of the node we wish to find
    */
	int GetNodeIndexFromImsi (uint64_t imsi);

   /**
    * Receives the IMSI-BSR map from the scheduler instances and updates #m_imsiBsrsMap accordingly
	* 
	* \param incomingBsrInfo the incoming IMSI-BSR map
	* \param srcNodeImsi the IMSI of the device which sent such info
    */
	void ReceiveBsrInfo (std::map<uint64_t, double> incomingBsrInfo, uint64_t srcNodeImsi);

   /**
    * Receives the IMSI-CQI map from the scheduler instances and updates #m_imsiCqisMap accordingly
	* 
	* \param incomingCqiInfo the incoming IMSI-CQI map
	* \param srcNodeImsi the IMSI of the device which sent such info
    */
	void ReceiveCqiInfo (std::map<uint64_t, double> incomingCqiInfo, uint64_t srcNodeImsi);

   /**
    * Uses the incoming IMSI-CQI map to update the network topology, if needed
	* 
	* Since the initial edge list takes into account IAB nodes only, UEs are added to such list
	* as the simulation progresses. All the UEs connected to a given node are represented by an unique 
	* node in the network topology.
	* 
	* \param incomingCqiInfo the incoming IMSI-CQI map
	* \param srcNodeImsi the IMSI of the device which sent such info
    */
	void UpdateNetworkTopologyInfo (const std::map<uint64_t, double>& incomingCqiInfo, uint64_t srcNodeImsi);

   /**
    * Using the list of edges activated my the MWM, updates the edge counters.
	* 
	* Each time a new centralized resource allocation in performed, the map #m_edgeCountersMap is updated by 
	* setting to 0 the counters of any edge in activeEdges and incrementing by 1 any other counter in the map.
    */
	void UpdateEdgeCounters (std::map<uint64_t, uint64_t> activeEdges);

   /**
    * Logs the curret IMSI-BSR map info. Mostly for debugging purposes.
    */
	void PrintBsrMaps ();

   /**
    * Logs the curret IMSI-CQI map info. Mostly for debugging purposes.
    */
	void PrintCqiMaps ();

   /**
    * Logs info regarding the IAB network topology
	* 
	* For each IAB node, prints the depth from the donor and the parent's IMSI
    */
	void PrintIabTopology ();

   /**
    * Logs the edge list
    */
	void PrintEdgeList ();

   /**
    * Logs a list of the network edges, with their corresponding weights
	* 
	* The meaning of the actual weights varies according to the selected edge computation policy.
    */
	void PrintEdgeWeights ();

   /**
    * Logs a list of the network edges, with their corresponding counters
	* 
	* Such counters indicate the amount of subframes elapsed since the last time that such edge has been marked as active by the MWM.
    */
	void PrintEdgeCounters ();

   /**
    * Logs the list of active edges
    */
	void PrintActiveEdges (const std::map<uint64_t, uint64_t>& activeEdges);

   /**
    * Logs the active edges buffer
    */
   void PrintActiveEdgesBuffer ();

   /**
    * Logs the list of IMSIs for which an info forwarding callback is available
    */
	void PrintInfoForwardingCallbackMap ();

    SrcDstImsiNodeInfoMap m_imsiBsrsMap;	// The current SRC IMSI-(TARGET IMSI-BSR) map
	SrcDstImsiNodeInfoMap m_imsiCqisMap;	// The current SRC IMSI-(TARGET IMSI-CQI) map
	SrcDstImsiNodeInfoMap m_edgeWeightsMap;		// The current SRC IMSI-(TARGET IMSI-EDGE WEIGHT) map
	SrcDstImsiNodeInfoMap m_edgeCountersMap;	/**< The current SRC IMSI-(TARGET IMSI-EDGE COUNTER) map. \n
												The edge counter keeps track of how many subframes have elapsed since the last
												time such edge has been marked as active by the MWM. */

	BsrReportCallback m_bsrMapRcvCallback;	// Callback used to receive the IMSI-BSR map info from the scheduler instances
	CqiReportCallback m_cqiMapRcvCallback;	// Callback used to receive the IMSI-CQI map info from the scheduler instances
	std::map<uint64_t, SchedReportCallback> m_infoForwardCallbacksMap;	// Callback used to send the scheduling info to the scheduler instances

	std::multimap<uint32_t, IabChildParent> m_depthChildParentImsiMap;		// Associates the node depth to the pair IAB's node IMSI and parent's IMSI
	std::vector<EdgeListPair> m_edgeList;	/**< First element of each entry holds the node IMSI, the second a list of its edges \n
                        						 Each edge is represented by the dest IMSI and entries are sorted by their depth in the network.
												 Specifically, each parent node precedes its children in the list */

	ObjectFactory m_weightPolicyFactory;  // The object factory used to create the policy computation handle
	Ptr<MmWaveIabWeightPolicy> m_weightPolicyPtr;	// Handle used to compute the edge weights

	uint8_t m_elapsedSfAlloc;		// Keeps track of the amount of subframes which have elapsed since the last allocation
	uint8_t m_sfAllocationPeriod;	// The period, in number of subframes, of the centralized allocation process
	uint8_t m_elapsedSfCollect;		// Keeps track of the amount of subframes which have elapsed since the last allocation
	uint8_t m_infoCollectPeriod;	// The period, in number of subframes, of the centralized allocation process
	bool m_central;

	std::vector <std::vector <IabChildParent>> m_activeEdgesBuffer;		/**< List of active edges. \n
																		Each main index represents the delay, in subframes, for the validity of	\n
																		such scheduling indication. \n
																		For each such index, we have a list of edges to be activated from (this + index) subframe. */
	
 	Ptr<OutputStreamWrapper> m_bsrStatsOutStream;
	Ptr<OutputStreamWrapper> m_cqiStatsOutStream;
};

}  //namespace ns3


#endif /* MMWAVE_IAB_CONTROLLER_H_ */
