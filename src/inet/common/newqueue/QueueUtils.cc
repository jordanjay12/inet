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

#include "inet/common/newqueue/contract/IPacketQueueElement.h"
#include "inet/common/newqueue/QueueUtils.h"

namespace inet {
namespace queue {

void animateSend(Packet *packet, cGate *gate)
{
    auto envir = getEnvir();
    if (envir->isGUI()) {
        packet->setSentFrom(gate->getOwnerModule(), gate->getId(), simTime());
        envir->beginSend(packet);
        while (gate->getNextGate() != nullptr) {
            envir->messageSendHop(packet, gate);
            gate = gate->getNextGate();
        }
        envir->endSend(packet);
    }
}

void checkPushPacketSupport(cGate *gate)
{
    auto startGate = gate->getPathStartGate();
    auto endGate = gate->getPathEndGate();
    auto startElement = check_and_cast<IPacketQueueElement *>(startGate->getOwnerModule());
    auto endElement = check_and_cast<IPacketQueueElement *>(endGate->getOwnerModule());
    if (!endElement->supportsPushPacket(endGate))
        throw cRuntimeError("Incompatible connection between gates");
    if (!startElement->supportsPushPacket(startGate))
        throw cRuntimeError("Incompatible connection between gates");
}

void checkPopPacketSupport(cGate *gate)
{
    auto startGate = gate->getPathStartGate();
    auto endGate = gate->getPathEndGate();
    auto startElement = check_and_cast<IPacketQueueElement *>(startGate->getOwnerModule());
    auto endElement = check_and_cast<IPacketQueueElement *>(endGate->getOwnerModule());
    if (!endElement->supportsPopPacket(endGate))
        throw cRuntimeError("Incompatible connection between gates");
    if (!startElement->supportsPopPacket(startGate))
        throw cRuntimeError("Incompatible connection between gates");
}

} // namespace queue
} // namespace inet

