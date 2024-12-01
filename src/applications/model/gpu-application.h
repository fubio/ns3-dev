// #ifndef MY_APPLICATION_H
// #define MY_APPLICATION_H

// #include "ns3/application.h"
// #include "ns3/net-device.h"
// #include "ns3/simulator.h"
// #include "ns3/log.h"
// #include <atomic>
// #include <mutex>
// #include <list>

// namespace ns3 {
// class Channel;

// NS_LOG_COMPONENT_DEFINE("MyApplication");

// class GPUApplication : public Application, public NetDevice {
// public:
//     // Static variables
//     static std::atomic<std::list<int>> globalActiveEvents; // Global counter for active events
//     static std::mutex globalLock;                          // Mutex for coordinating event scheduling
//     static int totalInstances;                             // Total application instances

//     //Constructor and Destructor
//     GPUApplication();
//     virtual ~GPUApplication();

//     /**
//      * @return the channel this NetDevice is connected to. The value
//      *         returned can be zero if the NetDevice is not yet connected
//      *         to any channel or if the underlying NetDevice has no
//      *         concept of a channel. i.e., callers _must_ check for zero
//      *         and be ready to handle it.
//      */
//     virtual Ptr<Channel> GetChannel() const = 0;

// protected:
//     // Overriding virtual methods from Application
//     virtual void StartApplication() override;
//     virtual void StopApplication() override;
//     std::vector<Ptr<Channel>> m_channels; // Vector of Channels for the application

// private:
//     EventId m_event;  // EventId for scheduling events
// };

// } // namespace ns3

// #endif // MY_APPLICATION_H
