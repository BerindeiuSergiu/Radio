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

Define_Module(Scheduler);



Scheduler::Scheduler()
{
    selfMsg = nullptr;
}

Scheduler::~Scheduler()
{
    cancelAndDelete(selfMsg);
}


void Scheduler::initialize()
{
       NrUsers = par("gateSize").intValue();
       B = par("B").intValue();
       if (B < 0) B = 0;

       weights.assign(NrUsers, 1);
       qlen.assign(NrUsers, 0);
       lastServed.assign(NrUsers, SIMTIME_ZERO);

       const char* wightsStr = par("weights").stringValue();

       cStringTokenizer tokenizer(wightsStr, ",");
       std::vector<std::string> tokens = tokenizer.asVector();

       for (int i = 0; i < NrUsers && i < (int)tokens.size(); i++) {
           int w = std::stoi(tokens[i]);
           weights[i] = (w > 0) ? w : 1;
       }

       rrTiePtr = 0;

       selfMsg = new cMessage("tick");
       scheduleAt(simTime(), selfMsg);
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
    // Start of scheduling cycle
       if (msg == selfMsg) {
           startCollection();
           return;
       }

       // Queue length responses from users
       if (msg->getKind() == KIND_QRSP && collecting) {
           int userId = msg->getArrivalGate()->getIndex();
           if (msg->hasPar("qlen"))
               qlen[userId] = (int)msg->par("qlen");
           else
               qlen[userId] = 0;

           delete msg;
           pending--;

           if (pending == 0) {
               finalizeAndSchedule();
           }
           return;
       }

       delete msg;
}
