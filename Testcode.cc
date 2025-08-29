#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

int main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  NodeContainer nodes;
  nodes.Create (4);

  InternetStackHelper internet;
  internet.Install (nodes);

  PointToPointHelper p2pFast;
  p2pFast.SetDeviceAttribute ("DataRate", StringValue ("10Gbps"));
  p2pFast.SetChannelAttribute ("Delay", StringValue ("2ms"));

  PointToPointHelper p2pSlow;
  p2pSlow.SetDeviceAttribute ("DataRate", StringValue ("1Gbps"));
  p2pSlow.SetChannelAttribute ("Delay", StringValue ("5ms"));

  // Topology: n0 ---10G--- n1 ---1G--- n2 ---10G--- n3
  NetDeviceContainer d01 = p2pFast.Install (nodes.Get(0), nodes.Get(1));
  NetDeviceContainer d12 = p2pSlow.Install (nodes.Get(1), nodes.Get(2));
  NetDeviceContainer d23 = p2pFast.Install (nodes.Get(2), nodes.Get(3));

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i01 = ipv4.Assign (d01);

  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i12 = ipv4.Assign (d12);

  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i23 = ipv4.Assign (d23);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // Common parameters
  uint16_t portBBR = 50000;
  uint16_t portCubic = 50001;
  uint16_t portL4S = 50002;

  // ---------------- TCP BBR Flow ----------------
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpBbr::GetTypeId()));
  Address sinkBBR (InetSocketAddress (i23.GetAddress(1), portBBR));
  PacketSinkHelper sinkHelperBBR ("ns3::TcpSocketFactory", sinkBBR);
  ApplicationContainer sinkAppBBR = sinkHelperBBR.Install (nodes.Get(3));
  sinkAppBBR.Start (Seconds (0.0));
  sinkAppBBR.Stop (Seconds (10.0));

  OnOffHelper clientBBR ("ns3::TcpSocketFactory", sinkBBR);
  clientBBR.SetAttribute ("DataRate", StringValue ("20Gbps"));
  clientBBR.SetAttribute ("PacketSize", UintegerValue (1024));
  ApplicationContainer clientAppBBR = clientBBR.Install (nodes.Get(0));
  clientAppBBR.Start (Seconds (1.0));
  clientAppBBR.Stop (Seconds (10.0));

  // ---------------- TCP CUBIC Flow ----------------
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpCubic::GetTypeId()));
  Address sinkCubic (InetSocketAddress (i23.GetAddress(1), portCubic));
  PacketSinkHelper sinkHelperCubic ("ns3::TcpSocketFactory", sinkCubic);
  ApplicationContainer sinkAppCubic = sinkHelperCubic.Install (nodes.Get(3));
  sinkAppCubic.Start (Seconds (0.0));
  sinkAppCubic.Stop (Seconds (10.0));

  OnOffHelper clientCubic ("ns3::TcpSocketFactory", sinkCubic);
  clientCubic.SetAttribute ("DataRate", StringValue ("20Gbps"));
  clientCubic.SetAttribute ("PacketSize", UintegerValue (1024));
  ApplicationContainer clientAppCubic = clientCubic.Install (nodes.Get(0));
  clientAppCubic.Start (Seconds (1.5));
  clientAppCubic.Stop (Seconds (10.0));

  // ---------------- TCP Prague (L4S) Flow ----------------
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpPrague::GetTypeId()));
  Config::SetDefault("ns3::RedQueueDisc::UseEcn", BooleanValue (true)); // ECN enabled

  Address sinkL4S (InetSocketAddress (i23.GetAddress(1), portL4S));
  PacketSinkHelper sinkHelperL4S ("ns3::TcpSocketFactory", sinkL4S);
  ApplicationContainer sinkAppL4S = sinkHelperL4S.Install (nodes.Get(3));
  sinkAppL4S.Start (Seconds (0.0));
  sinkAppL4S.Stop (Seconds (10.0));

  OnOffHelper clientL4S ("ns3::TcpSocketFactory", sinkL4S);
  clientL4S.SetAttribute ("DataRate", StringValue ("20Gbps"));
  clientL4S.SetAttribute ("PacketSize", UintegerValue (1024));
  ApplicationContainer clientAppL4S = clientL4S.Install (nodes.Get(0));
  clientAppL4S.Start (Seconds (2.0));
  clientAppL4S.Stop (Seconds (10.0));

  // Run Simulation
  Simulator::Stop (Seconds (11.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
