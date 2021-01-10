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

#include "Node.h"

Define_Module(Node); 

/*
*   When Orchestrator schedule itself a message, it gets notified and call this message, to send an order to a certain node.
*/
void Node::orchestrate_msgs(int line_index)
{
    //construct order pointer.
    Orchestrator_order_Base* order_i = new Orchestrator_order_Base();

    //Construct Sender, Receiver and time to send.
    //Parameters
    int nodes_size = getParentModule()->par("N").intValue();   //eg. N=8
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator (seed);
    std::uniform_int_distribution<int> distribution(1,nodes_size-1);

    //Assign Random with constraints
    int rand_sender = distribution(generator);
    int rand_rcv = 0;
    do{

        rand_rcv = distribution(generator);
    } while(rand_rcv == rand_sender || (nodes_size!=2 && this->last_one == rand_rcv));

    std::uniform_real_distribution<double> rand_distribution (0.0,10.0);
    double when_to_send = rand_distribution(generator) +simTime().dbl();

    std::cout <<"Orchestrator:\t"<<"Sender:\t"<<rand_sender << "\tReceiver:\t"<<rand_rcv <<"\tat\t"<<when_to_send<<endl;

    //Fetch Message from file with rounding
    std::ifstream myFile("../msgs/msgs.txt");
    std::string line;
    if (myFile.is_open())
    {
        int loop_mod = line_index % 1000;   //1000 is the max message lines we might have
        for (int i=0; i<loop_mod; i++)
        {
            std::getline (myFile,line);

            //wrap around
            if (myFile.eof())
            {
                myFile.close();
                myFile.open("../msgs/msgs.txt");
            }
        }
    }
    else
        std::cout <<"An error occurred, Can't find the msgs. file!!"<<endl;

    order_i->setSender_id(rand_sender);
    order_i->setRecv_id(rand_rcv);
    order_i->setMessage_body(line);
    order_i->setInterval(when_to_send);

    //to avoid two messages at the same time.
    this->last_one = rand_rcv;

    //send it to rand_sender
    send(order_i, "outs", rand_sender);
    return;
}

/*
* Orchestrator utility function to remind itself after some interval to propagate a message.
* If this ain't an Orch. it returns, and if we reached messages limit, don't add more.
*/
void Node::schedule_self_msg(int line_index)
{
    if (strcmp(this->getName(), "orchestrator")!=0)
        return;

    if (this->par("M").intValue() == this->messages_count)
        return;

    Orchestrator_order_Base* tmp = new Orchestrator_order_Base();
    tmp->setKind(line_index);
    double interval = exponential(1 / par("lambda").doubleValue());
    scheduleAt(simTime() + interval, tmp);
    this->messages_count++;
}

/*
* Init. function.
* Setting some parameters.
* Scheduling the first order for orchestrator to work on.
*/
void Node::initialize()
{

    if (std::strcmp(this->getName(),"orchestrator") == 0)
        {
            this->last_one = 0;
            this->messages_count = 0;
            schedule_self_msg(1);
        }
    else
    {
        for(int i = 0; i < n; ++i){
            this->acknowledges[i] = 0;
            this->nxt_to_send[i] = 0;
            this->win_begin[i] = 0;
            this->win_end[i] = 0;
        }
    }

}

void Node::finish()
{
    //print it one time only
    if (std::strcmp(this->getName(),"orchestrator") == 0)
    {
        std::cout <<"\n\n*****\t\tEnd of Transmissions*****\n\n";
        std::cout <<"Total Frames Created by Orchestrator:\t" <<this->messages_count<<endl;
        std::cout <<"Total ACKs Sent by Nodes:\t" <<this->ack_count<<endl;
        std::cout <<"Total NACKs Sent by Nodes:\t" <<this->nack_count<<endl;
        std::cout <<"Total Retransmissions Sent:\t" <<this->retransmit_count<<endl;
        std::cout <<"Total Dropped Frames:\t" <<this->drop_count<<endl;
        double total_msgs =drop_count+retransmit_count+nack_count+ack_count+messages_count;
        std::cout <<"\n\nUtility:\t"<<this->messages_count / total_msgs<<endl;
    }
}

/**
 * @brief add hamming parity check bits to the frame payload
 * 
 * @param frame the frame to modify its payload
 */
