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

#include "inet/common/newqueue/PacketDuplicator.h"
#include "inet/common/newqueue/QueueUtils.h"

namespace inet {
namespace queue {

Define_Module(PacketDuplicator);

void PacketDuplicator::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        outputGate = gate("out");
        consumer = check_and_cast<IPacketConsumer *>(outputGate->getPathEndGate()->getOwnerModule());
        consumer->addListener(this);
    }
}

void PacketDuplicator::pushPacket(Packet *packet, cGate *gate)
{
    int numDuplicates = par("numDuplicates");
    for (int i = 0; i < numDuplicates; i++) {
        EV_INFO << "Forwarding duplicate packet " << packet->getName() << "." << endl;
        auto duplicate = packet->dup();
        animateSend(duplicate, outputGate);
        consumer->pushPacket(duplicate, outputGate->getPathEndGate());
    }
    EV_INFO << "Forwarding original packet " << packet->getName() << "." << endl;
    animateSend(packet, outputGate);
    consumer->pushPacket(packet, outputGate->getPathEndGate());
}

void PacketDuplicator::notifyCanPushPacket(cGate *gate)
{
    for (auto listener : listeners)
        listener->notifyCanPushPacket(inputGate);
}

} // namespace queue
} // namespace inet
