#!/bin/sh
args=123456

fileNumber=`ls -lR test|grep "^-"|wc -l`


echo $test_cd

echo $fileNumber

if [ $fileNumber -ge 1 ];then
    echo "remove all of test files"
    rm -rf test/*
else
    echo -e 'do nothing\a'
fi
