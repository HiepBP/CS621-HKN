/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 *  Copyright (c) 2007,2008,2009 INRIA, UDCAST
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
 * Author: Amine Ismail <amine.ismail@sophia.inria.fr>
 *                      <amine.ismail@udcast.com>
 */

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "packet-loss-counter.h"
#include <tuple>

#include "seq-ts-header.h"
#include "udp-server.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("UdpServer");

NS_OBJECT_ENSURE_REGISTERED (UdpServer);

int total_packets_1 = 0;
int total_packets_2 = 0;
int total_packets = 0;
ns3::Time time_diff_1;
ns3::Time time_diff_2;
double time_diff;
uint16_t port_1 = 9;
uint16_t port_2 = 10;
uint16_t curr_port;
// int mode = 0;
// bool restart = false;

ns3::Time start_1;
ns3::Time end_1;
ns3::Time start_2;
ns3::Time end_2;

clock_t start;
clock_t end;
int count = 0;

clock_t n_s;
clock_t n_e;
int t_1, t_2 = 0;
bool f_1,f_2 = false;
TypeId
UdpServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::UdpServer")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<UdpServer> ()
    .AddAttribute ("Port",
                   "Port on which we listen for incoming packets.",
                   UintegerValue (100),
                   MakeUintegerAccessor (&UdpServer::m_port),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("PacketWindowSize",
                   "The size of the window used to compute the packet loss. This value should be a multiple of 8.",
                   UintegerValue (32),
                   MakeUintegerAccessor (&UdpServer::GetPacketWindowSize,
                                         &UdpServer::SetPacketWindowSize),
                   MakeUintegerChecker<uint16_t> (8,256))
    .AddTraceSource ("Rx", "A packet has been received",
                     MakeTraceSourceAccessor (&UdpServer::m_rxTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("RxWithAddresses", "A packet has been received",
                     MakeTraceSourceAccessor (&UdpServer::m_rxTraceWithAddresses),
                     "ns3::Packet::TwoAddressTracedCallback")
  ;
  return tid;
}

UdpServer::UdpServer ()
  : m_lossCounter (0)
{
  NS_LOG_FUNCTION (this);
  m_received=0;
}

UdpServer::~UdpServer ()
{
  NS_LOG_FUNCTION (this);
}

uint16_t
UdpServer::GetPacketWindowSize () const
{
  NS_LOG_FUNCTION (this);
  return m_lossCounter.GetBitMapSize ();
}

void
UdpServer::SetPacketWindowSize (uint16_t size)
{
  NS_LOG_FUNCTION (this << size);
  m_lossCounter.SetBitMapSize (size);
}

uint32_t
UdpServer::GetLost (void) const
{
  NS_LOG_FUNCTION (this);
  return m_lossCounter.GetLost ();
}

uint64_t
UdpServer::GetReceived (void) const
{
  NS_LOG_FUNCTION (this);
  return m_received;
}

void
UdpServer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
UdpServer::StartApplication (void)
{
  NS_LOG_FUNCTION (this);
  curr_port = m_port;

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (),
                                                   m_port);
      if (m_socket->Bind (local) == -1)
        {
          NS_FATAL_ERROR ("Failed to bind socket");
        }
    }

  m_socket->SetRecvCallback (MakeCallback (&UdpServer::HandleRead, this));

  if (m_socket6 == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket6 = Socket::CreateSocket (GetNode (), tid);
      Inet6SocketAddress local = Inet6SocketAddress (Ipv6Address::GetAny (),
                                                   m_port);
      if (m_socket6->Bind (local) == -1)
        {
          NS_FATAL_ERROR ("Failed to bind socket");
        }
    }

  m_socket6->SetRecvCallback (MakeCallback (&UdpServer::HandleRead, this));
}

void
UdpServer::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0)
    {
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
}

int
UdpServer::GetPacketCount_1() {
  return t_1;
}
int
UdpServer::GetPacketCount_2() {
  return t_2;
}
double
UdpServer::GetTimeDiff_1() {
    std::cout << "\n LOW ENT STARTED @ " << n_s << "  :  ENDED @ " << n_e <<"\n";
    time_diff = ( (n_e - n_s) / (CLOCKS_PER_SEC/1000.0) );
    std::cout << "TIME DIFF (ms) : " << time_diff << "  TOTAL PACKETS : " << t_1 <<"\n";
    return time_diff;
}
double
UdpServer::GetTimeDiff_2() {
    std::cout  << "\n HIGH ENT STARTED @ " << start << "  :  ENDED @ " << end <<"\n";
    time_diff = ( (end - start) / (CLOCKS_PER_SEC/1000.0) );
    std::cout << "TIME DIFF (ms) : " << time_diff << "  TOTAL PACKETS : " << t_2 << '\n';
    return time_diff;
}

void
UdpServer::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  Address localAddress;
  while ((packet = socket->RecvFrom (from)))
    {
      socket->GetSockName (localAddress);
      m_rxTrace (packet);
      m_rxTraceWithAddresses (packet, from, localAddress);
      if (packet->GetSize () > 0)
        {
          if((int)Simulator::Now().GetSeconds() == 2 && (!f_1)) {
            start = clock();
            end = clock();
            f_1 = true;
          }
          if((int)Simulator::Now().GetSeconds() == 10002 && (!f_2)) {
            n_s = start;
            n_e = end;
            time_diff = ((n_e-n_s) / (CLOCKS_PER_SEC/1000.0));

            start = clock();
            end = clock();
            f_2 = true;
          }

          if((int)Simulator::Now().GetSeconds() >= 2 && (int)Simulator::Now().GetSeconds() < 10002 && (f_1)) {
            t_1++;
          }
          if((int)Simulator::Now().GetSeconds() >= 10002 && (f_2)) {
            t_2++;
          }

          SeqTsHeader seqTs;
          packet->RemoveHeader (seqTs);
          uint32_t currentSequenceNumber = seqTs.GetSeq ();

          if (InetSocketAddress::IsMatchingType (from))
            {
              NS_LOG_INFO ("TraceDelay: RX " << packet->GetSize () <<
                           " bytes from "<< InetSocketAddress::ConvertFrom (from).GetIpv4 () <<
                           " Sequence Number: " << currentSequenceNumber <<
                           " Uid: " << packet->GetUid () <<
                           " TXtime: " << seqTs.GetTs () <<
                           " RXtime: " << Simulator::Now () <<
                           " Delay: " << Simulator::Now () - seqTs.GetTs ());
            }
          else if (Inet6SocketAddress::IsMatchingType (from))
            {
              NS_LOG_INFO ("TraceDelay: RX " << packet->GetSize () <<
                           " bytes from "<< Inet6SocketAddress::ConvertFrom (from).GetIpv6 () <<
                           " Sequence Number: " << currentSequenceNumber <<
                           " Uid: " << packet->GetUid () <<
                           " TXtime: " << seqTs.GetTs () <<
                           " RXtime: " << Simulator::Now () <<
                           " Delay: " << Simulator::Now () - seqTs.GetTs ());
            }

          m_lossCounter.NotifyReceived (currentSequenceNumber);
          m_received++;

          end = clock();
        } 

    }
}

} // Namespace ns3
