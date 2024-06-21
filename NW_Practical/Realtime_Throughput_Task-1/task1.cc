#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include <fstream>
#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"
#include "ns3/mobility-module.h"
using namespace ns3;
using namespace std;
NS_LOG_COMPONENT_DEFINE ("Lab-4-1");
int
main (int argc, char *argv[])
{
  double delay = 2;
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  CommandLine cmd;
  cmd.AddValue("delay", "P2P delay /latency in ms ", delay);
  cmd.Parse(argc,argv);
  NodeContainer nodes;
  nodes.Create (2);
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds(delay)));

  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);

  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  UdpEchoServerHelper echoServer1 (9000);
  UdpEchoServerHelper echoServer2 (9001);
  
  ApplicationContainer serverApps = echoServer1.Install (nodes.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  serverApps = echoServer2.Install (nodes.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient1 (interfaces.GetAddress (1), 9000);
  echoClient1.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient1.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient1.SetAttribute ("PacketSize", UintegerValue (1024));
  ApplicationContainer clientApps = echoClient1.Install (nodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));


  UdpEchoClientHelper echoClient2 (interfaces.GetAddress (1), 9001);
  echoClient2.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient2.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient2.SetAttribute ("PacketSize", UintegerValue (1024));
  clientApps = echoClient2.Install (nodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));


  //
  // Tracing
  //
    AsciiTraceHelper ascii;
    pointToPoint.EnableAscii(ascii.CreateFileStream ("lab-4-1.tr"), devices);
    pointToPoint.EnablePcap("lab-4-1",devices, false);

  //
  // Calculate Throughput using Flowmonitor
  //
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();
  //
  // Now, do the actual simulation.
  //
    NS_LOG_INFO ("Run Simulation.");
    AnimationInterface anim("MiniProject-First.xml");
    Simulator::Stop (Seconds(11.0));
    Simulator::Run ();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
      {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
        if ((t.sourceAddress=="10.1.1.1" && t.destinationAddress == "10.1.1.2"))
        {
            std::cout << "Flow " << i->first  << " (" << t.sourceAddress<<":"<<t.sourcePort << " -> " << t.destinationAddress <<":"<<t.destinationPort<< ")\n";
            std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
            std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
            std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1024/1024  << " Mbps\n";
        }
       }
  Simulator::Destroy ();
  return 0;
}
