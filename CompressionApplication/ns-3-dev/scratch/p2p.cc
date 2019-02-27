#include "ns3/netanim-module.h"
#include "ns3/core-module.h"
#include "ns3/global-route-manager.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/bridge-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("P2P");

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

int main(int argc, char *argv[])
{
	Time::SetResolution (Time::NS);
	LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
	LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  // LogComponentEnable ("PointToPointNetDevice", LOG_LEVEL_ALL);
  CommandLine cmd;
  cmd.Parse (argc, argv);

  /* Configuration. */

  /* Build nodes. */
  NodeContainer client;
  client.Create (1);
  NodeContainer router_0;
  router_0.Create (1);
  NodeContainer router_1;
  router_1.Create (1);
  NodeContainer server;
  server.Create (1);

  /* Build link. */
  PointToPointHelper p2p_client_r0;
  p2p_client_r0.SetDeviceAttribute ("DataRate", StringValue("8Mbps"));
  p2p_client_r0.SetChannelAttribute ("Delay", StringValue("1ms"));
  PointToPointHelper p2p_r0_r1;
  p2p_r0_r1.SetDeviceAttribute ("DataRate", StringValue("8Mbps"));
  p2p_r0_r1.SetChannelAttribute ("Delay", StringValue("1ms"));
  PointToPointHelper p2p_r1_server;
  p2p_r1_server.SetDeviceAttribute ("DataRate", StringValue("8Mbps"));
  p2p_r1_server.SetChannelAttribute ("Delay", StringValue("1ms"));

  /* Build link net device container. */
  NetDeviceContainer ndc_client_r0 = p2p_client_r0.Install (client.Get(0), router_0.Get(0));
  NetDeviceContainer ndc_r0_r1 = p2p_r0_r1.Install (router_0.Get(0), router_1.Get(0));
  NetDeviceContainer ndc_r1_server = p2p_r1_server.Install (server.Get(0), router_1.Get(0));

  Ptr<PointToPointNetDevice> p2p_r0 = DynamicCast<PointToPointNetDevice>(ndc_r0_r1.Get(0));
  p2p_r0->SetRouter(true);

  Ptr<PointToPointNetDevice> p2p_r1 = DynamicCast<PointToPointNetDevice>(ndc_r0_r1.Get(1));
  p2p_r1->SetRouter(true);

  /* Install the IP stack. */
  InternetStackHelper internetStackH;
  internetStackH.Install (client);
  internetStackH.Install (server);
  internetStackH.Install (router_0);
  internetStackH.Install (router_1);

  /* IP assign. */
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer iface_ndc_client_r0 = ipv4.Assign (ndc_client_r0);
  ipv4.SetBase ("10.0.1.0", "255.255.255.0");
  Ipv4InterfaceContainer iface_ndc_r0_r1 = ipv4.Assign (ndc_r0_r1);
  ipv4.SetBase ("10.0.2.0", "255.255.255.0");
  Ipv4InterfaceContainer iface_ndc_r1_server = ipv4.Assign (ndc_r1_server);

  /* Generate Route. */
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  /* Generate Application. */
  uint16_t port_udpEcho_0 = 9;
  UdpEchoServerHelper server_udpEcho_0 (port_udpEcho_0);
  ApplicationContainer server_app = server_udpEcho_0.Install (server.Get(0));
  server_app.Start (Seconds (1.0));
  server_app.Stop (Seconds (30.0));

  Time interPacketInterval_udpEcho_0 = Seconds (1);

  // ndc_client_r0.Get(0)->SetAttribute ("MaxPackets", UintegerValue (1));
  // ndc_client_r0.Get(0)->SetAttribute ("Interval", TimeValue (interPacketInterval_udpEcho_0));
  
  int packet_size = 1100;
  p2p_r0->SetPacketSize(packet_size);
  p2p_r1->SetPacketSize(packet_size);

  UdpEchoClientHelper client_udpEcho_0 (iface_ndc_r1_server.GetAddress(0), 9);
  client_udpEcho_0.SetAttribute ("MaxPackets", UintegerValue (1));
  client_udpEcho_0.SetAttribute ("Interval", TimeValue (interPacketInterval_udpEcho_0));
  client_udpEcho_0.SetAttribute ("PacketSize", UintegerValue (packet_size));
  ApplicationContainer client_app = client_udpEcho_0.Install (client.Get (0));
  client_app.Start (Seconds (2.0));
  client_app.Stop (Seconds (120.0));


	AnimationInterface anim ("p2p.xml");
	anim.SetConstantPosition (client.Get(0), 0.0, 0.0);
  anim.SetConstantPosition (router_0.Get(0), 10.0, 10.0);
  anim.SetConstantPosition (router_1.Get(0), 20.0, 20.0);
  anim.SetConstantPosition (server.Get(0), 30.0, 30.0);

  // AsciiTraceHelper ascii;
  AsciiTraceHelper ascii;
  p2p_client_r0.EnableAsciiAll (ascii.CreateFileStream ("client.tr"));
  p2p_client_r0.EnablePcap ("client", ndc_client_r0.Get(0), false, false);


  p2p_r1_server.EnableAsciiAll (ascii.CreateFileStream ("router_0.tr"));
  p2p_r1_server.EnablePcap ("router_0", ndc_r0_r1.Get(0), false, false);

  p2p_r1_server.EnableAsciiAll (ascii.CreateFileStream ("router_1.tr"));
  p2p_r1_server.EnablePcap ("router_1", ndc_r0_r1.Get(1), false, false);
  
  p2p_r1_server.EnableAsciiAll (ascii.CreateFileStream ("router_1_to_server.tr"));
  p2p_r1_server.EnablePcap ("router_1_to_server", ndc_r1_server.Get(1), false, false);

  p2p_r1_server.EnableAsciiAll (ascii.CreateFileStream ("server.tr"));
  p2p_r1_server.EnablePcap ("server", ndc_r1_server.Get(0), false, false);

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
