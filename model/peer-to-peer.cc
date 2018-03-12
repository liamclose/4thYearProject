/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "peer-to-peer.h"
#include "ns3/packet.h"
#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/object-vector.h"
#include <string>
#include "ns3/seq-ts-header.h"
#include <cstdlib>
#include <cstdio>
#include <list>
#include <iterator>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("P2PClient");


int sent = 0;
//sent = sent++;

  std::map<std::string, std::vector<Address>> peers;
  std::set<std::string> cache;
  int tcpPort = 2020;
  std::string file;

NS_OBJECT_ENSURE_REGISTERED (P2PClient);

TypeId
P2PClient::GetTypeId (void)
{
 // messages.Add("hello")
  
  static TypeId tid = TypeId ("ns3::P2PClient")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<P2PClient> ()
    .AddAttribute ("MaxPackets",
                   "The maximum number of packets the application will send",
                   UintegerValue (100),
                   MakeUintegerAccessor (&P2PClient::m_count),
                   MakeUintegerChecker<uint32_t> ())
   .AddAttribute ("Interval",
                   "The time to wait between packets", TimeValue (Seconds (2.0)),
                   MakeTimeAccessor (&P2PClient::m_interval),
                   MakeTimeChecker ())
    .AddAttribute("Mode",
                  "Whether this is serving requests or has a local cache.",
                  UintegerValue(0),
                  MakeUintegerAccessor (&P2PClient::m_mode),
                  MakeUintegerChecker<uint32_t>())
    .AddAttribute("CacheTime",
                  "How long chunks of data are cached for, in milliseconds.",
                  UintegerValue(1000),
                  MakeUintegerAccessor (&P2PClient::m_cacheTime),
                  MakeUintegerChecker<uint32_t>())
    .AddAttribute ("RemoteAddress",
                   "The destination Address of the outbound packets",
                   AddressValue (),
                   MakeAddressAccessor (&P2PClient::m_peerAddress),
                   MakeAddressChecker ())
    .AddAttribute ("RemotePort", "The destination port of the outbound packets",
                   UintegerValue (100),
                   MakeUintegerAccessor (&P2PClient::m_peerPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("PacketSize",
                   "Size of packets generated. The minimum packet size is 12 bytes which is the size of the header carrying the sequence number and the time stamp.",
                   UintegerValue (1024),
                   MakeUintegerAccessor (&P2PClient::m_size),
                   MakeUintegerChecker<uint32_t> (12,1500));
   /* .AddAttribute ("Packets",
                   "The packets",
                   ObjectVectorValue(),
                   MakeObjectVectorAccessor (&P2PClient::m_packets),
                   MakeObjectVectorChecker<Packet> ()); */
  return tid;
}

P2PClient::P2PClient ()
  : m_lossCounter (0)
{
  NS_LOG_INFO("===============???");
  NS_LOG_FUNCTION (this);
  m_sent = 0;
  m_received = 0;
  m_socket = 0;
  m_dataRate =DataRate("2Mbps");
  m_packetSize = 512;
  m_socket_tcp = 0;
  m_nPackets = 3;
  m_connected = false;
  m_packets = std::vector<std::string>(); 
  m_sendEvent = EventId ();
}

P2PClient::P2PClient (std::vector<std::string> packets)
  : m_lossCounter (0)
{
   m_packets = packets;
}

P2PClient::~P2PClient ()
{
  NS_LOG_FUNCTION (this);
}

void
P2PClient::SetRemote (Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = ip;
  m_peerPort = port;
}

void
P2PClient::SetRemote (Address addr)
{
  NS_LOG_FUNCTION (this << addr);
  m_peerAddress = addr;
}

void
P2PClient::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
P2PClient::StartApplication (void)
{
  NS_LOG_FUNCTION (this);
  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
           InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 5050);
             if (Ipv4Address::IsMatchingType(m_peerAddress) == true)
        {
          if (m_socket->Bind (local) == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
        }
      else if (Ipv6Address::IsMatchingType(m_peerAddress) == true)
        {
          if (m_socket->Bind6 () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (Inet6SocketAddress (Ipv6Address::ConvertFrom(m_peerAddress), m_peerPort));
        }
        else if (InetSocketAddress::IsMatchingType (m_peerAddress) == true)
        {
          NS_LOG_INFO("is this it?");
          if (m_socket->Bind (local) == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (m_peerAddress);
        }
      else if (Inet6SocketAddress::IsMatchingType (m_peerAddress) == true)
        {
          if (m_socket->Bind6 () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (m_peerAddress);
        }
      else
        {
          NS_ASSERT_MSG (false, "Incompatible address type: " << m_peerAddress);
        }
    }
  //Bind the tcp socket to port 2020
  if (m_socket_tcp == 0)
    {
      NS_LOG_INFO("TCP SOCKET CREATED");
      TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");
      m_socket_tcp = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), tcpPort);
      if (Ipv4Address::IsMatchingType(m_peerAddress) == true)
        {
          if (m_socket_tcp->Bind (local) == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
      
        }
     }
  NS_LOG_INFO("Setting up callback in client" << m_socket);
  //this is how the client receives responses from the server
  m_socket->SetRecvCallback (MakeCallback (&P2PClient::HandleRead, this));
  //theoretically this would be how the client gets the data...it doesn't work though
  //m_socket_tcp->SetRecvCallback(MakeCallback (&P2PClient::HandleTcp, this));
  m_socket_tcp->SetConnectCallback (
                                    MakeCallback (&P2PClient::HandleConnect, this),
                                    MakeCallback (&P2PClient::HandleConnectError, this));
  m_socket_tcp->SetAcceptCallback (
    MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
    MakeCallback (&P2PClient::HandleAccept, this));
   m_socket_tcp->SetCloseCallbacks (
    MakeCallback (&P2PClient::HandlePeerClose, this),
    MakeCallback (&P2PClient::HandlePeerError, this));
  //  m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
  m_socket->SetAllowBroadcast (true);
  m_socket_tcp->Listen();
 
  //m_socket_tcp->ShutdownSend ();
  m_sendEvent = Simulator::Schedule (Seconds (0.5), &P2PClient::Send, this);
}



void
P2PClient::StopApplication (void)
{
  NS_LOG_FUNCTION (this);
  Simulator::Cancel (m_sendEvent);
}

void P2PClient::SetMessages(std::vector<std::string> messages) {
   NS_LOG_FUNCTION(this);
   m_packets = messages;
}

void P2PClient::HandlePeerClose (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  std::cout << "closing connection";
}
 
void P2PClient::HandlePeerError (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}

void P2PClient::HandleConnectError (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}

void P2PClient::HandleConnect (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_INFO("Fucking finally");
    socket->SetRecvCallback (MakeCallback (&P2PClient::HandleTcp, this));
  //m_socketList.push_back (s);
    uint8_t buffer[1 + file.size()];
    std::fill(buffer, buffer+1+file.size(), 0x00);
  buffer[0] = 0;
  std::copy(file.c_str(), file.c_str()+file.size(), buffer+1);//check protocol
  //std::cout <<"sending: " << buffer << buffer+1 << "\n";
  socket->Send(Create<Packet>(buffer, 2+file.size()));

}
 

void P2PClient::HandleAccept (Ptr<Socket> s, const Address& from)
{
  NS_LOG_FUNCTION (this << s << from);
  NS_LOG_INFO("Handling an accept??" << InetSocketAddress::ConvertFrom(from).GetIpv4() << InetSocketAddress::ConvertFrom(from).GetPort());
  s->SetRecvCallback (MakeCallback (&P2PClient::HandleTcp, this));
  m_socketList.push_back (s);
  //  s->Send(Create<Packet>(m_packetSize));
}


void P2PClient::SetupTCPConnections(std::string filename) {
    NS_LOG_FUNCTION(this);
    //std::cout << file << peers.count(filename) << "\n";
    std::vector<Address> a = peers.at(filename);
    for (uint i=0;i<a.size(); i++) {
       TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");
       Ptr<Socket> s2 = Socket::CreateSocket (GetNode (), tid);
       s2->SetConnectCallback (MakeCallback (&P2PClient::HandleConnect, this),
                                    MakeCallback (&P2PClient::HandleConnectError, this));
      s2->Connect(InetSocketAddress (InetSocketAddress::ConvertFrom(a.at(i)).GetIpv4(), tcpPort));
    }
    //P2PClient::SendPacket();
  }



void P2PClient::SendPacket (Ptr<Socket> sock, Ptr<Packet> packet)
{
  int size = packet->GetSize();
  uint8_t *buffer = new uint8_t[size];
  std::fill(buffer, buffer+size, 0x00);
  packet->CopyData(buffer, size);
      //std::cout <<"Sending packet: " <<buffer << "\n";
  sock->Send (packet);
  if (buffer[0] ==0) {
    buffer[0] = 1;
  } else {
    buffer[0] = 0;
  }
  //  if (++m_tcpSent < m_nPackets)
  // {
  //  ScheduleTx (sock, Create<Packet>(buffer, size));
  // }
}


void P2PClient::ScheduleTx (Ptr<Socket> sock, Ptr<Packet> packet)
  { if (true)
  //  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      NS_LOG_INFO(tNext);
      //Ptr<Packet> packet = Create<Packet>();
      m_sendEvent = Simulator::Schedule (tNext, &P2PClient::SendPacket, this, sock, packet);
    }
}

void
P2PClient::Send (void)
{
  if (m_count == 0) { //useful for server which makes no initial reqs
    return;
  }
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_sendEvent.IsExpired ());
  SeqTsHeader seqTs;
  seqTs.SetSeq (m_sent);
  uint8_t start[12] = {0x00, 0x00, 0x04, 0x17, 0x27, 0x10, 0x19, 0x80, 0x00, 0x00, 0x00, 0x00};
  if (m_sent==m_packets.size()) {
    return;
  }
  
  file = m_packets[m_sent];
  uint8_t* send = new uint8_t[m_size+12];
  std::fill(send, send+m_size+12, 0x00);
  std::copy (start, start+12, send);
  std::copy (file.c_str(), file.c_str()+file.size(), send+12);
  send[95]=1;
  Ptr<Packet> p = Create<Packet> (send, m_size-(8+4)); // 8+4 : the size of the seqTs header
  uint8_t *buffer = new uint8_t[m_size];
  p->CopyData(buffer, m_size);
  p->AddHeader (seqTs);

  std::stringstream peerAddressStringStream;
  if (Ipv4Address::IsMatchingType (m_peerAddress))
    {
      peerAddressStringStream << Ipv4Address::ConvertFrom (m_peerAddress);
    }
  else if (Ipv6Address::IsMatchingType (m_peerAddress)) {
      peerAddressStringStream << Ipv6Address::ConvertFrom (m_peerAddress);
    }
  Ptr<Ipv4> ipv4 = this->m_node->GetObject<Ipv4>();
      m_localIpv4  =ipv4->GetAddress(1,0).GetLocal();
      NS_LOG_INFO("About to try to send" << m_sent);
    if ((m_socket->Send (p)) >= 0)
      {
        p->CopyData(buffer, m_size);
      ++m_sent;
      NS_LOG_INFO("------------------------------------------Have now sent " << m_sent);
      NS_LOG_INFO ("TraceDelay TX " << m_size << " bytes to "
                                    << peerAddressStringStream.str () << " Uid: "
                                    << p->GetUid () << " Time: "
                                    << (Simulator::Now ()).GetSeconds ());

    }
  else
    {
      NS_LOG_INFO ("Error while sending " << m_size << " bytes to "
                                          << peerAddressStringStream.str ());
    }


  if (m_sent < m_count)
    {
      m_sendEvent = Simulator::Schedule (m_interval, &P2PClient::Send, this);
    }
}


std::string P2PClient::UpdatePeers(std::string received) {
    NS_LOG_FUNCTION(this);
    std::istringstream iss(received);
    std::vector<std::string> parts(( std::istream_iterator<std::string>(iss)),
				   std::istream_iterator<std::string>());
    std::vector<Address> a;
    std::vector<Address>::iterator it;
    std::string filename = " " + parts[0] + " " +parts[1];
    if (peers.count(filename)!=0) {
      peers.erase(filename);
    }
    // std::cout << "File: " << filename <<"\n";
    //it = a.begin();
    for(uint i = 2; i<parts.size()-1; i=i+2) {
      it = a.end();
      // std::cout << i;
      //std::cout << "parts: " << parts.at(i) <<"\n";
      Ptr<Ipv4> ipv4 = this->m_node->GetObject<Ipv4>();
      m_localIpv4  =ipv4->GetAddress(1,0).GetLocal();
      Address A = InetSocketAddress(parts.at(i).c_str(), 2020);//std::stoi(parts.at(i+1)));
      if (!(InetSocketAddress::ConvertFrom(A).GetIpv4() ==m_localIpv4)) {
        NS_LOG_INFO("Found a peer" << InetSocketAddress::ConvertFrom(A).GetIpv4());
        a.insert(it, A);
      } else {
        NS_LOG_INFO(InetSocketAddress::ConvertFrom(A).GetIpv4() << m_localIpv4);
      }
    }
    //std::cout << "inserting:" <<filename <<"\n";
    peers.insert(std::pair<std::string, std::vector<Address>>(filename, a));
    return filename;
}

  void P2PClient::HandleTcp (Ptr<Socket> socket) {
    NS_LOG_FUNCTION (this << socket);
    Ptr<Packet> packet;
    Ptr<Packet> p;
    Address from;
    std::string data;
  while ((packet = socket->RecvFrom (from)))
    {
      if (packet->GetSize () == 0)
        { //EOF
          break;
        }
      m_totalRx += packet->GetSize ();
      if (InetSocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                       << "s packet sink received "
                       <<  packet->GetSize () << " bytes from "
                       << InetSocketAddress::ConvertFrom(from).GetIpv4 ()
                       << " port " << InetSocketAddress::ConvertFrom (from).GetPort ()
                       << " total Rx " << m_totalRx << " bytes");
        }
      else if (Inet6SocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                       << "s packet sink received "
                       <<  packet->GetSize () << " bytes from "
                       << Inet6SocketAddress::ConvertFrom(from).GetIpv6 ()
                       << " port " << Inet6SocketAddress::ConvertFrom (from).GetPort ()
                       << " total Rx " << m_totalRx << " bytes");
        }
      //logging, enable later?
      //m_rxTrace (packet, from);
      int size = packet->GetSize();
      uint8_t *buffer = new uint8_t[size];
      std::fill(buffer, buffer+size, 0x00);
      packet->CopyData(buffer, size);
      std::cout << "Buffer contents: " << buffer << buffer +2 << m_sent << "\n";
      data = std::string((char*) buffer+1);
      NS_LOG_INFO(buffer);
                                              //      data = data.substr(1,size);
        if (buffer[0]==0) {        
          //std::cout <<"File requested is: " << data <<"\n";

          if (cache.count(data)!=0||m_mode==1) {
          buffer[0] = 1;
          packet = Create<Packet>(buffer, size);
        } else {
          buffer[0] = 2;
          packet = Create<Packet>(buffer, size);
        }
        ScheduleTx (socket, Create<Packet>(buffer, size));
      } else if (buffer[0]==1){
        NS_LOG_INFO("Got data, updating local cache" << buffer);
        std::set<std::string>::iterator it = cache.begin();
        cache.insert(it, data);
        buffer[0] = 0;
        if (++m_tcpSent < m_nPackets) { //if we don't yet have all the data, request next bit
          ScheduleTx (socket, Create<Packet>(buffer, size));
        } else { //otherwise set the expiry.
          Simulator::Schedule (MilliSeconds (m_cacheTime), &P2PClient::ExpireCache, this, data);    
        }
      } else {
          std::cout << "Expired cache, need to do something about that\n";
          m_sent = m_sent -1;
               NS_LOG_INFO("------------------------------------------Have now sent " << m_sent);
          Simulator::Schedule (MilliSeconds (0.0), &P2PClient::Send, this);    
      }
     
    }

  }

