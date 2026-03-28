simple FLC
{
    parameters:
        xml config = xmldoc("conf.xml");
    gates:
        input in;
        output out; // ADDED: send scheduler adjustment command
}

network FLC_Net
{
    submodules:
        flc: FLC {
            parameters:
                @display("p=89,100");
        }
        gen: GeneratorFLC {
            parameters:
                @display("p=209,100");
        }
    connections:
        gen.out --> flc.in;
}
