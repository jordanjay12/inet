//
// Copyright (C) 2012 Opensim Ltd.
// Author: Tamas Borbely
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_MULTIPACKETFILTERBASE_H
#define __INET_MULTIPACKETFILTERBASE_H

#include <algorithm>
#include "inet/common/newqueue/contract/IPacketCollection.h"
#include "inet/common/newqueue/contract/IPacketFilter.h"
#include "inet/common/newqueue/contract/IPacketQueueElement.h"

namespace inet {
namespace queue {

class INET_API MultiPacketFilterBase : public cSimpleModule, public IPacketFilter, public IPacketQueueElement, public IPacketConsumer::IListener, public IPacketProvider::IListener
{
  protected:
    int numGates = 0;
    std::vector<cGate *> inputGates;
    std::vector<cGate *> outputGates;
    std::vector<IPacketProvider *> providers;
    std::vector<IPacketConsumer *> consumers;
    std::vector<IPacketCollection *> outputCollections;    // vector of out collections indexed by gate index (may contain duplicate elements)

    std::vector<IPacketConsumer::IListener *> consumerListeners;
    std::vector<IPacketProvider::IListener *> providerListeners;

  protected:
    virtual void initialize(int stage) override;
    // TODO: what does filterPacket mean? filter out or keep?
    virtual bool filterPacket(cGate *gate, Packet *packet) = 0;

  public:
    virtual IPacketConsumer *getConsumer(cGate *gate) override { return this; }
    virtual IPacketProvider *getProvider(cGate *gate) override { return this; }

    virtual bool supportsPushPacket(cGate *gate) override { return true; }
    virtual int getNumPushablePackets(cGate *gate) override { return -1; }
    virtual bool canPushAnyPacket(cGate *gate) override { return true; }
    virtual bool canPushPacket(Packet *packet, cGate *gate) override { return true; }
    virtual void pushPacket(Packet *packet, cGate *gate) override;

    virtual bool supportsPopPacket(cGate *gate) override { return true; }
    virtual int getNumPoppablePackets(cGate *gate) override { return -1; }
    virtual bool canPopSomePacket(cGate *gate) override;
    virtual Packet *canPopPacket(cGate *gate) override { throw cRuntimeError("Invalid operation"); }
    virtual Packet *popPacket(cGate *gate) override;

    virtual int getNumPackets();
    virtual b getTotalLength();

    virtual void addListener(IPacketConsumer::IListener *listener) override { consumerListeners.push_back(listener); }
    virtual void removeListener(IPacketConsumer::IListener *listener) override { consumerListeners.erase(find(consumerListeners.begin(), consumerListeners.end(), listener)); }

    virtual void addListener(IPacketProvider::IListener *listener) override { providerListeners.push_back(listener); }
    virtual void removeListener(IPacketProvider::IListener *listener) override { providerListeners.erase(find(providerListeners.begin(), providerListeners.end(), listener)); }

    virtual void notifyCanPushPacket(cGate *gate) override;
    virtual void notifyCanPopPacket(cGate *gate) override;
};

} // namespace queue
} // namespace inet

#endif // ifndef __INET_MULTIPACKETFILTERBASE_H

