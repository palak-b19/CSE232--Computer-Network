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
using namespace ns3;
// Declare the traffic matrix
std::map<std::pair<std::string, std::string>, uint32_t> trafficMatrix;
std::map<Ipv4Address, std::string> ipToNodeName;
//! Function to update the traffic matrix
// Function to print traffic matrix in a tabular format and output it to a file
void PrintTrafficMatrix(const std::map<std::pair<std::string, std::string>, uint32_t>& trafficMatrix) {
    std::ofstream outputFile("traffic_matrix.txt");
    // Get the unique set of nodes (sources and destinations)
    std::set<std::string> nodes;
    for (const auto& entry : trafficMatrix) {
        nodes.insert(entry.first.first);  // Source node
        nodes.insert(entry.first.second); // Destination node
    }
    // Convert set of nodes to a vector for indexing
    std::vector<std::string> nodeList(nodes.begin(), nodes.end());
    // Create a lookup for node indices
    std::map<std::string, int> nodeIndex;
    for (size_t i = 0; i < nodeList.size(); ++i) {
        nodeIndex[nodeList[i]] = i;
    }
    // Initialize the matrix
    std::vector<std::vector<uint32_t>> matrix(nodeList.size(), std::vector<uint32_t>(nodeList.size(), 0));
    // Fill the matrix
    for (const auto& entry : trafficMatrix) {
        const std::string& src = entry.first.first;
        const std::string& dst = entry.first.second;
        matrix[nodeIndex[src]][nodeIndex[dst]] = entry.second;
    }
    // Print the header row
    std::cout << "Traffic Matrix:" << std::endl;
    std::cout << std::setw(5) << " " << " ";
    outputFile << "Traffic Matrix:" << std::endl;
    outputFile << std::setw(5) << " " << " ";
    for (const auto& node : nodeList) {
        std::cout << std::setw(5) << node << " ";
        outputFile << std::setw(5) << node << " ";
    }
    std::cout << std::endl;
    outputFile << std::endl;
    // Print the matrix rows
    for (size_t i = 0; i < nodeList.size(); ++i) {
        std::cout << std::setw(5) << nodeList[i] << " ";
        outputFile << std::setw(5) << nodeList[i] << " ";
        for (size_t j = 0; j < nodeList.size(); ++j) {
            std::cout << std::setw(5) << matrix[i][j] << " ";
            outputFile << std::setw(5) << matrix[i][j] << " ";
        }
        std::cout << std::endl;
        outputFile << std::endl;
    }
    outputFile.close();
    std::cout << "Traffic matrix has been written to 'traffic_matrix.txt'." << std::endl;
}
void GenerateTrafficMatrix(const std::vector<std::string>& hosts, double lambda) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::poisson_distribution<> poissonDist(lambda);
    for (const auto& src : hosts) {
        for (const auto& dst : hosts) {
            if (src != dst) {
                uint32_t trafficLoad = poissonDist(gen); // Poisson-distributed traffic load
                trafficMatrix[std::make_pair(src, dst)] = trafficLoad;
            }
        }
    }
}
void PopulateIpToNodeNameMapping(NodeContainer hosts, NodeContainer routers, Ipv4InterfaceContainer interfaces) {
    // Add host names
    std::vector<std::string> hostNames = {"A", "B", "C", "D", "E", "F", "G"};
    for (uint32_t i = 0; i < hosts.GetN(); ++i) {
        Ptr<Ipv4> ipv4 = hosts.Get(i)->GetObject<Ipv4>();
        for (uint32_t j = 0; j < ipv4->GetNInterfaces(); ++j) {
            Ipv4Address ip = ipv4->GetAddress(j, 0).GetLocal();
            ipToNodeName[ip] = hostNames[i];
        }
    }
    // Add router names
    std::vector<std::string> routerNames = {"R1", "R2", "R3", "R4"};
    for (uint32_t i = 0; i < routers.GetN(); ++i) {
        Ptr<Ipv4> ipv4 = routers.Get(i)->GetObject<Ipv4>();
        for (uint32_t j = 0; j < ipv4->GetNInterfaces(); ++j) {
            Ipv4Address ip = ipv4->GetAddress(j, 0).GetLocal();
            ipToNodeName[ip] = routerNames[i];
        }
    }
}
NS_LOG_COMPONENT_DEFINE("CustomNetworkSimulation");
// Call this after assigning IP addresses
// PopulateIpToNodeNameMapping(hosts, routers, address.Assign(routerDevices));
// Updated PrintRoutingTable function
void PrintRoutingTable(Ptr<Node> node, std::ostream &os, const std::map<Ipv4Address, std::string> &ipToNodeName) {
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
    Ptr<Ipv4RoutingProtocol> routingProtocol = ipv4->GetRoutingProtocol();
    Ptr<Ipv4GlobalRouting> globalRouting = DynamicCast<Ipv4GlobalRouting>(routingProtocol);
    if (globalRouting != nullptr) {
        uint32_t numRoutes = globalRouting->GetNRoutes();
        os << "Routing table for node " 
           << ipToNodeName.at(ipv4->GetAddress(1, 0).GetLocal()) // Get the node's name
           << " (Node ID: " << node->GetId() << "):" << std::endl;
        for (uint32_t i = 0; i < numRoutes; ++i) {
            Ipv4RoutingTableEntry route = globalRouting->GetRoute(i);
            // Destination name
            std::ostringstream destStream;
            if (ipToNodeName.count(route.GetDest())) {
                destStream << ipToNodeName.at(route.GetDest());
            } else {
                route.GetDest().Print(destStream);
            }
            std::string destName = destStream.str();
            // Gateway name
            std::ostringstream gatewayStream;
            if (ipToNodeName.count(route.GetGateway())) {
                gatewayStream << ipToNodeName.at(route.GetGateway());
            } else {
                route.GetGateway().Print(gatewayStream);
            }
            std::string gatewayName = gatewayStream.str();
            // Print the routing entry
            os << "  Destination: " << destName
               << ", Gateway: " << gatewayName
               << ", Interface: " << route.GetInterface()
               << std::endl;
        }
        os << std::endl;
    } else {
        os << "Error: Routing protocol on node " << node->GetId() << " is not global routing!" << std::endl;
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
    // Create point-to-point helper for links
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
hostDevices[0].Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(errorModel)); // A - R1
hostDevices[1].Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(errorModel)); // B - R1
hostDevices[2].Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(errorModel)); // C - R3
hostDevices[3].Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(errorModel)); // D - R3
hostDevices[4].Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(errorModel)); // E - R2
hostDevices[5].Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(errorModel)); // F - R2
hostDevices[6].Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(errorModel)); // G - R4
routerDevices.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue(errorModel)); // R1 - R2
routerDevices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(errorModel)); // R1 - R3
routerDevices.Get(2)->SetAttribute("ReceiveErrorModel", PointerValue(errorModel)); // R3 - R4
routerDevices.Get(3)->SetAttribute("ReceiveErrorModel", PointerValue(errorModel)); // R2 - R4
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
    ApplicationContainer serverApps = echoServer.Install(hosts.Get(0));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));
    // Enable global routing
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    // Set up UDP Echo clients on remaining hosts (B-G)
    for (uint32_t i = 1; i < 7; ++i) {
        UdpEchoClientHelper echoClient(Ipv4Address("10.1.0.1"), 9); // Send to Host A
        echoClient.SetAttribute("MaxPackets", UintegerValue(2));
        echoClient.SetAttribute("Interval", TimeValue(Seconds(2.0)));
        echoClient.SetAttribute("PacketSize", UintegerValue(512));
        ApplicationContainer clientApps = echoClient.Install(hosts.Get(i));
        clientApps.Start(Seconds(2.0 + i));
        clientApps.Stop(Seconds(10.0));
    }
    
