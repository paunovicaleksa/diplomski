// Reminder, replace x and z everywhere but who cares for now.
class ThreeDAverageVariableKernel extends Kernel {

    ThreeDAverageVariableKernel(KernelParameters parameters, int nxMax) {
        super(parameters);

        // Input
        DFEVar inStream = io.input(”inStream”, dfeFloat(8, 24));

        DFEVar Z = io.scalarInput("Z", dfeUInt(32)),
               Y = io.scalarInput("Y", dfeUInt(32)),
               X = io.scalarInput("X", dfeUInt(32));
        OffsetExpr nx = stream.makeOffsetParam(”nx”, 3, nxMax);
        OffsetExpr nxy = stream.makeOffsetParam(”nxy”, 3 ∗ nx, nxMax ∗ nx);
        CounterChain cc = control.count.makeCounterChain();
        DFEVar z = cc.addCounter(Z, 1);
        DFEVar y = cc.addCounter(Y, 1);
        DFEVar x = cc.addCounter(X, 1);

        // simply output if border element.
        DFEVar border = (x.eq(0) | x.eq(X - 1)) |
                        (y.eq(0) | y.eq(Y - 1)) |
                        (z.eq(0) | z.eq(Z - 1));

        // Extract 8 point window around current point
        DFEVar window[] = new DFEVar[27];
        // init to zero
        for(int i = 0; i < 27; i++)
            window[i] = 0;

        float c0 = 1 / 36;
        float c1 = 1 / 6;
        // "convolution" kernel.
        window[0 * 9 + 1 * 3 + 1] = border? 0 : stream.offset(instream, (z - 1)∗nxy+y∗nx+x);

        window[1 * 9 + 0 * 3 + 1] = border? 0 : stream.offset(inStream, z * nxy+ (y - 1)∗nx + x);
        window[1 * 9 + 1 * 3 + 0] = border? 0 : stream.offset(inStream, z∗nxy+ y∗nx + (x - 1));
        window[1 * 9 + 1 * 3 + 2] = border? 0 : stream.offset(inStream, z∗nxy+ y∗nx + (x + 1));
        window[1 * 9 + 2 * 3 + 1] = border? 0 : stream.offset(inStream, z∗nxy+ (y + 1) ∗nx + x);

        window[2 * 9 + 1 * 3 + 1] = border? 0 : stream.offset(inStream, (z + 1)∗nxy+y∗nx+x);

        // Mimic the TPU kernel

        // Sum points in window and divide by 27 to average
        DFEVar sum = constant.var(dfeFloat(8, 24), 0);
        for (DFEVar dfeVar : window) {
            sum = sum + dfeVar;
        }

        sum = sum * c1 + inStream * c0;

        DFEVar result = border? inStream : sum / window.length;

        // Output
        io.output(”outStream”, result , dfeFloat(8, 24));
    }

}
