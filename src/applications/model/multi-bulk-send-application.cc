#include "multi-bulk-send-application.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("MultiBulkSendApplication");

NS_OBJECT_ENSURE_REGISTERED(MultiBulkSendApplication);

// Define the callback function

void
MultiBulkSendApplication::PacketSentCallback(Ptr<const Packet> packet)
{
    // NS_LOG_UNCOND("Packet sent at " << Simulator::Now().GetSeconds() << "s, Size: " << packet->GetSize() << " bytes. From Node: " << m_node->GetId());
    // packet->GetSize() << " bytes. From Node: " << m_node->GetId());
}

void
MultiBulkSendApplication::PacketRecievedCallback(Ptr<const Packet> packet, const Address& address)
{
    // NS_LOG_UNCOND("Packet received at: " << Simulator::Now().GetSeconds() << "packet size: " << packet->GetSize() << " bytes. Sent from Node" <<
    // m_node->GetId());
    // NS_LOG_UNCOND("Packet received: " << packet->GetSize() << " bytes");
    m_totalBytesReceived += packet->GetSize();
    if (m_totalBytesReceived >= m_bulkSendSize)
    {
        BulkSendSyncManager::GetInstance().NotifyCompletion(m_node->GetId());
        // NS_LOG_UNCOND("mbulksendSize: " << m_bulkSendSize);
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
    NS_LOG_UNCOND("Starting MultiBulkSendApplication on Node: " << m_node->GetId());
    if (m_bulkSends.empty())
    {
        NS_LOG_ERROR("BulkSendSizes vector is empty. No data to send.");
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
    m_totalBytesReceived = 0;
    m_bulkSendSize = 0;
    // std::cout <<"On Node: "<< m_node->GetId() <<  "m_currentBulkSendIndex: " << m_currentBulkSendIndex << " mstpes: " << m_steps << std::endl;
    if (m_currentBulkSendIndex >= m_steps)
    {
        NS_LOG_UNCOND("All bulk sends complete.");
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
        std::cout <<"Node: " << m_node->GetId() <<  " m_bulkSendSize: " << m_bulkSendSize << std::endl;

        for (int i = 0; i < bulkSends.size(); i++)
        {
            
            auto [bulkSendApp, tempBulkSendSize, tempSink, destNode] = bulkSends[i];
            // m_bulkSendSize += tempBulkSendSize;
            // if (m_node->GetId() == 1) {
            //                  NS_LOG_UNCOND("Starting bulk send #"
            //               << m_currentBulkSendIndex << " at time " << Simulator::Now().GetSeconds()
            //               << "of size " << tempBulkSendSize
            //               << " bytes. From Node: " << m_node->GetId() << " to Node: " << destNode);
            // }

            if (m_bulkSendSize == 0)
            {
                NS_LOG_UNCOND("Bulk send size is 0. Skipping.\n");
                // BulkSendSyncManager::GetInstance().NotifyCompletion();
            }
            if (tempBulkSendSize > 0)
            {
                bulkSendScheduled = true;
                 NS_LOG_UNCOND("Starting bulk send #"
                          << m_currentBulkSendIndex << " at time " << Simulator::Now().GetSeconds()
                          << " of size " << tempBulkSendSize
                          << " bytes. From Node: " << m_node->GetId() << " to Node: " << tempSink->GetNode()->GetId() << "\n");
                bulkSendApp->TraceConnectWithoutContext(
                    "Tx",
                    MakeCallback(&MultiBulkSendApplication::PacketSentCallback, this));
                NS_LOG_UNCOND("Setting max bytes to " << tempBulkSendSize);
                bulkSendApp->SetMaxBytes(tempBulkSendSize);
                tempSink->TraceConnectWithoutContext(
                    "Rx",
                    MakeCallback(&MultiBulkSendApplication::PacketRecievedCallback, this));
                // NS_LOG_UNCOND("Setting up bulk send at time " << Simulator::Now().GetSeconds());
                bulkSendApp->SetStartTime(Simulator::Now());
                bulkSendApp->SetStopTime(Simulator::Now() + Seconds(10));
                tempSink->SetStartTime(Simulator::Now());
                tempSink->SetStopTime(Simulator::Now() + Seconds(10));
                // NS_LOG_UNCOND("finished setting up bulk send #" << m_currentBulkSendIndex
                                                                // << " with sink on "
                                                                // << tempSink->GetNode()->GetId() << " which we know is on Node: " << destNode);
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