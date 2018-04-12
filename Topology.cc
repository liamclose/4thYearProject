#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/peer-to-peer-helper.h"
#include "ns3/ipv4.h"
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <map>
#include <tuple>
#include <cstdlib>
#include <limits>
#include <cstdint>
#include <arpa/inet.h>


using std::cerr;
using std::endl;
using std::ofstream;

using namespace ns3;

typedef std::map<std::string, std::vector<std::tuple<std::string, std::string, std::string>>> MapType;
  MapType AccessLogData;
int baseEpoch = std::numeric_limits<std::int32_t>::max();
int cacheTime = 0;

void printList(const std::list<std::string>& s) {
  std::list<std::string>::const_iterator i;
  for( i = s.begin(); i != s.end(); ++i)
    std::cout << *i << " ";
  std::cout << endl;
}

void setIps(Address serverAddress, uint16_t serverPort, MapType data, NetDeviceContainer devices) {
  MapType::iterator itNodes;
  int counter = 0;
  for (itNodes = data.begin(); itNodes != data.end(); itNodes++){  
    P2PClientHelper client (serverAddress, serverPort);
    std::vector<std::string> messages;
    std::vector<std::string> events;
    uint32_t maxPacketCount;
    std::string Ip = itNodes->first;
    for (uint i = 0; i<itNodes->second.size(); i++) {
      //optimize?
      std::string filePath = std::get<0>(itNodes->second.at(i));
      std::string timeStamp = std::get<1>(itNodes->second.at(i));
      std::string epochTime = std::get<2>(itNodes->second.at(i));
      std::string segment = filePath + timeStamp;
      messages.push_back(segment);
      events.push_back(epochTime);
    }
     Ptr<Node> node;
     Ptr<Ipv4> ipv4;
     Ipv4InterfaceAddress addr;
     Ipv4Address addressIp;
     const char * IPAddress;
     IPAddress = Ip.c_str();
     Ptr<NetDevice> device =  devices.Get(counter);
     node = device->GetNode();
     ipv4 = node->GetObject<Ipv4>(); // Get Ipv4 instance of the node
     int32_t interface = ipv4->GetInterfaceForDevice (device);
     if (interface == -1) {
       interface = ipv4->AddInterface (device);
     }
     Ipv4InterfaceAddress ipv4Addr;     
     ipv4Addr = Ipv4InterfaceAddress (Ipv4Address(IPAddress), Ipv4Mask ("/12"));         
     ipv4->AddAddress (interface, ipv4Addr);
     ipv4->SetMetric (interface, 1);
     ipv4->SetUp (interface);
     
     std::cout << "Address was set to "<< Ipv4Address(IPAddress) << " \n";
     std::cout << "Local Address "<< ipv4Addr << " \n";
     
     maxPacketCount = messages.size();
 
     ApplicationContainer apps;
     client.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
     if(cacheTime>0){
    client.SetAttribute ("CacheTime", UintegerValue (cacheTime));
     }     
     int minTime = 1999999;
     std::vector<std::string>::iterator itEvents = events.begin();
     std::vector<std::string> eventTimes;
     for (; itEvents != events.end(); itEvents++) {
       int eventTime = atoi((*itEvents).c_str())-baseEpoch;
       if (minTime>eventTime) {
     minTime = eventTime;
       }
       eventTimes.push_back(std::to_string(eventTime));
     }
     apps = client.Install (node, messages, eventTimes);
     apps.Start(MilliSeconds(minTime-10));
     apps.Stop(Seconds(300000));
     counter++;
     std::cout << "All Ips set app starting at: " << minTime <<"\n";
  }
}

