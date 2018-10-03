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

#include "inet/common/newqueue/base/MultiPacketFilterBase.h"
#include "inet/common/newqueue/QueueUtils.h"
#include "inet/common/Simsignals.h"

namespace inet {
namespace queue {

void MultiPacketFilterBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        numGates = gateSize("in");
        for (int i = 0; i < numGates; ++i) {
            auto inputGate = gate("in", i);
            auto connectedGate = inputGate->getPathStartGate();
            if (!connectedGate)
                throw cRuntimeError("In gate %d is not connected", i);
            auto provider = dynamic_cast<IPacketProvider *>(connectedGate->getOwnerModule());
            providers.push_back(provider);
            if (provider != nullptr)
                provider->addListener(this);
            inputGates.push_back(inputGate);
        }
        for (int i = 0; i < numGates; ++i) {
            auto outputGate = gate("out", i);
            auto connectedGate = outputGate->getPathEndGate();
            if (!connectedGate)
                throw cRuntimeError("Out gate %d is not connected", i);
            auto consumer = dynamic_cast<IPacketConsumer *>(connectedGate->getOwnerModule());
            consumers.push_back(consumer);
            if (consumer != nullptr)
                consumer->addListener(this);
            outputGates.push_back(outputGate);
            IPacketCollection *outModule = dynamic_cast<IPacketCollection *>(connectedGate->getOwnerModule());
            if (!outModule)
                throw cRuntimeError("Out gate %d should be connected a simple module implementing IPacketCollection", i);
            outputCollections.push_back(outModule);
        }
    }
}

// TODO: several functions are copied from PacketFilterBase and extended with multiple gate support via indices
void MultiPacketFilterBase::pushPacket(Packet *packet, cGate *gate)
{
    if (!filterPacket(gate, packet)) {
        EV_INFO << "Passing through packet " << packet->getName() << "." << endl;
        animateSend(packet, outputGates[gate->getIndex()]);
        consumers[gate->getIndex()]->pushPacket(packet, outputGates[gate->getIndex()]->getPathEndGate());
    }
    else {
        EV_INFO << "Filtering packet " << packet->getName() << "." << endl;
        PacketDropDetails details;
        details.setReason(OTHER_PACKET_DROP);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
    }
}

bool MultiPacketFilterBase::canPopSomePacket(cGate *gate)
{
    auto provider = providers[gate->getIndex()];
    auto providerGate = inputGates[gate->getIndex()]->getPathStartGate();
    while (true) {
        auto packet = provider->canPopPacket(providerGate);
        if (!filterPacket(gate, packet))
            return true;
        else {
            packet = provider->popPacket(providerGate);
            EV_INFO << "Filtering packet " << packet->getName() << "." << endl;
            PacketDropDetails details;
            details.setReason(OTHER_PACKET_DROP);
            emit(packetDroppedSignal, packet, &details);
            delete packet;
        }
    }
}

Packet *MultiPacketFilterBase::popPacket(cGate *gate)
{
    auto provider = providers[gate->getIndex()];
    auto providerGate = inputGates[gate->getIndex()]->getPathStartGate();
    while (true) {
        auto packet = provider->popPacket(providerGate);
        if (!filterPacket(gate, packet)) {
            EV_INFO << "Passing through packet " << packet->getName() << "." << endl;
            animateSend(packet, gate);
            return packet;
        }
        else {
            EV_INFO << "Filtering packet " << packet->getName() << "." << endl;
            PacketDropDetails details;
            details.setReason(OTHER_PACKET_DROP);
            emit(packetDroppedSignal, packet, &details);
            delete packet;
        }
    }
}

int MultiPacketFilterBase::getNumPackets()
{
    int len = 0;
    for (const auto & elem : outputCollections)
        len += (elem)->getNumPackets();
    return len;
}

b MultiPacketFilterBase::getTotalLength()
{
    b len = b(0);
    for (const auto & elem : outputCollections)
        len += (elem)->getTotalLength();
    return len;
}

void MultiPacketFilterBase::notifyCanPushPacket(cGate *gate)
{
    int index = gate->getPathStartGate()->getIndex();
    for (auto listener : consumerListeners)
        listener->notifyCanPushPacket(inputGates[index]);
}

void MultiPacketFilterBase::notifyCanPopPacket(cGate *gate)
{
    int index = gate->getPathEndGate()->getIndex();
    for (auto listener : providerListeners)
        listener->notifyCanPopPacket(outputGates[index]);
}

} // namespace queue
} // namespace inet

