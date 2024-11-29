#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/queue.h"
#include <fstream>

using namespace ns3;
std::ofstream logFile;
// Function to log queue lengths
void LogQueueLength(Ptr<Queue<Packet>> queue, std::string linkDescription) {
    uint32_t queueLength = queue->GetNPackets();
    logFile << Simulator::Now().GetSeconds() << "s: " << linkDescription
              << " Queue Length: " << queueLength << " packets" << std::endl;
}

int main(int argc, char *argv[]) {
    CommandLine cmd;
    cmd.Parse(argc, argv);

    logFile.open("queue_lengths.txt", std::ios::out);
    if (!logFile.is_open()) {
        std::cerr << "Error: Could not open log file!" << std::endl;
        return 1;
    }

    // Set up nodes
    NodeContainer hosts;
    hosts.Create(7); // Nodes A-G
    NodeContainer routers;
    routers.Create(4); // Routers R1-R4

    InternetStackHelper stack;
    stack.Install(hosts);
    stack.Install(routers);

    // Host-to-router links with different data rates
    std::string hostToRouterRates[7] = {"1Mbps", "2Mbps", "1.5Mbps", "3Mbps", "1Mbps", "2Mbps", "2.5Mbps"};
    NetDeviceContainer hostDevices[7];
    for (uint32_t i = 0; i < 7; ++i) {
        PointToPointHelper p2p;
        p2p.SetChannelAttribute("Delay", StringValue("2ms"));
        p2p.SetDeviceAttribute("DataRate", StringValue(hostToRouterRates[i]));

        hostDevices[i] = p2p.Install(NodeContainer(hosts.Get(i), routers.Get(i % 4)));

        std::string hostToRouterLink =  "R" + std::to_string((i % 4) + 1) + " -> "  + " Host " + std::string(1, 'A' + i) ;
        Ptr<NetDevice> device = hostDevices[i].Get(0); // Outgoing device at host
        Ptr<PointToPointNetDevice> p2pDevice = DynamicCast<PointToPointNetDevice>(device);
        if (p2pDevice) {
            Ptr<Queue<Packet>> queue = p2pDevice->GetQueue();
            Simulator::Schedule(Seconds(1.0), &LogQueueLength, queue, hostToRouterLink);
            Simulator::Schedule(Seconds(2.0), &LogQueueLength, queue, hostToRouterLink);
        }
    }

    // Router-to-router links with different data rates
    std::string routerToRouterRates[4] = {"4Mbps", "5Mbps", "3.5Mbps", "4.5Mbps"};
    NetDeviceContainer routerDevices[4];
    for (uint32_t i = 0; i < 4; ++i) {
        PointToPointHelper p2p;
        p2p.SetChannelAttribute("Delay", StringValue("2ms"));
        p2p.SetDeviceAttribute("DataRate", StringValue(routerToRouterRates[i]));

        if (i == 0) routerDevices[i] = p2p.Install(NodeContainer(routers.Get(0), routers.Get(1))); // R1 -> R2
        else if (i == 1) routerDevices[i] = p2p.Install(NodeContainer(routers.Get(0), routers.Get(2))); // R1 -> R3
        else if (i == 2) routerDevices[i] = p2p.Install(NodeContainer(routers.Get(2), routers.Get(3))); // R3 -> R4
        else if (i == 3) routerDevices[i] = p2p.Install(NodeContainer(routers.Get(1), routers.Get(3))); // R2 -> R4

        std::string linkDescription;
        if (i == 0) linkDescription = "R1 -> R2";
        else if (i == 1) linkDescription = "R1 -> R3";
        else if (i == 2) linkDescription = "R3 -> R4";
        else if (i == 3) linkDescription = "R2 -> R4";

        Ptr<NetDevice> device = routerDevices[i].Get(0); // Outgoing device at router
        Ptr<PointToPointNetDevice> p2pDevice = DynamicCast<PointToPointNetDevice>(device);
        if (p2pDevice) {
            Ptr<Queue<Packet>> queue = p2pDevice->GetQueue();
            Simulator::Schedule(Seconds(1.0), &LogQueueLength, queue, linkDescription);
            Simulator::Schedule(Seconds(2.0), &LogQueueLength, queue, linkDescription);
            Simulator::Schedule(Seconds(3.0), &LogQueueLength, queue, linkDescription);
        }
    }

    // Install Internet stack
    Ipv4AddressHelper address;
    address.SetBase("10.1.0.0", "255.255.255.0");
    for (uint32_t i = 0; i < 7; ++i) {
        address.Assign(hostDevices[i]);
    }
    for (uint32_t i = 0; i < 4; ++i) {
        address.Assign(routerDevices[i]);
    }

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // UDP Echo Server and Clients
    UdpEchoServerHelper echoServer(9);
    for (uint32_t i = 0; i < 7; ++i) {
        echoServer.Install(hosts.Get(i));
    }

    UdpEchoClientHelper echoClient(Ipv4Address("10.1.0.1"), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(2000));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(0.01)));
    echoClient.SetAttribute("PacketSize", UintegerValue(2048));
    for (uint32_t i = 1; i < 7; ++i) {
        echoClient.Install(hosts.Get(i));
    }

    Simulator::Stop(Seconds(20.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
