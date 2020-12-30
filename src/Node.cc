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
                std::cout <<"My ID: "<<my_id<<endl;
                std::getline (myfile,line);
                int rcv_id = std::stoi(line);
                std::getline (myfile,line);
                //Construct a frame
                Frame_Base* frame = new Frame_Base;
                int k = 0;
                for (std::string::size_type i = 0; i < line.size(); i++)
                {
                    std::cout <<"char[i]: "<<line[i]<<endl;
                    std::bitset<8> *tmp = new std::bitset<8>(line[i]);
                    frame->setPayload(k, (*tmp));
                    k += 1;
                }
                this->messages_info[frame] = rcv_id;
            }
            else
            {
                std::getline (myfile,line);
                std::getline (myfile,line);
            }
        }
        myfile.close();
    }
    else
    {
        std::cout <<"File ain't opened\nIt might not be created!\nAn error occured while opening!\n";
        std::cout <<"PROGRAM SHALL ABORT\n";
        std::exit(3);
    }
    return;
}

void Node::initialize()
{
    double interval = exponential(1 / par("lambda").doubleValue());
    scheduleAt(simTime() + interval, new cMessage(""));

    construct_msg_q();

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
    return;
}
bool Node::error_detect_correct (Frame_Base* frame)
{
    return true;
}
void Node::byte_stuff (Frame_Base* frame)
{
    return;
}
void Node::modify_msg (Frame_Base* frame)
{
    return;
}

void Node::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) { //Host wants to send

        int rand, dest;
        do { //Avoid sending to yourself
            rand = uniform(0, gateSize("outs"));
        } while(rand == getIndex());

        //Calculate appropriate gate number
        dest = rand;
        if (rand > getIndex())
            dest--;

        std::stringstream ss;
        ss << rand;
        EV << "Sending "<< ss.str() <<" from source " << getIndex() << "\n";
        delete msg;
        msg = new cMessage(ss.str().c_str());
        send(msg, "outs", dest);

        double interval = exponential(1 / par("lambda").doubleValue());
        EV << ". Scheduled a new packet after " << interval << "s";
        scheduleAt(simTime() + interval, new cMessage(""));
    }
    else {
        //atoi functions converts a string to int
        //Check if this is the proper destination
        if (atoi(msg->getName()) == getIndex())
            bubble("Message received");
        else
            bubble("Wrong destination");
        delete msg;
    }
}





