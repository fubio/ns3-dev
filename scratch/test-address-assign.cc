#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

int main() {
  // Step 1: Create nodes
  NodeContainer nodesAB, nodesBC;
  nodesAB.Create(2); // Nodes A and B
  nodesBC.Add(nodesAB.Get(1)); // Add Node B
  nodesBC.Create(1); // Node C

  // Step 2: Create Point-to-Point links
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
  pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

  NetDeviceContainer devicesAB = pointToPoint.Install(nodesAB);
  NetDeviceContainer devicesBC = pointToPoint.Install(nodesBC);

  // Step 3: Install Internet Stack
  InternetStackHelper stack;
  stack.Install(nodesAB);
  stack.Install(nodesBC.Get(1)); // Install on Node C

  // Step 4: Assign IP Addresses
  Ipv4AddressHelper address;
  address.SetBase("192.168.0.0", "255.255.255.0");
  Ipv4InterfaceContainer interfacesAB = address.Assign(devicesAB);

  address.SetBase("192.168.2.0", "255.255.255.0");
  Ipv4InterfaceContainer interfacesBC = address.Assign(devicesBC);

  // Print IP Addresses (Optional)
  NS_LOG_UNCOND("A <-> B: " << interfacesAB.GetAddress(0) << " <-> " << interfacesAB.GetAddress(1));
  NS_LOG_UNCOND("B <-> C: " << interfacesBC.GetAddress(0) << " <-> " << interfacesBC.GetAddress(1));

  // Step 5: Enable routing (if needed)
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  // Step 6: Run simulation
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
