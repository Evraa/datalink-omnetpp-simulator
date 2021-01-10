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

package data_link;

//
// TODO auto-generated module
//
simple Node
{
    parameters:
        double lambda = default(2.0);
        int M	= default(1000);
        
        double p_corrupt = default(0.7);
        double p_delay = default(0.6);
        int delay_range = default(7);
        double p_loss = default(0.6);
        double p_dup = default(0.6);

        gates:
        input ins[];
        output outs[];
}