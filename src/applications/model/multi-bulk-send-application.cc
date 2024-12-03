#include "multi-bulk-send-application.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("MultiBulkSendApplication");

NS_OBJECT_ENSURE_REGISTERED(MultiBulkSendApplication);
// Define the callback function

void MultiBulkSendApplication::PacketSentCallback(Ptr<const Packet> packet)
{
    NS_LOG_UNCOND("Packet sent at " << Simulator::Now().GetSeconds() << "s, Size: " << packet->GetSize() << " bytes. From Node: " << m_node->GetId());
}
void MultiBulkSendApplication::PacketRecievedCallback(Ptr<const Packet> packet, const Address& address)
{
    NS_LOG_UNCOND("Packet received: " << packet->GetSize() << " bytes. Sent from Node" << m_node->GetId());
    m_totalBytesReceived += packet->GetSize();
    if (m_totalBytesReceived >= m_bulkSendSize)
    {
        BulkSendSyncManager::GetInstance().NotifyCompletion();
        NS_LOG_UNCOND("mbulksendSize: "<< m_bulkSendSize);
        NS_LOG_UNCOND("Bulk send complete for step " << m_currentBulkSendIndex << " and from node " << m_node->GetId() << ". Total bytes received: " << m_totalBytesReceived << " at time " << Simulator::Now().GetSeconds());
    }
    // NS_LOG_INFO("Packet received: " << packet->GetSize() << " bytes");
}

TypeId
MultiBulkSendApplication::GetTypeId()
{
    static TypeId tid = TypeId("ns3::MultiBulkSendApplication")
                            .SetParent<BulkSendApplication>()
                            .SetGroupName("Applications")
                            .AddConstructor<MultiBulkSendApplication>();
    return tid;
}

MultiBulkSendApplication::MultiBulkSendApplication()
    : m_currentBulkSendIndex(0)
{
    NS_LOG_FUNCTION(this);
}

MultiBulkSendApplication::~MultiBulkSendApplication()
{
    NS_LOG_FUNCTION(this);
}

void MultiBulkSendApplication::SetBulkSendSizes(const std::vector<std::tuple<Ptr<BulkSendApplication>, uint64_t, Ptr<PacketSink>>>& bulkSends) {
    m_bulkSendSizes = bulkSends;
}

void
MultiBulkSendApplication::StartApplication()
{
    NS_LOG_UNCOND("Starting MultiBulkSendApplication on Node: " << m_node->GetId());
    if (m_bulkSendSizes.empty())
    {
        NS_LOG_ERROR("BulkSendSizes vector is empty. No data to send.");
        return;
    } else {
        BulkSendSyncManager::GetInstance().RegisterNode(MakeCallback(&MultiBulkSendApplication::ScheduleNextBulkSend, this));
    }
    MultiBulkSendApplication::ScheduleNextBulkSend();

}

void MultiBulkSendApplication::StopApplication()
{
    NS_LOG_FUNCTION(this);
}

// void
// MultiBulkSendApplication::SendData()
// {
//     BulkSendApplication::SendData();

//     // Check if the current bulk send is complete
//     if (m_totBytes >= m_maxBytes)
//     {
//         ScheduleNextBulkSend();
//     }
// }

void
MultiBulkSendApplication::ScheduleNextBulkSend()
{
    m_totalBytesReceived = 0;
    if (m_currentBulkSendIndex >= m_bulkSendSizes.size())
    {
        NS_LOG_UNCOND("All bulk sends complete.");
        return;
    } else {
        // auto [bulkSendApp, m_bulkSendSize, m_sink] = m_bulkSendSizes[m_currentBulkSendIndex];
        // Destructure into temporary local variables with different names
        auto [bulkSendApp, tempBulkSendSize, tempSink] = m_bulkSendSizes[m_currentBulkSendIndex];
        // Assign to member variables explicitly
        m_bulkSendSize = tempBulkSendSize;
        NS_LOG_UNCOND("Starting bulk send #" << m_currentBulkSendIndex << " at time " << Simulator::Now().GetSeconds() << "of size " << tempBulkSendSize << " bytes. From Node: " << m_node->GetId());
        if (m_bulkSendSize == 0)
        {
            NS_LOG_UNCOND("Bulk send size is 0. Skipping.");
            BulkSendSyncManager::GetInstance().NotifyCompletion();
        } else {
            bulkSendApp->TraceConnectWithoutContext("Tx", MakeCallback(&MultiBulkSendApplication::PacketSentCallback, this));
            bulkSendApp->SetMaxBytes(m_bulkSendSize);
            tempSink->TraceConnectWithoutContext("Rx", MakeCallback(&MultiBulkSendApplication::PacketRecievedCallback, this));
            bulkSendApp->SetStartTime(Simulator::Now());
            bulkSendApp->SetStopTime(Simulator::Now() + Seconds(10));
            tempSink->SetStartTime(Simulator::Now());
            tempSink->SetStopTime(Simulator::Now() + Seconds(10));
            NS_LOG_UNCOND("finished setting up bulk send #" << m_currentBulkSendIndex << "with sink on " << tempSink->GetNode()->GetId());
        }
    }
    m_currentBulkSendIndex++;
}

} // namespace ns3