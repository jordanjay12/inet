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

package inet.common.newqueue;

import inet.common.newqueue.base.PacketQueueBase;
import inet.common.newqueue.contract.IPacketQueue;

simple PacketQueue extends PacketQueueBase like IPacketQueue
{
    parameters:
        int frameCapacity = default(-1); // maximum number of packets in the queue, no limit by default
        int byteCapacity = default(-1); // maximum total length of packets in the queue, no limit by default
        bool enableDropping = default(true); // true means packets are dropped when the queue is overloaded
        string dropperClass @enum("dropHead", "dropTail") = default(""); // determines which packets are dropped when the queue is overloaded, undefined by default; the parameter must be the name of a C++ class which implements the IPacketDropper C++ interface and is registered via Register_Class
        string comparatorClass = default(""); // determines the order of packets in the queue, insertion order by default; the parameter must be the name of a C++ class which implements the IPacketComparator C++ interface and is registered via Register_Class
        string bufferModule = default(""); // relative module path to the IPacketBuffer module used by this queue, use implicite buffer by default
        string displayStringTextFormat = default("%p packets\n%l"); // %p number of packet, %l total length, %n name of next popped packet
        @class(PacketQueue);
        @signal[packetEnqueued](type=inet::Packet);
        @signal[packetDequeued](type=inet::Packet);
        @signal[packetDropped](type=inet::Packet);
        @signal[packetGenerated](type=inet::Packet);
        @statistic[queueingTime](title="queueing times"; source=queueingTime(packetDequeued); record=histogram,vector; unit=s; interpolationmode=none);
        @statistic[queueLength](title="queue length"; source=count(packetEnqueued) - count(packetDequeued) - count(packetDropped) - count(packetGenerated); record=max,timeavg,vector; interpolationmode=sample-hold);
        @statistic[packetDropQueueOverflow](title="packet drops: queue overflow"; source=packetDropReasonIsQueueOverflow(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
}
