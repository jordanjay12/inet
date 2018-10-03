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

#include "inet/common/newqueue/CompoundQueue.h"
#include "inet/common/newqueue/QueueUtils.h"
#include "inet/common/Simsignals.h"
#include "inet/common/StringFormat.h"

namespace inet {
namespace queue {

Define_Module(CompoundQueue);

void CompoundQueue::initialize(int stage)
{
    PacketQueueBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        displayStringTextFormat = par("displayStringTextFormat");
        frameCapacity = par("frameCapacity");
        byteCapacity = par("byteCapacity");
        inputGate = gate("in");
        outputGate = gate("out");
        inputQueue = check_and_cast<IPacketConsumer *>(gate("in")->getPathEndGate()->getOwnerModule());
        outputQueue = check_and_cast<IPacketProvider *>(gate("out")->getPathStartGate()->getOwnerModule());
        outputCollection = check_and_cast<IPacketCollection *>(outputQueue);
    }
    else if (stage == INITSTAGE_LAST) {
        checkPushPacketSupport(inputGate);
        checkPopPacketSupport(outputGate);
    }
}

int CompoundQueue::getNumPackets()
{
    return outputCollection->getNumPackets();
}

Packet *CompoundQueue::getPacket(int index)
{
    return outputCollection->getPacket(index);
}

void CompoundQueue::pushPacket(Packet *packet, cGate *gate)
{
    if ((frameCapacity != -1 && getNumPackets() >= frameCapacity) ||
        (byteCapacity != -1 && getTotalLength() + packet->getTotalLength() > B(byteCapacity)))
    {
        EV_INFO << "Dropping packet " << packet->getName() << " because the queue is full." << endl;
        PacketDropDetails details;
        details.setReason(QUEUE_OVERFLOW);
        details.setLimit(frameCapacity);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
    }
    else {
        animateSend(packet, inputGate);
        inputQueue->pushPacket(packet, inputGate->getPathEndGate());
        updateDisplayString();
    }
}

Packet *CompoundQueue::popPacket(cGate *gate)
{
    auto packet = outputQueue->popPacket(outputGate->getPathStartGate());
    animateSend(packet, outputGate->getPathStartGate());
    updateDisplayString();
    return packet;
}

void CompoundQueue::removePacket(Packet *packet)
{
    outputCollection->removePacket(packet);
    updateDisplayString();
}

b CompoundQueue::getTotalLength()
{
    b length = b(0);
    int numPackets = getNumPackets();
    for (int i = 0; i < numPackets; i++)
        length += getPacket(i)->getTotalLength();
    return length;
}

void CompoundQueue::updateDisplayString()
{
    auto text = StringFormat::formatString(displayStringTextFormat, [&] (char directive) {
        static std::string result;
        switch (directive) {
            case 'p':
                result = std::to_string(outputCollection->getNumPackets());
                break;
            case 'l':
                result = getTotalLength().str();
                break;
            case 'n':
                result = !outputCollection->isEmpty() ? outputCollection->getPacket(0)->getFullName() : "";
                break;
            default:
                throw cRuntimeError("Unknown directive: %c", directive);
        }
        return result.c_str();
    });
    getDisplayString().setTagArg("t", 0, text);
}

} // namespace queue
} // namespace inet

