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

#include "inet/common/INETUtils.h"
#include "inet/common/newqueue/RedDropper.h"

namespace inet {
namespace queue {

Define_Module(RedDropper);

RedDropper::~RedDropper()
{
    delete[] minths;
    delete[] maxths;
    delete[] maxps;
    delete[] pkrates;
    delete[] count;
    q_time = 0;
}

void RedDropper::initialize(int stage)
{
    MultiPacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        wq = par("wq");
        if (wq < 0.0 || wq > 1.0)
            throw cRuntimeError("Invalid value for wq parameter: %g", wq);

        minths = new double[numGates];
        maxths = new double[numGates];
        maxps = new double[numGates];
        pkrates = new double[numGates];
        count = new double[numGates];

        cStringTokenizer minthTokens(par("minths"));
        cStringTokenizer maxthTokens(par("maxths"));
        cStringTokenizer maxpTokens(par("maxps"));
        cStringTokenizer pkrateTokens(par("pkrates"));
        for (int i = 0; i < numGates; ++i) {
            minths[i] = minthTokens.hasMoreTokens() ? utils::atod(minthTokens.nextToken()) :
                (i > 0 ? minths[i - 1] : 5.0);
            maxths[i] = maxthTokens.hasMoreTokens() ? utils::atod(maxthTokens.nextToken()) :
                (i > 0 ? maxths[i - 1] : 50.0);
            maxps[i] = maxpTokens.hasMoreTokens() ? utils::atod(maxpTokens.nextToken()) :
                (i > 0 ? maxps[i - 1] : 0.02);
            pkrates[i] = pkrateTokens.hasMoreTokens() ? utils::atod(pkrateTokens.nextToken()) :
                (i > 0 ? pkrates[i-1] : 150);
            count[i] = -1;

            if (minths[i] < 0.0)
                throw cRuntimeError("minth parameter must not be negative");
            if (maxths[i] < 0.0)
                throw cRuntimeError("maxth parameter must not be negative");
            if (minths[i] >= maxths[i])
                throw cRuntimeError("minth must be smaller than maxth");
            if (maxps[i] < 0.0 || maxps[i] > 1.0)
                throw cRuntimeError("Invalid value for maxp parameter: %g", maxps[i]);
            if (pkrates[i] < 0.0)
                throw cRuntimeError("Invalid value for pkrates parameter: %g", pkrates[i]);
        }
    }
}

bool RedDropper::filterPacket(cGate *gate, Packet *packet)
{
    const int i = gate->getIndex();
    ASSERT(i >= 0 && i < numGates);
    const double minth = minths[i];
    const double maxth = maxths[i];
    const double maxp = maxps[i];
    const double pkrate = pkrates[i];
    const int queueLength = getNumPackets();

    if (queueLength > 0)
    {
        // TD: This following calculation is only useful when the queue is not empty!
        avg = (1 - wq) * avg + wq * queueLength;
    }
    else
    {
        // TD: Added behaviour for empty queue.
        const double m = SIMTIME_DBL(simTime() - q_time) * pkrate;
        avg = pow(1 - wq, m) * avg;
    }

    if (minth <= avg && avg < maxth)
    {
        count[i]++;
        const double pb = maxp * (avg - minth) / (maxth - minth);
        const double pa = pb / (1 - count[i] * pb); // TD: Adapted to work as in [Floyd93].
        if (dblrand() < pa)
        {
            EV << "Random early packet drop (avg queue len=" << avg << ", pa=" << pa << ")\n";
            count[i] = 0;
            return true;
        }
    }
    else if (avg >= maxth) {
        EV << "Avg queue len " << avg << " >= maxth, dropping packet.\n";
        count[i] = 0;
        return true;
    }
    else if (queueLength >= maxth) {    // maxth is also the "hard" limit
        EV << "Queue len " << queueLength << " >= maxth, dropping packet.\n";
        count[i] = 0;
        return true;
    }
    else
    {
        count[i] = -1;
    }

    return false;
}

// TODO: revive this code, this is more complicated than it seems at first, because the output queues can be popped anytime
//void RedDropper::sendOut(Packet *packet)
//{
//    AlgorithmicDropperBase::sendOut(packet);
//    // TD: Set the time stamp q_time when the queue gets empty.
//    const int queueLength = getNumPackets();
//    if (queueLength == 0)
//        q_time = simTime();
//}

} // namespace queue
} // namespace inet

