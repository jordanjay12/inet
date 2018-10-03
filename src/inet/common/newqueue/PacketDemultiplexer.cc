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

#include "inet/common/newqueue/PacketDemultiplexer.h"
#include "inet/common/newqueue/QueueUtils.h"

namespace inet {
namespace queue {

Define_Module(PacketDemultiplexer);

void PacketDemultiplexer::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        for (int i = 0; i < gateSize("in"); i++)
            inputGates.push_back(gate("in", i));
        outputGate = gate("out");
        consumer = check_and_cast<IPacketConsumer *>(outputGate->getPathEndGate()->getOwnerModule());
        consumer->addListener(this);
    }
}

void PacketDemultiplexer::pushPacket(Packet *packet, cGate *gate)
{
    EV_INFO << "Forwarding pushed packet " << packet->getName() << "." << endl;
    animateSend(packet, outputGate);
    consumer->pushPacket(packet, outputGate->getPathEndGate());
}

void PacketDemultiplexer::notifyCanPushPacket(cGate *gate)
{
    for (int i = 0; i < (int)inputGates.size(); i++)
        for (auto listener : listeners)
            // NOTE: notifying a listener may prevent others from pushing
            if (consumer->canPushAnyPacket(outputGate))
                listener->notifyCanPushPacket(inputGates[i]);
}

} // namespace queue
} // namespace inet

