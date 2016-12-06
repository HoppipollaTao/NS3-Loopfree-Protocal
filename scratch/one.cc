#include <iostream>
#include <string>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/csma-module.h"

using namespace std;
using namespace ns3;

NS_LOG_COMPONENT_DEFINE("two");
static inline std::string
PrintReceivedPacket (Address& from)
{
  InetSocketAddress iaddr = InetSocketAddress::ConvertFrom (from);

  std::ostringstream oss;
  oss << "--\nReceived one packet! Socket: " << iaddr.GetIpv4 ()
      << " port: " << iaddr.GetPort ()
      << " at time = " << Simulator::Now ().GetSeconds ()
      << "\n--";

  return oss.str ();
}
//回调函数
static void recvCallback(Ptr<Socket> sock)
{
	Address from;
//	Ptr<Packet> packet = sock->Recv();
	 Ptr<Packet> packet = sock->RecvFrom (from);
	NS_LOG_UNCOND (PrintReceivedPacket (from));

	Ptr<Packet> q = packet->Copy();


	    PacketMetadata::ItemIterator metadataIterator = q->BeginItem();
	    PacketMetadata::Item item;
	 //   NS_LOG_UNCOND ("yeah!!! i = " << metadataIterator.HasNext());
	    while (metadataIterator.HasNext())
	      {
	        item = metadataIterator.Next();
	        NS_LOG_FUNCTION("item name: " << item.tid.GetName());

	        // If we want to have an ip header
	        if(item.tid.GetName() == "ns3::Ipv4Header")
	        {
	          Callback<ObjectBase *> constr = item.tid.GetConstructor();
	          NS_ASSERT(!constr.IsNull());

	          // Ptr<> and DynamicCast<> won't work here as all headers are from ObjectBase, not Object
	          ObjectBase *instance = constr();
	          NS_ASSERT(instance != 0);

	          Ipv4Header* ipv4Header = dynamic_cast<Ipv4Header*> (instance);
	          NS_ASSERT(ipv4Header != 0);

	          ipv4Header->Deserialize(item.current);
	          int i = ipv4Header->GetTtl();

	          // The ipv4Header can now obtain the source of the packet
	 //         src_ip = ipv4Header->GetSource();
	          NS_LOG_UNCOND ("yeah!!! i = " << i);
	          // Finished, clear the ip header and go back
	          delete ipv4Header;
	          break;
	        }

	    }
	    NS_LOG_UNCOND (PrintReceivedPacket (from));
	    //cout << "size:" << packet->GetSize() << endl;
}

int main(int argc, char *argv[])
{
	ns3::PacketMetadata::Enable ();
	NodeContainer nodes;
	nodes.Create(5);

	InternetStackHelper stack;
	stack.Install(nodes);

	CsmaHelper csmaHelper;
	NetDeviceContainer cmsaNetDevice = csmaHelper.Install(nodes);

	Ipv4AddressHelper addressHelper;
	addressHelper.SetBase("192.168.1.0", "255.255.255.0");
	Ipv4InterfaceContainer interfaces = addressHelper.Assign(cmsaNetDevice);

	//server sockets
	TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
	Ptr<Socket> server = Socket::CreateSocket(nodes.Get(0), tid);
	InetSocketAddress addr = InetSocketAddress(Ipv4Address::GetAny(), 10);
	server->Bind(addr);

	server->SetRecvCallback(MakeCallback(&recvCallback)); //设置回调函数

	//client sockets
	Ptr<Socket> client = Socket::CreateSocket(nodes.Get(4), tid);
	InetSocketAddress serverAddr = InetSocketAddress(interfaces.GetAddress(0), 10);
	client->Connect(serverAddr);
	//new
	Ptr<Packet> packet = Create<Packet> (500);
	// 添加IP头
	ns3::Ipv4Header iph;
	iph.SetTtl(32);
	  //再接下来就是将TCP头加到空的数据包的前面：
	packet->AddHeader(iph);
	client->Send(packet);
	client->Close();
	csmaHelper.EnablePcap("two", nodes);

	Simulator::Run();
	Simulator::Destroy();


	return 0;
}