void Node::add_hamming (Frame_Base* frame)
{
    std::cout<<"Add hamming\n"<<endl;
    std::vector<bool> payload = frame->getPayload();
    int m = payload.size();
    int r = 0;
    while ((1<<r) < r+m+1)
        r++;
    std::vector<bool> hamming(r+m+1);
    for (int i = 1, j = 0; i <= r+m; i++)
    {
        // if i not a power of 2
        // (number of ones in binary representation of i not equal one)
        if(__builtin_popcount(i) != 1)
            hamming[i] = payload[j++];
    }
    for (int i = 0; i < r; i++)
    {
        int num = (1<<i);
        for (int j = num+1; j <= r+m; j++)
        {
            if (j&num)
                hamming[num] = hamming[num] ^ hamming[j];
        }
    }
    frame->setPayload(hamming);
}

/**
 * @brief detect and correct errors in frame payload using hamming codes
 * and remove the parity bits from the payload
 * 
 * @param frame the frame to detect errors in and remove them
 * @return true if there was a detected error
 */
bool Node::error_detect_correct (Frame_Base* frame)
{
    std::vector<bool> payload = frame->getPayload();
    int z = payload.size();
    int r = ceil(log2(z));
    int m = z-r-1;
    int errorBit = 0;
    for (int i = 0; i < r; i++)
    {
        int num = (1<<i);
        bool parity = payload[num];
        for (int j = num+1; j <= r+m; j++)
        {
            if (j&num)
                parity ^= payload[j];
        }
        if (parity)
            errorBit |= num;
    }
    if (errorBit)
    {
        payload[errorBit] = payload[errorBit] ^ 1;
        std::cout<<"Error detected at bit "<<errorBit<<" and corrected\n";
        //TODO: Sayed, print the message with the error here!!
    }
    else
        std::cout<<"There was no error detected in the frame\n";

    std::vector<bool> finalPayload(m);
    for (int i = 3, j = 0; i <= r+m; i++)
    {
        if (__builtin_popcount(i) != 1)
            finalPayload[j++] = payload[i];
    }

    frame->setPayload(finalPayload);
    if (errorBit)
        return true;
    return false;
}

/**
 * @brief frame the message and add byte stuffing
 * 
 * @param msg the message string to create the frame for it
 * @return Frame_Base* a frame containing the message with byte stuffing
 */
Frame_Base* Node::byte_stuff (const std::string& msg)
{
    std::cout <<"Add byte stuffing.\n";
    // add byte stuffing
    std::vector<char>bytes;
    bytes.push_back(FLAG);
    for (int i = 0; i < (int)msg.size(); i++)
    {
        char byte = msg[i];
        if (byte == FLAG || byte == ESC)
            bytes.push_back(ESC);
        bytes.push_back(byte);
    }
    bytes.push_back(FLAG);
    // frame and transform to bits
    std::vector<bool> payload;
    for (int i = 0; i < (int)bytes.size(); i++)
    {
        std::bitset<8> bits = std::bitset<8>(bytes[i]);
        for (int j = 7; j >= 0; j--)
            payload.push_back(bits[j]);
    }
    Frame_Base* frm = new Frame_Base();
    frm->setPayload(payload);
    return frm;
}

/**
 * @brief Do the opposite of byte stuffing, removing the framing 
 * and removing the byte stuffing
 * 
 * @param frame the frame to extract the message from
 * @return std::string the message string
 */
std::string Node::byte_destuff (Frame_Base* frame)
{
    std::cout <<"Remove byte stuffing.\n";

    std::vector<bool> payload = frame->getPayload();
    std::vector<char> bytes;
    for (int i = 8; i < (int)payload.size()-8;)
    {
        char byte = 0;
        for (int j = 7; j >= 0; j--, i++)
            if (payload[i])
                byte |= (1<<j);
        bytes.push_back(byte);
    }

    std::string msg;
    for (int i = 0; i < (int)bytes.size(); i++)
    {
        char byte = bytes[i];
        if (byte == ESC)
        {
            i++;
            if (i >= (int)bytes.size())
            {
                std::cout<<"Error: There was an ESC without character at the end of the frame";
                break;
            }
            byte = bytes[i];
        }
        msg.push_back(byte);
    }
    return msg;
}

