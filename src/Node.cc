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
     * Corrupting msg follows bernoulli trails to check if corruption will be performed or not
     * since the events are all equally probable so:
     * The generated random number follows uniform distribution
     * The corruption itself follows uniform distribution
     * The Corrupted index(bit) in the payload follows uniform distribution
    */

    double rand_corrupt = uniform(0,1);
    // double p_corrupt = par("p_corrupt").doubleValue();
    double p_corrupt = 0.7;
    if(rand_corrupt < p_corrupt )
    {
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

bool Node::delay_msg (double& delayedTime)
{
    /*
     * Delaying msg using bernoulli distribution
    */
    double rand_delay = uniform(0,1);
    // double p_delay = par("p_delay").doubleValue();
    double p_delay = 0.6;

    if(rand_delay < p_delay )
    {
        // double rand_delay = uniform(0,1)*par("delay_range").doubleValue();
        delayedTime = rand_delay;
        return true;
    }
    delayedTime = 0;
    return false;
}

bool Node::loss_msg ()
{
    /*
     * losing msg using bernoulli distribution
    */
    double rand_loss = uniform(0,1);
    // double p_loss = par("p_loss").doubleValue();
    double p_loss = 0.6;

    if(rand_loss < p_loss )
        return true;
    return false;
}

bool Node::dup_msg ()
{
    /*
     * duplicating msg using bernoulli distribution
    */
    double rand_dup = uniform(0,1);
    // double p_dup = par("p_dup").doubleValue();
    double p_dup = 0.65;

    if(rand_dup < p_dup )
        return true;
    return false;
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

