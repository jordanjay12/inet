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

#ifndef __INET_PACKETSERVERBASE_H
#define __INET_PACKETSERVERBASE_H

#include "inet/common/newqueue/contract/IPacketCollector.h"
#include "inet/common/newqueue/contract/IPacketProducer.h"
#include "inet/common/newqueue/contract/IPacketQueueElement.h"

namespace inet {
namespace queue {

class INET_API PacketServerBase : public cSimpleModule, public IPacketCollector, public IPacketProducer, public IPacketQueueElement, public IPacketProvider::IListener
{
  protected:
    cGate *inputGate = nullptr;
    cGate *outputGate = nullptr;
    IPacketProvider *provider = nullptr;
    IPacketConsumer *consumer = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

  public:
    virtual IPacketProvider *getProvider(cGate *gate) override { return provider; }
    virtual IPacketConsumer *getConsumer(cGate *gate) override { return consumer; }

    virtual bool supportsPushPacket(cGate *gate) override { return outputGate == gate; }
    virtual bool supportsPopPacket(cGate *gate) override { return inputGate == gate; }
};

} // namespace queue
} // namespace inet

#endif // ifndef __INET_PACKETSERVERBASE_H
