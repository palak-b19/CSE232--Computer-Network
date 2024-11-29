#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include <fstream>
#include <iomanip>
#include <map>

using namespace ns3;

// Global variables for trace file
std::ofstream traceFile;

// Function for logging packet traces
void PacketTrace(Ptr<const Packet> packet, Ptr<Ipv4> ipv4, uint32_t interface) {
    Ipv4Header ipv4Header;
    packet->PeekHeader(ipv4Header); // Extract IPv4 header

    // Extract source and destination addresses
    Ipv4Address source = ipv4Header.GetSource();
    Ipv4Address destination = ipv4Header.GetDestination();

    traceFile << Simulator::Now().GetSeconds() << " Packet " << packet->GetUid()
              << " at Node " << ipv4->GetObject<Node>()->GetId()
              << " on Interface " << interface
              << " Source: " << source
              << " Destination: " << destination
              << std::endl;
}

int main(int argc, char *argv[]) {
    CommandLine cmd;
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    // Create 7 hosts (A-G) and 4 routers (R1-R4)
    NodeContainer hosts;
    hosts.Create(7);
    NodeContainer routers;
    routers.Create(4);

    InternetStackHelper stack;
    stack.Install(hosts);
    stack.Install(routers);

    // Point-to-point links
    PointToPointHelper p2p;
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));
    p2p.SetDeviceAttribute("DataRate", StringValue("1Mbps"));

    // Host-router connections
    NetDeviceContainer hostDevices[7];
    hostDevices[0] = p2p.Install(NodeContainer(hosts.Get(0), routers.Get(0))); // A - R1
    hostDevices[1] = p2p.Install(NodeContainer(hosts.Get(1), routers.Get(0))); // B - R1
    hostDevices[2] = p2p.Install(NodeContainer(hosts.Get(2), routers.Get(2))); // C - R3
    hostDevices[3] = p2p.Install(NodeContainer(hosts.Get(3), routers.Get(2))); // D - R3
    hostDevices[4] = p2p.Install(NodeContainer(hosts.Get(4), routers.Get(1))); // E - R2
    hostDevices[5] = p2p.Install(NodeContainer(hosts.Get(5), routers.Get(1))); // F - R2
    hostDevices[6] = p2p.Install(NodeContainer(hosts.Get(6), routers.Get(3))); // G - R4

    // Router-router connections
    NetDeviceContainer routerDevices;
    routerDevices.Add(p2p.Install(NodeContainer(routers.Get(0), routers.Get(1)))); // R1 - R2
    routerDevices.Add(p2p.Install(NodeContainer(routers.Get(0), routers.Get(2)))); // R1 - R3
    routerDevices.Add(p2p.Install(NodeContainer(routers.Get(2), routers.Get(3)))); // R3 - R4
    routerDevices.Add(p2p.Install(NodeContainer(routers.Get(1), routers.Get(3)))); // R2 - R4

    // Assign IP addresses
    Ipv4AddressHelper address;
    address.SetBase("10.1.0.0", "255.255.255.0");
    for (uint32_t i = 0; i < 7; ++i) {
        address.Assign(hostDevices[i]);
    }
    address.SetBase("10.1.1.0", "255.255.255.0");
    for (uint32_t i = 0; i < routerDevices.GetN(); ++i) {
        address.Assign(routerDevices.Get(i));
    }

    // Install applications
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApps = echoServer.Install(hosts);
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    UdpEchoClientHelper echoClient(Ipv4Address("10.1.0.1"), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(100));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(0.01)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer clientApps = echoClient.Install(hosts.Get(1));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));

    // Set up FlowMonitor
    FlowMonitorHelper flowHelper;
    Ptr<FlowMonitor> flowMonitor = flowHelper.InstallAll();

    // Open trace file and configure trace logging
    traceFile.open("packet-traces.txt");
    Config::ConnectWithoutContext("/NodeList/*/$ns3::Ipv4L3Protocol/Rx",
                                  MakeCallback(&PacketTrace));

    Simulator::Stop(Seconds(10.0));
    Simulator::Run();

    flowMonitor->SerializeToXmlFile("flow-monitor.xml", true, true);
    traceFile.close();
    Simulator::Destroy();

    return 0;
}
