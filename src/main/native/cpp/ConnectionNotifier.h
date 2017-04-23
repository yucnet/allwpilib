/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2015. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#ifndef NT_CONNECTIONNOTIFIER_H_
#define NT_CONNECTIONNOTIFIER_H_

#include "ntcore_cpp.h"

#include "CallbackManager.h"
#include "Handle.h"
#include "IConnectionNotifier.h"

namespace nt {

namespace impl {

class ConnectionNotifierThread
    : public CallbackThread<ConnectionNotifierThread, ConnectionNotification> {
 public:
  ConnectionNotifierThread(int inst) : m_inst(inst) {}

  bool Matches(const ListenerData& listener,
               const ConnectionNotification& data) {
    return true;
  }

  void SetListener(ConnectionNotification* data, unsigned int listener_uid) {
    data->listener =
        Handle(m_inst, listener_uid, Handle::kConnectionListener).handle();
  }

  void DoCallback(
      std::function<void(const ConnectionNotification& event)> callback,
      const ConnectionNotification& data) {
    callback(data);
  }

  int m_inst;
};

}  // namespace impl

class ConnectionNotifier
    : public IConnectionNotifier,
      public CallbackManager<ConnectionNotifier,
                             impl::ConnectionNotifierThread> {
  friend class ConnectionNotifierTest;
  friend class CallbackManager<ConnectionNotifier,
                               impl::ConnectionNotifierThread>;

 public:
  explicit ConnectionNotifier(int inst);

  void Start();

  unsigned int Add(
      std::function<void(const ConnectionNotification& event)> callback);
  unsigned int AddPolled(unsigned int poller_uid);

  void NotifyConnection(bool connected, const ConnectionInfo& conn_info,
                        unsigned int only_listener = UINT_MAX) override;

 private:
  int m_inst;
};

}  // namespace nt

#endif  // NT_CONNECTIONNOTIFIER_H_
