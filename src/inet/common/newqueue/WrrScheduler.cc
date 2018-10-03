//
// Copyright (C) 2012 Andras Varga
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

#include "inet/common/INETUtils.h"
#include "inet/common/newqueue/WrrScheduler.h"

namespace inet {
namespace queue {

Define_Module(WrrScheduler);

WrrScheduler::~WrrScheduler()
{
    delete[] weights;
    delete[] buckets;
}

void WrrScheduler::initialize(int stage)
{
    PacketSchedulerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        weights = new int[inputProviders.size()];
        buckets = new int[inputProviders.size()];

        cStringTokenizer tokenizer(par("weights"));
        int i;
        for (i = 0; i < (int)inputProviders.size() && tokenizer.hasMoreTokens(); ++i)
            buckets[i] = weights[i] = (int)utils::atoul(tokenizer.nextToken());

        if (i < (int)inputProviders.size())
            throw cRuntimeError("Too few values given in the weights parameter.");
        if (tokenizer.hasMoreTokens())
            throw cRuntimeError("Too many values given in the weights parameter.");
    }
}

int WrrScheduler::schedulePacket() const
{
    bool isEmpty = true;
    for (int i = 0; i < (int)inputProviders.size(); ++i) {
        if (inputProviders[i]->canPopSomePacket(inputGates[i]->getPathStartGate())) {
            isEmpty = false;
            if (buckets[i] > 0) {
                buckets[i]--;
                return i;
            }
        }
    }

    if (isEmpty)
        return -1;

    int result = -1;
    for (int i = 0; i < (int)inputProviders.size(); ++i) {
        buckets[i] = weights[i];
        if (result == -1 && buckets[i] > 0 && inputProviders[i]->canPopSomePacket(inputGates[i]->getPathStartGate())) {
            buckets[i]--;
            result = i;
        }
    }
    return result;
}

} // namespace queue
} // namespace inet

