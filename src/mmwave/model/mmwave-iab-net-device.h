/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/* *
 * Copyright (c) 2016, University of Padova, Dep. of Information Engineering, SIGNET lab. 
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
 * Author: Michele Polese <michele.polese@gmail.com>
 * 
 */

 

#ifndef MMWAVE_IAB_NET_DEVICE_H
#define MMWAVE_IAB_NET_DEVICE_H

#include <ns3/net-device.h>
#include <ns3/event-id.h>
#include <ns3/mac48-address.h>
#include <ns3/traced-callback.h>
#include <ns3/nstime.h>
#include "ns3/lte-ue-rrc.h"
#include "ns3/epc-ue-nas.h"
#include "ns3/mmwave-ue-mac.h"
#include "ns3/mmwave-ue-phy.h"
#include "ns3/mmwave-enb-phy.h"
#include "ns3/mmwave-enb-mac.h"
#include "ns3/lte-enb-rrc.h"

namespace ns3 {

class Packet;
class PacketBurst;
class Node;
// class LteEnbNetDevice;
// class MmWaveEnbNetDevice;


/**
  * \ingroup mmWave
  * This class represents an IAB NetDevice. It hosts both the functionalities
  * of an UE and of a gNB, the first for the backhaul and the latter for the access
  */
class MmWaveIabNetDevice : public NetDevice
{
public: 
	// methods inherited from NetDevide. 
	static TypeId GetTypeId (void);

	MmWaveIabNetDevice ();
	virtual ~MmWaveIabNetDevice ();

    virtual void DoDispose (void);

    virtual void SetIfIndex (const uint32_t index);
    virtual uint32_t GetIfIndex (void) const;
    virtual Ptr<Channel> GetChannel (void) const;
    virtual void SetAddress (Address address);
    virtual Address GetAddress (void) const;
    virtual bool SetMtu (const uint16_t mtu);
    virtual uint16_t GetMtu (void) const;
    virtual bool IsLinkUp (void) const;
    virtual void AddLinkChangeCallback (Callback<void> callback);
    virtual bool IsBroadcast (void) const;
    virtual Address GetBroadcast (void) const;
    virtual bool IsMulticast (void) const;
    virtual Address GetMulticast (Ipv4Address multicastGroup) const;
    virtual bool IsBridge (void) const;
    virtual bool IsPointToPoint (void) const;
    virtual bool SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber);
    virtual Ptr<Node> GetNode (void) const;
    virtual void SetNode (Ptr<Node> node);
    virtual bool NeedsArp (void) const;
    virtual Address GetMulticast (Ipv6Address addr) const;
    virtual void SetReceiveCallback (ReceiveCallback cb);
    virtual void SetPromiscReceiveCallback (PromiscReceiveCallback cb);
    virtual bool SupportsSendFrom (void) const;
    virtual bool Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);

    Ipv4Address GetPacketDestination (Ptr<Packet> packet);
  
    // not supported
    void Receive (Ptr<Packet> p);
    /** 
     * receive a packet from the Access RRC layer in order to forward it to the EpcIabApplication layers
     * 
     * \param p the packet
     */
    void ReceiveAccess (Ptr<Packet> p);

    /** 
     * receive a packet from the backhaul EpcUeNas layer in order to forward it to the EpcIabApplication layer
     * 
     * \param p the packet
     */
    void ReceiveBackhaul (Ptr<Packet> p);

  	Ptr<EpcUeNas> GetNas (void) const;

  	/**
  	 * Get the IMSI associated with the UE in the BH link
  	 */
	uint64_t GetImsi () const;

	/**
	 * Get the CellId associated with the gNB in the access link
	 */
    uint16_t GetCellId () const;

	// ---------------------------- Backhaul methods ------------------------

	Ptr<MmWaveUePhy> GetBackhaulPhy (void) const;

	Ptr<MmWaveUeMac> GetBackhaulMac (void) const;

	Ptr<LteUeRrc> GetBackhaulRrc () const;

	void SetBackhaulTargetEnb (Ptr<NetDevice> enb);

	Ptr<NetDevice> GetBackhaulTargetEnb (void);

  void SetBackhaulAntennaNum (uint16_t antennaNum);

  uint16_t GetBackhaulAntennaNum () const;

	// ---------------------------- Access methods ------------------------

  Ptr<MmWaveEnbPhy> GetAccessPhy (void) const;

  Ptr<MmWaveEnbMac> GetAccessMac (void) const;

  Ptr<LteEnbRrc> GetAccessRrc () const;

	void SetAccessAntennaNum (uint16_t antennaNum);

  uint16_t GetAccessAntennaNum () const;

protected:
    NetDevice::ReceiveCallback m_rxCallback;
    virtual void DoInitialize (void);
	/**
	* \brief Propagate attributes and configuration to sub-modules.
	*
	* Several attributes (e.g., the IMSI) are exported as the attributes of the
	* MmWaveIabNetDevice from a user perspective, but are actually used also in other
	* sub-modules (the RRC, the PHY, etc.). This method takes care of updating
	* the configuration of all these sub-modules so that their copy of attribute
	* values are in sync with the one in the MmWaveIabNetDevice.
	*/
	void UpdateConfig ();

private:
	
  Mac48Address m_macaddress;
  Ptr<Node> m_node;
  mutable uint16_t m_mtu;
  bool m_linkUp;
  uint32_t m_ifIndex;

    // TODO this is probably not needed
	bool DoSend (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);

	// Backhaul
	Ptr<NetDevice> m_donorEnb;
	Ptr<MmWaveUePhy> m_backhaulPhy; // TODO update it with the single PHY
	Ptr<MmWaveUeMac> m_backhaulMac;
	Ptr<LteUeRrc> m_backhaulRrc;
	uint16_t m_backhaulAntennaNum;

	// Access
	Ptr<MmWaveEnbPhy> m_accessPhy;
	Ptr<MmWaveEnbMac> m_accessMac;
	Ptr<LteEnbRrc> m_accessRrc;
  Ptr<MmWaveMacScheduler> m_scheduler;
	uint16_t m_accessAntennaNum;

	bool m_isConstructed;
	bool m_isConfigured;

	// Common
	Ptr<EpcUeNas> m_nas; // TODO one or more NAS? 
	uint64_t m_imsi; 
	uint16_t m_cellId;
	
};

} // namespace ns3

#endif 
//MMWAVE_IAB_NET_DEVICE_H