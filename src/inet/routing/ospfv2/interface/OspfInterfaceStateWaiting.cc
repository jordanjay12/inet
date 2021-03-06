//
// Copyright (C) 2006 Andras Babos and Andras Varga
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/routing/ospfv2/interface/OspfInterfaceStateDown.h"
#include "inet/routing/ospfv2/interface/OspfInterfaceStateLoopback.h"
#include "inet/routing/ospfv2/interface/OspfInterfaceStateWaiting.h"
#include "inet/routing/ospfv2/messagehandler/MessageHandler.h"
#include "inet/routing/ospfv2/router/OspfArea.h"
#include "inet/routing/ospfv2/router/OspfRouter.h"

namespace inet {
namespace ospf {

void InterfaceStateWaiting::processEvent(OspfInterface *intf, OspfInterface::OspfInterfaceEventType event)
{
    if ((event == OspfInterface::BACKUP_SEEN) ||
        (event == OspfInterface::WAIT_TIMER))
    {
        calculateDesignatedRouter(intf);
    }
    else if (event == OspfInterface::INTERFACE_DOWN) {
        intf->reset();
        changeState(intf, new InterfaceStateDown, this);
    }
    else if (event == OspfInterface::LOOP_INDICATION) {
        intf->reset();
        changeState(intf, new InterfaceStateLoopback, this);
    }
    else if (event == OspfInterface::HELLO_TIMER) {
        if (intf->getType() == OspfInterface::BROADCAST) {
            intf->sendHelloPacket(Ipv4Address::ALL_OSPF_ROUTERS_MCAST);
        }
        else {    // OspfInterface::NBMA
            unsigned long neighborCount = intf->getNeighborCount();
            int ttl = (intf->getType() == OspfInterface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;
            for (unsigned long i = 0; i < neighborCount; i++) {
                Neighbor *neighbor = intf->getNeighbor(i);
                if (neighbor->getPriority() > 0) {
                    intf->sendHelloPacket(neighbor->getAddress(), ttl);
                }
            }
        }
        intf->getArea()->getRouter()->getMessageHandler()->startTimer(intf->getHelloTimer(), intf->getHelloInterval());
    }
    else if (event == OspfInterface::ACKNOWLEDGEMENT_TIMER) {
        intf->sendDelayedAcknowledgements();
    }
}

} // namespace ospf
} // namespace inet

