#!/bin/bash

docker run -i --rm -v $(pwd):/usr/src forderud/qtwasm:latest /usr/src/scripts/qmllint-wrapper.sh $@
