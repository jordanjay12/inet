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

#ifndef __INET_PACKETCONSUMERBASE_H
#define __INET_PACKETCONSUMERBASE_H

#include <algorithm>
#include "inet/common/newqueue/contract/IPacketConsumer.h"

namespace inet {
namespace queue {

class INET_API PacketConsumerBase : public IPacketConsumer
{
  protected:
    std::vector<IListener *> listeners;

  public:
    virtual int getNumPushablePackets(cGate *gate) override {  return -1; }
    virtual bool canPushAnyPacket(cGate *gate) override { return true; }
    virtual bool canPushPacket(Packet *packet, cGate *gate) override { return true; }
    virtual void addListener(IListener *listener) override { listeners.push_back(listener); }
    virtual void removeListener(IListener *listener) override { listeners.erase(find(listeners.begin(), listeners.end(), listener)); }
};

} // namespace queue
} // namespace inet

#endif // ifndef __INET_PACKETCONSUMERBASE_H

