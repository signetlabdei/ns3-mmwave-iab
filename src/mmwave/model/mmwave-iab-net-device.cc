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


#include <ns3/simulator.h>
#include <ns3/callback.h>
#include <ns3/node.h>
#include <ns3/packet.h>
#include <ns3/log.h>
#include <ns3/ipv4-l3-protocol.h>
#include <ns3/tcp-l4-protocol.h>
#include <ns3/udp-l4-protocol.h>
#include <ns3/mmwave-iab-net-device.h>


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmWaveIabNetDevice");

NS_OBJECT_ENSURE_REGISTERED (MmWaveIabNetDevice);

TypeId MmWaveIabNetDevice::GetTypeId ()
{
	static TypeId
	    tid =
	    TypeId ("ns3::MmWaveIabNetDevice")
	    .SetParent<NetDevice> ()
    	.AddConstructor<MmWaveIabNetDevice> ()
		.AddAttribute ("Mtu", "The MAC-level Maximum Transmission Unit",
					   UintegerValue (30000),
					   MakeUintegerAccessor (&MmWaveIabNetDevice::SetMtu,
											 &MmWaveIabNetDevice::GetMtu),
					   MakeUintegerChecker<uint16_t> ())
		.AddAttribute ("AccessPhy",
			           "The PHY associated to this IAB node in the access",
			           PointerValue (),
			           MakePointerAccessor (&MmWaveIabNetDevice::m_accessPhy),
		               MakePointerChecker <MmWaveEnbPhy> ())
		.AddAttribute ("AccessMac",
			           "The MAC associated to this IAB node in the access",
					   PointerValue (),
					   MakePointerAccessor (&MmWaveIabNetDevice::m_accessMac),
					   MakePointerChecker <MmWaveEnbMac> ())
		.AddAttribute ("AccessRrc",
						"The RRC layer associated to this IAB node in the access",
						PointerValue (),
						MakePointerAccessor (&MmWaveIabNetDevice::m_accessRrc),
						MakePointerChecker <LteEnbRrc> ())
		.AddAttribute ("CellId",
					   "Cell Identifier for the access",
					   UintegerValue (0),
					   MakeUintegerAccessor (&MmWaveIabNetDevice::m_cellId),
					   MakeUintegerChecker<uint16_t> ())
		.AddAttribute ("AccessAntennaNum",
					   "Antenna number of the device in the access",
					   UintegerValue (64),
					   MakeUintegerAccessor (&MmWaveIabNetDevice::SetAccessAntennaNum,
											 &MmWaveIabNetDevice::GetAccessAntennaNum),
					   MakeUintegerChecker<uint16_t> ())
		.AddAttribute ("BackhaulPhy",
			           "The PHY associated to this IAB node in the backhaul",
			           PointerValue (),
			           MakePointerAccessor (&MmWaveIabNetDevice::m_backhaulPhy),
		               MakePointerChecker <MmWaveUePhy> ())
		.AddAttribute ("BackhaulMac",
			           "The MAC associated to this IAB node in the backhaul",
					   PointerValue (),
					   MakePointerAccessor (&MmWaveIabNetDevice::m_backhaulMac),
					   MakePointerChecker <MmWaveUeMac> ())
		.AddAttribute ("BackhaulRrc",
						"The RRC layer associated to this IAB node in the backhaul",
						PointerValue (),
						MakePointerAccessor (&MmWaveIabNetDevice::m_backhaulRrc),
						MakePointerChecker <LteUeRrc> ())
		.AddAttribute ("EpcUeNas",
	                   "The NAS associated to this UeNetDevice",
	                   PointerValue (),
	                   MakePointerAccessor (&MmWaveIabNetDevice::m_nas),
	                   MakePointerChecker <EpcUeNas> ())
		.AddAttribute ("BackhaulAntennaNum",
					   "Antenna number of the device in the backhaul",
					   UintegerValue (64),
					   MakeUintegerAccessor (&MmWaveIabNetDevice::SetBackhaulAntennaNum,
											   &MmWaveIabNetDevice::GetBackhaulAntennaNum),
					   MakeUintegerChecker<uint16_t> ())
		.AddAttribute ("Imsi",
						"International Mobile Subscriber Identity assigned to this UE",
						UintegerValue (0),
						MakeUintegerAccessor (&MmWaveIabNetDevice::m_imsi),
						MakeUintegerChecker<uint64_t> ())
		.AddAttribute ("Scheduler",
						"The Scheduler associated with the MAC",
						PointerValue (),
					    MakePointerAccessor (&MmWaveIabNetDevice::m_scheduler),
					    MakePointerChecker <MmWaveMacScheduler> ())
		.AddAttribute ("TxPower",
		               "Transmission power in dBm",
		               DoubleValue (33.0),
		               MakeDoubleAccessor (&MmWaveIabNetDevice::SetTxPower,
		                                   &MmWaveIabNetDevice::GetTxPower),
		               MakeDoubleChecker<double> ())
	 .AddAttribute ("NoiseFigure",
               "Loss (dB) in the Signal-to-Noise-Ratio due to non-idealities in the receiver."
               " According to Wikipedia (http://en.wikipedia.org/wiki/Noise_figure), this is "
               "\"the difference in decibels (dB) between"
               " the noise output of the actual receiver to the noise output of an "
               " ideal receiver with the same overall gain and bandwidth when the receivers "
               " are connected to sources at the standard noise temperature T0.\" "
               "In this model, we consider T0 = 290K.",
               DoubleValue (7.0),
               MakeDoubleAccessor (&MmWaveIabNetDevice::SetNoiseFigure,
                                   &MmWaveIabNetDevice::GetNoiseFigure),
               MakeDoubleChecker<double> ())
;

	return tid;
}

