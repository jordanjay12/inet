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

#include "inet/common/newqueue/PacketProducer.h"
#include "inet/common/newqueue/QueueUtils.h"
#include "inet/common/packet/chunk/BitCountChunk.h"
#include "inet/common/packet/chunk/BitsChunk.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/StringFormat.h"

namespace inet {
namespace queue {

Define_Module(PacketProducer);

void PacketProducer::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        outputGate = gate("out");
        packetNameFormat = par("packetNameFormat");
        packetRepresentation = par("packetRepresentation");
        packetLengthParameter = &par("packetLength");
        productionIntervalParameter = &par("productionInterval");
        consumer = check_and_cast<IPacketConsumer *>(outputGate->getPathEndGate()->getOwnerModule());
        consumer->addListener(this);
        productionTimer = new cMessage("ProductionTimer");
        WATCH(numPacket);
        WATCH(totalLength);
    }
    else if (stage == INITSTAGE_LAST)
        checkPushPacketSupport(outputGate);
}

void PacketProducer::handleMessage(cMessage *message)
{
    if (message == productionTimer) {
        if (consumer->canPushAnyPacket(outputGate->getPathEndGate())) {
            producePacket();
            schedulerProductionTimer();
        }
    }
    else
        throw cRuntimeError("Unknown message");
}

void PacketProducer::schedulerProductionTimer()
{
    scheduleAt(simTime() + productionIntervalParameter->doubleValue(), productionTimer);
}

void PacketProducer::producePacket()
{
    auto packet = createPacket();
    EV_INFO << "Producing packet " << packet->getName() << "." << endl;
    animateSend(packet, outputGate);
    consumer->pushPacket(packet, outputGate->getPathEndGate());
    numPacket++;
    totalLength += packet->getTotalLength();
}

const char *PacketProducer::createPacketName()
{
    return StringFormat::formatString(packetNameFormat, [&] (char directive) {
        static std::string result;
        switch (directive) {
            case 'n':
                result = getFullName();
                break;
            case 'c':
                result = std::to_string(numPacket);
                break;
            default:
                throw cRuntimeError("Unknown directive: %c", directive);
        }
        return result.c_str();
    });
}

const Ptr<const Chunk> PacketProducer::createPacketContent()
{
    auto packetLength = b(packetLengthParameter->intValue());
    if (!strcmp(packetRepresentation, "bitCount"))
        return makeShared<BitCountChunk>(packetLength);
    else if (!strcmp(packetRepresentation, "bits")) {
        static int total = 0;
        const auto& packetContent = makeShared<BitsChunk>();
        std::vector<bool> bits;
        bits.resize(b(packetLength).get());
        for (int i = 0; i < (int)bits.size(); i++)
            bits[i] = (total + i) % 2 == 0;
        total += bits.size();
        packetContent->setBits(bits);
        return packetContent;
    }
    else if (!strcmp(packetRepresentation, "byteCount"))
        return makeShared<ByteCountChunk>(packetLength);
    else if (!strcmp(packetRepresentation, "bytes")) {
        static int total = 0;
        const auto& packetContent = makeShared<BytesChunk>();
        std::vector<uint8_t> bytes;
        bytes.resize(B(packetLength).get());
        for (int i = 0; i < (int)bytes.size(); i++)
            bytes[i] = (total + i) % 256;
        total += bytes.size();
        packetContent->setBytes(bytes);
        return packetContent;
    }
    else
        throw cRuntimeError("Unknown representation");
}

Packet *PacketProducer::createPacket()
{
    const auto packetName = createPacketName();
    const auto& packetContent = createPacketContent();
    return new Packet(packetName, packetContent);
}

void PacketProducer::notifyCanPushPacket(cGate *gate)
{
    if (gate->getPathStartGate() == outputGate && !productionTimer->isScheduled()) {
        producePacket();
        schedulerProductionTimer();
    }
}

} // namespace queue
} // namespace inet

