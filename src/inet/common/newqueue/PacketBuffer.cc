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

#include "inet/common/newqueue/PacketBuffer.h"
#include "inet/common/Simsignals.h"
#include "inet/common/StringFormat.h"

namespace inet {
namespace queue {

Define_Module(PacketBuffer);

void PacketBuffer::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        displayStringTextFormat = par("displayStringTextFormat");
        frameCapacity = par("frameCapacity");
        byteCapacity = par("byteCapacity");
    }
}

bool PacketBuffer::canAddPacket(Packet *packet)
{
    return (frameCapacity == -1 || (int)packets.size() + 1 <= frameCapacity) &&
           (byteCapacity == -1 || totalLength + packet->getTotalLength() <= B(byteCapacity));
}

void PacketBuffer::addPacket(IListener *listener, Packet *packet)
{
    // TODO: isOverloaded, dropperFunction?
    if (!canAddPacket(packet) && !makeRoomForPacket(listener, packet)) {
        // TODO: first insert, then drop, use dropFunction (dropTail/dropHead)
        EV_INFO << "Dropping packet " << packet->getName() << " at the buffer.\n";
        listener->notifyPacketRemoved(packet);
        PacketDropDetails details;
        details.setReason(QUEUE_OVERFLOW);
        details.setLimit(frameCapacity);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
    }
    else {
        EV_INFO << "Adding packet " << packet->getName() << " to the buffer.\n";
        totalLength += packet->getTotalLength();
        packets.push_back({listener, packet});
        updateDisplayString();
        emit(packetEnqueuedSignal, packet); // TODO: packetAddedSignal?
    }
}

void PacketBuffer::removePacket(IListener *listener, Packet *packet)
{
    EV_INFO << "Removing packet " << packet->getName() << " from the buffer.\n";
    totalLength -= packet->getTotalLength();
    packets.erase(find(packets.begin(), packets.end(), std::pair<IListener *, Packet *>(listener, packet)));
    updateDisplayString();
    emit(packetDequeuedSignal, packet); // TODO: packetRemovedSignal?
    listener->notifyPacketRemoved(packet);
}

void PacketBuffer::updateDisplayString()
{
    auto text = StringFormat::formatString(displayStringTextFormat, [&] (char directive) {
        static std::string result;
        switch (directive) {
            case 'p':
                result = std::to_string(packets.size());
                break;
            case 'l':
                result = totalLength.str();
                break;
            case 'n':
                result = !packets.empty() ? packets[0].second->getFullName() : "";
                break;
            default:
                throw cRuntimeError("Unknown directive: %c", directive);
        }
        return result.c_str();
    });
    getDisplayString().setTagArg("t", 0, text);
}

} // namespace queue
} // namespace inet

