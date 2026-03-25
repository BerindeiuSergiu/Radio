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

#ifndef __SCHED_TST_SCHEDULER_H_
#define __SCHED_TST_SCHEDULER_H_

#include <omnetpp.h>
#include <vector>

using namespace omnetpp;

/**
 * p[i] = (tnow - t_last_served[i]) * W[i]         (WRR)
 */
class Scheduler : public cSimpleModule
{
public:
    Scheduler();
    ~Scheduler();
private:
    cMessage *selfMsg;
    int NrUsers;
    int B = 0;


    static const int KIND_QREQ = 1001; // request queue length
    static const int KIND_QRSP = 1002; // response with queue length

    // for queue length collection
    bool collecting = false;
    int pending = 0;

    // course parameters/state
    std::vector<int> weights;          // W[i]
    std::vector<int> qlen;             // queue lengths collected at start of Tc
    std::vector<simtime_t> lastServed; // t_last_served[i]

    int rrTiePtr = 0; // tie-break pointer to keep it fair when p[i] ties

    void startCollection();
    void finalizeAndSchedule();


  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

#endif
