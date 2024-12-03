// Filename: tcp-bulk-send.cc

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/packet-sink.h"

#include <fstream>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpBulkSendExample");

// Global variables
Time firstPacketSentTime = Seconds(0);
Time lastPacketReceivedTime = Seconds(0);
uint64_t totalBytesSent = 0;
uint32_t maxBytes = 62500; // Set to 2 million bytes
bool secondBulkSendScheduled = false;

// Forward declaration of the function to start the second bulk send
void StartSecondBulkSend();

void PacketSentCallback(Ptr<const Packet> packet)
{
    if (firstPacketSentTime == Seconds(0))
    {
        firstPacketSentTime = Simulator::Now();
    }
    totalBytesSent += packet->GetSize();
    if (totalBytesSent >= maxBytes && !secondBulkSendScheduled)
    {
        secondBulkSendScheduled = true;
        NS_LOG_UNCOND("starting second bulk send at " << Simulator::Now().GetSeconds() << " seconds");
        Simulator::ScheduleNow(&StartSecondBulkSend);
    }
}

// Add these global variables
static uint32_t totalBytesReceived = 0;
static uint32_t roundsCompleted = 0;

// Modify the PacketReceivedCallback function
void PacketReceivedCallback(Ptr<const Packet> packet, const Address &address) {
    totalBytesReceived += packet->GetSize();

    // Define the byte threshold for each round
    uint32_t bytesPerRound = 62500; // Adjust based on your application's round size

    if (totalBytesReceived >= bytesPerRound * (roundsCompleted + 1)) {
        roundsCompleted++;
        std::cout << "After round " << roundsCompleted << ", bytes received: " << totalBytesReceived
                  << " at time: " << Simulator::Now().GetSeconds() << " seconds." << std::endl;
    }
}
// void PacketReceivedCallback(Ptr<const Packet> packet, const Address& address)
// {
//     lastPacketReceivedTime = Simulator::Now();
//     // Calculate the data transfer duration
//     Time transferDuration = lastPacketReceivedTime - firstPacketSentTime;
//     // std::cout << "Data Transfer Duration: " << transferDuration.GetSeconds() << " seconds" << std::endl;
// }

NodeContainer nodes;
Ipv4InterfaceContainer interfaces;

void StartSecondBulkSend()
{
    uint16_t port = 10; // Different port for the second application

    // Create the second BulkSendApplication
    BulkSendHelper source("ns3::TcpSocketFactory", InetSocketAddress(interfaces.GetAddress(1), port));
    uint32_t secondMaxBytes = 62500; // Another 2 million bytes
    source.SetAttribute("MaxBytes", UintegerValue(secondMaxBytes));
    ApplicationContainer sourceApps = source.Install(nodes.Get(0));
    sourceApps.Start(Simulator::Now());
    sourceApps.Stop(Simulator::Now() + Seconds(10));

    // Create the second PacketSinkApplication
    PacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApps = sink.Install(nodes.Get(1));
    sinkApps.Start(Simulator::Now());
    sinkApps.Stop(Simulator::Now() + Seconds(10));

    // Connect the callbacks
    Ptr<BulkSendApplication> bulkSendApp = DynamicCast<BulkSendApplication>(sourceApps.Get(0));
    bulkSendApp->TraceConnectWithoutContext("Tx", MakeCallback(&PacketSentCallback));

    Ptr<PacketSink> packetSink = DynamicCast<PacketSink>(sinkApps.Get(0));
    packetSink->TraceConnectWithoutContext("Rx", MakeCallback(&PacketReceivedCallback));
}

int main(int argc, char* argv[])
{
    bool tracing = false;
    // u_int32_t maxBytes = 62500; // Set to 62,500 bytes

    // Allow the user to override defaults via command-line arguments
    CommandLine cmd(__FILE__);
    cmd.AddValue("tracing", "Flag to enable/disable tracing", tracing);
    cmd.AddValue("maxBytes", "Total number of bytes for application to send", maxBytes);
    cmd.Parse(argc, argv);

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
    //configuring sending to node one
    BulkSendHelper source("ns3::TcpSocketFactory", InetSocketAddress(interfaces.GetAddress(1), port));
    source.SetAttribute("MaxBytes", UintegerValue(maxBytes));
    ApplicationContainer sourceApps = source.Install(nodes.Get(0));
    sourceApps.Start(Seconds(0));
    sourceApps.Stop(Seconds(10));

    // Create the first PacketSinkApplication
    PacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
    // PacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(interfaces.GetAddress(1), port));
    ApplicationContainer sinkApps = sink.Install(nodes.Get(1));
    sinkApps.Start(Seconds(0));
    sinkApps.Stop(Seconds(10));

    // Connect the callbacks
    Ptr<BulkSendApplication> bulkSendApp = DynamicCast<BulkSendApplication>(sourceApps.Get(0));
    bulkSendApp->TraceConnectWithoutContext("Tx", MakeCallback(&PacketSentCallback));

    Ptr<PacketSink> packetSink = DynamicCast<PacketSink>(sinkApps.Get(0));
    packetSink->TraceConnectWithoutContext("Rx", MakeCallback(&PacketReceivedCallback));

    // Set up tracing if enabled
    if (tracing)
    {
        AsciiTraceHelper ascii;
        pointToPoint.EnableAsciiAll(ascii.CreateFileStream("tcp-bulk-send.tr"));
        pointToPoint.EnablePcapAll("tcp-bulk-send", false);
    }

    NS_LOG_INFO("Run Simulation.");
    Simulator::Stop(Seconds(20)); // Extend simulation time
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Done.");

    // Output statistics
    Ptr<PacketSink> sink1 = DynamicCast<PacketSink>(sinkApps.Get(0));
    std::cout << "Total Bytes Received: " << sink1->GetTotalRx() << std::endl;
    std::cout << "Total time: " << lastPacketReceivedTime.GetSeconds() << " seconds" << std::endl;

    return 0;
}