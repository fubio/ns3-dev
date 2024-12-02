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

int main(int argc, char* argv[])
{
    bool tracing = false;
    uint32_t maxBytes = 62500;
    // Allow the user to override defaults via command-line arguments
    CommandLine cmd(__FILE__);
    cmd.AddValue("tracing", "Flag to enable/disable tracing", tracing);
    cmd.AddValue("maxBytes", "Total number of bytes for application to send", maxBytes);
    cmd.Parse(argc, argv);
    auto applicationsNode1 = std::vector<std::tuple<Ptr<BulkSendApplication>, uint64_t, Ptr<PacketSink>>>{};
    auto applicationsNode2 = std::vector<std::tuple<Ptr<BulkSendApplication>, uint64_t, Ptr<PacketSink>>>{};

    NS_LOG_INFO("Create nodes.");
    nodes.Create(2);

    NS_LOG_INFO("Create channels.");

    // Create point-to-point link
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("500Kbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("5ms"));

    NetDeviceContainer devices;
    devices = pointToPoint.Install(nodes);

    // Install the internet stack
    InternetStackHelper internet;
    internet.Install(nodes);

    NS_LOG_INFO("Assign IP Addresses.");
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    interfaces = ipv4.Assign(devices);

    NS_LOG_INFO("Create Applications.");

    // Create the first BulkSendApplication
    uint16_t port = 9;
    BulkSendHelper source("ns3::TcpSocketFactory", InetSocketAddress(interfaces.GetAddress(1), port));
    source.SetAttribute("MaxBytes", UintegerValue(maxBytes));
    ApplicationContainer sourceApps = source.Install(nodes.Get(0));

    // Create the first PacketSinkApplication
    PacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApps = sink.Install(nodes.Get(1));
    std::cout << "got past socket factory" << std::endl;
    // Cast and insert into applicationsNode1
    Ptr<BulkSendApplication> bulkSendApp1 = DynamicCast<BulkSendApplication>(sourceApps.Get(0));
    Ptr<PacketSink> packetSink1 = DynamicCast<PacketSink>(sinkApps.Get(0));
    std::cout << "got past dynamic cast" << std::endl;
    // if (!bulkSendApp1 || !packetSink1) {
    //     std::cerr << "Casting to BulkSendApplication or PacketSink failed for Application Set 1." << std::endl;
    //     NS_LOG_ERROR("Casting to BulkSendApplication or PacketSink failed for Application Set 1.");
    //     return 1;
    // }
    // std::cout << "worked so far" << std::endl;

    applicationsNode1.push_back(std::make_tuple(bulkSendApp1, maxBytes, packetSink1));
    applicationsNode2.push_back(std::make_tuple(nullptr, 0, nullptr));
    std::cout << "inserted into applicationsNode1" << std::endl;
    // Do the same thing but opposite
    port = 10; // Different port for the second application
    source = BulkSendHelper("ns3::TcpSocketFactory", InetSocketAddress(interfaces.GetAddress(0), port));
    source.SetAttribute("MaxBytes", UintegerValue(maxBytes));
    sourceApps = source.Install(nodes.Get(1));

    auto bulkSendApp2 = DynamicCast<BulkSendApplication>(sourceApps.Get(0));
auto packetSink2 = DynamicCast<PacketSink>(sinkApps.Get(0));

if (!bulkSendApp2 || !packetSink2) {
    std::cerr << "Casting to BulkSendApplication or PacketSink failed for Application Set 2." << std::endl;
    NS_LOG_ERROR("Casting to BulkSendApplication or PacketSink failed for Application Set 2.");
    return 1;
} else {
    std::cout << "worked so far!!!" << std::endl;
}
    // Create the first PacketSinkApplication
    sink = PacketSinkHelper("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
    sinkApps = sink.Install(nodes.Get(0));

    applicationsNode2.push_back({DynamicCast<BulkSendApplication>(sourceApps.Get(0)), maxBytes, DynamicCast<PacketSink>(sinkApps.Get(0))});
    applicationsNode1.push_back({nullptr, 0, nullptr});

    MultiBulkSendApplication multiBulkSendAppNode1;
    multiBulkSendAppNode1.SetBulkSendSizes(applicationsNode1);
    nodes.Get(0)->AddApplication(&multiBulkSendAppNode1);
    multiBulkSendAppNode1.SetStartTime(Seconds(0.0));
    multiBulkSendAppNode1.SetStopTime(Seconds(10.0));

    MultiBulkSendApplication multiBulkSendAppNode2;
    multiBulkSendAppNode2.SetBulkSendSizes(applicationsNode2);
    nodes.Get(1)->AddApplication(&multiBulkSendAppNode2);
    multiBulkSendAppNode2.SetStartTime(Seconds(0.0));
    multiBulkSendAppNode2.SetStopTime(Seconds(10.0));

    // // Connect the callbacks
    // Ptr<BulkSendApplication> bulkSendApp = DynamicCast<BulkSendApplication>(sourceApps.Get(0));
    // bulkSendApp->TraceConnectWithoutContext("Tx", MakeCallback(&PacketSentCallback));

    // Ptr<PacketSink> packetSink = DynamicCast<PacketSink>(sinkApps.Get(0));
    // packetSink->TraceConnectWithoutContext("Rx", MakeCallback(&PacketReceivedCallback));

    // // Set up tracing if enabled
    // if (tracing)
    // {
    //     AsciiTraceHelper ascii;
    //     pointToPoint.EnableAsciiAll(ascii.CreateFileStream("tcp-bulk-send.tr"));
    //     pointToPoint.EnablePcapAll("tcp-bulk-send", false);
    // }

    // NS_LOG_INFO("Run Simulation.");
    Simulator::Stop(Seconds(20)); // Extend simulation time
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Done.");

    // // Output statistics
    // Ptr<PacketSink> sink1 = DynamicCast<PacketSink>(sinkApps.Get(0));
    // std::cout << "Total Bytes Received: " << sink1->GetTotalRx() << std::endl;

    return 0;
}