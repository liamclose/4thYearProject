/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef PEER_TO_PEER_HELPER_H
#define PEER_TO_PEER_HELPER_H

#include "ns3/peer-to-peer.h"
#include "ns3/peer-to-peer-server.h"
#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include <stdint.h>
#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"
#include "ns3/udp-server.h"
#include "ns3/udp-client.h"
#include "ns3/log.h"

namespace ns3 {

class P2PClientHelper {

  public:
    P2PClientHelper();
    P2PClientHelper(Address address, uint16_t port);
    P2PClientHelper(Address address);
    void SetAttribute(std::string name, const AttributeValue &value);
  ApplicationContainer Install (NodeContainer c, std::vector<std::string> messages, std::vector<std::string> events);


  private:
    ObjectFactory m_factory;
};

class P2PServerHelper
{
public:
  /**
   * Create UdpServerHelper which will make life easier for people trying
   * to set up simulations with udp-client-server application.
   *
   */
  P2PServerHelper ();

  /**
   * Create UdpServerHelper which will make life easier for people trying
   * to set up simulations with udp-client-server application.
   *
   * \param port The port the server will wait on for incoming packets
   */
  P2PServerHelper (uint16_t port);

  /**
   * Record an attribute to be set in each Application after it is is created.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);

  /**
   * Create one UDP server application on each of the Nodes in the
   * NodeContainer.
   *
   * \param c The nodes on which to create the Applications.  The nodes
   *          are specified by a NodeContainer.
   * \returns The applications created, one Application per Node in the
   *          NodeContainer.
   */
  ApplicationContainer Install (NodeContainer c);

  /**
   * \brief Return the last created server.
   *
   * This function is mainly used for testing.
   *
   * \returns a Ptr to the last created server application
   */
  Ptr<P2PServer> GetServer (void);
private:
  ObjectFactory m_factory; //!< Object factory.
  Ptr<P2PServer> m_server; //!< The last created server application
};

}

#endif /* PEER_TO_PEER_HELPER_H */