/**
* Corrupting msg follows bernoulli trails to check if corruption will be performed or not
* since the events are all equally probable so:
* The generated random number follows uniform distribution
* The corruption itself follows uniform distribution
* The Corrupted index(bit) in the payload follows uniform distribution
*/
void Node::modify_msg (Frame_Base* frame)
{
    double rand_corrupt = uniform(0,1);

    // double p_corrupt = 0.7;
    double p_corrupt = par("p_corrupt").doubleValue();
    
    if(rand_corrupt < p_corrupt )
    {
        std::cout <<"Message is Modified"<<endl;
        int payloadSize = frame->getPayload().size();
        if(payloadSize)
        {
            int rand_corrupt_index = uniform(0,1)*payloadSize;
            std::cout<<"index = "<<rand_corrupt_index<<endl;
            message_vec modified_msg = frame->getPayload();
            modified_msg[rand_corrupt_index] = !modified_msg[rand_corrupt_index];
            frame->setPayload(modified_msg);
        }
    }

    return;
}

/*
*  Delaying msg using bernoulli distribution
*/
bool Node::delay_msg (double& delayedTime)
{
    double rand_delay = uniform(0,1);
    double p_delay = par("p_delay").doubleValue();
    // double p_delay = 0.6;

    if(rand_delay < p_delay )
    {
        std::cout <<"Message is Delayed"<<endl;
        double rand_delay = uniform(0,1)*par("delay_range").doubleValue();
        delayedTime = rand_delay;
        return true;
    }
    delayedTime = 0;
    return false;
}

/*
*   losing msg using bernoulli distribution
*/
bool Node::loss_msg ()
{
    double rand_loss = uniform(0,1);
    double p_loss = par("p_loss").doubleValue();
    // double p_loss = 0.6;

    if(rand_loss < p_loss )
        {
            std::cout <<"Message is Dropped"<<endl;
            this->drop_count++;
            return true;
        }
    return false;
}

/*
* duplicating msg using bernoulli distribution
*/
bool Node::dup_msg ()
{
    double rand_dup = uniform(0,1);
    double p_dup = par("p_dup").doubleValue();
    // double p_dup = 0.65;

    if(rand_dup < p_dup )
        {
            std::cout <<"Message is Duplicated"<<endl;
            return true;
        }
    return false;
}

/*
*   A regular node received an order from orchestrator, so it fetches the order information.
*   adds stuffing and hamming.
*   and the node sends itself a message, notifying itself at the time -that the message should be sent at- 
*   to be appended to the buffer, and to be sent, if the window size allows it.
*/
void Node::buffer_msg (cMessage *msg)
{
    //up cast the message.
    Orchestrator_order_Base* order_rcv = check_and_cast<Orchestrator_order_Base *> (msg);
    //Sanity check
    assert(order_rcv->getSender_id() == this->getIndex());
    //Message info.
    int rcv_id = order_rcv->getRecv_id();
                                            //WHY DID KAREEM REMOVED THIS!
    double interval = order_rcv->getInterval();
    std::string message_to_frame = order_rcv->getMessage_body();

    //stuff the message
    Frame_Base* msg_frame = this->byte_stuff(message_to_frame);
    //add hamming
    this->add_hamming(msg_frame);
                                            //YOU DON'T NEED THE TUPLE
    //add a tuple for my new message                                                    //PS. I added direct dest gate.
    //std::tuple<int, double, Frame_Base*>* temp= new std::tuple<int, double, Frame_Base*>(idx, interval, msg_frame);
    //buffer the message
    //this->messages_info[idx].push_back(msg_frame);
    msg_frame->setKind(rcv_id);
    msg_frame->setName("Add to buffer");
    // scheduleAt(simTime() + interval, msg_frame);
    scheduleAt(interval, msg_frame);

    std::cout <<"Current SimTime:\t"<<simTime()<<endl;
    std::cout <<"Node:\t"<<this->getIndex()<<"\tScheduled the message:\t"<<message_to_frame<<endl;
    std::cout <<"To be Sent to:\t"<<rcv_id<<"\tat:\t"<<interval<<endl;
    return;
}

/*
*   Utility function used in go-back-n algorithm.
*/
bool Node::between(int idx, int b){
    int c = this->win_end[idx], a = this->win_begin[idx];
    if(c >= a)
        return b >= a && b < c;
    return b < c || b >= a;
}

