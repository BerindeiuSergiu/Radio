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

#include "Scheduler.h"
#include <string>
#include <algorithm> // for std::min

Define_Module(Scheduler);



Scheduler::Scheduler()
{
    selfMsg = nullptr;
}

Scheduler::~Scheduler()
{
    cancelAndDelete(selfMsg);
    if (csvFile.is_open()) {
        csvFile.close();
    }
}


void Scheduler::initialize()
{
    NrOfHighPriorityUsers = par("NrOfHighPriorityUsers").intValue();
    NrOfMediumPriorityUsers = par("NrOfMediumPriorityUsers").intValue();
    NrOfLowPriorityUsers = par("NrOfLowPriorityUsers").intValue();

    NrUsers = par("gateSize").intValue();
    NrOfChannels = par("NrOfChannels").intValue();
    schedulingPeriod = par("schedulingPeriod").doubleValue();
    HighPriorityWeight = par("HighPriorityWeight").intValue();
    MediumPriorityWeight = par("MediumPriorityWeight").intValue();
    LowPriorityWeight = par("LowPriorityWeight").intValue();

    for (int i = 0; i < NrOfHighPriorityUsers; i++) {
        prio[i] = 0;
        userWeights[i] = HighPriorityWeight;
    }

    for (int i = 0; i < NrOfMediumPriorityUsers; i++) {
        prio[NrOfHighPriorityUsers + i] = 1;
        userWeights[NrOfHighPriorityUsers + i] = MediumPriorityWeight;
    }

    for (int i = 0; i < NrOfLowPriorityUsers; i++) {
        prio[NrOfHighPriorityUsers + NrOfMediumPriorityUsers + i] = 2;
        userWeights[NrOfHighPriorityUsers + NrOfMediumPriorityUsers + i] = LowPriorityWeight;
    }

    for (int i = 0; i < 10; i++) {
        q[i] = 0;
        NrBlocks[i] = 0;
        last_time_served[i] = simTime();
    }

    selfMsg = new cMessage("selfMsg");
    scheduleAt(simTime(), selfMsg);

    // Initialize output vectors (like professor's FLC version)
    delayHighPriority.setName("high priority mean delay");
    delayMediumPriority.setName("medium priority mean delay");
    delayLowPriority.setName("low priority mean delay");
    weightVector.setName("weight vector");

    // Open CSV file for recording data
    csvFile.open("results/scheduler_data.csv", std::ios::app);  // append mode
    if (csvFile.is_open()) {
        csvFile << "time,weight\n";  // CSV header
    }
}


void Scheduler::startCollection()
{
    collecting = true;
    pending = NrUsers;

    for (int i = 0; i < NrUsers; i++) {
        cMessage *req = new cMessage("qlenReq");
        req->setKind(KIND_QREQ);
        send(req, "txScheduling", i);
    }
}

void Scheduler::finalizeAndSchedule()
{
    collecting = false;
    std::vector<int> grant(NrUsers, 0);

    auto anyBacklog = [&]() -> bool {
        for (int i = 0; i < NrUsers; i++)
            if (qlen[i] > 0) return true;
        return false;
    };

    int remaining = B;
    while (remaining > 0 && anyBacklog()) {
        int best = -1;
        double bestP = -1.0;

        // tie-break: scan users starting at rrTiePtr
        for (int k = 0; k < NrUsers; k++) {
            int i = (rrTiePtr + k) % NrUsers;
            if (qlen[i] <= 0) continue;

            // p[i] = (tnow - lastServed[i]) * W[i]
            // RR is obtained when all W[i] = 1.
            simtime_t dt = simTime() - lastServed[i];
            double p = dt.dbl() * (double)weights[i];

            if (p > bestP) {
                bestP = p;
                best = i;
            }
        }

        if (best < 0) break;

        grant[best] += 1;
        qlen[best] -= 1;
        lastServed[best] = simTime();
        rrTiePtr = (best + 1) % NrUsers;
        remaining--;
    }

    // this is used to send nr of blocks to each user
    for (int i = 0; i < NrUsers; i++) {
        cMessage *cmd = new cMessage("grant");
        cmd->addPar("nrOfBlocks") = grant[i];
        send(cmd, "txScheduling", i);
    }

    // Record weight vectors (like professor's FLC version)
    if (NrOfHighPriorityUsers > 0) {
        weightVector.record(userWeights[0]);  // Record first high priority user weight
        
        // Also write to CSV
        if (csvFile.is_open()) {
            csvFile << simTime().dbl() << "," << userWeights[0] << "\n";
            csvFile.flush();
        }
    }

    // continue in the next cycle
    simtime_t Tc = par("schedulingPeriod");
    scheduleAt(simTime() + Tc, selfMsg);
}