// Routing table tracking snippet
for (uint32_t i = 0; i < routers.GetN(); ++i) {
    Ptr<Node> router = routers.Get(i);
    Ptr<Ipv4> ipv4 = router->GetObject<Ipv4>();
    Ptr<Ipv4RoutingProtocol> routingProtocol = ipv4->GetRoutingProtocol();
    Ptr<Ipv4GlobalRouting> globalRouting = DynamicCast<Ipv4GlobalRouting>(routingProtocol);
    if (globalRouting!= nullptr) {
        uint32_t numRoutes = globalRouting->GetNRoutes();
        std::cout << "Routing table for Router R" << (i + 1) << ":" << std::endl;
        for (uint32_t j = 0; j < numRoutes; ++j) {
            Ipv4RoutingTableEntry route = globalRouting->GetRoute(j);
            std::cout << "  Destination: " << route.GetDest()
                      << ", Next Hop: " << route.GetGateway()
                      << ", Interface: " << route.GetInterface() << std::endl;
        }
        std::cout << std::endl;
    } else {
        std::cerr << "Error: Routing protocol is not global routing!" << std::endl;
    }
}
    // Create NetAnim animation
    AnimationInterface anim("custom_network_topology.xml");
    anim.EnableIpv4RouteTracking("route-tracking.xml", Seconds(1.0), Seconds(10.0), Seconds(5.0));
    // Set custom positions for hosts
    anim.SetConstantPosition(hosts.Get(0), 10, 10); // Host A
    anim.SetConstantPosition(hosts.Get(1), 20, 10); // Host B
    anim.SetConstantPosition(hosts.Get(2), 30, 40); // Host C
    anim.SetConstantPosition(hosts.Get(3), 40, 40); // Host D
    anim.SetConstantPosition(hosts.Get(4), 30, 10); // Host E
    anim.SetConstantPosition(hosts.Get(5), 40, 10); // Host F
    anim.SetConstantPosition(hosts.Get(6), 10, 40); // Host G
    // Set custom positions for routers
    anim.SetConstantPosition(routers.Get(0), 20, 20); // Router R1
    anim.SetConstantPosition(routers.Get(1), 40, 20); // Router R2
    anim.SetConstantPosition(routers.Get(2), 40, 30); // Router R3
    anim.SetConstantPosition(routers.Get(3), 20, 30); // Router R4
    // Set custom names for routers
    anim.UpdateNodeDescription(routers.Get(0), "R1");
    anim.UpdateNodeDescription(routers.Get(1), "R2");
    anim.UpdateNodeDescription(routers.Get(2), "R3");
    anim.UpdateNodeDescription(routers.Get(3), "R4");
    // Set custom names for hosts
    anim.UpdateNodeDescription(hosts.Get(0), "A");
    anim.UpdateNodeDescription(hosts.Get(1), "B");
    anim.UpdateNodeDescription(hosts.Get(2), "C");
    anim.UpdateNodeDescription(hosts.Get(3), "D");
    anim.UpdateNodeDescription(hosts.Get(4), "E");
    anim.UpdateNodeDescription(hosts.Get(5), "F");
    anim.UpdateNodeDescription(hosts.Get(6), "G");
    PopulateIpToNodeNameMapping(hosts, routers, address.Assign(routerDevices));
    for (uint32_t i = 0; i < routers.GetN(); ++i) {
    Ptr<Node> router = routers.Get(i);
    PrintRoutingTable(router, std::cout, ipToNodeName);
}
   GenerateTrafficMatrix(hostNames,80.0);
    // PrintTrafficMatrix(/trafficMatrix);
    // Run the simulation
    Simulator::Stop(Seconds(60.0));
    Simulator::Run();
    PrintTrafficMatrix(trafficMatrix);
    Simulator::Destroy();
    return 0;
}
