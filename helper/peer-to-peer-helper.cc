/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "peer-to-peer-helper.h"
//#include "ns3/peer-to-peer-server.h"
#include "ns3/peer-to-peer.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("P2PClientHelper");

P2PClientHelper::P2PClientHelper ()
{
  m_factory.SetTypeId (P2PClient::GetTypeId ());
}

P2PClientHelper::P2PClientHelper (Address address, uint16_t port)
{
  m_factory.SetTypeId (P2PClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("RemotePort", UintegerValue (port));
}

P2PClientHelper::P2PClientHelper (Address address)
{
  m_factory.SetTypeId (P2PClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
}

void P2PClientHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer P2PClientHelper::Install (NodeContainer c, std::vector<std::string> messages)
{
  ApplicationContainer apps;
  NS_LOG_INFO("installing p2p");
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
  NS_LOG_INFO(*i);
      Ptr<Node> node = *i;
      Ptr<P2PClient> client = m_factory.Create<P2PClient> ();
      (*client).SetMessages(messages);
  NS_LOG_INFO(client);
      node->AddApplication (client);
      apps.Add (client);
    }
  return apps;
}

//server

P2PServerHelper::P2PServerHelper ()
{
  m_factory.SetTypeId (P2PServer::GetTypeId ());
}

P2PServerHelper::P2PServerHelper (uint16_t port)
{
  m_factory.SetTypeId (P2PServer::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));
}

void
P2PServerHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
P2PServerHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;

      m_server = m_factory.Create<P2PServer> ();
      node->AddApplication (m_server);
      apps.Add (m_server);

    }
  return apps;
}

Ptr<P2PServer>
P2PServerHelper::GetServer (void)
{
  return m_server;
}
}

