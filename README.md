# Brutus tipping point

> This is Brutus' code for the 2021-2022 VEX game, Tipping point

It's using the [PROS](https://pros.cs.purdue.edu/) C API, and requires PROS to be installed to compile.

## fun stuff
- I made an API for creating recordings of motor movement, which can be found in [replay.h](include/replay.h)
	- [Here](https://youtu.be/wI5-owzy2kQ)'s a tutorial I made for it as well
	- UPDATE: put it on github [as its own library](https://github.com/BushRobotics/replay.h)


## debugging
you can test-compile by running `pros make`

you can run `pros terminal` to see the console output from `printf`

## running
`pros mu` builds & uploads to the brain

## COMPETITION
run `comp_upload.sh` to upload the code to the brain with and without auton. slot 1 is auton. slot 2 is no auton.
