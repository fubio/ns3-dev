#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/packet-sink.h"
#include "ns3/point-to-point-module.h"
// #include "ns3/multi-bulk-send-application.h"

#include <fstream>
#include <string>
using json = nlohmann::json;

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ScclSim");

NodeContainer nodes;
Ipv4InterfaceContainer interfaces;

int
main(int argc, char* argv[])
{
    const int maxLinksPerNodes = 10;
    const int maxNodes = 100;
    const int maxSteps = 50;
    const int maxSendsPerStep = 100;
    std::string dataRate = "500Kbps";
    std::string delay = "5ms";
    uint32_t dataSize = 62500;
    CommandLine cmd(__FILE__);
    cmd.AddValue("dataSize", "Total number of bytes for application to send", dataSize);
    cmd.AddValue("dataRate", "Rate a which data can be sent across each channel", dataRate);
    cmd.AddValue("delta", "propogation delay across each channel", delay);
    cmd.Parse(argc, argv);
    // read in the json file
    std::ifstream f("scratch/Broadcast.json", std::ifstream::in);
    if (!f.is_open())
    {
        std::cerr << "Failed to open test.json" << std::endl;
        return 1; // Or handle error as needed
    }
    json j; // create unitiialized json object

    f >> j; // initialize json object with what was read from file
    // define variables to hold the values of parts of the json object
    auto topologyName = j.at("topology").at("name").get<std::string>();
    auto topology = j.at("topology").at("links");
    auto steps = j.at("steps");
    int numNodes = topology.size();
    if (numNodes > maxNodes)
    {
        std::cerr << "Number of nodes exceeds maximum number of nodes" << std::endl;
        return 1;
    }

    // Create vector with tuples which contains the applications and send sizes at each step
    std::vector<std::vector<std::vector<std::tuple<Ptr<BulkSendApplication>, uint64_t, Ptr<PacketSink>, uint64_t>>>>
        applicationArr(maxNodes, std::vector<std::vector<std::tuple<Ptr<BulkSendApplication>, uint64_t, Ptr<PacketSink>, uint64_t>>>(maxSteps, std::vector<std::tuple<Ptr<BulkSendApplication>, uint64_t, Ptr<PacketSink>, uint64_t>>(maxSendsPerStep)));
    std::cout << "Topology name: " << topologyName << std::endl;
    // Create point-to-point link
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue(dataRate));
    pointToPoint.SetChannelAttribute("Delay", StringValue(delay));

    // Creates a node container with numNodes nodes
    nodes.Create(numNodes);

    // Assign IP addresses
    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");

    // for each i,j and direction in topology, create a point to point link (unidirectional) between
    // nodes i and j we have two links for each pair of nodes which creates a bidirectional link
    std::tuple<NetDeviceContainer, Ipv4InterfaceContainer> devicesArr[maxNodes][maxNodes]
                                                                     [maxLinksPerNodes];
    std::cout << numNodes << std::endl;
    int i = 0;
    for (auto top : topology)
    {
        int j = 0;
        for (int numConnection : top)
        {
            for (int k = 0; k < numConnection; k++)
            {
                auto devices = pointToPoint.Install(nodes.Get(i), nodes.Get(j));
                interfaces = ipv4.Assign(devices);
                devicesArr[i][j][k] = std::make_tuple(devices, interfaces);
                ipv4.NewNetwork();
            }
            j++;
        }
        i++;
    }
    uint16_t port = 1;
    // Create Applications TODO look into rounds
    int stepNum = 0;
    for (auto step : steps)
    {
        std::cout << "stepNum: " << stepNum << std::endl;
        int rounds = step.at("rounds").get<int>();
        auto sends = step.at("sends");
        int sendCountArr[maxNodes][maxNodes] = {{0}};
        for (auto send : sends)
        {
            // Get the source and destination of the send
            std::cout << "getting source and dest" << send << std::endl;
            int src = send[1].get<int>();
            int dest = send[2].get<int>();
            sendCountArr[src][dest]++;

            std::cout << "source: " << src << " dest: " << dest << std::endl;
            std::cout << "arr[0][0]" << sendCountArr[0][0] << std::endl;
        }
        int sendNum = 0;
        for (int src = 0; src < numNodes; src++)
        {
            for (int dest = 0; dest < numNodes; dest++)
            {
                auto count = sendCountArr[src][dest];
                if (count == 0)
                {
                    continue;
                }
                std::cout << "getting src: " << src << " dest: " << dest << std::endl;
                auto numLinks = topology[src][dest].get<int>();
                std::cout << "numLinks: " << numLinks << std::endl;
                if (count != 0 && static_cast<float>(count) / static_cast<float>(numLinks) >
                    static_cast<float>(rounds))
                {
                    std::cerr
                        << "Number of sends per round exceeds number of links availible per round, on count " << count << " source: " << src << " dest: " << dest << "with links: " << numLinks
                        << std::endl;
                    return 1;
                }
                uint32_t dataToSend[maxLinksPerNodes] = {0};
                for (int i = 0; i < std::min(numLinks, count) && count > 0; i++)
                {
                    dataToSend[i] += dataSize;
                    count -= numLinks;
                }
                for (int i = 0; i < numLinks; i++)
                {
                    std::cout << " getting src: " << src << " dest " << dest << " i/k " << i <<" sending bytes: " <<dataToSend[i] << std::endl;
                    auto [devices, interfaces] = devicesArr[src][dest][i];
                    BulkSendHelper source("ns3::TcpSocketFactory",
                                          InetSocketAddress(interfaces.GetAddress(1), port));
                    ApplicationContainer sourceApps = source.Install(nodes.Get(src));
                    // Create the first PacketSinkApplication
                    PacketSinkHelper sink("ns3::TcpSocketFactory",
                                          InetSocketAddress(Ipv4Address::GetAny(), port));
                    ApplicationContainer sinkApps = sink.Install(nodes.Get(dest));
                    Ptr<BulkSendApplication> bulkSendApp =
                        DynamicCast<BulkSendApplication>(sourceApps.Get(0));
                    Ptr<PacketSink> packetSink = DynamicCast<PacketSink>(sinkApps.Get(0));
                    applicationArr[src][stepNum][sendNum] =
                        std::make_tuple(bulkSendApp, dataToSend[i], packetSink, dest);
                    port++;
    
                }
                sendNum++;
            }

            // std::cout << "source: " << std::get<0>(key) << " dest: " << std::get<1>(key)
            //           << " count: " << value << std::endl;
            
        }
        sendNum = 0;
        stepNum++;
    }
    BulkSendSyncManager::GetInstance().SetTotalNodes(numNodes);
    for (int i = 0; i < numNodes; i++) {
            Ptr<MultiBulkSendApplication> multiBulkSendAppNode = CreateObject<MultiBulkSendApplication>();
            multiBulkSendAppNode->SetBulkSendSizes(applicationArr[i]);
            multiBulkSendAppNode->SetSteps(steps.size());
            nodes.Get(i)->AddApplication(multiBulkSendAppNode);
            multiBulkSendAppNode->SetStartTime(Seconds(0.0));
            multiBulkSendAppNode->SetStopTime(Seconds(10.0));
    }

    for (int j = 0;j < 5; j++) {
        for(int i = 0; i < 5; i++) {
            auto [bulkSendApp, tempBulkSendSize, tempSink, destNode] = applicationArr[1][j][i];
            std::cout << "bulkSendSize: " << tempBulkSendSize << " destNode: " << destNode << std::endl;
        }
    }
    Simulator::Stop(Seconds(20)); // Extend simulation time
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Done.");
    return 0;
}