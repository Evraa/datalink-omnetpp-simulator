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


void Node::orchestrate_msgs(int line_index)
{
    //convert string -> vector of bits
    Orchestrator_order_Base* order_i = new Orchestrator_order_Base();

    //Construct Sender, Receiver and time to send.
    //Parameters
    int nodes_size = getParentModule()->par("N").intValue();   //eg. N=8
    double lower_bound = 2;
    double upper_bound = getParentModule()->par("arrival_time").doubleValue();  //eg. 20s

    std::uniform_real_distribution<double> unif(lower_bound,upper_bound);
    std::default_random_engine re;

    //Assign Random with constraints
    int rand_sender = uniform(0, nodes_size);    //rand -> 0:7
    int rand_rcv = 0;
    do{
        rand_rcv = uniform(0, nodes_size);    //rand -> 0:7 != rand_sender "NO SELF MSGS"
    } while(rand_rcv == rand_sender || (nodes_size!=2 && this->last_one == rand_rcv));

    //When to send this message?
//    double sim_time =  simTime().dbl();
//    double when_to_send = unif(re) + sim_time;
    double when_to_send = exponential(1 / par("lambda").doubleValue());

    //Fetch Message from file with rounding
    std::ifstream myfile("../msgs/msgs.txt");
    std::string line;
    if (myfile.is_open())
    {
        int loop_mod = line_index % 1000;   //1000 is the max message lines we might have
        for (int i=0; i<loop_mod; i++)
        {
            std::getline (myfile,line);

            //wrap around
            if (myfile.eof())
            {
                myfile.close();
                myfile.open("../msgs/msgs.txt");
            }
        }
    }
    else
        std::cout <<"An error occured, Can't find the msgs. file!!"<<endl;

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
void Node::schedule_self_msg(int line_index)
{
    if (strcmp(this->getName(), "orchestrator")!=0)
        return;

    this->last_one = 0;
    Orchestrator_order_Base* tmp = new Orchestrator_order_Base();
    tmp->setKind(line_index);

    double interval = exponential(1 / par("lambda").doubleValue());
    scheduleAt(simTime() + interval, tmp);
}

void Node::initialize()
{

    if (std::strcmp(this->getName(),"orchestrator") == 0)
        schedule_self_msg(1);
    else
        std::cout << "My name is:\t"<<this->getName() <<"\tMy id is:\t"<<this->getIndex()<<"\tI am awake!"<<endl;

}

void Node::add_haming (Frame_Base* frame)
{
    /*
     * You got a frame that contains: [payload[64], parity, start/end, ACK, NACK]
     * Implement Hamming Code so that the parity bits are set appropriately
     * Note that: Byte stuffing occurs before hamming, so hamming takes care of that too.
     *            Since we have 512 bits MAX, parity bits Max is: 10
     *            Some payload characters: bitset <8>, will be zero padding, so you might wanna ignore these
     *            and accordingly ignore them when correcting.
     * */
    //EX:
//    std::cout <<"Added Parity bits\n";
//    std::bitset<10>* bits = new std::bitset<10>;
//    (*bits) = 011110011;
//    frame->setParity((*bits));
    return;
}
bool Node::error_detect_correct (Frame_Base* frame)
{
    std::cout <<"Detected True\n";
    return true;
}

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

void Node::modify_msg (Frame_Base* frame)
{
    /**
     * When Delaying you'll need send_delay() function
     * When corrupting you'll need some propabilistic paramters.
     */

    std::cout <<"Message is Modified\n";

    return;
}

void Node::buffer_msg (cMessage *msg)
{
    //A regular node received an order from orchestrator.
    Orchestrator_order_Base* order_rcv = check_and_cast<Orchestrator_order_Base *> (msg);
    //Sanity check
    assert(order_rcv->getSender_id() == this->getIndex());
    //Message info.
    int rcv_id = order_rcv->getRecv_id();
    int dest_gate = rcv_id;
    if (rcv_id > getIndex())
        dest_gate--;
    double interval = order_rcv->getInterval();
    std::string message_to_frame = order_rcv->getMessage_body();

    //stuff the message
    Frame_Base* msg_frame = this->byte_stuff(message_to_frame);
    //add hamming
    this->add_haming(msg_frame);
    //add a tuple for my new message                                                    //PS. I added direct dest gate.
    std::tuple<int, double, Frame_Base*>* temp= new std::tuple<int, double, Frame_Base*>(dest_gate, interval, msg_frame);
    //buffer the message
    this->messages_info.push(temp);
    std::cout <<"Current SimTime:\t"<<simTime()<<endl;
    std::cout <<"Node:\t"<<this->getIndex()<<"\tScheduled the message:\t"<<message_to_frame<<endl;
    std::cout <<"To be Sent to:\t"<<rcv_id<<"\tAfter:\t"<<interval<<endl;
    return;
}

void Node::handleMessage(cMessage *msg)
{
    if (std::strcmp(this->getName(),"orchestrator") == 0)
    {
        Orchestrator_order_Base* msg_rcv = check_and_cast<Orchestrator_order_Base *> (msg);
        orchestrate_msgs(msg_rcv->getKind());
        this->schedule_self_msg(msg_rcv->getKind()+1);
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
            //this->modify_msg(next_frame);

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
            //here is the receiving logic
            return;
        }
    }

}





