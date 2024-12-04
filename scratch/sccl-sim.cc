#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/packet-sink.h"
// #include "ns3/multi-bulk-send-application.h"

#include <fstream>
#include <string>
using json = nlohmann::json;

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ScclSim");

NodeContainer nodes;
Ipv4InterfaceContainer interfaces;

int main(int argc, char* argv[])
{
    const int maxNodes = 100;
    std::string dataRate = "500Kbps";
    std::string delay = "5ms";
    uint32_t dataSize = 62500;
    CommandLine cmd(__FILE__);
    cmd.AddValue("dataSize", "Total number of bytes for application to send", dataSize);
    cmd.AddValue("dataRate", "Rate a which data can be sent across each channel", dataRate);
    cmd.AddValue("delta", "propogation delay across each channel", delay);
    cmd.Parse(argc, argv);
    // read in the json file
	std::ifstream f("scratch/test.json", std::ifstream::in);
    if (!f.is_open()) {
        std::cerr << "Failed to open test.json" << std::endl;
        return 1;  // Or handle error as needed
    }
    json j; //create unitiialized json object

    f >> j; // initialize json object with what was read from file
    auto topologyName = j.at("topology").at("name").get<std::string>();
    auto topology = j.at("topology").at("links");
    int numNodes = topology.size();
    if (numNodes > maxNodes) {
        std::cerr << "Number of nodes exceeds maximum number of nodes" << std::endl;
        return 1;
    }

    //Create vector with tuples which contains the applications and send sizes at each step
    std::vector<std::tuple<Ptr<BulkSendApplication>, uint64_t, Ptr<PacketSink>>> applicationArr[maxNodes];

    // Create point-to-point link
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue(dataRate));
    pointToPoint.SetChannelAttribute("Delay", StringValue(delay));

    //Creates a node container with numNodes nodes
    nodes.Create(numNodes);

    // Assign IP addresses
    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");

    std::list<std::tuple<NetDeviceContainer, Ipv4InterfaceContainer>> devicesArr[maxNodes][maxNodes];
    std::cout << numNodes << std::endl;
    int i = 0;
    for (auto top : topology) {
        int j = 0;
        for (int numConnection : top){
            // Don't create duplicate connections or connections to itself
            if (i < j) {
                for (int k = 0; k < numConnection; k++){
                    auto devices = pointToPoint.Install(nodes.Get(i), nodes.Get(j));
                    interfaces = ipv4.Assign(devices);
                    devicesArr[i][j].push_back(std::make_tuple(devices, interfaces));
                    ipv4.NewNetwork();
                }
            }
            j++;
        }
        i++;
    }


	return 0;
}