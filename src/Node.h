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
#include <chrono>
#include <string.h>     //strncmp
#include "orchestrator_order_m.h"
#include "frame_m.h"

using namespace omnetpp;

/**
 * TODO - Generated class
 */
class Node : public cSimpleModule
{
    static const int n = 8;
    std::deque <Frame_Base*> messages_info[n][n];
    int acknowledges[n][n];
    int nxt_to_send[n][n];
    int win_begin[n][n];
    int win_end[n][n];
    double last_ack_time[n][n];
    double last_send_time[n][n];

    const double SEND_INTERVAL = 0.000005;
    const double SEND_TIMEOUT = 0.15;
    const double ACK_TIMEOUT = 1.0;
    const double NEXT_TIME_STEP = 0;
    const int MAX_WINDOW_SIZE = 7;
    //Flags for stuffing
    const char FLAG = 1;
    const char ESC = 2;
    //for distributing messages so that no node receive two consecutive messages
    int last_one = 0;
    //STAT
    int messages_count = 0;
    int ack_count = 0;
    int nack_count = 0;
    int retransmit_count = 0;
    int drop_count = 0;
    int acked_msgs = 0;

  protected:
    virtual void initialize();                                      //Evram
    virtual void orchestrate_msgs(int line_index);                  //Evram
    virtual void buffer_msg (cMessage *msg);                        //Evram
    virtual void schedule_self_msg(int line_index);                 //Evram
    virtual void finish();                                          //Evram

    virtual Frame_Base* byte_stuff (const std::string& msg);        //Sayed
    virtual void add_hamming (Frame_Base* frame);                   //Sayed
    virtual bool error_detect_correct (Frame_Base* frame);          //Sayed
    virtual std::string byte_destuff (Frame_Base* frame);           //Sayed

    virtual void handleMessage(cMessage *msg);                      //Kareem
    virtual bool between(int idx, int val);                         //Kareem
    virtual int current_window_size(int idx);                       //Kareem

    virtual void modify_msg(Frame_Base *frame);                     //Omar
    virtual bool delay_msg(double &delayed_time);                   //Omar
    virtual bool loss_msg();                                        //Omar
    virtual bool dup_msg();                                         //Omar

};

#endif
