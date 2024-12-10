#include "simple-multi-bulk-send-application.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SimpleMultiBulkSendApplication");

NS_OBJECT_ENSURE_REGISTERED(SimpleMultiBulkSendApplication);
// Define the callback function

void SimpleMultiBulkSendApplication::PacketSentCallback(Ptr<const Packet> packet)
{
    m_packetsSent++;
    // NS_LOG_UNCOND("Packet sent at " << Simulator::Now().GetSeconds() << "s, Size: " << packet->GetSize() << " bytes. From Node: " << m_node->GetId());
}
void SimpleMultiBulkSendApplication::PacketRecievedCallback(Ptr<const Packet> packet, const Address& address)
// void SimpleMultiBulkSendApplication::PacketRecievedCallback(Ptr<const Packet> packet)
{
    m_packetsReceived++;
    // NS_LOG_UNCOND("Packet received: " << packet->GetSize() << " bytes. Sent from Node" << m_node->GetId() << " total bytes received: " << m_totalBytesReceived);
    // NS_LOG_UNCOND("Packet received: " << packet->GetSize() << " num packets received: " << m_packetsReceived << " total bytes received: " << m_totalBytesReceived << " total sent: " << m_packetsSent);
    m_totalBytesReceived += packet->GetSize();
    if (m_totalBytesReceived >= m_bulkSendSize)
    {
    // if (m_packetsReceived >= m_packetsSent && m_totalBytesReceived >= m_bulkSendSize)
    // {
        BulkSendSyncManager::GetInstance().NotifyCompletion(0);
        // NS_LOG_UNCOND("mbulksendSize: "<< m_bulkSendSize);
        // NS_LOG_UNCOND("Bulk send complete for step " << m_currentBulkSendIndex << " and from node " << m_node->GetId() << ". Total bytes received: " << m_totalBytesReceived << " at time " << Simulator::Now().GetSeconds() << "ms");
    }
    // NS_LOG_INFO("Packet received: " << packet->GetSize() << " bytes");
}

TypeId
SimpleMultiBulkSendApplication::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SimpleMultiBulkSendApplication")
                            .SetParent<Application>()
                            .SetGroupName("Applications")
                            .AddConstructor<SimpleMultiBulkSendApplication>();
    return tid;
}

SimpleMultiBulkSendApplication::SimpleMultiBulkSendApplication()
    : m_currentBulkSendIndex(0)
{
    NS_LOG_FUNCTION(this);
}

SimpleMultiBulkSendApplication::~SimpleMultiBulkSendApplication()
{
    NS_LOG_FUNCTION(this);
}

void SimpleMultiBulkSendApplication::SetBulkSendSizes(const std::vector<std::tuple<Ptr<BulkSendApplication>, uint64_t, Ptr<PacketSink>, NetDeviceContainer>>& bulkSends) {
    m_bulkSendSizes = bulkSends;
}

void
SimpleMultiBulkSendApplication::StartApplication()
{
    // NS_LOG_UNCOND("Starting MultiBulkSendApplication on Node: " << m_node->GetId());
    if (m_bulkSendSizes.empty())
    {
        NS_LOG_ERROR("BulkSendSizes vector is empty. No data to send.");
        return;
    } else {
        BulkSendSyncManager::GetInstance().RegisterNode(MakeCallback(&SimpleMultiBulkSendApplication::ScheduleNextBulkSend, this));
    }
    // NS_LOG_UNCOND("size of bulsend: " << m_bulkSendSizes.size());
    SimpleMultiBulkSendApplication::ScheduleNextBulkSend();

}

void SimpleMultiBulkSendApplication::StopApplication()
{
    NS_LOG_FUNCTION(this);
}

void
SimpleMultiBulkSendApplication::ScheduleNextBulkSend()
{
    m_packetsReceived = 0;
    m_packetsSent = 0;
    if (m_sink)
    {
        m_sink->TraceDisconnectWithoutContext("Rx", MakeCallback(&SimpleMultiBulkSendApplication::PacketRecievedCallback, this));
    }
    m_totalBytesReceived = 0;
    if (m_currentBulkSendIndex >= m_bulkSendSizes.size())
    {
        // NS_LOG_UNCOND("All bulk sends complete.");
        std::cout << Simulator::Now().GetSeconds() << std::endl;
        Simulator::Stop();
    } else {
        // auto [bulkSendApp, m_bulkSendSize, m_sink] = m_bulkSendSizes[m_currentBulkSendIndex];
        // Destructure into temporary local variables with different names
        auto [bulkSendApp, tempBulkSendSize, tempSink, devices] = m_bulkSendSizes[m_currentBulkSendIndex];
        // Assign to member variables explicitly
        m_bulkSendSize = tempBulkSendSize;
        m_sink = tempSink;
        // NS_LOG_UNCOND("Starting bulk send #" << m_currentBulkSendIndex << " at time " << Simulator::Now().GetSeconds() << "of size " << tempBulkSendSize << " bytes. From Node: " << m_node->GetId());
        if (m_bulkSendSize == 0)
        {
            // NS_LOG_UNCOND("Bulk send size is 0. Skipping.");
            BulkSendSyncManager::GetInstance().NotifyCompletion(0);
        } else {
            bulkSendApp->TraceConnectWithoutContext("Tx", MakeCallback(&SimpleMultiBulkSendApplication::PacketSentCallback, this));
            bulkSendApp->SetMaxBytes(tempBulkSendSize);
            tempSink->TraceConnectWithoutContext("Rx", MakeCallback(&SimpleMultiBulkSendApplication::PacketRecievedCallback, this));
            // devices.Get(0)->TraceConnectWithoutContext("PhyTxEnd", MakeCallback(&SimpleMultiBulkSendApplication::PacketSentCallback, this));
            // devices.Get(1)->TraceConnectWithoutContext("PhyRxEnd", MakeCallback(&SimpleMultiBulkSendApplication::PacketRecievedCallback, this));
            bulkSendApp->SetStartTime(Simulator::Now());
            bulkSendApp->SetStopTime(Simulator::Now() + Seconds(10));
            tempSink->SetStartTime(Simulator::Now());
            tempSink->SetStopTime(Simulator::Now() + Seconds(10));
            // NS_LOG_UNCOND("finished setting up bulk send #" << m_currentBulkSendIndex << "with sink on " << tempSink->GetNode()->GetId());
        }
    }
    m_currentBulkSendIndex++;
}

} // namespace ns3