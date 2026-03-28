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
#include <fstream>

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
    int NrOfChannels;

    // Legacy fields retained for old methods (not used in new proportionalFair path)
    int B = 0;
    static const int KIND_QREQ = 1001;
    static const int KIND_QRSP = 1002;
    bool collecting = false;
    int pending = 0;
    std::vector<int> weights;
    std::vector<int> qlen;
    std::vector<simtime_t> lastServed;
    int rrTiePtr = 0;

    int NrOfHighPriorityUsers;
    int NrOfMediumPriorityUsers;
    int NrOfLowPriorityUsers;

    int HighPriorityWeight;
    int MediumPriorityWeight;
    int LowPriorityWeight;

    int userWeights[10];
    int q[10];
    double p[10];
    double r[10];
    int prio[10];
    int NrBlocks[10];
    simtime_t last_time_served[10];

    double schedulingPeriod;

    // Output vectors for data recording (like professor's version)
    cOutVector delayHighPriority;
    cOutVector delayMediumPriority;
    cOutVector delayLowPriority;
    cOutVector weightVector;

    // CSV file for manual recording
    std::ofstream csvFile;

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

    //for proportional fair
    virtual void proportionalFair();
    void finalizeAndSchedule();
    void startCollection();
};

#endif
