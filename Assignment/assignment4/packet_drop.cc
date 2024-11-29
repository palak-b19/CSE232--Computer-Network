#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/error-model.h" 
#include <map>
#include <utility>
#include <string>
#include <iostream>
#include <random>
#include <fstream>
#include <iomanip>
#include "ns3/flow-monitor-module.h"
#include <iomanip>

using namespace ns3;


// Declare the traffic matrix
std::map<std::pair<std::string, std::string>, uint32_t> trafficMatrix;
std::map<Ipv4Address, std::string> ipToNodeName;
NS_LOG_COMPONENT_DEFINE("CustomNetworkSimulation");


// Function to print the packet drop rates in a matrix format
void PrintPacketDropMatrix(std::map<std::pair<std::string, std::string>, uint32_t> trafficMatrix, 
                           std::vector<std::string> hostNames) {
    std::ofstream outFile("packet_srop.txt");  // Open file for writing

    if (!outFile.is_open()) {
        std::cerr << "Error opening file for writing!" << std::endl;
        return;
    }

    // Print the header row
    outFile << "Packet Drops:\n";
    outFile << std::setw(10) << "From:";
    for (const auto &to : hostNames) {
        outFile << std::setw(10) << to;
    }
    outFile << std::endl;

    // Print each row
    for (const auto &from : hostNames) {
        outFile << std::setw(10) << from;
        for (const auto &to : hostNames) {
            if (from == to) {
                outFile << std::setw(10) << 0; // No self-drops
            } else {
                auto it = trafficMatrix.find(std::make_pair(from, to));
                if (it != trafficMatrix.end()) {
                    outFile << std::setw(10) << it->second;
                } else {
                    outFile << std::setw(10) << 0; // No data
                }
            }
        }
        outFile << std::endl;
    }

    outFile.close(); // Close the file after writing
}

// Custom Check for lost packets using FlowMonitor
void CheckForLostPackets(Ptr<FlowMonitor> flowMonitor, 
                         Ptr<Ipv4FlowClassifier> classifier,
                         std::map<std::pair<std::string, std::string>, uint32_t> &trafficMatrix,
                         std::map<Ipv4Address, std::string> &ipToNodeName) {
    FlowMonitor::FlowStatsContainer stats = flowMonitor->GetFlowStats();
    for (const auto &flow : stats) {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(flow.first);

        // Get source and destination node names
        std::string sourceNode = ipToNodeName[t.sourceAddress];
        std::string destNode = ipToNodeName[t.destinationAddress];

        // Debug: Print traffic information for this flow
        std::cout << "Flow from " << sourceNode << " to " << destNode << ": "
                  << "Lost Packets = " << flow.second.lostPackets << ", "
                  << "Tx Packets = " << flow.second.txPackets << ", "
                  << "Rx Packets = " << flow.second.rxPackets << std::endl;

        // Update the traffic matrix
        trafficMatrix[{sourceNode, destNode}] += flow.second.lostPackets;
    }
}


