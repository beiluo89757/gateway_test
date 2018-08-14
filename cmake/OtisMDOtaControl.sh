#!/bin/bash

echo -e "===>Shell kill the test\n"

#ps -ef | grep OtisMD | awk '{print $2}' | xargs kill -9

killall -9 test

echo -e "===>Cover the test\n"
cp -rf ./gateway.bakBin  ./test

#echo -e "===>Wait..\n"
#sleep 3

echo -e "===>Start the test\n"
chmod 777 *
sync
./test &
