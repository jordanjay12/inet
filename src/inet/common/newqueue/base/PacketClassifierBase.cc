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

#include "inet/common/newqueue/base/PacketClassifierBase.h"
#include "inet/common/newqueue/QueueUtils.h"

namespace inet {
namespace queue {

void PacketClassifierBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        for (int i = 0; i < gateSize("out"); i++) {
            auto outputGate = gate("out", i);
            auto outputConsumer = dynamic_cast<IPacketConsumer *>(outputGate->getPathEndGate()->getOwnerModule());
            outputGates.push_back(outputGate);
            outputConsumers.push_back(outputConsumer);
            if (outputConsumer != nullptr)
                outputConsumer->addListener(this);
        }
    }
    else if (stage == INITSTAGE_LAST) {
        for (auto outputGate : outputGates)
            checkPushPacketSupport(outputGate);
        checkPushPacketSupport(inputGate);
    }
}

void PacketClassifierBase::pushPacket(Packet *packet, cGate *gate)
{
    EV_INFO << "Classifying packet " << packet->getName() << ".\n";
    int index = classifyPacket(packet);
    animateSend(packet, outputGates[index]);
    outputConsumers[index]->pushPacket(packet, outputGates[index]->getPathEndGate());
}

void PacketClassifierBase::notifyCanPushPacket(cGate *gate)
{
    for (auto listener : listeners)
        listener->notifyCanPushPacket(inputGate);
}

} // namespace queue
} // namespace inet

