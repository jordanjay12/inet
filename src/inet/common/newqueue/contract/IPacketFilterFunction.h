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

#ifndef __INET_IPACKETFILTERFUNCTION_H
#define __INET_IPACKETFILTERFUNCTION_H

#include "inet/common/packet/Packet.h"

namespace inet {
namespace queue {

/**
 * This class defines the interface for packet filter functions.
 */
class INET_API IPacketFilterFunction
{
  public:
    virtual ~IPacketFilterFunction() {}

    /**
     * Returns true if the filter drops the given packet.
     */
    virtual bool filterPacket(Packet *packet) const = 0;
};

} // namespace queue
} // namespace inet

#endif // ifndef __INET_IPACKETFILTERFUNCTION_H

