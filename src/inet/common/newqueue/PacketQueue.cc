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

#include "inet/common/ModuleAccess.h"
#include "inet/common/newqueue/PacketComparatorFunction.h"
#include "inet/common/newqueue/PacketDropperFunction.h"
#include "inet/common/newqueue/PacketQueue.h"
#include "inet/common/newqueue/QueueUtils.h"
#include "inet/common/Simsignals.h"
#include "inet/common/StringFormat.h"

namespace inet {
namespace queue {

Define_Module(PacketQueue);

void PacketQueue::initialize(int stage)
{
    PacketQueueBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        outputGate = gate("out");
        frameCapacity = par("frameCapacity");
        byteCapacity = par("byteCapacity");
        enableDropping = par("enableDropping");
        displayStringTextFormat = par("displayStringTextFormat");
        // buffer
        buffer = getModuleFromPar<IPacketBuffer>(par("bufferModule"), this, false);
        if (buffer != nullptr)
            buffer->addListener(this);
        // copmarator function
        const char *comparatorClass = par("comparatorClass");
        if (*comparatorClass != '\0')
            packetComparatorFunction = check_and_cast<IPacketComparatorFunction *>(createOne(comparatorClass));
        if (packetComparatorFunction != nullptr)
            queue.setup(packetComparatorFunction);
        // dropper function
        const char *dropperClass = par("dropperClass");
        if (*dropperClass != '\0')
            packetDropperFunction = check_and_cast<IPacketDropperFunction *>(createOne(dropperClass));
        if ((frameCapacity != -1 || byteCapacity != -1) && enableDropping && packetDropperFunction == nullptr)
            throw cRuntimeError("Packet dropper is not specified");
        updateDisplayString();
        scheduleAt(0, new cMessage("StartConsuming"));
    }
    else if (stage == INITSTAGE_LAST) {
        checkPushPacketSupport(inputGate);
        checkPopPacketSupport(outputGate);
    }
}

void PacketQueue::handleMessage(cMessage *message)
{
    for (auto listener : consumerListeners)
        listener->notifyCanPushPacket(inputGate);
    delete message;
}

bool PacketQueue::isOverloaded()
{
    return (frameCapacity != -1 && getNumPackets() > frameCapacity) ||
           (byteCapacity != -1 && getTotalLength() > B(byteCapacity));
}

int PacketQueue::getNumPackets()
{
    return queue.getLength();
}

Packet *PacketQueue::getPacket(int index)
{
    return check_and_cast<Packet *>(queue.get(index));
}

void PacketQueue::pushPacket(Packet *packet, cGate *gate)
{
    EV_INFO << "Pushing packet " << packet->getName() << " into the queue." << endl;
    queue.insert(packet);
    emit(packetEnqueuedSignal, packet);
    if (buffer != nullptr)
        buffer->addPacket(this, packet);
    else if (isOverloaded()) {
        if (enableDropping)
            packetDropperFunction->dropPackets(this);
        else
            throw cRuntimeError("Cannot push packet into overloaded queue");
    }
    updateDisplayString();
    if (getNumPackets() != 0)
        for (auto listener : providerListeners)
            listener->notifyCanPopPacket(outputGate);
}

Packet *PacketQueue::popPacket(cGate *gate)
{
    auto packet = check_and_cast<Packet *>(queue.front());
    EV_INFO << "Popping packet " << packet->getName() << " from the queue." << endl;
    if (buffer != nullptr)
        buffer->removePacket(this, packet);
    else {
        queue.pop();
        updateDisplayString();
        emit(packetDequeuedSignal, packet);
    }
    animateSend(packet, outputGate);
    return packet;
}

void PacketQueue::removePacket(Packet *packet)
{
    EV_INFO << "Removing packet " << packet->getName() << " from the queue." << endl;
    if (buffer != nullptr)
        buffer->removePacket(this, packet);
    else {
        queue.remove(packet);
        updateDisplayString();
        emit(packetDequeuedSignal, packet);
    }
}

void PacketQueue::notifyPacketRemoved(Packet *packet)
{
    queue.remove(packet);
    updateDisplayString();
    emit(packetDequeuedSignal, packet);
}

void PacketQueue::updateDisplayString()
{
    auto text = StringFormat::formatString(displayStringTextFormat, [&] (char directive) {
        static std::string result;
        switch (directive) {
            case 'p':
                result = std::to_string(queue.getLength());
                break;
            case 'l':
                result = getTotalLength().str();
                break;
            case 'n':
                result = !queue.isEmpty() ? queue.front()->getFullName() : "";
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

