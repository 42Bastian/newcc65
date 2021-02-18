New options:
	-Dmacro		define macro


New type:
Now it accepts register, even with structs or strings ;-)

Sure the 65SC02 doesn't have a more than A,X,Y but I decided to use
zeropage locations from 8..$7F for simulating registers.
If this space is exhausted, cc65 will generate STATICs, which are not as fast as the registers but faster than stack-variables.

