# Query / suggestion about SSA and Phi functions

The current definition has labels, branches to labels, and Phi functions just after labels.

I propose an alternative, which I call "goto with values".  This alternative would work as follows:

* Each label would have zero or more phi-value : type pairs.  These phi-values are simply new local SSA variables.
* Each branch label (both labels in a conditional branch, the single labels in an unconditional branch, etc.) would list locally visible values being "sent" to the branch target.

Example:
x = 3;
y = 25;
goto l(x, y);
...
l: (x2:int<32>,y2:int<8>)

This form makes it clear that the association ("assignment") of values to phi-variables has to happen as part of the control transfer.  Phi-functions are simply one way of representing that, but they're not not a way that seems immediately helpful for code generation.

I further observe that the current definition apparently allows any value to be passed in to a Phi, but I think ti should be restricted to global/constant SSA variables or SSA variables defined in the sending block.  The new form perhaps makes that clear, not least because with it you can explicitly disallow referring to local SSA variables defined in other blocks.