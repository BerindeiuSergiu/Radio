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

#include <iostream>
#include <omnetpp.h>
#include "gen_flc.h"

using namespace std;
using namespace omnetpp;

#define ev EV

Define_Module(GeneratorFLC);

void GeneratorFLC::initialize()
{
    flc_time = 1;
    sendMessageEvent = new cMessage("sendMessageEvent");
    scheduleAt(0, sendMessageEvent);
}

void GeneratorFLC::handleMessage(cMessage *msg)
{
    simtime_t next_time;
    ASSERT(msg==sendMessageEvent);
    ev << " START FLC GENERATOR\n";
    cMessage *msgr = new cMessage("start_flc");
    send(msgr,"out");
    next_time = simTime()+ flc_time;
    scheduleAt(next_time,sendMessageEvent);
}

void GeneratorFLC::finish()
{
    ev << "*** Module: " << getFullPath() << "***" << endl;
}
