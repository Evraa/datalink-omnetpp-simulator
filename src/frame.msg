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
	#include <vector>
	typedef std::vector <bool> message_vec;
}}
class noncobject message_vec;


packet Frame {
    @customize(true);  // see the generated C++ header for more info
    message_vec payload;
    int ACK;
    int frame_seq;
//    int NACK;
}
