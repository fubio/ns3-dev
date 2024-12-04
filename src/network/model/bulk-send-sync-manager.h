#ifndef BULK_SEND_SYNC_MANAGER_H
#define BULK_SEND_SYNC_MANAGER_H

#include <vector>
#include <functional>
#include "ns3/core-module.h"

namespace ns3 {

class BulkSendSyncManager
{
public:
    static BulkSendSyncManager& GetInstance();
    void SetRounds(std::vector<u_int32_t> rounds);
    void RegisterNode(std::function<void()> sendCallback);
    void SetTotalNodes(uint32_t total);
    void NotifyCompletion();

private:
    BulkSendSyncManager();
    ~BulkSendSyncManager();

    BulkSendSyncManager(const BulkSendSyncManager&) = delete;
    BulkSendSyncManager& operator=(const BulkSendSyncManager&) = delete;
    uint32_t m_totalNodes;
    uint32_t m_completedNodes;
    std::vector<std::function<void()>> m_sendCallbacks;
    std::vector<u_int32_t> m_rounds;
    u_int32_t m_currentRound;
};

} // namespace ns3

#endif // BULK_SEND_SYNC_MANAGER_H