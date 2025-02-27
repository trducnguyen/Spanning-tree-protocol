import ned.IdealChannel;


simple Txc1
{
    parameters:
        @display("i=block/routing");
    gates:
        input in[];  // declare in[] and out[] to be vector gates
        output out[];
}

//
// Using local channel type definition to reduce the redundancy
// of connection definitions.
//
network Tictoc1
{
    @display("bgb=505,380");
    types:
        channel Channel extends ned.DelayChannel
        {
            delay = 500ms;
        }
    submodules:
        tic0: Txc1 {
            @display("p=52,50");
        }
        tic1: Txc1 {
            @display("p=157,193");
        }
        tic2: Txc1 {
            @display("p=62,232");
        }
        tic3: Txc1 {
            @display("p=356,131");
        }
        tic4: Txc1 {
            @display("p=211,89");
        }
        tic5: Txc1 {
            @display("p=317,29");
        }
    connections:
        tic0.out++ --> Channel --> tic1.in++;
        tic0.in++ <-- Channel <-- tic1.out++;

        tic0.out++ --> Channel --> tic5.in++;
        tic0.in++ <-- Channel <-- tic5.out++;

        tic1.out++ --> Channel --> tic2.in++;
        tic1.in++ <-- Channel <-- tic2.out++;

        tic1.out++ --> Channel --> tic3.in++;
        tic1.in++ <-- Channel <-- tic3.out++;

        tic1.out++ --> Channel --> tic4.in++;
        tic1.in++ <-- Channel <-- tic4.out++;

        tic3.out++ --> Channel --> tic4.in++;
        tic3.in++ <-- Channel <-- tic4.out++;

        tic4.out++ --> Channel --> tic5.in++;
        tic4.in++ <-- Channel <-- tic5.out++;

}
