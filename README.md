# grotto
A tool for optimizing your Professor Oak Challenge in Pokémon B2W2, written in C.

grotto estimates the amount of time needed to capture all Pokémons in a series of hidden grottos and the optimal amount of biking for reloading them.
The estimation is simply done through repetitive simulation and averaging. The current settings are set for { TODO where are the grottos anyway }, but a new set of hidden
grottos may be simulated (see below).

## Usage
`grotto [skip ...]`

### Arguments :
`skip ...` : any number of integers between 0 inclusively and `NODE_COUNT` exclusively. Any node with its index found here will be ignored during all simulations. Optional.

### Example output
```
$ grotto 0 2
approx. 123456s
Number of 256 steps during reload : 10
```

With `grotto 0 2`, we specify nodes 0 and 2 should be completely ignored. The approximate average time for capturing all Pokémons in every grotto we don't ignore is 123456s.
While none of them are cleared, the optimal amout of biking to do after searching them all is 10 * 256 steps.

Note : on default settings, node 6 does not represent a grotto that must be cleared, but rather the location at which the biking is done when all grottos have been searched.

## Compilation
A Makefile is provided with simple settings for gcc.  
***Note : The Makefile expects a `bin` folder at the root of the project folder. Because binaries are ignored by the gitignore, you must create this folder yourself.***

### Targets

`make` : default setting. No optimization flag or debugging information.

`make release` : enables maximum optimization (`-O3`).

`make debug` : disables all optimization (`-O0`) and enables debugging information (`-g`).

## Changing settings
You may want to change the simulation's settings, whether it's to simulate another set of grottos or to make the estimation faster or more precise.

### Changing speed / precision
The `ITERATIONS` macro (default value : 50 000) is entirely responsible for this. Increase for better precision but slower run-time.
For reference, 100 000 iterations on a Ryzen 7 3700X @ 4.19GHz boosted and 3000MHz DDR4 takes about 25s, with an estimated precision of about 0.1%.

### Changing grottos
`NODE_COUNT` (default value : 7) should first be changed, along with `MAX_EVENTS` (default value : 3) if a grotto has more than 3 Pokémons to be captured.

Then, the constants `gains`, `losses` and `events` should be changed.  
`gains[X * NODE_COUNT + Y]` should be the amount of biking done to travel from grotto X to Y (starting at 0).  
`losses[X * NODE_COUNT + Y]` should be the total amount of time needed to travel from grotto X to Y, minus gains[X * NODE_COUNT + Y].  
`events[X][Y]` should be the probability of the Y'th Pokémon of the X'th grotto appearing when the grotto is reloaded, in 1/10 %.

`MAX_TOTAL` and `setsPerLoss` should normally not be changed as long as `LAST_NODE_MULTIPLIER` and `EVENT_ROLL_PROB` aren't. The last two are always valid for B2W2 games,
so there should never be need for change.
