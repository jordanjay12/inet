//
// Copyright (C) 2018 OpenSim Ltd.
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
// @author: Zoltan Bojthe
//

#include "inet/common/ProtocolGroup.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#include "inet/transportlayer/tcp_common/TcpProtocolDissector.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::tcp, TcpProtocolDissector);

void TcpProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    const auto& header = packet->popAtFront<tcp::TcpHeader>();
    callback.startProtocolDataUnit(&Protocol::tcp);
    callback.visitChunk(header, &Protocol::tcp);
    if (packet->getDataLength() != b(0)) {
        auto dataProtocol = ProtocolGroup::tcpprotocol.findProtocol(header->getDestPort());
        if (dataProtocol == nullptr)
            dataProtocol = ProtocolGroup::tcpprotocol.findProtocol(header->getSrcPort());
        callback.dissectPacket(packet, dataProtocol);
    }
    callback.endProtocolDataUnit(&Protocol::tcp);
}

} // namespace inet

