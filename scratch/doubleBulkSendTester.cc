#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/packet-sink.h"
// #include "ns3/multi-bulk-send-application.h"

#include <fstream>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TestMultiBulkSend");

NodeContainer nodes;
Ipv4InterfaceContainer interfaces;

typedef std::tuple<Ptr<BulkSendApplication>, uint64_t, Ptr<PacketSink>, uint64_t> ApplicationTuple;
typedef std::vector<std::vector<std::vector<ApplicationTuple>>> ApplicationArr;
int main(int argc, char* argv[])
{
    bool tracing = false;
    uint32_t maxBytes = 62500;
    // Allow the user to override defaults via command-line arguments
    CommandLine cmd(__FILE__);
    cmd.AddValue("tracing", "Flag to enable/disable tracing", tracing);
    cmd.AddValue("maxBytes", "Total number of bytes for application to send", maxBytes);
    cmd.Parse(argc, argv);
    int maxNodes = 3;
    int maxSteps = 2;
    int maxSendsPerStep = 2;
    auto applicationsNode1 = std::vector<std::tuple<Ptr<BulkSendApplication>, uint64_t, Ptr<PacketSink>>>{};
    auto applicationsNode2 = std::vector<std::tuple<Ptr<BulkSendApplication>, uint64_t, Ptr<PacketSink>>>{};
    //arr[node][step][sends]
    ApplicationArr applicationArr(maxNodes, std::vector<std::vector<ApplicationTuple>>(maxSteps, std::vector<std::tuple<Ptr<BulkSendApplication>, uint64_t, Ptr<PacketSink>, uint64_t>>(maxSendsPerStep)));
    
    NS_LOG_INFO("Create nodes.");
    nodes.Create(3);

    NS_LOG_INFO("Create channels.");

    // Create point-to-point link
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("500Kbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("5ms"));

    NetDeviceContainer devices;
    //returns NetDeviceContainer
    NetDeviceContainer devicesAB = pointToPoint.Install(nodes.Get(0), nodes.Get(1));
    NetDeviceContainer devicesBC = pointToPoint.Install(nodes.Get(1), nodes.Get(2));
    NetDeviceContainer devicesAC = pointToPoint.Install(nodes.Get(0), nodes.Get(2));


    // Install the internet stack
    InternetStackHelper internet;
    internet.Install(nodes);

    NS_LOG_INFO("Assign IP Addresses.");
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("192.168.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfacesAB = ipv4.Assign(devicesAB);
    ipv4.SetBase("192.168.2.0", "255.255.255.0");
    Ipv4InterfaceContainer interfacesBC = ipv4.Assign(devicesBC);
    ipv4.SetBase("192.168.3.0", "255.255.255.0");
    Ipv4InterfaceContainer interfacesAC = ipv4.Assign(devicesAC);

    NS_LOG_UNCOND("A <-> B: " << interfacesAB.GetAddress(0) << " <-> " << interfacesAB.GetAddress(1));
  NS_LOG_UNCOND("B <-> C: " << interfacesBC.GetAddress(0) << " <-> " << interfacesBC.GetAddress(1));
  NS_LOG_UNCOND("C <-> A: " << interfacesAC.GetAddress(0) << " <-> " << interfacesAC.GetAddress(1));


    NS_LOG_INFO("Create Applications.");

    // Create the first BulkSendApplication for A->B    
    uint16_t port = 9;
    BulkSendHelper source("ns3::TcpSocketFactory", InetSocketAddress(interfacesAB.GetAddress(1), port));
    // source.SetAttribute("MaxBytes", UintegerValue(maxBytes));
    ApplicationContainer sourceAppsAB = source.Install(nodes.Get(0));

    // Create the first PacketSinkApplication
    PacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(interfacesAB.GetAddress(1), port));
    ApplicationContainer sinkAppsAB = sink.Install(nodes.Get(1));
    // Cast and insert into applicationsNode1
    Ptr<BulkSendApplication> bulkSendApp1 = DynamicCast<BulkSendApplication>(sourceAppsAB.Get(0));
    Ptr<PacketSink> packetSink1 = DynamicCast<PacketSink>(sinkAppsAB.Get(0));
    port = 10;
    //Creating A->C Applications
    BulkSendHelper sourceAC("ns3::TcpSocketFactory", InetSocketAddress(interfacesAC.GetAddress(1), port));
    ApplicationContainer sourceAppsCA = sourceAC.Install(nodes.Get(0));
    
    //create the first PacketSinkApplication
    PacketSinkHelper sinkAC("ns3::TcpSocketFactory", InetSocketAddress(interfacesAC.GetAddress(1), port));
    //install on node 2
    ApplicationContainer sinkAppsAC = sinkAC.Install(nodes.Get(2));
    Ptr<BulkSendApplication> bulkSendApp2 = DynamicCast<BulkSendApplication>(sourceAppsCA.Get(0));
    Ptr<PacketSink> packetSink2 = DynamicCast<PacketSink>(sinkAppsAC.Get(0));
    applicationArr[0][0] = {std::make_tuple(bulkSendApp1, maxBytes, packetSink1, 1), std::make_tuple(bulkSendApp2, maxBytes, packetSink2, 2)};
    applicationArr[1][0] = {std::make_tuple(nullptr, 0, nullptr, 1), std::make_tuple(nullptr, 0, nullptr, 1)};
    applicationArr[2][0] = {std::make_tuple(nullptr, 0, nullptr, 1), std::make_tuple(nullptr, 0, nullptr, 1)};
    std::cout << "inserted into applicationsNode1" << std::endl;

    port = 11; // Different port for the second application
    source = BulkSendHelper("ns3::TcpSocketFactory", InetSocketAddress(interfacesBC.GetAddress(1), port));
    // source.SetAttribute("MaxBytes", UintegerValue(maxBytes));
    ApplicationContainer sourceApps = source.Install(nodes.Get(1));

    // Create the first PacketSinkApplication
    sink = PacketSinkHelper("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApps = sink.Install(nodes.Get(2));
    
    Ptr<BulkSendApplication> bulkSendApp = DynamicCast<BulkSendApplication>(sourceApps.Get(0));
    Ptr<PacketSink> packetSink = DynamicCast<PacketSink>(sinkApps.Get(0));

    applicationArr[0][1] = {std::make_tuple(nullptr, 0, nullptr, 1), std::make_tuple(nullptr, 0, nullptr, 1)};
    applicationArr[1][1] = {std::make_tuple(bulkSendApp,maxBytes, packetSink, 2), std::make_tuple(nullptr, 0, nullptr, 1)};
    applicationArr[2][1] = {std::make_tuple(nullptr, 0, nullptr, 1), std::make_tuple(nullptr, 0, nullptr, 1)};
    BulkSendSyncManager::GetInstance().SetTotalNodes(3);

    Ptr<MultiBulkSendApplication> multiBulkSendAppNode1 = CreateObject<MultiBulkSendApplication>();
    multiBulkSendAppNode1->SetBulkSendSizes(applicationArr[0]);
    multiBulkSendAppNode1->SetSteps(2);
    nodes.Get(0)->AddApplication(multiBulkSendAppNode1);
    multiBulkSendAppNode1->SetStartTime(Seconds(0.0));
    multiBulkSendAppNode1->SetStopTime(Seconds(10.0));

    Ptr<MultiBulkSendApplication> multiBulkSendAppNode2 = CreateObject<MultiBulkSendApplication>();
    multiBulkSendAppNode2->SetBulkSendSizes(applicationArr[1]);
    multiBulkSendAppNode2->SetSteps(2);
    nodes.Get(1)->AddApplication(multiBulkSendAppNode2);
    multiBulkSendAppNode2->SetStartTime(Seconds(0.0));
    multiBulkSendAppNode2->SetStopTime(Seconds(10.0));

      Ptr<MultiBulkSendApplication> multiBulkSendAppNode3 = CreateObject<MultiBulkSendApplication>();
    multiBulkSendAppNode3->SetBulkSendSizes(applicationArr[2]);
    multiBulkSendAppNode3->SetSteps(2);
    nodes.Get(2)->AddApplication(multiBulkSendAppNode3);
    multiBulkSendAppNode3->SetStartTime(Seconds(0.0));
    multiBulkSendAppNode3->SetStopTime(Seconds(10.0));

    // NS_LOG_INFO("Run Simulation.");
    Simulator::Stop(Seconds(20)); // Extend simulation time
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Done.");

    // // // Output statistics
    // // Ptr<PacketSink> sink1 = DynamicCast<PacketSink>(sinkApps.Get(0));
    // // std::cout << "Total Bytes Received: " << sink1->GetTotalRx() << std::endl;

    return 0;
}