MmWaveIabNetDevice::MmWaveIabNetDevice (void)
	: m_isConstructed (false),
	m_isConfigured (false)
{
  NS_LOG_FUNCTION (this);
}


MmWaveIabNetDevice::~MmWaveIabNetDevice (void)
{
  NS_LOG_FUNCTION (this);
}

void
MmWaveIabNetDevice::DoDispose (void)
{
	m_node = 0;
	NetDevice::DoDispose ();
}

void
MmWaveIabNetDevice::SetIfIndex (const uint32_t index)
{
	m_ifIndex = index;
}
uint32_t
MmWaveIabNetDevice::GetIfIndex (void) const
{
	return m_ifIndex;
}
Ptr<Channel>
MmWaveIabNetDevice::GetChannel (void) const
{
	return 0;
}
void
MmWaveIabNetDevice::SetAddress (Address address)
{
	NS_LOG_FUNCTION (this << address);
	m_macaddress = Mac48Address::ConvertFrom (address);
}
Address
MmWaveIabNetDevice::GetAddress (void) const
{
	NS_LOG_FUNCTION (this);
	return m_macaddress;
}
bool
MmWaveIabNetDevice::SetMtu (const uint16_t mtu)
{
	m_mtu = mtu;
	return true;
}
uint16_t
MmWaveIabNetDevice::GetMtu (void) const
{
	return m_mtu;
}
bool
MmWaveIabNetDevice::IsLinkUp (void) const
{
	return m_linkUp;
}
void
MmWaveIabNetDevice::AddLinkChangeCallback (Callback<void> callback)
{

}
bool
MmWaveIabNetDevice::IsBroadcast (void) const
{
	return false;
}
Address
MmWaveIabNetDevice::GetBroadcast (void) const
{
	return Mac48Address::GetBroadcast ();
}
bool
MmWaveIabNetDevice::IsMulticast (void) const
{
	return false;
}
Address
MmWaveIabNetDevice::GetMulticast (Ipv4Address multicastGroup) const
{
	return Mac48Address ("01:00:5e:00:00:00");
}
bool
MmWaveIabNetDevice::IsBridge (void) const
{
	return false;
}
bool
MmWaveIabNetDevice::IsPointToPoint (void) const
{
	return false;
}

