package data_link;

@license(LGPL);


import ned.IdealChannel;

network Data_Link
{
    //    These @display is GUI, don't bother
    parameters:
        int N = default(8);
        double arrival_time = default(2.0);
        @display("bgb=349,228;i=block/classifier2");
    submodules:
        orchestrator: Node {
            @display("i=misc/globe");
        }
        nodes[N]: Node {
            @display("i=block/network2;is=n");
        }
    connections:
        for i=0..sizeof(nodes)-1, for j=i+1..sizeof(nodes)-1 {
            nodes[i].outs++ --> IdealChannel --> nodes[j].ins++;
            nodes[i].ins++ <-- IdealChannel { @display("ls=black,2,s"); } <-- nodes[j].outs++;
        }
        for i=0..sizeof(nodes)-1 {
            orchestrator.outs++ --> IdealChannel { @display("ls=#E9E9E9,1,d"); } --> nodes[i].ins++;
        }

}
