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

//
// TODO generated message class
//
cplusplus {{
//	Any includes goes here
	#include <bitset>
	typedef std::bitset<10> parity_bits;
	typedef std::bitset<8> char_bits;
}}
class noncobject parity_bits;
class noncobject char_bits; 


packet Frame {
    @customize(true);  // see the generated C++ header for more info
    //max char count is 32, after byte stuffing, will be 64
    //in bits 256*2
    char_bits payload [64];	//max: 256 bits	.. min: 14 bits .. by Ev Definition and assumption.
    parity_bits parity; 		//max: 10 bits .. min: 5
    int ACK;
    int NACK;
    char_bits start_flag;
    char_bits end_flag;
    
}
