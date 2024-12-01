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
};

} // namespace ns3

#endif // BULK_SEND_SYNC_MANAGER_H