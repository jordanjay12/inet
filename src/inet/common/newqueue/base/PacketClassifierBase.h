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

#ifndef __INET_PACKETCLASSIFIERBASE_H
#define __INET_PACKETCLASSIFIERBASE_H

#include "inet/common/newqueue/base/PacketConsumerBase.h"
#include "inet/common/newqueue/contract/IPacketQueueElement.h"

namespace inet {
namespace queue {

class INET_API PacketClassifierBase : public cSimpleModule, public PacketConsumerBase, public IPacketQueueElement, public IPacketConsumer::IListener
{
  protected:
    cGate *inputGate = nullptr;
    std::vector<cGate *> outputGates;
    std::vector<IPacketConsumer *> outputConsumers;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual int classifyPacket(Packet *packet) const = 0;

  public:
    virtual bool supportsPushPacket(cGate *gate) override { return true; }
    virtual bool supportsPopPacket(cGate *gate) override { return false; }

    virtual void pushPacket(Packet *packet, cGate *gate) override;

    virtual void notifyCanPushPacket(cGate *gate) override;
};

} // namespace queue
} // namespace inet

#endif // ifndef __INET_PACKETCLASSIFIERBASE_H

