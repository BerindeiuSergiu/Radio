//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "Sink.h"

Define_Module(Sink);

void Sink::initialize()
{
    lifetimeSignal = registerSignal("lifetime");
    lifetimeSignalPrio0 = registerSignal("lifetimePriority0");
    lifetimeSignalPrio1 = registerSignal("lifetimePriority1");
    lifetimeSignalPrio2 = registerSignal("lifetimePriority2");

    prioTimes[0].setName("high priority mean delay");
    prioTimes[1].setName("medium priority mean delay");
    prioTimes[2].setName("low priority mean delay");
}

void Sink::handleMessage(cMessage *msg)
{
    simtime_t lifetime = simTime() - msg->getCreationTime();

    if (msg->arrivedOn("rxPackets")) {
        int priority = 0;
        if (msg->hasPar("priority")) {
            priority = (int)msg->par("priority");
        }

        double delay = lifetime.dbl();
        totalPackets[priority]++;
        totalTime[priority] += delay;

        EV << "Received " << msg->getName() << ", delay: " << delay << "s, priority: " << priority << "\n";

        if (totalPackets[priority] > 0) {
            prioTimes[priority].record(totalTime[priority] / totalPackets[priority]);
        }

        if (priority == 0 && totalPackets[0] == 10) {
            double hpMeanDelay = totalTime[0] / totalPackets[0];
            cPar& meanDelayHP_p = par("meanDelayHP");
            meanDelayHP_p.setDoubleValue(hpMeanDelay);
            EV << "Real Mean Delay: " << hpMeanDelay << "\n";

            totalTime[0] = 0;
            totalPackets[0] = 0;
        }

        delete msg;
        return;
    }

    delete msg;
}
