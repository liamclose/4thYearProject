/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

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
#include "ns3/packet-loss-counter.h"

#include "ns3/seq-ts-header.h"
#include "peer-to-peer-server.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("P2PServer");

NS_OBJECT_ENSURE_REGISTERED (P2PServer);


TypeId
P2PServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::P2PServer")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<P2PServer> ()
    .AddAttribute ("Port",
                   "Port on which we listen for incoming packets.",
                   UintegerValue (100),
                   MakeUintegerAccessor (&P2PServer::m_port),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("PacketWindowSize",
                   "The size of the window used to compute the packet loss. This value should be a multiple of 8.",
                   UintegerValue (32),
                   MakeUintegerAccessor (&P2PServer::GetPacketWindowSize,
                                         &P2PServer::SetPacketWindowSize),
                   MakeUintegerChecker<uint16_t> (8,256))
  ;
  return tid;
}

P2PServer::P2PServer ()
  : m_lossCounter (0)
{
  NS_LOG_FUNCTION (this);
  m_received=0;
}

P2PServer::~P2PServer ()
{
  NS_LOG_FUNCTION (this);
}

uint16_t
P2PServer::GetPacketWindowSize () const
{
  NS_LOG_FUNCTION (this);
  return m_lossCounter.GetBitMapSize ();
}

void
P2PServer::SetPacketWindowSize (uint16_t size)
{
  NS_LOG_FUNCTION (this << size);
  m_lossCounter.SetBitMapSize (size);
}

uint32_t
P2PServer::GetLost (void) const
{
  NS_LOG_FUNCTION (this);
  return m_lossCounter.GetLost ();
}

uint64_t
P2PServer::GetReceived (void) const
{
  NS_LOG_FUNCTION (this);
  return m_received;
}

  void P2PServer::DoDispose (void) {
    NS_LOG_FUNCTION (this);
    Application::DoDispose ();
  }

  Ptr<Packet> P2PServer::CreateReplyPacket(uint8_t* bytes, int size) {
    Ptr<Packet> p = Create<Packet> (bytes, size);
    return p;
  }

  int P2PServer::ParseAction(uint8_t* message) {
    //uint8_t* buffer = new uint8_t[12];
    NS_LOG_INFO(message[0]);
    return message[11];
  }

void P2PServer::Reply(Address from,Ptr<Packet> pckt) {
  NS_LOG_FUNCTION(this);
  NS_LOG_INFO("sending a thing back");
      //maybe need a unique id, ot_udp.c - probably necessary for realism
    //also hash pointer?
    //if we are using the unique id need some logic after first connect
    //otherwise....
  int size = pckt->GetSize();
  uint8_t *buffer = new uint8_t[size];
  pckt->CopyData(buffer, size);
  std::copy(buffer+12, buffer+size, buffer);
  NS_LOG_INFO(buffer);
  int action = ParseAction(buffer);
  NS_LOG_INFO(action);

  //maybe need to account for bittorent magic bits?
  //need to add another set of receive/sends for the connection establishment
  uint8_t send[12] = {0x00000000, 0x00, 0x00};
  uint64_t numwant;
  switch(action) {
    case(0):
      //do things based on a connect
      send[0] = 0;
      NS_LOG_INFO("connect" << send);
      break;
    case(1):
      //do things based on an announce
      //receive - check - announce - reply
      //check for 0 bytes left?
      numwant = buffer[92]*16777216 + buffer[93]*65536 + buffer[94]*256 + buffer[95];
      NS_LOG_INFO("announce" << numwant);
      break;
    case(2):
      //do things based on a scrape
      NS_LOG_INFO("scrape");
      break;
  }
  

  Ptr<Packet> p = CreateReplyPacket(send, 125);
  if ((m_socket->SendTo (p,0,from)) >= 0)
    {
      NS_LOG_INFO ("TraceDelay TX " << 125 << " bytes to "
                                    << from  << " Uid: "
                                    << p->GetUid () << " Time: "
                                    << (Simulator::Now ()).GetSeconds ());

    }
  else
    {
      NS_LOG_INFO ("Error while sending " << 125 << " bytes to "
		   << from);
    }

  //  m_socket->SendTo(p,0,from);
}
  
void
P2PServer::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

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

  m_socket->SetRecvCallback (MakeCallback (&P2PServer::HandleRead, this));

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

  m_socket6->SetRecvCallback (MakeCallback (&P2PServer::HandleRead, this));

}

void
P2PServer::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0)
    {
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
}

void
P2PServer::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    { if (packet->GetSize () > 0)
        {
          SeqTsHeader seqTs;
          
          packet->RemoveHeader (seqTs);
          uint32_t currentSequenceNumber = seqTs.GetSeq ();
           int size = packet->GetSize();
          uint8_t *buffer = new uint8_t[size];
          packet->CopyData(buffer, size);
          NS_LOG_INFO("Server");
          NS_LOG_INFO(buffer);
          NS_LOG_INFO ("TraceDelay: RX " << packet->GetSize () <<
                          " bytes from "<< InetSocketAddress::ConvertFrom (from).GetIpv4 () <<
                           " Sequence Number: " << currentSequenceNumber <<
                           " Uid: " << packet->GetUid () <<
                           " Packet: " << buffer <<
                           " TXtime: " << seqTs.GetTs () <<
                           " RXtime: " << Simulator::Now () <<
                           " Delay: " << Simulator::Now () - seqTs.GetTs ());
          m_lossCounter.NotifyReceived (currentSequenceNumber);
          m_received++;
          NS_LOG_INFO(InetSocketAddress::ConvertFrom(from).GetPort());
	 
	  Reply(from,packet);
        } else {
                NS_LOG_INFO("what the fuck"); 
}
    }
}

}
