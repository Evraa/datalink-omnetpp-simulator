package data_link;

@license(LGPL);


import ned.IdealChannel;

network Data_Link
{
    parameters:
        int N = default(8);
        @display("bgb=349,228;i=block/classifier2");
    submodules:
        nodes[N]: Node {
            @display("i=block/network2;is=n");
        }
    connections:
        for i=0..sizeof(nodes)-1, for j=i+1..sizeof(nodes)-1 {
            nodes[i].outs++ --> IdealChannel --> nodes[j].ins++;
            nodes[i].ins++ <-- IdealChannel { @display("ls=black,2,s"); } <-- nodes[j].outs++;
        }
}
