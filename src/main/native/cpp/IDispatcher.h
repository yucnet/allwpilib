/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2017. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#ifndef NT_IDISPATCHER_H_
#define NT_IDISPATCHER_H_

#include <memory>

#include "Message.h"

namespace nt {

class NetworkConnection;

// Interface for generation of outgoing messages to break a dependency loop
// between Storage and Dispatcher.
class IDispatcher {
 public:
  IDispatcher() = default;
  IDispatcher(const IDispatcher&) = delete;
  IDispatcher& operator=(const IDispatcher&) = delete;
  virtual ~IDispatcher() = default;
  virtual void QueueOutgoing(std::shared_ptr<Message> msg,
                             NetworkConnection* only,
                             NetworkConnection* except) = 0;
};

}  // namespace nt

#endif  // NT_IDISPATCHER_H_