bool
MmWaveIabNetDevice::SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
{
	NS_FATAL_ERROR ("Send from not supported");
	return false;
}

Ptr<Node>
MmWaveIabNetDevice::GetNode (void) const
{
	return m_node;
}

void
MmWaveIabNetDevice::SetNode (Ptr<Node> node)
{
	m_node = node;
}

bool
MmWaveIabNetDevice::NeedsArp (void) const
{
	return false;
}

Address
MmWaveIabNetDevice::GetMulticast (Ipv6Address addr) const
{
	Address dummy;
	return dummy;
}

void
MmWaveIabNetDevice::SetReceiveCallback (ReceiveCallback cb)
{
	NS_LOG_FUNCTION (this);
	m_rxCallback = cb;
}

void
MmWaveIabNetDevice::SetPromiscReceiveCallback (PromiscReceiveCallback cb)
{

}

bool
MmWaveIabNetDevice::SupportsSendFrom (void) const
{
	return false;
}

void
MmWaveIabNetDevice::Receive(Ptr<Packet> p)
{
	NS_FATAL_ERROR("Not supported");
}

// for packets from the Access RRC to the Iab application
void
MmWaveIabNetDevice::ReceiveAccess (Ptr<Packet> p)
{
	NS_LOG_FUNCTION (this << p);
	m_rxCallback (this, p, TcpL4Protocol::PROT_NUMBER, Address ());
}

// for packets from the EpcUeNas to the Iab application
void
MmWaveIabNetDevice::ReceiveBackhaul (Ptr<Packet> p)
{
	NS_LOG_FUNCTION (this << p);
	m_rxCallback (this, p, UdpL4Protocol::PROT_NUMBER, Address ());
}

bool
MmWaveIabNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
	bool ret = DoSend ( packet, dest, protocolNumber);
	return ret;
}

Ipv4Address
MmWaveIabNetDevice::GetPacketDestination (Ptr<Packet> packet)
{
	Ipv4Address dest_ip;
	Ptr<Packet> q = packet->Copy();

	Ipv4Header ipHeader;
	q->PeekHeader (ipHeader);
	dest_ip = ipHeader.GetDestination();
	return dest_ip;
}

void
MmWaveIabNetDevice::DoInitialize (void)
{
	NS_LOG_FUNCTION (this);
	m_isConstructed = true;
	UpdateConfig ();
	m_backhaulPhy->SetTxPower (m_txPower);
	m_backhaulPhy->SetNoiseFigure (m_noiseFigure);
	m_backhaulPhy->DoInitialize ();
	m_accessPhy->SetTxPower (m_txPower);
	m_accessPhy->SetNoiseFigure (m_noiseFigure);
	m_accessPhy->DoInitialize ();
	if(m_backhaulRrc!=0)
	{
		m_backhaulRrc->Initialize ();
	}
	if(m_accessRrc!=0)
	{
		m_accessRrc->Initialize ();
	}
	//TODO MAC?
}

void
MmWaveIabNetDevice::UpdateConfig (void)
{
  NS_LOG_FUNCTION (this);

  if (m_isConstructed)
    {
      NS_LOG_LOGIC (this << " Updating configuration: IMSI " << m_imsi);
      m_nas->SetImsi (m_imsi);
      m_backhaulRrc->SetImsi (m_imsi);

      if (!m_isConfigured)
		{
			NS_LOG_LOGIC (this << " Configure cell " << m_cellId);
			// we have to make sure that this function is called only once
			// this method has no effect for the first 4 paramters, but starts the tx of SI
			m_accessRrc->ConfigureCell (72, 72, 0, 0, m_cellId);
			m_isConfigured = true;
		}
    }
  else
    {
      /*
       * NAS and RRC instances are not be ready yet, so do nothing now and
       * expect ``DoInitialize`` to re-invoke this function.
       */
    }
}