/*
*   Utility function used in go-back-n algorithm.
*/
int Node::current_window_size(int idx){
    int ret = this->win_end[idx] - this->win_begin[idx];
    if(this->win_begin[idx] <= this->win_end[idx])
        return ret;
    return this->MAX_WINDOW_SIZE + 1 + ret;
}

/*
*   TODO: Add documentation.
*/
void Node::handleMessage(cMessage *msg)
{
    if (std::strcmp(this->getName(),"orchestrator") == 0)
    {
        Orchestrator_order_Base* msg_rcv = check_and_cast<Orchestrator_order_Base *> (msg);
        int nodes_size = getParentModule()->par("N").intValue();   //eg. N=8
        int kind = msg_rcv->getKind();

        for (int i=0; i<nodes_size; i++)
            orchestrate_msgs(kind++);

        this->schedule_self_msg(kind);
        return;
    }
    else
    {
        //Regular Node
        if (strcmp(msg->getSenderModule()->getName(), "orchestrator")==0)
        {
            //Add the ordered message to the buffer of messages to be sent.
            buffer_msg (msg);

            //To modify a frame


            //To duplicate a frame
            //leave it at the messages_info Q., hamming is already added, Kareem needs to check..IMP

            //To send a message
            //Frame_Base* tmp = std::get<0>(*this->messages_info.front());
            //int rcv_id = std::get<0>(*this->messages_info.front());
            //double interval = std::get<0>(*this->messages_info.front());
            //send(next_frame, "outs", dest_gate);

            //Here lies the sending logic..

         }
        else
        {
            if(msg->isSelfMessage()){
                if(strcmp(msg->getName(), "Add to buffer")==0){
                    int idx = msg->getKind();
                    std::cout << "Add to buffer *************" <<endl;
                    Frame_Base* msg_frame = check_and_cast<Frame_Base *> (msg);
                    std::cout << "Add to buffer of node " << this->getIndex() << " to send to node " << idx;
                    std::cout << " at time " << simTime() << " the message " << this->byte_destuff(msg_frame) << endl;
                    this->messages_info[idx].push_back(msg_frame);
                    this->win_end[idx] = (this->win_end[idx]+1);
                    std::cout << this->win_end[idx] << endl;
                    cMessage * cmsg = new cMessage();
                    cmsg->setName("Send Message");
                    cmsg->setKind(idx);
                    scheduleAt(simTime()+ this->NEXT_TIME_STEP, cmsg); // TBC
                    std::cout << "********************" <<endl;
                }
                if(strcmp(msg->getName(), "Send Message")==0){
                    std::cout << "Send Message *************" <<endl;
                    int idx = msg->getKind(), dest_gate = idx;
                    if(this->nxt_to_send[idx] != this->win_end[idx]){
                        std::cout << "next to send from node " << this->getIndex() << " to node "<< idx << " is " << this->nxt_to_send[idx] << endl;
                        std::cout << "window end is " << this->win_end[idx] << endl;
                        delete msg;
                        if(idx > this->getIndex())
                            dest_gate--;
                        int to_send = this->nxt_to_send[idx];
                        Frame_Base* msg_frame = this->messages_info[idx][to_send];
                        msg_frame->setACK(this->acknowledges[idx]);
                        msg_frame->setName("Receive Message");
                        msg_frame->setKind(this->getIndex());
                        msg_frame->setFrame_seq(this->nxt_to_send[idx]);
                        std::cout << "Send message from" << this->getIndex() << " to node " << idx << " at dest gate" << dest_gate;
                        std::cout << " at time " << simTime() << " the message " << this->byte_destuff(msg_frame) << endl;
                        send(msg_frame, "outs", dest_gate);
                        this->nxt_to_send[idx] = (this->nxt_to_send[idx] + 1);
                        std::cout <<"Next to send: "<<this->nxt_to_send[idx]<<endl;
                        if(this->nxt_to_send[idx] != this->win_end[idx]){
                            cMessage * cmsg = new cMessage();
                            cmsg->setName("Send Message");
                            cmsg->setKind(idx);
                            scheduleAt(simTime() + this->SEND_INTERVAL, cmsg);   // TBC
                        }
                        cMessage * cmsg = new cMessage();
                        cmsg->setName("ACK TIMEOUT");
                        cmsg->setKind(idx);
                        scheduleAt(simTime() + this->ACK_TIMEOUT, cmsg);   // TBC
                        this->last_send_time[idx] = simTime().dbl();          // TBC
                    }
                    std::cout << "********************" <<endl;
                }
                if(strcmp(msg->getName(), "ACK TIMEOUT")==0){
                    int idx = msg->getKind();
                    double curTime = simTime().dbl() - this->ACK_TIMEOUT;
                    if(this->last_ack_time[idx] < curTime){
                        this->nxt_to_send[idx] = this->win_begin[idx];
                        if(this->nxt_to_send[idx] != this->win_end[idx]){
                            cMessage * cmsg = new cMessage();
                            cmsg->setName("Send Message");
                            cmsg->setKind(idx);
                            scheduleAt(simTime() + this->SEND_INTERVAL, cmsg);   // TBC
                        }
                    }
                }
                if(strcmp(msg->getName(), "SEND TIMEOUT")==0){
                    int idx = msg->getKind(), dest_gate = idx;
                    double curTime = simTime().dbl() - this->SEND_TIMEOUT;
                    if(this->last_send_time[idx] < curTime){
                        Frame_Base* msg_frame = new Frame_Base();
                        msg_frame->setACK(this->acknowledges[idx]);
                        msg_frame->setName("Receive ACK");
                        msg_frame->setKind(this->getIndex());
                        if(idx > this->getIndex())
                            dest_gate--;
                        //this needs to be send_delay
                        send(msg_frame, "outs", dest_gate);
                        this->last_send_time[idx] = simTime().dbl();          // TBC
                    }
                }
            }
            else{
                std::cout << "Receive  *************" <<endl;
                //received a message from node.
                Frame_Base* msg_frame = check_and_cast<Frame_Base *> (msg);
                int send_ind = msg_frame->getKind();
                std:: cout << "Node " << this->getIndex() << " received from " << send_ind << " the message ";
                std::cout << msg->getName() << endl;
                int ack = msg_frame->getACK();
                // std::cout <<"this is ack:\t"<<ack<<endl;    
                // message_vec payload = msg_frame->getPayload();
                std::cout << win_begin[send_ind] << " " << win_end[send_ind] << " " << nxt_to_send[send_ind];
                if(this->between(send_ind, ack-1))
                {
                    std::cout << "before sliding window *****" << endl;
                    std::cout << "window begin: " << this->win_begin[send_ind] << " window end: " << this->win_end[send_ind];
                    std::cout << " acknowledgement: " << this->acknowledges[send_ind] << endl;
                    while(this->win_begin[send_ind] < ack){
                        this->win_begin[send_ind] = (this->win_begin[send_ind] + 1);
                        //for stat.
                        this->acked_msgs++;
                    }
                    this->last_ack_time[send_ind] = simTime().dbl();          // TBC
                    std::cout << " after sliding window *****" << endl;
                    std::cout << "window begin: " << this->win_begin[send_ind] << " window end: " << this->win_end[send_ind];
                    std::cout << " acknowledgement: " << this->acknowledges[send_ind] << endl;
                    if(this->nxt_to_send[send_ind] != this->win_end[send_ind]){
                        cMessage * cmsg = new cMessage();
                        cmsg->setName("Send Message");
                        cmsg->setKind(send_ind);
                        scheduleAt(simTime() + this->SEND_INTERVAL, cmsg);   // TBC
                    }
                }
                if(strcmp(msg->getName(), "Receive Message")==0){
                    int r = msg_frame->getFrame_seq();         // TBC
                    if(r == this->acknowledges[send_ind]){
                        this->error_detect_correct(msg_frame);  // stats
                        std::string str = this->byte_destuff(msg_frame);
                        std::cout << "message received " << str << endl;
                        this->acknowledges[send_ind] = (this->acknowledges[send_ind]+1);
                        cMessage * cmsg = new cMessage();
                        cmsg->setName("SEND TIMEOUT");
                        cmsg->setKind(send_ind);
                        scheduleAt(simTime() + this->SEND_TIMEOUT, cmsg);   // TBC
                    }
                }
                std::cout << "********************" <<endl;
            }
        }
    }

}

