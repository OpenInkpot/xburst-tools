#!/bin/bash
../host-app/xbboot set_addr 80002000
../host-app/xbboot bulk_write ../target-stage1/stage1.bin
../host-app/xbboot start1 80002000
../host-app/xbboot set_addr 81c00000
../host-app/xbboot bulk_write ../target-echokernel/echo-kernel.bin
../host-app/xbboot flush_cache
../host-app/xbboot start2 81c00000
