/*
 * 
 * this is the architecture
 * n0-----r0-----r1-----n1
 *    L0     L1     L2
 * L0 -> 8Mbps
 * L1 -> 1 to 10 Mbps
 * L2 -> 8Mbps
 * n0 client, n1 server
 * 
 */

#include "ns3/netanim-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("PointToPoint");

int main (int argc, char *argv[])
{
	Time::SetResolution (Time::NS);
	LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
	LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

	NodeContainer nodes;
	nodes.Create (4);
	
	InternetStackHelper stack;
	stack.Install (nodes);
	
	PointToPointHelper p0p1;
	p0p1.SetDeviceAttribute ("DataRate", StringValue ("8Mbps"));
	p0p1.SetChannelAttribute ("Delay", StringValue ("1ms"));

    PointToPointHelper p1p2;
	p1p2.SetDeviceAttribute ("DataRate", StringValue ("8Mbps"));
	p1p2.SetChannelAttribute ("Delay", StringValue ("5ms"));

	PointToPointHelper p2p3;
	p2p3.SetDeviceAttribute ("DataRate", StringValue ("8Mbps"));
	p2p3.SetChannelAttribute ("Delay", StringValue ("5ms"));

	Ipv4AddressHelper address;
	address.SetBase ("10.1.1.0", "255.255.255.0"); 
	NetDeviceContainer devices;
	devices = p0p1.Install (nodes.Get (0), nodes.Get (1));
	Ipv4InterfaceContainer interfaces = address.Assign (devices);
	
	devices = p1p2.Install (nodes.Get (1), nodes.Get (2));
	address.SetBase ("10.1.2.0", "255.255.255.0"); 
	interfaces = address.Assign (devices);

    devices = p2p3.Install (nodes.Get (2), nodes.Get (3));
	address.SetBase ("10.1.3.0", "255.255.255.0"); 
	interfaces = address.Assign (devices);
	
	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  
	UdpEchoServerHelper echoServer (9);

	ApplicationContainer serverApps = echoServer.Install (nodes.Get (3));
	serverApps.Start (Seconds (1.0));
	serverApps.Stop (Seconds (10.0));

	UdpEchoClientHelper echoClient (interfaces.GetAddress (1), 9);
	echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
	echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
	echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

	ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
	clientApps.Start (Seconds (2.0));
	clientApps.Stop (Seconds (10.0));

	AnimationInterface anim ("ppp.xml");
	anim.SetConstantPosition (nodes.Get(0), 0.0, 0.0);
    anim.SetConstantPosition (nodes.Get(1), 2.0, 2.0);
    anim.SetConstantPosition (nodes.Get(2), 3.0, 3.0);
    anim.SetConstantPosition (nodes.Get(2), 4.0, 4.0);

	Simulator::Run ();
	Simulator::Destroy ();
	
	return 0;
}