#include "multi-bulk-send-application.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("MultiBulkSendApplication");

NS_OBJECT_ENSURE_REGISTERED(MultiBulkSendApplication);

// Define the callback function

void
MultiBulkSendApplication::PacketSentCallback(Ptr<const Packet> packet)
{
    m_packetsSent++;
    NS_LOG_UNCOND("Packet sent at " << Simulator::Now().GetSeconds() << "s, Size: " << packet->GetSize() << " bytes. From Node: " << m_node->GetId());
    // packet->GetSize() << " bytes. From Node: " << m_node->GetId());
}

void
MultiBulkSendApplication::PacketRecievedCallback(Ptr<const Packet> packet, const Address& address)
{
    m_packetsReceived++;
    NS_LOG_UNCOND("Packet received at: " << Simulator::Now().GetSeconds() << "packet size: " << packet->GetSize() << " bytes. Sent from Node" << m_node->GetId() << " total bytes received: " << m_totalBytesReceived);
    // m_node->GetId());
    m_totalBytesReceived += packet->GetSize();
    if (m_totalBytesReceived >= m_bulkSendSize)
    {
        BulkSendSyncManager::GetInstance().NotifyCompletion(m_node->GetId());
        NS_LOG_UNCOND("Bulk send complete for step "
                      <<(m_currentBulkSendIndex - 1) << " and from node " << m_node->GetId()
                      << ". Total bytes received: " << m_totalBytesReceived << " at time "
                      << Simulator::Now().GetSeconds() <<  " requested size: " << m_bulkSendSize);
    }

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

void
MultiBulkSendApplication::SetBulkSendSizes(
    const std::vector<
        std::vector<std::tuple<Ptr<BulkSendApplication>, uint64_t, Ptr<PacketSink>, u_int64_t>>>&
        bulkSends)
{
    m_bulkSends = bulkSends;
}

void
MultiBulkSendApplication::StartApplication()
{
    // NS_LOG_UNCOND("Starting MultiBulkSendApplication on Node: " << m_node->GetId());
    if (m_bulkSends.empty())
    {
        // NS_LOG_ERROR("BulkSendSizes vector is empty. No data to send.");
        return;
    }
    else
    {
        BulkSendSyncManager::GetInstance().RegisterNode(
            MakeCallback(&MultiBulkSendApplication::ScheduleNextBulkSend, this));
    }
    MultiBulkSendApplication::ScheduleNextBulkSend();
}

void
MultiBulkSendApplication::StopApplication()
{
    NS_LOG_FUNCTION(this);
}

void MultiBulkSendApplication::SetSteps(uint64_t steps)
{
    m_steps = steps;
}
void
MultiBulkSendApplication::ScheduleNextBulkSend()
{
    m_packetsSent = 0;
    m_packetsReceived = 0;
     for (int i = 0; i < m_sink.size(); i++)
        {
            m_sink[i]->TraceDisconnectWithoutContext("Rx", MakeCallback(&MultiBulkSendApplication::PacketRecievedCallback, this));
        }
    m_sink.clear();
    m_totalBytesReceived = 0;
    m_bulkSendSize = 0;
    if (m_currentBulkSendIndex >= m_steps)
    {
        // NS_LOG_UNCOND("All bulk sends complete on Node: " << m_node->GetId() << " at time: " << Simulator::Now().GetSeconds() << " m_currentBulkSendIndex: " << m_currentBulkSendIndex << " m_steps: " << m_steps);
        NS_LOG_UNCOND(Simulator::Now().GetSeconds());
        Simulator::Stop();
    }
    else
    {
        // auto [bulkSendApp, m_bulkSendSize, m_sink] = m_bulkSendSizes[m_currentBulkSendIndex];
        // Destructure into temporary local variables with different names
        auto bulkSends = m_bulkSends[m_currentBulkSendIndex];
        bool bulkSendScheduled = false;
        for (int i = 0; i < bulkSends.size(); i++)
        {
            
            auto [bulkSendApp, tempBulkSendSize, tempSink, destNode] = bulkSends[i];
            m_bulkSendSize += tempBulkSendSize;
        }
        // std::cout <<"Node: " << m_node->GetId() <<  " m_bulkSendSize: " << m_bulkSendSize << std::endl;

        for (int i = 0; i < bulkSends.size(); i++)
        {
            
            auto [bulkSendApp, tempBulkSendSize, tempSink, destNode] = bulkSends[i];

            if (m_bulkSendSize == 0)
            {
                // NS_LOG_UNCOND("Bulk send size is 0. Skipping.\n");
                // BulkSendSyncManager::GetInstance().NotifyCompletion();
            }
            if (tempBulkSendSize > 0)
            {
                m_sink.push_back(tempSink);
                bulkSendScheduled = true;
                 NS_LOG_UNCOND("Starting bulk send #"
                          << m_currentBulkSendIndex << " at time " << Simulator::Now().GetSeconds()
                          << " of size " << tempBulkSendSize
                          << " bytes. From Node: " << m_node->GetId() << " to Node: " << tempSink->GetNode()->GetId() << "\n");
                bulkSendApp->TraceConnectWithoutContext(
                    "Tx",
                    MakeCallback(&MultiBulkSendApplication::PacketSentCallback, this));
                bulkSendApp->SetMaxBytes(tempBulkSendSize);
                tempSink->TraceConnectWithoutContext(
                    "Rx",
                    MakeCallback(&MultiBulkSendApplication::PacketRecievedCallback, this));
                bulkSendApp->SetStartTime(Simulator::Now());
                bulkSendApp->SetStopTime(Simulator::Now() + Seconds(10));
                tempSink->SetStartTime(Simulator::Now());
                tempSink->SetStopTime(Simulator::Now() + Seconds(10));
            }
        }
        if (!bulkSendScheduled)
        {
            NS_LOG_UNCOND("No bulk send scheduled for step " << m_currentBulkSendIndex << " on Node " << m_node->GetId() << "\n");
            // m_currentBulkSendIndex++;
            BulkSendSyncManager::GetInstance().NotifyCompletion(m_node->GetId());
        }

    }
    m_currentBulkSendIndex++;
}

} // namespace ns3