//void Scheduler::handleMessage(cMessage *msg)
//{
//    int userWeights[NrUsers];
//    if (msg == selfMsg){
//        //finds out the length of each queue !!
//	  for(int i =0;i<NrUsers;i++){
//            cMessage *cmd = new cMessage("cmd");
//            //set parameter value, e.g., nr of blocks to be sent from the queue by user i
//            send(cmd,"txScheduling",i);
//        }
//        scheduleAt(simTime()+par("schedulingPeriod").doubleValue(), selfMsg);
//
//    }
//
//}


void Scheduler::handleMessage(cMessage *msg)
{
    for (int i = 0; i < NrOfHighPriorityUsers; i++) {
        userWeights[i] = par("HighPriorityWeight").intValue();
    }

    for (int i = 0; i < NrOfMediumPriorityUsers; i++) {
        userWeights[NrOfHighPriorityUsers + i] = par("MediumPriorityWeight").intValue();
    }

    for (int i = 0; i < NrOfLowPriorityUsers; i++) {
        userWeights[NrOfHighPriorityUsers + NrOfMediumPriorityUsers + i] = par("LowPriorityWeight").intValue();
    }

    if (msg == selfMsg) {
        proportionalFair();
        scheduleAt(simTime() + schedulingPeriod, selfMsg);
        return;
    }

    for (int i = 0; i < NrUsers; i++) {
        if (msg->arrivedOn("rxInfo", i)) {
            q[i] = msg->par("ql_info");
            EV << "[Scheduler] rxInfo user " << i << " q=" << q[i] << "\n";
            delete msg;
            return;
        }
    }

    if (msg->arrivedOn("rxFLC")) {
        if (msg->hasPar("newW")) {
            HighPriorityWeight = (int)msg->par("newW");
        }
        else if (msg->hasPar("deltaW")) {
            HighPriorityWeight += (int)msg->par("deltaW");
        }
        if (HighPriorityWeight < 1) HighPriorityWeight = 1;
        if (HighPriorityWeight > 100) HighPriorityWeight = 100;
        par("HighPriorityWeight").setIntValue(HighPriorityWeight);
        EV << "[Scheduler] Applied FLC HPWeight=" << HighPriorityWeight << "\n";
        delete msg;
        return;
    }

    delete msg;
}

void Scheduler::proportionalFair()
{
    EV << "[proportionalFair] Called at time " << simTime() << "\n";
    int allocatedBlocks[10];
    for (int i = 0; i < NrUsers; i++) {
        allocatedBlocks[i] = 0;
    }

    for (int c = 0; c < NrOfChannels; c++) {
        for (int j = 0; j < NrUsers; j++) {
            r[j] = uniform(0.1, 10);
            p[j] = r[j] * ((simTime() - last_time_served[j]).dbl()) * userWeights[j];
        }

        int servedUser = -1;
        double maxScore = 0;

        for (int j = 0; j < NrUsers; j++) {
            if ((q[j] - allocatedBlocks[j]) < 1) continue;
            if (allocatedBlocks[j] >= userWeights[j]) continue;
            if (p[j] > maxScore) {
                maxScore = p[j];
                servedUser = j;
            }
        }

        if (servedUser != -1) {
            allocatedBlocks[servedUser]++;
        }
    }

    for (int i = 0; i < NrUsers; i++) {
        if (allocatedBlocks[i] > 0) {
            int grant = std::min(allocatedBlocks[i], q[i]);
            q[i] -= grant;
            last_time_served[i] = simTime();

            cMessage *cmd = new cMessage("cmd");
            cmd->addPar("nrBlocks").setLongValue(grant);
            cmd->addPar("priorityType").setLongValue(prio[i]);
            EV << "[Scheduler] send grant=" << grant << " user=" << i << " prio=" << prio[i] << "\n";
            send(cmd, "txScheduling", i);
        }
    }

    // Record weight vector data to CSV (record FLC adjustments)
    if (NrOfHighPriorityUsers > 0 && csvFile.is_open()) {
        csvFile << simTime().dbl() << "," << userWeights[0] << "\n";
        csvFile.flush();
        EV << "[CSV] Recorded weight=" << userWeights[0] << " at time=" << simTime() << "\n";
    }
}

