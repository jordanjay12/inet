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

#ifndef __INET_PACKETQUEUE_H
#define __INET_PACKETQUEUE_H

#include "inet/common/newqueue/base/PacketQueueBase.h"
#include "inet/common/newqueue/contract/IPacketBuffer.h"
#include "inet/common/newqueue/contract/IPacketComparatorFunction.h"
#include "inet/common/newqueue/contract/IPacketDropperFunction.h"

namespace inet {
namespace queue {

class INET_API PacketQueue : public PacketQueueBase, public IPacketBuffer::IListener
{
  protected:
    cGate *inputGate = nullptr;
    cGate *outputGate = nullptr;
    int frameCapacity = -1;
    int byteCapacity = -1;
    bool enableDropping = false;
    const char *displayStringTextFormat = nullptr;

    cPacketQueue queue;

    IPacketBuffer *buffer = nullptr;
    IPacketDropperFunction *packetDropperFunction = nullptr;
    IPacketComparatorFunction *packetComparatorFunction = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual void updateDisplayString();
    virtual bool isOverloaded();

  public:
    virtual ~PacketQueue() { delete packetComparatorFunction; }

    virtual int getMaxNumPackets() override { return frameCapacity; }
    virtual b getMaxTotalLength() override { return byteCapacity == -1 ? b(-1) : b(B(byteCapacity)); }
    virtual b getTotalLength() override { return b(queue.getBitLength()); }

    virtual bool isEmpty() override { return getNumPackets() == 0; }
    virtual int getNumPackets() override;
    virtual Packet *getPacket(int index) override;
    virtual void removePacket(Packet *packet) override;

    virtual bool supportsPushPacket(cGate *gate) override { return inputGate == gate; }
    virtual int getNumPushablePackets(cGate *gate) override { return -1; }
    virtual bool canPushAnyPacket(cGate *gate) override { return enableDropping ? true : getNumPackets() < getMaxNumPackets(); }
    virtual bool canPushPacket(Packet *packet, cGate *gate) override { return canPushAnyPacket(gate); }
    virtual void pushPacket(Packet *packet, cGate *gate) override;

    virtual bool supportsPopPacket(cGate *gate) override { return outputGate == gate; }
    virtual int getNumPoppablePackets(cGate *gate) override { return -1; }
    virtual bool canPopSomePacket(cGate *gate) override { return !isEmpty(); }
    virtual Packet *canPopPacket(cGate *gate) override { return !isEmpty() ? getPacket(0) : nullptr; }
    virtual Packet *popPacket(cGate *gate) override;

    virtual void notifyPacketRemoved(Packet *packet) override;
};

} // namespace queue
} // namespace inet

#endif // ifndef __INET_PACKETQUEUE_H

