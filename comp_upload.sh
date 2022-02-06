#!/bin/bash
pros mu --slot 1 --name "AUTON"
sed -i "s%#define DO_AUTON%//#define DO_AUTON%g" ./src/main.c # we do a little replacing
pros mu --slot 2 --name "NO AUTON"
sed -i "s%//#define DO_AUTON%#define DO_AUTON%g" ./src/main.c # undo the replacement

sed -i "s%//#define REC_AUTON%#define REC_AUTON%g" ./src/main.c # enable rec auton
pros mu --slot 3 --name "REC AUTON"
sed -i "s%#define REC_AUTON%//#define REC_AUTON%g" ./src/main.c # disable rec auton

