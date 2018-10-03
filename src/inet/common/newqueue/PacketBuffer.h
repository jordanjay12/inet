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

#ifndef __INET_PACKETBUFFER_H
#define __INET_PACKETBUFFER_H

#include <algorithm>
#include "inet/common/newqueue/contract/IPacketBuffer.h"

namespace inet {
namespace queue {

class INET_API PacketBuffer : public cSimpleModule, public IPacketBuffer
{
  protected:
    std::vector<IListener *> listeners;

    const char *displayStringTextFormat = nullptr;
    int frameCapacity = -1;
    int byteCapacity = -1;

    b totalLength = b(0);
    std::vector<std::pair<IListener *, Packet *>> packets;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void updateDisplayString();
    virtual bool canAddPacket(Packet *packet);
    virtual bool makeRoomForPacket(IListener *listener, Packet *packet) { return false; }

  public:
    virtual void addPacket(IListener *listener, Packet *packet) override;
    virtual void removePacket(IListener *listener, Packet *packet) override;

    virtual void addListener(IListener *listener) override { listeners.push_back(listener); }
    virtual void removeListener(IListener *listener) override { listeners.erase(find(listeners.begin(), listeners.end(), listener)); }
};

} // namespace queue
} // namespace inet

#endif // ifndef __INET_PACKETBUFFER_H

