#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/queue.h"

using namespace ns3;

// Function to log queue lengths
void LogQueueLength(Ptr<Queue<Packet>> queue, std::string routerName, std::string linkName) {
    uint32_t queueLength = queue->GetNPackets();
    std::cout << Simulator::Now().GetSeconds() << "s: " << routerName << " -> " << linkName
              << " Queue Length: " << queueLength << " packets" << std::endl;
}

int main(int argc, char *argv[]) {
    CommandLine cmd;
    cmd.Parse(argc, argv);

    // Set up nodes
    NodeContainer hosts;
    hosts.Create(7); // Nodes A-G
    NodeContainer routers;
    routers.Create(4); // Routers R1-R4

    InternetStackHelper stack;
    stack.Install(hosts);
    stack.Install(routers);

    // Set up point-to-point links
    PointToPointHelper p2p;
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));
    p2p.SetDeviceAttribute("DataRate", StringValue("1Mbps"));

    // Host-to-router links
    NetDeviceContainer hostDevices[7];
    for (uint32_t i = 0; i < 7; ++i) {
        hostDevices[i] = p2p.Install(NodeContainer(hosts.Get(i), routers.Get(i % 4)));
    }

    // Router-to-router links
    NetDeviceContainer routerDevices[4];
    routerDevices[0] = p2p.Install(NodeContainer(routers.Get(0), routers.Get(1))); // R1 -> R2
    routerDevices[1] = p2p.Install(NodeContainer(routers.Get(0), routers.Get(2))); // R1 -> R3
    routerDevices[2] = p2p.Install(NodeContainer(routers.Get(2), routers.Get(3))); // R3 -> R4
    routerDevices[3] = p2p.Install(NodeContainer(routers.Get(1), routers.Get(3))); // R2 -> R4

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

    // Configure tracing for queue lengths
    for (uint32_t i = 0; i < 4; ++i) {
        Ptr<NetDevice> device = routerDevices[i].Get(0); // Outgoing device at router
        Ptr<PointToPointNetDevice> p2pDevice = DynamicCast<PointToPointNetDevice>(device);
        if (p2pDevice) {
            Ptr<Queue<Packet>> queue = p2pDevice->GetQueue();
            std::string routerName = "R" + std::to_string(i + 1);
            std::string linkName = "Link " + std::to_string(i + 1);

            Simulator::Schedule(Seconds(1.0), &LogQueueLength, queue, routerName, linkName);
            Simulator::Schedule(Seconds(2.0), &LogQueueLength, queue, routerName, linkName);
            Simulator::Schedule(Seconds(3.0), &LogQueueLength, queue, routerName, linkName);
            // Add more schedule calls if needed
        }
    }

    // UDP Echo Server and Clients
    UdpEchoServerHelper echoServer(9);
    for (uint32_t i = 0; i < 7; ++i) {
        echoServer.Install(hosts.Get(i));
    }

    UdpEchoClientHelper echoClient(Ipv4Address("10.1.0.1"), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(2000));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(0.0001)));
    echoClient.SetAttribute("PacketSize", UintegerValue(2048));
    for (uint32_t i = 1; i < 7; ++i) {
        echoClient.Install(hosts.Get(i));
    }

    Simulator::Stop(Seconds(10.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