// send from EpcIabApplication to the access or the backhaul
bool
MmWaveIabNetDevice::DoSend (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
	NS_LOG_FUNCTION (this << dest << protocolNumber);
    if (protocolNumber == TcpL4Protocol::PROT_NUMBER)
	{
		return m_accessRrc->SendData(packet);
	}
	else if (protocolNumber == UdpL4Protocol::PROT_NUMBER)
	{
	    return m_nas->Send (packet);
	}
	else
	{
		NS_LOG_INFO("unsupported protocol " << protocolNumber << ", only IPv4 is supported");
	  	return false;
	}
}

Ptr<EpcUeNas>
MmWaveIabNetDevice::GetNas (void) const
{
	return m_nas;
}

uint64_t
MmWaveIabNetDevice::GetImsi () const
{
	return m_imsi;
}


uint16_t
MmWaveIabNetDevice::GetCellId () const
{
	return m_cellId;
}

Ptr<MmWaveUePhy>
MmWaveIabNetDevice::GetBackhaulPhy (void) const
{
	return m_backhaulPhy;
}

Ptr<MmWaveUeMac>
MmWaveIabNetDevice::GetBackhaulMac (void) const
{
	return m_backhaulMac;
}

Ptr<LteUeRrc>
MmWaveIabNetDevice::GetBackhaulRrc () const
{
	return m_backhaulRrc;
}

void
MmWaveIabNetDevice::SetBackhaulTargetEnb (Ptr<NetDevice> enb)
{
	m_donorEnb = enb;
}

Ptr<NetDevice>
MmWaveIabNetDevice::GetBackhaulTargetEnb (void)
{
	return m_donorEnb;
}

void
MmWaveIabNetDevice::SetBackhaulAntennaNum (uint16_t antennaNum)
{
	NS_ASSERT_MSG (std::floor (std::sqrt(antennaNum)) == std::sqrt(antennaNum), "Only square antenna arrays are currently supported.");
	m_backhaulAntennaNum = antennaNum;
}

uint16_t
MmWaveIabNetDevice::GetBackhaulAntennaNum () const
{
	return m_backhaulAntennaNum;
}

Ptr<MmWaveEnbPhy>
MmWaveIabNetDevice::GetAccessPhy (void) const
{
	return m_accessPhy;
}

Ptr<MmWaveEnbMac>
MmWaveIabNetDevice::GetAccessMac (void) const
{
	return m_accessMac;
}

Ptr<LteEnbRrc>
MmWaveIabNetDevice::GetAccessRrc () const
{
	return m_accessRrc;
}

Ptr<MmWaveMacScheduler>
MmWaveIabNetDevice::GetMacScheduler() const
{
	return m_scheduler;
}

void
MmWaveIabNetDevice::RequestToSetIabBsrMapCallback (BsrReportCallback infoSendCallback)
{
	m_scheduler->SetIabBsrMapReportCallback (infoSendCallback);
}

void
MmWaveIabNetDevice::RequestToSetIabCqiMapCallback (CqiReportCallback infoSendCallback)
{
	m_scheduler->SetIabCqiMapReportCallback (infoSendCallback);
}

void
MmWaveIabNetDevice::SetAccessAntennaNum (uint16_t antennaNum)
{
	NS_ASSERT_MSG (std::floor (std::sqrt(antennaNum)) == std::sqrt(antennaNum), "Only square antenna arrays are currently supported.");
	m_accessAntennaNum = antennaNum;
}

uint16_t
MmWaveIabNetDevice::GetAccessAntennaNum () const
{
	return m_accessAntennaNum;
}

void
MmWaveIabNetDevice::SetTxPower (double power)
{
	m_txPower = power;
}
double
MmWaveIabNetDevice::GetTxPower () const
{
	return m_txPower;
}

void
MmWaveIabNetDevice::SetNoiseFigure (double nf)
{
	m_noiseFigure = nf;
}
double
MmWaveIabNetDevice::GetNoiseFigure () const
{
	return m_noiseFigure;
}

}
