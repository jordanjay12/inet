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

#ifndef __INET_COMPOUNDQUEUE_H
#define __INET_COMPOUNDQUEUE_H

#include "inet/common/newqueue/base/PacketQueueBase.h"
#include "inet/common/newqueue/contract/IPacketCollection.h"
#include "inet/common/newqueue/contract/IPacketConsumer.h"
#include "inet/common/newqueue/contract/IPacketProvider.h"

namespace inet {
namespace queue {

class INET_API CompoundQueue : public PacketQueueBase
{
  protected:
    int frameCapacity = -1;
    int byteCapacity = -1;
    const char *displayStringTextFormat = nullptr;
    cGate *inputGate = nullptr;
    cGate *outputGate = nullptr;
    IPacketConsumer *inputQueue = nullptr;
    IPacketProvider *outputQueue = nullptr;
    IPacketCollection *outputCollection = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void updateDisplayString();
    virtual b getTotalLength();

  public:
    virtual int getMaxNumPackets() override { return frameCapacity; }
    virtual b getMaxTotalLength() override { return B(byteCapacity); }

    virtual bool isEmpty() override { return outputCollection->isEmpty(); }
    virtual int getNumPackets() override;
    virtual Packet *getPacket(int index) override;
    virtual void removePacket(Packet *packet) override;

    virtual bool supportsPushPacket(cGate *gate) override { return inputGate == gate; }
    virtual int getNumPushablePackets(cGate *gate) override { return -1; }
    virtual bool canPushPacket(Packet *packet, cGate *gate) override { return true; }
    virtual void pushPacket(Packet *packet, cGate *gate) override;

    virtual bool supportsPopPacket(cGate *gate) override { return outputGate == gate; }
    virtual int getNumPoppablePackets(cGate *gate) override { return -1; }
    virtual Packet *canPopPacket(cGate *gate) override { throw cRuntimeError("Invalid operation"); }
    virtual Packet *popPacket(cGate *gate) override;
};

} // namespace queue
} // namespace inet

#endif // ifndef __INET_COMPOUNDQUEUE_H
