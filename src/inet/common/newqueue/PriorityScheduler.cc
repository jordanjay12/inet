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

#include "inet/common/newqueue/PriorityScheduler.h"

namespace inet {
namespace queue {

Define_Module(PriorityScheduler);

void PriorityScheduler::initialize(int stage)
{
    PacketSchedulerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        for (auto inputProvider : inputProviders)
            inputCollections.push_back(check_and_cast<IPacketCollection *>(inputProvider));
}

int PriorityScheduler::schedulePacket() const
{
    for (int i = 0; i < (int)inputProviders.size(); i++) {
        auto inputProvider = inputProviders[i];
        if (inputProvider->canPopSomePacket(inputGates[i]->getPathStartGate()))
            return i;
    }
    return -1;
}

int PriorityScheduler::getNumPackets()
{
    int size = 0;
    for (auto inputCollection : inputCollections)
        size += inputCollection->getNumPackets();
    return size;
}

Packet *PriorityScheduler::getPacket(int index)
{
    for (auto inputCollection : inputCollections) {
        auto numPackets = inputCollection->getNumPackets();
        if (index < numPackets)
            return inputCollection->getPacket(index);
        else
            index -= numPackets;
    }
    throw cRuntimeError("Invalid operation");
}

void PriorityScheduler::removePacket(Packet *packet)
{
    for (auto inputCollection : inputCollections) {
        for (int j = 0; j < inputCollection->getNumPackets(); j++) {
            if (inputCollection->getPacket(j) == packet) {
                inputCollection->removePacket(packet);
                return;
            }
        }
    }
    throw cRuntimeError("Invalid operation");
}

} // namespace queue
} // namespace inet

