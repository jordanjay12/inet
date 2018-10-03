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

#include "inet/common/newqueue/PacketMultiplexer.h"
#include "inet/common/newqueue/QueueUtils.h"
#include "inet/common/Simsignals.h"

namespace inet {
namespace queue {

Define_Module(PacketMultiplexer);

void PacketMultiplexer::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        for (int i = 0; i < gateSize("out"); i++)
            outputGates.push_back(gate("out", i));
        provider = check_and_cast<IPacketProvider *>(inputGate->getPathStartGate()->getOwnerModule());
        provider->addListener(this);
    }
}

Packet *PacketMultiplexer::popPacket(cGate *gate)
{
    auto packet = provider->popPacket(inputGate->getPathStartGate());
    EV_INFO << "Forwarding popped packet " << packet->getName() << "." << endl;
    animateSend(packet, gate);
    return packet;
}

void PacketMultiplexer::notifyCanPopPacket(cGate *gate)
{
    for (int i = 0; i < (int)outputGates.size(); i++)
        for (auto listener : listeners)
            // NOTE: notifying a listener may prevent others from popping
            if (provider->canPopSomePacket(inputGate->getPathStartGate()))
                listener->notifyCanPopPacket(outputGates[i]);
}

} // namespace queue
} // namespace inet

