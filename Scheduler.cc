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
    NrOfChannels = 10;//read from omnetpp.ini can be removed

    currentUser = 0;
    selfMsg = new cMessage("selfMsg");
    scheduleAt(simTime(), selfMsg);
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
    if (msg == selfMsg) {
        int userIndex = currentUser; //doar un user/runda

        cMessage *cmd = new cMessage("cmd");
        // nr. de pachete pe care are voie sa le trimita din coada userIndex (adica user-ul curent)
        cmd->addPar("nrOfBlocks") = 1;

        EV << "[Scheduler] t=" << simTime() << "s - Scheduling User[" << userIndex 
           << "] with " << cmd->par("nrOfBlocks").longValue() << " blocks\n";

        // comanda merge doar la userIndex
        send(cmd, "txScheduling", userIndex);

        //se calculeaza urmatorul user pentru runda viitoare
        currentUser = (currentUser + 1) % NrUsers;
        
        double periodInSeconds = par("schedulingPeriod").doubleValue() / 1000.0;
        simtime_t nextScheduling = simTime() + periodInSeconds;
        EV << "[Scheduler] Next scheduling at t=" << nextScheduling << "s\n";
        scheduleAt(nextScheduling, selfMsg);
    }
    else {
        delete msg;
    }
}
