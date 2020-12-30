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

void Node::construct_msg_q(){
    //Open the file
    std::ifstream myfile("../msgs/msgs.txt");
    std::string line;
    if (myfile.is_open())
    {
        while (!myfile.eof())
        {
            std::getline (myfile,line);
            int my_id = std::stoi(line);
            if (this->getIndex() == my_id)
            {
                std::cout <<"I am: "<<this->getIndex() <<"\tReading my msgs."<<endl;
                std::getline (myfile,line);
                int rcv_id = std::stoi(line);
                std::getline (myfile,line);
                //Construct a frame
                Frame_Base* frame = new Frame_Base;
                int k = 0;
                for (std::string::size_type i = 0; i < line.size(); i++)
                {
                    std::bitset<8> *tmp = new std::bitset<8>(line[i]);
                    frame->setPayload(k, (*tmp));
                    k += 1;
                }
                //Add it to the uo_map.
                std::tuple<int, Frame_Base*>* temp= new std::tuple<int, Frame_Base*>(rcv_id, frame);
                this->messages_info.push(temp);
            }
            else
            {
                std::getline (myfile,line);
                std::getline (myfile,line);
            }
        }
        myfile.close();
        std::cout <<"I am: "<<this->getIndex()<<"\tDone reading my msgs."<<endl;

    }
    else
    {
        std::cout <<"File ain't opened\nIt might not be created!\nAn error occured while opening!\n";
        std::cout <<"PROGRAM SHALL ABORT\n";
        std::exit(3);
    }
    return;
}

void Node::schedule_self_msg()
{
    double interval = exponential(1 / par("lambda").doubleValue());
    EV << ". Scheduled a new packet after " << interval << "s";
    scheduleAt(simTime() + interval, new Frame_Base());
}

void Node::initialize()
{
    //First of all, read the msgs and their Senders.
    construct_msg_q();

    //IDK if this needs to be here, it's up to Kareem.
    if (this->messages_info.size()>0)
        this->schedule_self_msg();
}

void Node::add_haming (Frame_Base* frame)
{
    /*
     * You got a frame that contains: [payload[64], parity, start/end, ACK, NACK]
     * Implement Hamming Code so that the parity bits are set appropriately
     * Note that: Byte stuffing occures before hamming, so hamming takes care of that too.
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
void Node::byte_stuff (Frame_Base* frame)
{
    std::cout <<"Added byte stuff.\n";
    return;
}
void Node::modify_msg (Frame_Base* frame)
{
    /**
     * When Delaying you'll need send_delay() function
     * When curropting you'll need some propabilistic paramters.
     */
    std::cout <<"Message is Modified\n";
    return;
}

void Node::handleMessage(cMessage *msg)
{

    if (msg->isSelfMessage() && this->messages_info.size()>0)
    {
        //Host wants to send a msg.
        //Fetch Receiver id
        int rcv_id = std::get<0>(*this->messages_info.front());
        //Fetch The Frame
        Frame_Base* next_frame = std::get<1>(*this->messages_info.front());
        this->messages_info.pop();

        //Add parity and stuffing
        this->byte_stuff(next_frame);
        this->add_haming(next_frame);
        //Curropt the msg
        this->modify_msg(next_frame);

        //cout...
        std::cout <<"Host: "<<this->getIndex()<< "\tSending: "<<next_frame->getPayload(0) << "\tTo: "<<rcv_id<<endl;

        //at what gate?
        int dest_gate = rcv_id;
        if (rcv_id > getIndex())
            dest_gate--;

        //send it..
        send(next_frame, "outs", dest_gate);

        //Still Got More?
        if (this->messages_info.size()>0)
            this->schedule_self_msg();
    }
    else
    {
        //This is a Receiving func.
        Frame_Base* msg_rcv = check_and_cast<Frame_Base *> (msg);

        //error detect and correct
        this->error_detect_correct(msg_rcv);
        //cout..
        std::cout <<"RSV: "<<this->getIndex()<< "\tReceived: "<<msg_rcv->getPayload(0)<<endl;
    }
}





