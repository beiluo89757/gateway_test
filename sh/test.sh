#!/bin/sh

# args=123456

# fileNumber=`ls -lR test|grep "^-"|wc -l`


# echo $test_cd

# echo $fileNumber

# if [ $fileNumber -ge 1 ];then
#     echo "remove all of test files"
#     rm -rf test/*
# else
#     echo -e 'do nothing\a'
# fi


#Check if any of $pid (could be plural) are running
checkpid() {
        local i
echo "1111112222"
        for i in $* ; do
            echo "111111"
                # [ -d "/proc/$i" ] && return 0
        done
        return 1
}
checkpid
