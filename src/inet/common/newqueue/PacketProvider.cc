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

#include "inet/common/newqueue/PacketProvider.h"
#include "inet/common/newqueue/QueueUtils.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/common/Simsignals.h"
#include "inet/common/StringFormat.h"

namespace inet {
namespace queue {

Define_Module(PacketProvider);

void PacketProvider::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        outputGate = gate("out");
        packetNameFormat = par("packetNameFormat");
        providingIntervalParameter = &par("providingInterval");
        providingTimer = new cMessage("StartProviding");
        WATCH(numPacket);
    }
    else if (stage == INITSTAGE_LAST) {
        checkPopPacketSupport(outputGate);
        schedulerProvidingTimer();
    }
}

void PacketProvider::handleMessage(cMessage *message)
{
    if (message == providingTimer)
        for (auto listener : listeners)
            listener->notifyCanPopPacket(outputGate);
    else
        throw cRuntimeError("Unknown message");
}

void PacketProvider::schedulerProvidingTimer()
{
    scheduleAt(simTime() + providingIntervalParameter->doubleValue(), providingTimer);
}

Packet *PacketProvider::canPopPacket(cGate *gate)
{
    if (providingTimer->isScheduled())
        return nullptr;
    else {
        if (nextPacket == nullptr)
            nextPacket = createPacket();
        return nextPacket;
    }
}

Packet *PacketProvider::popPacket(cGate *gate)
{
    if (providingTimer->isScheduled())
        throw cRuntimeError("Cannot provide packet");
    Packet *packet;
    if (nextPacket == nullptr)
        packet = createPacket();
    else {
        packet = nextPacket;
        nextPacket = nullptr;
    }
    EV_INFO << "Providing packet " << packet->getName() << "." << endl;
    animateSend(packet, outputGate);
    schedulerProvidingTimer();
    numPacket++;
    totalLength += packet->getTotalLength();
    return packet;
}

Packet *PacketProvider::createPacket()
{
    auto name = StringFormat::formatString(packetNameFormat, [&] (char directive) {
        static std::string result;
        switch (directive) {
            case 'n':
                result = getFullName();
                break;
            case 'c':
                result = std::to_string(numPacket);
                break;
            default:
                throw cRuntimeError("Unknown directive: %c", directive);
        }
        return result.c_str();
    });
    const auto& content = makeShared<ByteCountChunk>(b(par("packetLength")));
    return new Packet(name, content);
}

} // namespace queue
} // namespace inet