void P2PClient::ExpireCache(std::string file) {
  NS_LOG_INFO("Expiring " << file << " from cache.");
        if (cache.size()>0) {
          cache.erase(file);
        } else {
          NS_LOG_ERROR("file does not exist");
          return;
        }
                  std::stringstream peerAddressStringStream;
        uint8_t *buffer = new uint8_t[m_size-12];
      std::fill(buffer, buffer+m_size-12, 0x00);
        buffer[11] = 1;
        buffer[83] = 3;
        SeqTsHeader seqTs;
        Ptr<Packet> p = Create<Packet> (buffer, m_size-(8+4));
        p->AddHeader (seqTs);
         if ((m_socket->Send (p)) >= 0) {
                  NS_LOG_INFO ("TraceDelay TX " << m_size << " bytes to "
                                    << peerAddressStringStream.str () << " Uid: "
                                    << p->GetUid () << " Time: "
                                    << (Simulator::Now ()).GetSeconds ());
         } else {
           NS_LOG_INFO("Error sending packet.");
         }
      }
  
void P2PClient::HandleRead (Ptr<Socket> socket) {
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    { 
      if (packet->GetSize () > 0)
        {
          SeqTsHeader seqTs;
          packet->RemoveHeader (seqTs);
          uint32_t currentSequenceNumber = seqTs.GetSeq ();
           int size = packet->GetSize();
          uint8_t *buffer = new uint8_t[size];
                std::fill(buffer, buffer+size, 0x00);
          packet->CopyData(buffer, size);
          NS_LOG_INFO ("TraceDelay: RX " << packet->GetSize () <<
                          " bytes from "<< InetSocketAddress::ConvertFrom (from).GetIpv4 () <<
                           " Sequence Number: " << currentSequenceNumber <<
                           " Uid: " << packet->GetUid () <<
                           " Packet: " << buffer <<
                           " TXtime: " << seqTs.GetTs () <<
                           " RXtime: " << Simulator::Now () <<
                           " Delay: " << Simulator::Now () - seqTs.GetTs ());
          
          // m_lossCounter.NotifyReceived (currentSequenceNumber);
          //look into why we'd have it and why it doesn't work :(
          m_received++;
          Ptr<Packet> p;
          std::stringstream peerAddressStringStream;
          std::string s;
          switch(buffer[0]) {
          case(0):
            buffer[83] = 2;
                buffer[11] = 1;
                p = Create<Packet> (buffer, m_size-(8+4));
                p->AddHeader (seqTs);
                if ((m_socket->Send (p)) >= 0) {
                  NS_LOG_INFO ("TraceDelay TX " << m_size << " bytes to "
                                    << peerAddressStringStream.str () << " Uid: "
                                    << p->GetUid () << " Time: "
                                    << (Simulator::Now ()).GetSeconds ());

                }
                else {
                  NS_LOG_INFO ("Error while sending " << m_size << " bytes to "
                                          << peerAddressStringStream.str ());
                }
                NS_LOG_INFO("Dealt with connect request");
                break;
          case(1):
            NS_LOG_INFO("Received announce response");
            s = UpdatePeers(std::string((char*) buffer+1));
            SetupTCPConnections(s);
            break;
          default:
            NS_LOG_INFO("how");
          }

          } else {
                NS_LOG_INFO("what the fuck");
      }
    }
 }
}
