# LD PRELOAD for data provenance

PoC. It opens a pthread with a named unix socket, which then writes the string into a global space.

Now one could imagine to [encode and decode structured data](https://neetcode.io/problems/string-encode-and-decode?list=neetcode150) or [even use a JSON parser](https://github.com/DaveGamble/cJSON) so that it is available to the injected shared library.

Then, you only have to change the SLURM scripts to give `(jobname, timestamp, argv)` to the program, and it can add it to all I/O calls.

If you go one step further, you could curl it to some data pipeline thingy or graph database...

## How to try out
Have `socat` installed and run `./run.sh`
