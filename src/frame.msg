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
//EX:
	#include <bitset>
//	#include <vector>
	typedef std::bitset<9> parity_vec;
	typedef std::bitset<8> char_vec;
	
//	typedef std::vector <std::bitset<8> > char_vec;
	
}}
class noncobject parity_vec;
class noncobject char_vec; 


packet Frame {
    @customize(true);  // see the generated C++ header for more info
    char_vec payload [32];	//max: 256 bits	.. min: 14 bits .. by Ev Definition and assumption.
    parity_vec parity; 		//max: 9 bits .. min: 5
}
