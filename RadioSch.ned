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


network RadioSch
{
    parameters:
        int highPriorityUsersCount = default(2);
        int highPriorityWeight = default(4);
        
        int mediumPriorityUsersCount = default(5);
        int mediumPriorityWeight = default(2);
        
        int lowPriorityUsersCount = default(3);
        int lowPriorityWeight = default(1);

        int usersCount = default(highPriorityUsersCount + mediumPriorityUsersCount + lowPriorityUsersCount);
        int NrOfChannels = default(10);

        int schedulingPeriod @unit(ms) = default(1 ms);
        
        double delayLimit;

        @display("bgb=660,329");
    submodules:
        scheduler: Scheduler {
            NrOfHighPriorityUsers = parent.highPriorityUsersCount;
            NrOfMediumPriorityUsers = parent.mediumPriorityUsersCount;
            NrOfLowPriorityUsers = parent.lowPriorityUsersCount;
            gateSize = parent.usersCount;
            NrOfChannels = parent.NrOfChannels;
            HighPriorityWeight = parent.highPriorityWeight;
            MediumPriorityWeight = parent.mediumPriorityWeight;
            LowPriorityWeight = parent.lowPriorityWeight;
            schedulingPeriod = parent.schedulingPeriod;
        }
        sink: Sink {
            gateSize = parent.usersCount;
        }
        user[usersCount]: User;
        flc: FLC {
            @display("p=69,112.3");
        //    config = xmldoc("conf.xml");  // ADDED
        }
        gen: GeneratorFLC {
            @display("p=69.4,138.8");
        }
    connections:
        for i=0..usersCount - 1 {
            scheduler.txScheduling[i] --> user[i].rxScheduling;
            user[i].txPackets --> sink.rxPackets[i];
            user[i].txInfo --> scheduler.rxInfo[i];
            // user[i].txStatus --> scheduler.rxStatus[i];
        }

        gen.out --> flc.in;
        flc.out --> scheduler.rxFLC;  // ADDED FLC-->Scheduler command
}

// NOTE: additional FLC integration can be added in a separate network module if needed.
