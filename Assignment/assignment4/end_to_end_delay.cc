#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include <iomanip>
#include <map>
#include <vector>
#include <string>
#include <fstream>

using namespace ns3;

// Declare IP to Node name mapping
std::map<Ipv4Address, std::string> ipToNodeName;

NS_LOG_COMPONENT_DEFINE("EndToEndDelaySimulation");

int main(int argc, char *argv[]) {
    CommandLine cmd;
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);
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

    PointToPointHelper p2p;
    NetDeviceContainer hostDevices[7];
    NetDeviceContainer routerDevices;

    // Host-to-Router connections
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));
    p2p.SetDeviceAttribute("DataRate", StringValue("1Mbps"));
    hostDevices[0] = p2p.Install(NodeContainer(hosts.Get(0), routers.Get(0))); // A - R1
    hostDevices[1] = p2p.Install(NodeContainer(hosts.Get(1), routers.Get(0))); // B - R1
    hostDevices[2] = p2p.Install(NodeContainer(hosts.Get(2), routers.Get(2))); // C - R3
    hostDevices[3] = p2p.Install(NodeContainer(hosts.Get(3), routers.Get(2))); // D - R3
    hostDevices[4] = p2p.Install(NodeContainer(hosts.Get(4), routers.Get(1))); // E - R2
    hostDevices[5] = p2p.Install(NodeContainer(hosts.Get(5), routers.Get(1))); // F - R2
    hostDevices[6] = p2p.Install(NodeContainer(hosts.Get(6), routers.Get(3))); // G - R4

    // Router-to-Router connections
    p2p.SetDeviceAttribute("DataRate", StringValue("3Mbps"));
    routerDevices.Add(p2p.Install(NodeContainer(routers.Get(0), routers.Get(1)))); // R1 - R2
    p2p.SetDeviceAttribute("DataRate", StringValue("2.5Mbps"));
    routerDevices.Add(p2p.Install(NodeContainer(routers.Get(0), routers.Get(2)))); // R1 - R3
    p2p.SetDeviceAttribute("DataRate", StringValue("1.5Mbps"));
    routerDevices.Add(p2p.Install(NodeContainer(routers.Get(2), routers.Get(3)))); // R3 - R4
    p2p.SetDeviceAttribute("DataRate", StringValue("1Mbps"));
    routerDevices.Add(p2p.Install(NodeContainer(routers.Get(1), routers.Get(3)))); // R2 - R4

    // Assign IP addresses
    Ipv4AddressHelper address;
    std::string baseIp = "10.1.";
    for (uint32_t i = 0; i < 7; ++i) {
        address.SetBase(Ipv4Address((baseIp + std::to_string(i) + ".0").c_str()), "255.255.255.0");
        address.Assign(hostDevices[i]);
    }

    for (uint32_t i = 0; i < routerDevices.GetN(); ++i) {
        address.SetBase(Ipv4Address((baseIp + std::to_string(i + 7) + ".0").c_str()), "255.255.255.0");
        address.Assign(routerDevices.Get(i));
    }

    // IP-to-Node Mapping
    ipToNodeName[Ipv4Address("10.1.0.1")] = "A";
    ipToNodeName[Ipv4Address("10.1.1.1")] = "B";
    ipToNodeName[Ipv4Address("10.1.2.1")] = "C";
    ipToNodeName[Ipv4Address("10.1.3.1")] = "D";
    ipToNodeName[Ipv4Address("10.1.4.1")] = "E";
    ipToNodeName[Ipv4Address("10.1.5.1")] = "F";
    ipToNodeName[Ipv4Address("10.1.6.1")] = "G";

    // Set up UDP Echo server on all hosts
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApps;
    for (uint32_t i = 0; i < 7; ++i) {
        serverApps.Add(echoServer.Install(hosts.Get(i)));
    }
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    // Set up UDP Echo clients
    for (uint32_t i = 0; i < 7; ++i) {
        for (uint32_t j = 0; j < 7; ++j) {
            if (i != j) { // Avoid self-traffic
                UdpEchoClientHelper echoClient(Ipv4Address(("10.1." + std::to_string(j) + ".1").c_str()), 9);
                echoClient.SetAttribute("MaxPackets", UintegerValue(1000));
                echoClient.SetAttribute("Interval", TimeValue(Seconds(0.01)));
                echoClient.SetAttribute("PacketSize", UintegerValue(1024));
                ApplicationContainer clientApps = echoClient.Install(hosts.Get(i));
                clientApps.Start(Seconds(2.0 + i + j));
                clientApps.Stop(Seconds(10.0));
            }
        }
    }

    // Enable routing
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Flow Monitor setup
    Ptr<FlowMonitor> flowMonitor;
    FlowMonitorHelper flowHelper;
    flowMonitor = flowHelper.InstallAll();

    // Run simulation
    Simulator::Stop(Seconds(60.0));
    Simulator::Run();

    // Analyze Flow Monitor results
    std::vector<std::vector<double>> avgDelays(7, std::vector<double>(7, 0.0));
    std::vector<std::vector<double>> varDelays(7, std::vector<double>(7, 0.0));

    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowHelper.GetClassifier());
    for (const auto& flowStat : flowMonitor->GetFlowStats()) {
        Ipv4FlowClassifier::FiveTuple tuple = classifier->FindFlow(flowStat.first);
        auto srcIter = ipToNodeName.find(tuple.sourceAddress);
        auto dstIter = ipToNodeName.find(tuple.destinationAddress);

        if (srcIter == ipToNodeName.end() || dstIter == ipToNodeName.end()) {
            continue;
        }

        uint32_t srcIndex = srcIter->second[0] - 'A';
        uint32_t dstIndex = dstIter->second[0] - 'A';

        double avgDelay = flowStat.second.delaySum.GetSeconds() / flowStat.second.rxPackets;
        double delayVar = ((flowStat.second.jitterSum.GetSeconds() / flowStat.second.rxPackets) * 2); // Approximation

        avgDelays[srcIndex][dstIndex] = avgDelay;
        varDelays[srcIndex][dstIndex] = delayVar;
    }

    // Print results
    std::ofstream outFile("delay_calculation.txt");
    if (!outFile.is_open()) {
    std::cerr << "Error: Could not open the file for writing results." << std::endl;
    Simulator::Destroy();
    return 1;
}
    outFile << std::fixed << std::setprecision(6);
    outFile << "Average End-to-End Delays (seconds):" << std::endl;
    outFile << "To:       A          B          C          D          E          F          G" << std::endl;
    for (uint32_t i = 0; i < 7; ++i) {
        outFile << char('A' + i) << "   ";
        for (uint32_t j = 0; j < 7; ++j) {
            outFile << avgDelays[i][j] << "    ";
        }
        outFile << std::endl;
    }

    outFile << "\nVariance of Delays (seconds):" << std::endl;
    outFile << "To:       A          B          C          D          E          F          G" << std::endl;
    for (uint32_t i = 0; i < 7; ++i) {
        outFile << char('A' + i) << "   ";
        for (uint32_t j = 0; j < 7; ++j) {
            outFile << varDelays[i][j] << "    ";
        }
        outFile << std::endl;
    }
    outFile.close();

    // Clean up
    Simulator::Destroy();
    return 0;
}