int
main (int argc, char *argv[])
{
  std::string centralSpeed = "1Gbps";
  std::string outerSpeed = "100Mbps";  
  std::string dataFile = "10NodeSampleData.txt";
  //std::string fileSize = ;  Needed?

  CommandLine cmd;
  cmd.AddValue("centralSpeed", "Set the speed of the central network (default value: 1Gbps)", centralSpeed);
  cmd.AddValue("outerSpeed", "Set the speed of the outer network (default value: 100Mbps)", outerSpeed);
  cmd.AddValue("cacheTime", "Set the amount of time data is stored in the cache (default value: 100 seconds)", cacheTime);
  cmd.AddValue("dataFile", "Title of the data file (default value: 10NodeSampleData.txt)", dataFile);
  cmd.Parse (argc, argv);

  ns3::PacketMetadata::Enable ();

  std::ifstream infile(dataFile);
  //std::ifstream infile("cleanData1.txt");
  std::string line;
  std::vector<std::tuple<std::string, std::string, std::string>> bar;
  std::tuple<std::string, std::string, std::string> foo;

  while (std::getline(infile, line))
  {    
    std::istringstream iss(line);
    std::vector<std::string> data;
    while(iss.good()){
       std::string substr;
       getline( iss, substr, ',' );
       data.push_back(substr);
    }
    //std::cout << "Data 1: " << data.at(0) << endl;
    //std::cout << "Data 2: " << data.at(1) << endl;
    //std::cout << "Data 3: " << data.at(2) << endl;
    //std::cout << "Data 4: " << data.at(3) << endl;
    
    if (AccessLogData.count(data.at(0))>0) {
      std::cout << data.at(0) <<"==================================\n";
      bar = AccessLogData.at(data.at(0));
      AccessLogData.erase(data.at(0));
    } else {
      bar = std::vector<std::tuple<std::string, std::string, std::string>>();
      std::cout << data.at(0) <<"\n";
    } if (data.size()>3) {
      foo = make_tuple(data.at(1), data.at(2), data.at(3));    
      bar.push_back(foo);
      AccessLogData.insert(MapType::value_type(data.at(0), bar));
    } else {
      std::cout << "Invalid data, skipping.";
    }
  }

  Time::SetResolution (Time::NS);
  //LogComponentEnable ("P2PClient", LOG_LEVEL_INFO);
  LogComponentEnable ("P2PServer", LOG_LEVEL_INFO);
   //LogComponentEnable ("UdpSocketImpl", LOG_LEVEL_INFO);
 
  MapType TopLeftData;
  MapType TopRightData;
  MapType BottomLeftData;
  MapType BottomRightData;
  MapType ServerData;
 
  //For printing/testing values
  std::list<std::string> TopLeftIps;
  std::list<int> TopLeftEpochs;  
  std::list<std::string> TopRightIps;
  std::list<int> TopRightEpochs;
  std::list<std::string> BottomLeftIps;
  std::list<int> BottomLeftEpochs;
  std::list<std::string> BottomRightIps;
  std::list<int> BottomRightEpochs;
  std::list<std::string> ServerIps;
  std::list<int> ServerEpochs;
  //For testing Epoch Values
  std::list<std::string> TopLeftEpochsStrings;
  std::list<std::string> BottomRightEpochsStrings;
 
  MapType::iterator it;
  std::vector<std::tuple<std::string, std::string, std::string>> dataVector;
  std::cout << AccessLogData.size();
   for (it = AccessLogData.begin(); it != AccessLogData.end(); it++){
    std::string parsedAddress = it->first;
    dataVector = it->second;
    std::vector<std::tuple<std::string, std::string, std::string>>::iterator vIt = dataVector.begin();
    std::string epochTimeStr;
    for (; vIt!=dataVector.end(); vIt++) {
      epochTimeStr = std::get<2>((*vIt));
      // std::cout << "New Epoch is " << atoi(epochTimeStr.c_str()) << " ";
      // std::cout << "Current Epoch is " << baseEpoch << " ";
      bool isLess = atoi(epochTimeStr.c_str()) < baseEpoch;
      if(isLess) {
    baseEpoch = atoi(epochTimeStr.c_str());
      //std::cout << "Base Epoch set to " << baseEpoch << "Other time: " << epochTimeStr <<"\n";
      } else {
      //std::cout << baseEpoch << " " <<epochTimeStr << "\n";
      }
      std::cout << "Base: " << baseEpoch << " New: " << epochTimeStr << " Diff: " << (baseEpoch - atoi(epochTimeStr.c_str())) << "\n";
    }

    std::istringstream iss(parsedAddress);
    std::vector<std::string> partBofIp;
    while(iss.good()){
       std::string substr;
       getline( iss, substr, '.' );
       partBofIp.push_back(substr);
    }
    std::string B = partBofIp.at(1);

    if(atoi(B.c_str()) < 16 && atoi(B.c_str())>0){      
          TopLeftData.insert(MapType::value_type(parsedAddress, dataVector));
          TopLeftIps.push_back(parsedAddress);
          TopLeftEpochsStrings.push_back(epochTimeStr);
          TopLeftEpochs.push_back(atoi(epochTimeStr.c_str()));
      }
      else if(atoi(B.c_str()) >= 16  && atoi(B.c_str()) < 32 ){
          
          TopRightData.insert(MapType::value_type(parsedAddress, dataVector));
          TopRightIps.push_back(parsedAddress);      
          TopRightEpochs.push_back(atoi(epochTimeStr.c_str()));
      }
      else if(atoi(B.c_str()) >= 32  && atoi(B.c_str()) < 48 ){
          BottomLeftData.insert(MapType::value_type(parsedAddress, dataVector));
          BottomLeftIps.push_back(parsedAddress);
          BottomLeftEpochs.push_back(atoi(epochTimeStr.c_str()));
      }
      else if (atoi(B.c_str()) >= 48  && atoi(B.c_str()) < 64){
        BottomRightIps.push_back(parsedAddress);
        BottomRightData.insert(MapType::value_type(parsedAddress, dataVector));
        BottomRightEpochsStrings.push_back(epochTimeStr);
        BottomRightEpochs.push_back(atoi(epochTimeStr.c_str()));
      } else {
    ServerIps.push_back(parsedAddress);
    ServerData.insert(MapType::value_type(parsedAddress, dataVector));
    ServerEpochs.push_back(atoi(epochTimeStr.c_str()));
    
      }
   }
   baseEpoch =baseEpoch - 15;

   //Create Nodes

  //5 for the central server and 5 branches
  NodeContainer csmaNodesServer;
  csmaNodesServer.Create (5+ServerData.size());


  NodeContainer csmaNodesTopLeft;
  csmaNodesTopLeft.Add(csmaNodesServer.Get(1));
  csmaNodesTopLeft.Create (TopLeftData.size());
 
  NodeContainer csmaNodesTopRight;
  csmaNodesTopRight.Add(csmaNodesServer.Get(2));
  csmaNodesTopRight.Create (TopRightData.size());

  NodeContainer csmaNodesBottomLeft;
  csmaNodesBottomLeft.Add(csmaNodesServer.Get(3));
  csmaNodesBottomLeft.Create (BottomLeftData.size());

  NodeContainer csmaNodesBottomRight;
  csmaNodesBottomRight.Add(csmaNodesServer.Get(4));
  csmaNodesBottomRight.Create (BottomRightData.size());

  NodeContainer nonServerNodes;
  NodeContainer::Iterator iter;
  iter = csmaNodesServer.Begin();
  iter++;
  iter++;
  iter++;
  iter++;
  iter++;
  for (; iter != csmaNodesServer.End (); iter++) {
    //so that we can install the internet stack on the nodes that only appear in the centre
    nonServerNodes.Add(*iter);
  }

  //Create csma helpers
  CsmaHelper csmaServer;
  csmaServer.SetChannelAttribute ("DataRate", StringValue (centralSpeed));
  csmaServer.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  CsmaHelper csmaTopLeft;
  csmaTopLeft.SetChannelAttribute ("DataRate", StringValue (outerSpeed));
  csmaTopLeft.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  CsmaHelper csmaTopRight;
  csmaTopRight.SetChannelAttribute ("DataRate", StringValue (outerSpeed));
  csmaTopRight.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  CsmaHelper csmaBottomLeft;
  csmaBottomLeft.SetChannelAttribute ("DataRate", StringValue (outerSpeed));
  csmaBottomLeft.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  CsmaHelper csmaBottomRight;
  csmaBottomRight.SetChannelAttribute ("DataRate", StringValue (outerSpeed));
  csmaBottomRight.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  //Create Hardware

  NetDeviceContainer csmaDevicesServer;
  csmaDevicesServer = csmaServer.Install(csmaNodesServer);

  NetDeviceContainer csmaDevicesTopLeft;
  csmaDevicesTopLeft = csmaTopLeft.Install(csmaNodesTopLeft);

  NetDeviceContainer csmaDevicesTopRight;
  csmaDevicesTopRight = csmaTopRight.Install(csmaNodesTopRight);

  NetDeviceContainer csmaDevicesBottomLeft;
  csmaDevicesBottomLeft = csmaBottomLeft.Install(csmaNodesBottomLeft);

  NetDeviceContainer csmaDevicesBottomRight;
  csmaDevicesBottomRight = csmaBottomRight.Install(csmaNodesBottomRight);

 
  //Install the internet
  InternetStackHelper stack;  
  Ipv4StaticRoutingHelper staticRoutingHelper;
  stack.SetRoutingHelper (staticRoutingHelper);
  stack.Install(csmaNodesServer.Get(0));
  stack.Install(nonServerNodes);
  stack.Install(csmaNodesTopLeft);
  stack.Install(csmaNodesTopRight);
  stack.Install(csmaNodesBottomLeft);
  stack.Install(csmaNodesBottomRight);
 
  //Assign IP Addresses
  Ipv4AddressHelper address;
  address.SetBase ("10.64.0.0", "255.192.0.0");
  Ipv4InterfaceContainer csmaInterfacesServer;
  csmaInterfacesServer = address.Assign(csmaDevicesServer);


  NetDeviceContainer nonServerDevices;
  NetDeviceContainer::Iterator iterDev;
  iterDev = csmaDevicesServer.Begin();
  iterDev++; //Can we maybe make this a +5?
  iterDev++;
  iterDev++;
  iterDev++;
  iterDev++;
  for (; iterDev != csmaDevicesServer.End (); iterDev++) {
    //need this for setting the IPs of the nodes that talk directly to the server
    nonServerDevices.Add(*iterDev);
  }

  //Now the dynamic ones
    Address serverAddress = Address(csmaInterfacesServer.GetAddress(0));
    uint16_t serverPort = 4000;
    setIps(serverAddress, serverPort, ServerData, nonServerDevices);
    for (iterDev = nonServerDevices.Begin(); iterDev!=nonServerDevices.End();iterDev++) {    
     Ptr<Node> node;
     Ptr<Ipv4> ipv4;
     node = (*iterDev)->GetNode();
     ipv4 = node->GetObject<Ipv4>(); // Get Ipv4 instance of the node
     ipv4->RemoveAddress(1,0);
    }
    setIps(serverAddress, serverPort, TopLeftData, csmaDevicesTopLeft);

    address.SetBase("10.0.0.0", "255.240.0.0");
    Ipv4InterfaceContainer csmaInterfaceTopLeft;
    csmaInterfaceTopLeft = address.Assign(csmaDevicesTopLeft);
    
    Ptr<Ipv4StaticRouting> staticRoutingServer;
    staticRoutingServer = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> (csmaNodesServer.Get(0)->GetObject<Ipv4> ()->GetRoutingProtocol ());
    staticRoutingServer->AddNetworkRouteTo(Ipv4Address("10.0.0.0") ,Ipv4Mask("/12"),Ipv4Address("10.64.0.2"), 1,0 );
    staticRoutingServer->AddNetworkRouteTo(Ipv4Address("10.16.0.0") ,Ipv4Mask("/12"),Ipv4Address("10.64.0.3"), 1,0 );
    staticRoutingServer->AddNetworkRouteTo(Ipv4Address("10.32.0.0") ,Ipv4Mask("/12"),Ipv4Address("10.64.0.4"), 1,0 );  
    staticRoutingServer->AddNetworkRouteTo(Ipv4Address("10.48.0.0") ,Ipv4Mask("/12"),Ipv4Address("10.64.0.5"), 1,0 );
         
    for (iter = csmaNodesTopLeft.Begin (); iter != csmaNodesTopLeft.End (); iter++)
    {
      Ptr<Ipv4StaticRouting> staticRouting;
      Ptr<Ipv4> ipv4 = (*iter)->GetObject<Ipv4>();
      staticRouting = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> (ipv4->GetRoutingProtocol ());
      staticRouting->SetDefaultRoute ("10.0.0.1", 1 );
    }
    
    setIps(serverAddress, serverPort, TopRightData, csmaDevicesTopRight);
    address.SetBase("10.16.0.0", "255.240.0.0");
    Ipv4InterfaceContainer csmaInterfaceTopRight;
    csmaInterfaceTopRight= address.Assign(csmaDevicesTopRight);

    for (iter = csmaNodesTopRight.Begin (); iter != csmaNodesTopRight.End (); iter++)
    {
      Ptr<Ipv4StaticRouting> staticRouting;
      Ptr<Ipv4> ipv4 = (*iter)->GetObject<Ipv4>();
      staticRouting = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> ((*iter)->GetObject<Ipv4> ()->GetRoutingProtocol ());
      int n = ipv4->GetNInterfaces();
      Ipv4Address loc = ipv4->GetAddress(n-1,0).GetLocal();
      staticRouting->AddHostRouteTo (loc, n-1, 0);
      staticRouting->SetDefaultRoute ("10.16.0.1", 1 );
    }

    setIps(serverAddress, serverPort, BottomLeftData, csmaDevicesBottomLeft);
    address.SetBase("10.32.0.0", "255.240.0.0");
    Ipv4InterfaceContainer csmaInterfaceBottomLeft;
    csmaInterfaceBottomLeft = address.Assign(csmaDevicesBottomLeft);

    for (iter = csmaNodesBottomLeft.Begin (); iter != csmaNodesBottomLeft.End (); iter++)
    {
      Ptr<Ipv4StaticRouting> staticRouting;
      Ptr<Ipv4> ipv4 = (*iter)->GetObject<Ipv4>();
      staticRouting = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> ((*iter)->GetObject<Ipv4> ()->GetRoutingProtocol ());
      int n = ipv4->GetNInterfaces();
      Ipv4Address loc = ipv4->GetAddress(n-1,0).GetLocal();
      staticRouting->AddHostRouteTo (loc, n-1, 0);
      staticRouting->SetDefaultRoute ("10.32.0.1", 1 );
    }

    Ptr<Ipv4StaticRouting> staticRouting;
    setIps(serverAddress, serverPort, BottomRightData, csmaDevicesBottomRight);
    address.SetBase("10.48.0.0", "255.240.0.0");
    Ipv4InterfaceContainer csmaInterfaceBottomRight;
    csmaInterfaceBottomRight = address.Assign(csmaDevicesBottomRight);
    for (iter = csmaNodesBottomRight.Begin (); iter != csmaNodesBottomRight.End (); iter++)
    {
      Ptr<Ipv4StaticRouting> staticRouting;
            Ptr<Ipv4> ipv4 = (*iter)->GetObject<Ipv4>();
      staticRouting = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> ((*iter)->GetObject<Ipv4> ()->GetRoutingProtocol ());
      int n = ipv4->GetNInterfaces();
      Ipv4Address loc = ipv4->GetAddress(n-1,0).GetLocal();
      staticRouting->AddHostRouteTo (loc, n-1, 0);
      staticRouting->SetDefaultRoute ("10.48.0.1", 1 );

    }
    
    P2PServerHelper server(serverPort);
    ApplicationContainer apps = server.Install(csmaNodesServer.Get(0));

    P2PClientHelper server2 (serverAddress, serverPort);
    server2.SetAttribute ("MaxPackets",  UintegerValue (0));//(maxPacketCount));
    server2.SetAttribute ("Mode", UintegerValue(1));
    apps = server2.Install (csmaNodesServer.Get(0), std::vector<std::string>(), std::vector<std::string>());

    apps.Start (Seconds (0.0));
    apps.Stop (Seconds (300000));
        Simulator::Schedule (Seconds (299999), &P2PClient::PrintDataServed,csmaNodesServer.Get(0)->GetApplication(1)->GetObject<P2PClient>(), AccessLogData.size());
   Ipv4GlobalRoutingHelper::PopulateRoutingTables();

   Ptr<OutputStreamWrapper> routingStream2 = Create<OutputStreamWrapper>("dynamic-global-routing2.routes", std::ios::out);
  // staticRoutingServer->PrintRoutingTable(routingStream2);
   Ipv4RoutingHelper::PrintRoutingTableAllAt(Time(Seconds(9.0)), routingStream2);
   AsciiTraceHelper ascii;
   csmaServer.EnableAsciiAll (ascii.CreateFileStream ("myfirst.tr"));
   csmaTopLeft.EnableAsciiAll (ascii.CreateFileStream ("topleft.tr"));
   //NS_LOG_INFO ("Run Simulation.");
   Simulator::Run ();
   Simulator::Destroy ();   
}
