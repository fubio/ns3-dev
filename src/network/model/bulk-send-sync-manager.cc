#include "bulk-send-sync-manager.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("BulkSendSyncManager");

BulkSendSyncManager& BulkSendSyncManager::GetInstance()
{
    static BulkSendSyncManager instance;
    return instance;
}

BulkSendSyncManager::BulkSendSyncManager()
    : m_totalNodes(0),
      m_completedNodes(0)
{
}

BulkSendSyncManager::~BulkSendSyncManager()
{
}

void BulkSendSyncManager::SetTotalNodes(uint32_t total)
{
    m_totalNodes = total;
    m_completedNodes = 0;
}

void BulkSendSyncManager::RegisterNode(std::function<void()> sendCallback)
{
    m_sendCallbacks.push_back(sendCallback);
    m_totalNodes += 1;
    NS_LOG_UNCOND("have " << m_totalNodes << " nodes registered");
}

void BulkSendSyncManager::NotifyCompletion()
{
    m_completedNodes++;
    NS_LOG_INFO("Bulk send completed by a node. Total completed: " << m_completedNodes << "/" << m_totalNodes);
    NS_LOG_UNCOND("Bulk send completed by a node. Total completed: " << m_completedNodes << "/" << m_totalNodes);
    if (m_completedNodes >= m_totalNodes)
    {
        NS_LOG_INFO("All nodes have completed the current bulk send round. Initiating next round.");
        NS_LOG_UNCOND("All nodes have completed the current bulk send round. Initiating next round at time: " << Simulator::Now().GetSeconds() << " seconds");
        m_completedNodes = 0; // Reset for the next round
        for (auto &callback : m_sendCallbacks)
        {
            // callback(); // Trigger the next bulk send on each node
            Simulator::ScheduleNow(callback);
        }
    }
}

} // namespace ns3