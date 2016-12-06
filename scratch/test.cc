/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 */

/* Test program for multi-interface host, static routing

         Destination host (192.168.1.1)
                 |
                 | 10.20.1.0/24
              DSTRTR 192.168.254.0
  10.10.1.0/24 /   \  10.10.2.0/24
              / 10.5.1.0/24
192.168.252.0Rtr1----Rtr2 192.168.253.0
 10.1.1.0/24 |      | 10.1.2.0/24
             |      /
              \    /
             Source(192.168.251.0)
                |10.1.0.0/24
                |
              source host(192.168.251.3)
*/

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SocketBoundRoutingExample");


int 
main (int argc, char *argv[])
{
	ns3::PacketMetadata::Enable ();
  // Allow the user to override any of the defaults and the above
  // DefaultValue::Bind ()s at run-time, via command-line arguments
  CommandLine cmd;
  cmd.Parse (argc, argv);

  Ptr<Node> nSrc = CreateObject<Node> ();
  Ptr<Node> nDst = CreateObject<Node> ();
  Ptr<Node> nRtr1 = CreateObject<Node> ();
  Ptr<Node> nRtr2 = CreateObject<Node> ();
  Ptr<Node> nDstRtr = CreateObject<Node> ();
  Ptr<Node> nSrchost= CreateObject<Node>();

  NodeContainer c = NodeContainer (nSrc, nDst, nRtr1, nRtr2, nDstRtr);
  NodeContainer d = NodeContainer (nSrchost);

  InternetStackHelper internet;
  internet.Install (c);
  internet.Install (d);
  // Point-to-point links
  NodeContainer nSrcnRtr1 = NodeContainer (nSrc, nRtr1);
  NodeContainer nSrcnRtr2 = NodeContainer (nSrc, nRtr2);
  NodeContainer nRtr1nDstRtr = NodeContainer (nRtr1, nDstRtr);
  NodeContainer nRtr2nDstRtr = NodeContainer (nRtr2, nDstRtr);
  NodeContainer nRtr1nRtr2 = NodeContainer (nRtr1, nRtr2);
  NodeContainer nDstRtrnDst = NodeContainer (nDstRtr, nDst);
  NodeContainer nSrchostnSrc=NodeContainer(nSrchost,nSrc);

  // We create the channels first without any IP addressing information
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer dSrcdRtr1 = p2p.Install (nSrcnRtr1);
  NetDeviceContainer dSrcdRtr2 = p2p.Install (nSrcnRtr2);
  NetDeviceContainer dRtr1dDstRtr = p2p.Install (nRtr1nDstRtr);
  NetDeviceContainer dRtr2dDstRtr = p2p.Install (nRtr2nDstRtr);
  NetDeviceContainer dRtr1dRtr2=p2p.Install(nRtr1nRtr2);
  NetDeviceContainer dDstRtrdDst = p2p.Install (nDstRtrnDst);
  NetDeviceContainer dSrchostdSrc=p2p.Install(nSrchostnSrc);

  Ptr<PointToPointNetDevice> deviceA = CreateObject<PointToPointNetDevice> ();
   deviceA->SetAddress (Mac48Address::Allocate ());
   nSrc->AddDevice (deviceA);

   Ptr<PointToPointNetDevice> deviceC = CreateObject<PointToPointNetDevice> ();
    deviceC->SetAddress (Mac48Address::Allocate ());
    nDstRtr->AddDevice (deviceC);

  Ptr<PointToPointNetDevice> device1 = CreateObject<PointToPointNetDevice> ();
     device1->SetAddress (Mac48Address::Allocate ());
     nRtr1->AddDevice (device1);

  Ptr<PointToPointNetDevice> device2 = CreateObject<PointToPointNetDevice> ();
      device2->SetAddress (Mac48Address::Allocate ());
      nRtr2->AddDevice (device2);

  Ptr<PointToPointNetDevice> device3 = CreateObject<PointToPointNetDevice> ();
    device3->SetAddress (Mac48Address::Allocate ());
    nDstRtr->AddDevice (device3);

  Ptr<PointToPointNetDevice> device4=CreateObject<PointToPointNetDevice>();
    device4->SetAddress(Mac48Address::Allocate());
    nSrchost->AddDevice(device4);

  Ptr<NetDevice> SrcToRtr1=dSrcdRtr1.Get (0);
  Ptr<NetDevice> SrcToRtr2=dSrcdRtr2.Get (0);

  // Later, we add IP addresses.
  Ipv4AddressHelper ipv4;

  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer iSrciRtr1 = ipv4.Assign (dSrcdRtr1);
  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer iSrciRtr2 = ipv4.Assign (dSrcdRtr2);
  ipv4.SetBase ("10.10.1.0", "255.255.255.0");
  Ipv4InterfaceContainer iRtr1iDstRtr = ipv4.Assign (dRtr1dDstRtr);
  ipv4.SetBase ("10.10.2.0", "255.255.255.0");
  Ipv4InterfaceContainer iRtr2iDstRtr = ipv4.Assign (dRtr2dDstRtr);
  ipv4.SetBase("10.5.1.0","255.255.255.0");
  Ipv4InterfaceContainer iRtr1iRtr2= ipv4.Assign(dRtr1dRtr2);
  ipv4.SetBase ("10.20.1.0", "255.255.255.0");
  Ipv4InterfaceContainer iDstRtrDst = ipv4.Assign (dDstRtrdDst);
  ipv4.SetBase ("10.1.0.0","255.255.255.0");
  Ipv4InterfaceContainer iSrchostiSrc=ipv4.Assign(dSrchostdSrc);


  Ptr<Ipv4> ipv4Src = nSrc->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Rtr1 = nRtr1->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Rtr2 = nRtr2->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4DstRtr = nDstRtr->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Dst = nDst->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Srchost=nSrchost->GetObject<Ipv4>();

  int32_t ifIndexA = ipv4Src->AddInterface (deviceA);
  int32_t ifIndexC = ipv4Dst->AddInterface (deviceC);

  int32_t ifIndex1 = ipv4Rtr1->AddInterface (device1);
  int32_t ifIndex2 = ipv4Rtr2->AddInterface (device2);
  int32_t ifIndex3 = ipv4DstRtr->AddInterface (device3);
  int32_t ifIndex4 = ipv4Srchost->AddInterface(device4);


  Ipv4InterfaceAddress ifInAddrA = Ipv4InterfaceAddress (Ipv4Address ("192.168.251.0"), Ipv4Mask ("/24"));
  ipv4Src->AddAddress (ifIndexA, ifInAddrA);
  ipv4Src->SetMetric (ifIndexA, 1);
  ipv4Src->SetUp (ifIndexA);


  Ipv4InterfaceAddress ifInAddrC = Ipv4InterfaceAddress (Ipv4Address ("192.168.1.1"),Ipv4Mask("/32"));
  ipv4Dst->AddAddress (ifIndexC, ifInAddrC);
  ipv4Dst->SetMetric (ifIndexC, 1);
  ipv4Dst->SetUp (ifIndexC);

  Ipv4InterfaceAddress ifInAddr1 = Ipv4InterfaceAddress (Ipv4Address ("192.168.252.0"), Ipv4Mask ("/24"));
  ipv4Rtr1->AddAddress (ifIndex1, ifInAddr1);
  ipv4Rtr1->SetMetric (ifIndex1, 1);
  ipv4Rtr1->SetUp (ifIndex1);

  Ipv4InterfaceAddress ifInAddr2 = Ipv4InterfaceAddress (Ipv4Address ("192.168.253.0"), Ipv4Mask ("/24"));
  ipv4Rtr2->AddAddress (ifIndex2, ifInAddr2);
  ipv4Rtr2->SetMetric (ifIndex2, 1);
  ipv4Rtr2->SetUp (ifIndex2);

  Ipv4InterfaceAddress ifInAddr3 = Ipv4InterfaceAddress (Ipv4Address ("192.168.254.0"), Ipv4Mask ("/24"));
  ipv4DstRtr->AddAddress (ifIndex3, ifInAddr3);
  ipv4DstRtr->SetMetric (ifIndex3, 1);
  ipv4DstRtr->SetUp (ifIndex3);

  Ipv4InterfaceAddress ifInAddr4 = Ipv4InterfaceAddress (Ipv4Address ("192.168.251.3"), Ipv4Mask ("/32"));
    ipv4DstRtr->AddAddress (ifIndex4, ifInAddr4);
    ipv4DstRtr->SetMetric (ifIndex4, 1);
    ipv4DstRtr->SetUp (ifIndex4);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> staticRoutingSrc = ipv4RoutingHelper.GetStaticRouting (ipv4Src);
  Ptr<Ipv4StaticRouting> staticRoutingRtr1 = ipv4RoutingHelper.GetStaticRouting (ipv4Rtr1);
  Ptr<Ipv4StaticRouting> staticRoutingRtr2 = ipv4RoutingHelper.GetStaticRouting (ipv4Rtr2);
  Ptr<Ipv4StaticRouting> staticRoutingDstRtr = ipv4RoutingHelper.GetStaticRouting (ipv4DstRtr);
  Ptr<Ipv4StaticRouting> staticRoutingDst = ipv4RoutingHelper.GetStaticRouting (ipv4Dst);

  staticRoutingSrc->AddNeighborRoute(ifInAddrA,ifInAddr1);
  staticRoutingSrc->AddNeighborRoute(ifInAddrA,ifInAddr2);
  staticRoutingSrc->AddNeighborRoute(ifInAddrA,ifInAddr4);

  staticRoutingRtr1->AddNeighborRoute(ifInAddr1,ifInAddrA);
  staticRoutingRtr1->AddNeighborRoute(ifInAddr1,ifInAddr2);
  staticRoutingRtr1->AddNeighborRoute(ifInAddr1,ifInAddr3);

  staticRoutingRtr2-> AddNeighborRoute( ifInAddr2,ifInAddrA);
  staticRoutingRtr2-> AddNeighborRoute( ifInAddr2,ifInAddr1);
  staticRoutingRtr2-> AddNeighborRoute( ifInAddr2,ifInAddr3);

  staticRoutingDstRtr-> AddNeighborRoute(ifInAddr3,ifInAddr1);
  staticRoutingDstRtr-> AddNeighborRoute( ifInAddr3,ifInAddr2);
  staticRoutingDstRtr-> AddNeighborRoute( ifInAddr3,ifInAddrC);


  // Create static routes from Src to Dst
  //staticRoutingRtr1->AddHostRouteTo (Ipv4Address ("192.168.255.0"), Ipv4Address ("10.10.1.2"), 2,3);



  staticRoutingRtr1->AddHostRouteTo (Ipv4Address ("192.168.1.1"), Ipv4Address ("10.5.1.2"), 3,2);
  //staticRoutingRtr2->AddHostRouteTo (Ipv4Address ("192.168.1.1"), Ipv4Address ("10.10.2.2"), 2,2);
  staticRoutingRtr2->AddHostRouteTo (Ipv4Address ("192.168.1.1"), Ipv4Address ("10.5.1.1"), 3,2);

  // Two routes to same destination - setting separate metrics. 
  // You can switch these to see how traffic gets diverted via different routes
  staticRoutingSrc->AddHostRouteTo (Ipv4Address ("192.168.1.1"), Ipv4Address ("10.1.1.2"), 1,3);
  //staticRoutingSrc->AddHostRouteTo (Ipv4Address ("10.20.1.2"), Ipv4Address ("10.1.2.2"), 2,10);

  // Creating static routes from DST to Source pointing to Rtr1 VIA Rtr2(!)
  //staticRoutingDst->AddHostRouteTo (Ipv4Address ("192.168.251.3"), Ipv4Address ("10.10.2.1"), 1);
  //staticRoutingDstRtr->AddHostRouteTo (Ipv4Address ("192.168.1.1"), Ipv4Address ("10.20.1.2"),3,1);
  //staticRoutingRtr2->AddHostRouteTo (Ipv4Address ("192.168.251.3"), Ipv4Address ("10.1.2.1"), 1);

  Ptr<NetDevice> dev=ipv4Src->GetNetDevice(1);
  Ptr<Packet> p=Create<Packet>(100);
  Ipv4Header ipHeader;
  Ipv4Address m_dstAddress=Ipv4Address("192.168.1.1");
  Ipv4Address m_srcAddress=Ipv4Address("192.168.251.3");

  //Ipv4Address m_dstAddress=Ipv4Address("192.168.251.3");
  //Ipv4Address m_srcAddress=Ipv4Address("192.168.251.3");
  ipHeader.SetDestination(m_dstAddress);
  ipHeader.SetOriginalId(m_srcAddress);
  ipHeader.SetSource(m_srcAddress);
  ipHeader.SetPayloadSize(128);
  ipHeader.SetTtl(16);
  p->AddHeader(ipHeader);

  dev->Send(p,m_dstAddress,0x0800);

  AsciiTraceHelper ascii;
   p2p.EnableAsciiAll(ascii.CreateFileStream("test.tr"));
   p2p.EnablePcapAll("test", true);
  Simulator::Run();
  	Simulator::Destroy();

  return 0;
}

