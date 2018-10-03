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

#include "inet/common/newqueue/base/PacketFilterBase.h"
#include "inet/common/newqueue/QueueUtils.h"
#include "inet/common/Simsignals.h"

namespace inet {
namespace queue {

void PacketFilterBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        outputGate = gate("out");
        provider = dynamic_cast<IPacketProvider *>(inputGate->getPathStartGate()->getOwnerModule());
        if (provider != nullptr)
            provider->addListener(this);
        consumer = dynamic_cast<IPacketConsumer *>(outputGate->getPathEndGate()->getOwnerModule());
        if (consumer != nullptr)
            consumer->addListener(this);
    }
    else if (stage == INITSTAGE_LAST) {
        if (consumer != nullptr) {
            checkPushPacketSupport(inputGate);
            checkPushPacketSupport(outputGate);
        }
        if (provider != nullptr) {
            checkPopPacketSupport(inputGate);
            checkPopPacketSupport(outputGate);
        }
    }
}

void PacketFilterBase::pushPacket(Packet *packet, cGate *gate)
{
    if (filterPacket(packet)) {
        EV_INFO << "Passing through packet " << packet->getName() << "." << endl;
        animateSend(packet, outputGate);
        consumer->pushPacket(packet, outputGate->getPathEndGate());
    }
    else {
        EV_INFO << "Filtering out packet " << packet->getName() << "." << endl;
        PacketDropDetails details;
        details.setReason(OTHER_PACKET_DROP);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
    }
}

bool PacketFilterBase::canPopSomePacket(cGate *gate)
{
    auto providerGate = inputGate->getPathStartGate();
    while (true) {
        auto packet = provider->canPopPacket(providerGate);
        if (packet == nullptr)
            return false;
        else if (filterPacket(packet))
            return true;
        else {
            packet = provider->popPacket(providerGate);
            EV_INFO << "Filtering out packet " << packet->getName() << "." << endl;
            PacketDropDetails details;
            details.setReason(OTHER_PACKET_DROP);
            emit(packetDroppedSignal, packet, &details);
            delete packet;
        }
    }
}

Packet *PacketFilterBase::popPacket(cGate *gate)
{
    auto providerGate = inputGate->getPathStartGate();
    while (true) {
        auto packet = provider->popPacket(providerGate);
        if (filterPacket(packet)) {
            EV_INFO << "Passing through packet " << packet->getName() << "." << endl;
            animateSend(packet, outputGate);
            return packet;
        }
        else {
            EV_INFO << "Filtering out packet " << packet->getName() << "." << endl;
            PacketDropDetails details;
            details.setReason(OTHER_PACKET_DROP);
            emit(packetDroppedSignal, packet, &details);
            delete packet;
        }
    }
}

void PacketFilterBase::notifyCanPushPacket(cGate *gate)
{
    for (auto listener : consumerListeners)
        listener->notifyCanPushPacket(inputGate);
}

void PacketFilterBase::notifyCanPopPacket(cGate *gate)
{
    for (auto listener : providerListeners)
        listener->notifyCanPopPacket(outputGate);
}

} // namespace queue
} // namespace inet

