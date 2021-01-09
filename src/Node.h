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

#ifndef __DATA_LINK_NODE_H_
#define __DATA_LINK_NODE_H_
#include <assert.h>
#include <omnetpp.h>
#include <fstream>
#include <string>
#include <tuple>
#include <queue>
#include <bitset>
#include <vector>
#include <random>
#include <string.h>     //strncmp
#include "orchestrator_order_m.h"
#include "frame_m.h"

using namespace omnetpp;

/**
 * TODO - Generated class
 */
class Node : public cSimpleModule
{
    //A public Q that contains pointers to tuples
    //each tuple has an int (receiver id) and a pointer to the Frame to be sent to.
    std::queue <std::tuple<int, double, Frame_Base*>* > messages_info;
    const char FLAG = 1;
    const char ESC = 2;
    int last_one;

  protected:
    virtual void initialize();                                      //Evram
    virtual void orchestrate_msgs(int line_index);                  //Evram
    virtual void buffer_msg (cMessage *msg);                        //Evram
    virtual Frame_Base* byte_stuff (const std::string& msg);        //Sayed
    virtual void add_haming (Frame_Base* frame);                    //Sayed
    virtual void schedule_self_msg(int line_index);                 //Common
    virtual void handleMessage(cMessage *msg);                      //Kareem
    virtual void modify_msg(Frame_Base *frame);                     //Omar
    virtual std::string byte_destuff (Frame_Base* frame);           //Sayed
    virtual bool error_detect_correct (Frame_Base* frame);          //Sayed
};

#endif
