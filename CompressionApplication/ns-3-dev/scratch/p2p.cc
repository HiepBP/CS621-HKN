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
    /* Configuration. */
  //std::cout << "Enter Compression Link Capacity (xMbps) : ";
  std::string link_capacity = "8Mbps";

  CommandLine cmd;
  cmd.AddValue("link_capacity","User defined link capacity for compression link",link_capacity);
  cmd.Parse (argc, argv);

  double transmission_times[2] = {};
  std::cout << link_capacity << '\n';

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
  p2p_r0_r1.SetDeviceAttribute ("DataRate", StringValue(link_capacity));
  p2p_r0_r1.SetChannelAttribute ("Delay", StringValue("1ms"));
  PointToPointHelper p2p_r1_server;
  p2p_r1_server.SetDeviceAttribute ("DataRate", StringValue("8Mbps"));
  p2p_r1_server.SetChannelAttribute ("Delay", StringValue("1ms"));

  /* Build link net device container. */
  NetDeviceContainer ndc_client_r0 = p2p_client_r0.Install (client.Get(0), router_0.Get(0));
  NetDeviceContainer ndc_r0_r1 = p2p_r0_r1.Install (router_0.Get(0), router_1.Get(0));
  NetDeviceContainer ndc_r1_server = p2p_r1_server.Install (server.Get(0), router_1.Get(0));

  Ptr<PointToPointNetDevice> p2p_r0 = DynamicCast<PointToPointNetDevice>(ndc_r0_r1.Get(0));
  p2p_r0->SetCompress(true);

  Ptr<PointToPointNetDevice> p2p_r1 = DynamicCast<PointToPointNetDevice>(ndc_r0_r1.Get(1));
  p2p_r1->SetCompress(true);

  Ptr<PointToPointNetDevice> p2p_server = DynamicCast<PointToPointNetDevice>(ndc_r1_server.Get(1));

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

  int packet_size = 1100;
  p2p_r0->SetPacketSize(packet_size);
  p2p_r1->SetPacketSize(packet_size);

  /* Generate Application. */
  uint16_t port_udpEcho_0 = 9;
  UdpServerHelper server_udpEcho_0 (port_udpEcho_0);
  ApplicationContainer server_app = server_udpEcho_0.Install (server.Get(0));
  server_app.Start (Seconds (1.0));
  server_app.Stop (Seconds (20000.0));
  //Time interPacketInterval_udpEcho_0 = MilliSeconds(500);

  UdpClientHelper udp_client_0 (iface_ndc_r1_server.GetAddress(0), port_udpEcho_0);
  udp_client_0.SetAttribute ("MaxPackets", UintegerValue (6000));
 // udp_client_0.SetAttribute ("Interval", TimeValue (interPacketInterval_udpEcho_0));
  udp_client_0.SetAttribute ("PacketSize", UintegerValue (1100));
  udp_client_0.SetAttribute ("Entropy", BooleanValue(false));
  ApplicationContainer client_app_1 = udp_client_0.Install (client.Get (0));
  Ptr<UdpClient> udp_client_1 = DynamicCast<UdpClient>(client_app_1.Get(0));
  udp_client_1->GetPayload();
  client_app_1.Start (Seconds (2.0));
  client_app_1.Stop (Seconds (6005.0));

  UdpClientHelper udp_client_2 (iface_ndc_r1_server.GetAddress(0), port_udpEcho_0);
  udp_client_2.SetAttribute ("MaxPackets", UintegerValue (6000));
 // udp_client_2.SetAttribute ("Interval", TimeValue (interPacketInterval_udpEcho_0));
  udp_client_2.SetAttribute ("PacketSize", UintegerValue (1100));
  udp_client_2.SetAttribute ("Entropy", BooleanValue(true));
  ApplicationContainer client_app_2 = udp_client_2.Install (client.Get (0));
  Ptr<UdpClient> udp_client_3 = DynamicCast<UdpClient>(client_app_2.Get(0));
  udp_client_3->GetPayload();
  client_app_2.Start (Seconds (10002.0));
  client_app_2.Stop (Seconds (16005.0));

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
  p2p_r0_r1.EnablePcap ("router_0", ndc_r0_r1.Get(0), false, false);

  p2p_r1_server.EnableAsciiAll (ascii.CreateFileStream ("router_1.tr"));
  p2p_r0_r1.EnablePcap ("router_1", ndc_r0_r1.Get(1), false, false);

  p2p_r1_server.EnableAsciiAll (ascii.CreateFileStream ("server.tr"));
  p2p_r1_server.EnablePcap ("server", ndc_r1_server.Get(0), false, false);

  Simulator::Run ();
  Simulator::Destroy ();

  std::cout << "::::: RECEIVED : " << server_udpEcho_0.GetPacketCount_1() << ":::::  TIME DIFF : " << server_udpEcho_0.GetTimeDiff_1() << "ms\n";
  std::cout << "::::: RECEIVED : " << server_udpEcho_0.GetPacketCount_2() << ":::::  TIME DIFF : " << server_udpEcho_0.GetTimeDiff_2() << "ms\n";

  transmission_times[0] = server_udpEcho_0.GetTimeDiff_1();
  transmission_times[1] = server_udpEcho_0.GetTimeDiff_2();

  for (int i=0; i < int(sizeof(transmission_times)/sizeof(*transmission_times)); i++) {
    std::cout << "UDP TRAIN " << i << " : " << transmission_times[i] << " ms \n";
  }
  double difference = transmission_times[1] - transmission_times[0];
  std::cout << "DIFFERENCE : " << difference << "ms\n";
  if (abs(difference) > 100) {
    std::cout << "COMPRESSION DETECTED!!!\n";
  }
  else {
    std::cout << "COMPRESSION NOT DETECTED!!!\n";
  }
  return 0;
}