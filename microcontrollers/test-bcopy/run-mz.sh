#!/bin/sh

#trace="-t bcopy.trace"
../../pic32mz -m $trace ../wifire/boot.hex test-bcopy.hex
