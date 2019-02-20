#include "ns3/netanim-module.h"
#include "ns3/core-module.h"
#include "ns3/global-route-manager.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/bridge-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("PointToPoint");

int main(int argc, char *argv[])
{
	Time::SetResolution (Time::NS);
	LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
	LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  CommandLine cmd;
  cmd.Parse (argc, argv);

  /* Configuration. */

  /* Build nodes. */
  NodeContainer client;
  client.Create (1);
  NodeContainer server;
  server.Create (1);
  NodeContainer router_0;
  router_0.Create (1);
  NodeContainer router_1;
  router_1.Create (1);

  /* Build link. */
  PointToPointHelper p2p_p2p_0;
  p2p_p2p_0.SetDeviceAttribute ("DataRate", StringValue("8Mbps"));
  p2p_p2p_0.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (10000)));
  PointToPointHelper p2p_p2p_1;
  p2p_p2p_1.SetDeviceAttribute ("DataRate", StringValue("8Mbps"));
  p2p_p2p_1.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (10000)));
  PointToPointHelper p2p_p2p_2;
  p2p_p2p_2.SetDeviceAttribute ("DataRate", StringValue("8Mbps"));
  p2p_p2p_2.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (10000)));

  /* Build link net device container. */
  NodeContainer all_p2p_0;
  all_p2p_0.Add (client);
  all_p2p_0.Add (router_0);
  NetDeviceContainer ndc_p2p_0 = p2p_p2p_0.Install (all_p2p_0);
  NodeContainer all_p2p_1;
  all_p2p_1.Add (router_0);
  all_p2p_1.Add (router_1);
  NetDeviceContainer ndc_p2p_1 = p2p_p2p_1.Install (all_p2p_1);
  NodeContainer all_p2p_2;
  all_p2p_2.Add (server);
  all_p2p_2.Add (router_1);
  NetDeviceContainer ndc_p2p_2 = p2p_p2p_2.Install (all_p2p_2);

  /* Install the IP stack. */
  InternetStackHelper internetStackH;
  internetStackH.Install (client);
  internetStackH.Install (server);
  internetStackH.Install (router_0);
  internetStackH.Install (router_1);

  /* IP assign. */
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer iface_ndc_p2p_0 = ipv4.Assign (ndc_p2p_0);
  ipv4.SetBase ("10.0.1.0", "255.255.255.0");
  Ipv4InterfaceContainer iface_ndc_p2p_1 = ipv4.Assign (ndc_p2p_1);
  ipv4.SetBase ("10.0.2.0", "255.255.255.0");
  Ipv4InterfaceContainer iface_ndc_p2p_2 = ipv4.Assign (ndc_p2p_2);

  /* Generate Route. */
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  /* Generate Application. */
  uint16_t port_udpEcho_0 = 9;
  UdpEchoServerHelper server_udpEcho_0 (port_udpEcho_0);
  ApplicationContainer apps_udpEcho_0 = server_udpEcho_0.Install (server.Get(0));
  apps_udpEcho_0.Start (Seconds (1.0));
  apps_udpEcho_0.Stop (Seconds (10.0));
  
  Time interPacketInterval_udpEcho_0 = Seconds (1.0);

  UdpEchoClientHelper client_udpEcho_0 (iface_ndc_p2p_2.GetAddress(0), 9);
  client_udpEcho_0.SetAttribute ("MaxPackets", UintegerValue (1));
  client_udpEcho_0.SetAttribute ("Interval", TimeValue (interPacketInterval_udpEcho_0));
  client_udpEcho_0.SetAttribute ("PacketSize", UintegerValue (1024));
  apps_udpEcho_0 = client_udpEcho_0.Install (client.Get (0));
  client_udpEcho_0.SetFill(apps_udpEcho_0.Get(0),"Hello World!");
  apps_udpEcho_0.Start (Seconds (2.0));
  apps_udpEcho_0.Stop (Seconds (10.0));


	AnimationInterface anim ("p2p.xml");
	anim.SetConstantPosition (client.Get(0), 0.0, 0.0);
  anim.SetConstantPosition (router_0.Get(0), 10.0, 10.0);
  anim.SetConstantPosition (router_1.Get(0), 20.0, 20.0);
  anim.SetConstantPosition (server.Get(0), 30.0, 30.0);

  AsciiTraceHelper ascii;
  p2p_p2p_0.EnableAsciiAll (ascii.CreateFileStream ("client.tr"));
  p2p_p2p_0.EnablePcap ("client", ndc_p2p_0.Get(0), false, false);

  // AsciiTraceHelper ascii;
  p2p_p2p_2.EnableAsciiAll (ascii.CreateFileStream ("server.tr"));
  p2p_p2p_2.EnablePcap ("server", ndc_p2p_0.Get(0), false, false);

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