int main(int argc, char *argv[]) {

    CommandLine cmd;
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    

    // Enable logging for debugging
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    // Create 7 hosts (A-G) and 4 routers (R1-R4)
    NodeContainer hosts;
    hosts.Create(7);
    NodeContainer routers;
    routers.Create(4);
       std::vector<std::string> hostNames = {"A", "B", "C", "D", "E", "F", "G"};

    InternetStackHelper stack;
    Ipv4GlobalRoutingHelper globalRouting; // Ensure global routing is used
stack.SetRoutingHelper(globalRouting);
    stack.Install(hosts);
    stack.Install(routers);

 PointToPointHelper p2p;

    // Define link parameters based on the image topology
    // Connect hosts to their respective routers
    NetDeviceContainer hostDevices[7];
    NetDeviceContainer routerDevices;
    // Host-to-Router links with capacities from Table 3
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    p2p.SetDeviceAttribute("DataRate", StringValue("1Mbps")); // A - R1
    hostDevices[0] = p2p.Install(NodeContainer(hosts.Get(0), routers.Get(0)));

    p2p.SetDeviceAttribute("DataRate", StringValue("1Mbps")); // B - R1
    hostDevices[1] = p2p.Install(NodeContainer(hosts.Get(1), routers.Get(0)));

    p2p.SetDeviceAttribute("DataRate", StringValue("1Mbps")); // C - R3
    hostDevices[2] = p2p.Install(NodeContainer(hosts.Get(2), routers.Get(2)));

    p2p.SetDeviceAttribute("DataRate", StringValue("2Mbps")); // D - R3
    hostDevices[3] = p2p.Install(NodeContainer(hosts.Get(3), routers.Get(2)));

    p2p.SetDeviceAttribute("DataRate", StringValue("1Mbps")); // E - R2
    hostDevices[4] = p2p.Install(NodeContainer(hosts.Get(4), routers.Get(1)));

    p2p.SetDeviceAttribute("DataRate", StringValue("1Mbps")); // F - R2
    hostDevices[5] = p2p.Install(NodeContainer(hosts.Get(5), routers.Get(1)));

    p2p.SetDeviceAttribute("DataRate", StringValue("1Mbps")); // G - R4
    hostDevices[6] = p2p.Install(NodeContainer(hosts.Get(6), routers.Get(3)));

    // Router-to-Router links with capacities from Table 3
    p2p.SetDeviceAttribute("DataRate", StringValue("3Mbps")); // R1 - R2
    routerDevices.Add(p2p.Install(NodeContainer(routers.Get(0), routers.Get(1))));

    p2p.SetDeviceAttribute("DataRate", StringValue("2.5Mbps")); // R1 - R3
    routerDevices.Add(p2p.Install(NodeContainer(routers.Get(0), routers.Get(2))));

    p2p.SetDeviceAttribute("DataRate", StringValue("1.5Mbps")); // R3 - R4
    routerDevices.Add(p2p.Install(NodeContainer(routers.Get(2), routers.Get(3))));

    p2p.SetDeviceAttribute("DataRate", StringValue("1Mbps")); // R2 - R4
    routerDevices.Add(p2p.Install(NodeContainer(routers.Get(1), routers.Get(3))));

    // Create an Error Model
Ptr<RateErrorModel> errorModel = CreateObject<RateErrorModel>();
errorModel->SetAttribute("ErrorRate", DoubleValue(0.01)); // 1% packet drop rate

// Apply the error model to each device
for (uint32_t i = 0; i < 7; ++i) {
        hostDevices[i].Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(errorModel)); 
    }
    for (uint32_t i = 0; i < routerDevices.GetN(); ++i) {
        routerDevices.Get(i)->SetAttribute("ReceiveErrorModel", PointerValue(errorModel)); 
    }

    // Assign IP addresses to all links
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

    // Set up UDP Echo server on Host A (host 0)
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApps;
    for (uint32_t i = 0; i < 7; ++i) {
        ApplicationContainer serverApp = echoServer.Install(hosts.Get(i));
        serverApps.Add(serverApp);
    }
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    // Enable global routing
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Set up UDP Echo clients on remaining hosts (B-G)
    for (uint32_t i = 0; i < 7; ++i) {
        for (uint32_t j = 0; j < 7; ++j) {
            if (i != j) { // Avoid sending traffic to itself
                
                UdpEchoClientHelper echoClient(Ipv4Address(("10.1." + std::to_string(j) + ".1").c_str()), 9);

                // UdpEchoClientHelper echoClient(Ipv4Address("10.1.0.1"), 9);
                echoClient.SetAttribute("MaxPackets", UintegerValue(1000));
                echoClient.SetAttribute("Interval", TimeValue(Seconds(0.01)));
                echoClient.SetAttribute("PacketSize", UintegerValue(1024));

                ApplicationContainer clientApps = echoClient.Install(hosts.Get(i));
                clientApps.Start(Seconds(2.0 + i + j));
                clientApps.Stop(Seconds(10.0));
            }
        }
    }
    // Set up FlowMonitor
    Ptr<FlowMonitor> flowMonitor;
    FlowMonitorHelper flowHelper;
    flowMonitor = flowHelper.InstallAll();

    // IP to Node Name Mapping
    ipToNodeName[Ipv4Address("10.1.0.1")] = "A";
    ipToNodeName[Ipv4Address("10.1.1.1")] = "B";
    ipToNodeName[Ipv4Address("10.1.2.1")] = "C";
    ipToNodeName[Ipv4Address("10.1.3.1")] = "D";
    ipToNodeName[Ipv4Address("10.1.4.1")] = "E";
    ipToNodeName[Ipv4Address("10.1.5.1")] = "F";
    ipToNodeName[Ipv4Address("10.1.6.1")] = "G";

    // Run the simulation
    Simulator::Stop(Seconds(60.0));
    Simulator::Run();

    // Analyze the packet loss
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowHelper.GetClassifier());
    CheckForLostPackets(flowMonitor, classifier, trafficMatrix, ipToNodeName);

    // Print the packet drop matrix
    PrintPacketDropMatrix(trafficMatrix, hostNames);

    // Clean up and exit
    Simulator::Destroy();
    return 0;
}
