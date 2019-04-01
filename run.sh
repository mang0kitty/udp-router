#!/usr/bin/env bash
trap "pkill -P $$" EXIT

./my-router run A 10000 > routerA.txt &
./my-router run B 10001 > routerB.txt &
./my-router run C 10002 > routerC.txt & 
./my-router run D 10003 > routerD.txt & 
./my-router run E 10004 > routerE.txt & 

./my-router configure A 10000 A B 10001 4
./my-router configure A 10000 A E 10004 1

./my-router configure B 10001 B A 10000 4
./my-router configure B 10001 B C 10002 3
./my-router configure B 10001 B E 10004 2
./my-router configure B 10001 B F 10005 1

./my-router configure C 10002 C B 10001 3
./my-router configure C 10002 C D 10003 4
./my-router configure C 10002 C F 10005 1

./my-router configure D 10003 D C 10002 4
./my-router configure D 10003 D F 10005 3

./my-router configure E 10004 E A 10000 1
./my-router configure E 10004 E B 10001 2
./my-router configure E 10004 E F 10005 3

sleep 5

./my-router send 10003 C B "Hello B, C sending you a message!"

./my-router run F 10005 > routerF.txt & 

./my-router configure F 10005 F B 10001 1
./my-router configure F 10005 F C 10002 1
./my-router configure F 10005 F D 10003 3
./my-router configure F 10005 F E 10004 3

sleep 5

./my-router send 10003 C F "Hello F, C sending you a message!"

sleep 1
