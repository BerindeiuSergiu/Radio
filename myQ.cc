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

#include "myQ.h"

Define_Module(MyQ);

static const int KIND_QREQ = 1001;
static const int KIND_QRSP = 1002;

void MyQ::initialize()
{
    queue.setName("queue");
}

//void MyQ::handleMessage(cMessage *msg)
//{
//    //int ql;
//    //ql = queue.getLength();
//    if (msg->arrivedOn("rxPackets")){
//        queue.insert(msg);
//    } else if (msg->arrivedOn("rxScheduling")){
//        //read parameters from msg
////int nrOfRadioBlocks = (int)msg->par("nrOfBlocks");
//        delete msg;
//        //empty the queue !
//        while(!queue.isEmpty()){ //for(int j = 0;j < nrOfRadioBlocks;j++ )
//          msg = (cMessage *)queue.pop();
//          send(msg, "txPackets");
//        }
//    }
//}

//void MyQ::handleMessage(cMessage *msg)
//{
//    if (msg->arrivedOn("rxPackets")) {
//            queue.insert(msg);
//            EV << "[Queue] Packet arrived, queue length now: " << queue.getLength() << "\n";
//        }
//        else if (msg->arrivedOn("rxScheduling")) {
//            int nrOfBlocks = 1;// setup def
//
//            //read parameters from msg
//            if (msg->hasPar("nrOfBlocks")) {
//                nrOfBlocks = (int) msg->par("nrOfBlocks");
//            }
//
//            int queueLengthBefore = queue.getLength();
//            EV << "[Queue] Scheduling command received: " << nrOfBlocks
//               << " blocks allowed, queue length: " << queueLengthBefore << "\n";
//
//            delete msg;
//
//            // se scoate maxim nrOfBlocks din coada
//            int packetsSent = 0;
//            for (int j = 0; j < nrOfBlocks && !queue.isEmpty(); ++j) {
//                cMessage *pkt = (cMessage *)queue.pop();
//                send(pkt, "txPackets");
//                packetsSent++;
//            }
//
//            EV << "[Queue] Sent " << packetsSent << " packets, "
//               << queue.getLength() << " remaining in queue\n";
//        }
//        else {
//            // fallback default
//            delete msg;
//        }
//}

void MyQ::handleMessage(cMessage *msg)
{
    if (msg->arrivedOn("rxPackets")) {
        queue.insert(msg);
        cMessage *qInfo = new cMessage("qInfo");
        qInfo->addPar("ql_info");
        qInfo->par("ql_info").setLongValue(queue.getLength());
        send(qInfo, "txInfo");
        return;
    }

    if (msg->arrivedOn("rxScheduling")) {
        int nrOfBlocks = 0;
        int priority = 0; // default

        if (msg->hasPar("nrOfBlocks")) {
            nrOfBlocks = (int)msg->par("nrOfBlocks");
        }
        else if (msg->hasPar("nrBlocks")) {
            nrOfBlocks = (int)msg->par("nrBlocks");
        }
        if (msg->hasPar("priorityType")) {
            priority = (int)msg->par("priorityType");
        }

        EV << "[MyQ] grant=" << nrOfBlocks << " priority=" << priority << " queue=" << queue.getLength() << "\n";

        delete msg;

        for (int j = 0; j < nrOfBlocks && !queue.isEmpty(); ++j) {
            cMessage *pkt = (cMessage *)queue.pop();
            pkt->addPar("priority");
            pkt->par("priority").setLongValue(priority);
            send(pkt, "txPackets");
        }

        // SEND queue length info on each scheduling decision
        cMessage *qInfo = new cMessage("qInfo");
        qInfo->addPar("ql_info");
        qInfo->par("ql_info").setLongValue(queue.getLength());
        send(qInfo, "txInfo"); // ADDED: out to scheduler

        return;
    }

    delete msg;
}

