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

#include "inet/common/newqueue/base/PacketDuplicatorBase.h"
#include "inet/common/newqueue/QueueUtils.h"
#include "inet/common/Simsignals.h"

namespace inet {
namespace queue {

void PacketDuplicatorBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        outputGate = gate("out");
        consumer = dynamic_cast<IPacketConsumer *>(outputGate->getPathEndGate()->getOwnerModule());
        if (consumer != nullptr)
            consumer->addListener(this);
    }
    else if (stage == INITSTAGE_LAST) {
        checkPushPacketSupport(inputGate);
        checkPushPacketSupport(outputGate);
    }
}

void PacketDuplicatorBase::pushPacket(Packet *packet, cGate *gate)
{
    int numDuplicates = getNumPacketDuplicates(packet);
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

void PacketDuplicatorBase::notifyCanPushPacket(cGate *gate)
{
    for (auto listener : consumerListeners)
        listener->notifyCanPushPacket(inputGate);
}

} // namespace queue
} // namespace inet

