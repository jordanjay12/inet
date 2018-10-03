//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_PACKETQUEUEBASE_H
#define __INET_PACKETQUEUEBASE_H

#include <algorithm>
#include "inet/common/newqueue/contract/IPacketQueue.h"
#include "inet/common/newqueue/contract/IPacketQueueElement.h"

namespace inet {
namespace queue {

class INET_API PacketQueueBase : public cSimpleModule, public IPacketQueue, public IPacketQueueElement
{
  protected:
    std::vector<IPacketConsumer::IListener *> consumerListeners;
    std::vector<IPacketProvider::IListener *> providerListeners;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

  public:
    virtual bool canPopSomePacket(cGate *gate) override { return getNumPackets() > 0; }
    virtual bool canPushAnyPacket(cGate *gate) override { return true; }

    virtual void addListener(IPacketConsumer::IListener *listener) override { consumerListeners.push_back(listener); }
    virtual void removeListener(IPacketConsumer::IListener *listener) override { consumerListeners.erase(find(consumerListeners.begin(), consumerListeners.end(), listener)); }

    virtual void addListener(IPacketProvider::IListener *listener) override { providerListeners.push_back(listener); }
    virtual void removeListener(IPacketProvider::IListener *listener) override { providerListeners.erase(find(providerListeners.begin(), providerListeners.end(), listener)); }
};

} // namespace queue
} // namespace inet

#endif // ifndef __INET_PACKETQUEUEBASE_H

