# ericoproject2
My entry for Activision's 2025 Studio Summit Coding Competition, cleaned up a bit.

This demo was for a coding competition that Activision held among employees for an open source RISCV computer called [TinySys](https://github.com/ecilasun/tinysys) by [Engin Cilasun](https://github.com/ecilasun). 

## How to Run
Assuming this repo is cloned into the TinySys under the `tinysys/software/samples/`directory, run `make again` to build the .elf and have it copied into the correct directory for the TinySys emulator to detect it. 

## Some Notes
I only cared to implement one effect which was a spinning torus that is both flat-shaded and ASCII rendered in homage to [this](https://www.a1k0n.net/2011/07/20/donut-math.html) work. I figured it would be an interesting enough challenge to given my limited graphics programming experience, let alone on a bespoke system. 

The gist of the novel work is in `BenDemoLibrary/poly.cpp` where I just perform a scanline render onto the grid of a text console rather than individual pixels of a frame buffer. 70% of the code was already written by Ben Stragnell, who generously released the source code for his previous demo competition entry; all I did was heavily modify it for the one effect. I even took the 3D model data for the Torus.


## Special Thanks

* Ben Stragnell for his generosity of knowledge and either implicit approval or future forgiveness for nicking his code
* Engin Cilasun for blowing my Computer Engineering degree out of the water with his amazing FPGA creations
* Wade Brainerd for encouraging me to do make an entry (in my free time of course) to the competition and putting up with my questions
* Aki Hogge for pointing me in the right direction for implementing ASCII effect
* Jordan Diaz for lighting a fire under me with his even more amazing competition entry
* Chris Achenbach for helping me with code and cope
* William Cornell for talking me through more code