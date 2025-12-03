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
        int usersCount;
//= default(3);

        @display("bgb=660,329");
    submodules:
        scheduler: Scheduler {
            gateSize = parent.usersCount;
        }
        sink: Sink {
            gateSize = parent.usersCount;
        }
        user[usersCount]: User;
    connections:
        for i=0..usersCount - 1 {
            scheduler.txScheduling[i] --> user[i].rxScheduling;
            user[i].txPackets --> sink.rxPackets[i];
        }
}
