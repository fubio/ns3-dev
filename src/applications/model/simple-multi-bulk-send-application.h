// Filename: multi-bulk-send-application.h
/*
In BulkSendApplication the only functions we can call are as follows
    virtual void SetRemote(const Address& addr);
    Address GetRemote() const;
    static TypeId GetTypeId();
    void SetMaxBytes(uint64_t maxBytes);
    Ptr<Socket> GetSocket() const;


     * @brief Specify application start time
     * @param start Start time for this application,
     *        relative to the current simulation time.
     *
     * Applications start at various times in the simulation scenario.
     * This method specifies when the application should be
     * started.  The application subclasses should override the
     * private "StartApplication" method defined below, which is called at the
     * time specified, to cause the application to begin.

    void SetStartTime(Time start);

*/
#include "bulk-send-application.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "ns3/bulk-send-sync-manager.h"
#include "ns3/packet-sink.h"
#include "ns3/net-device-container.h"

#include <vector>

namespace ns3
{

class SimpleMultiBulkSendApplication : public Application
{
  public:
    static TypeId GetTypeId();

    SimpleMultiBulkSendApplication();
    virtual ~SimpleMultiBulkSendApplication();

    void SetBulkSendSizes(const std::vector<std::tuple<Ptr<BulkSendApplication>, uint64_t, Ptr<PacketSink>, NetDeviceContainer>>& bulkSends);

  private:
    void PacketSentCallback(Ptr<const Packet> packet);
    void PacketRecievedCallback(Ptr<const Packet> packet, const Address& address);
    // void PacketRecievedCallback(Ptr<const Packet> packet);
    void StartApplication() override;
    void StopApplication() override;
    void ScheduleNextBulkSend();
    //each elment in the vector is a step, in each bucket we have the size of the bulksend and the bulksend application
    std::vector<std::tuple<Ptr<BulkSendApplication>, uint64_t, Ptr<PacketSink>, NetDeviceContainer>> m_bulkSendSizes;
    size_t m_currentBulkSendIndex;
    uint64_t m_totalBytesReceived;
    uint64_t m_packetsSent;
    uint64_t m_packetsReceived;
    uint64_t m_bulkSendSize;
    Ptr<PacketSink> m_sink;
};

} // namespace ns3