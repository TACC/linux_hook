# linux_hook
linux_hook is a mini framework to hook functions in shared libraries under Linux.
It only works on x86_64 at this time. I might extend it to support Power PC and <br>
ARM in future. [udis86](https://github.com/vmt/udis86) was adopted to disasseble binary code on x86_64. 

Get started,<br> 
`git clone https://github.com/TACC/linux_hook` <br>
`cd linux_hook` <br>
`make` <br>

Test

`LD_PRELOAD=./hook_open.so touch aaa bbb ccc` <br>

You should see, 

`DBG> new_open(aaa)` <br>
`DBG> new_open(bbb)` <br>
`DBG> new_open(ccc)` <br>

One more test to monitor open() in run a simple hello world in python. 

`LD_PRELOAD=./hook_open.so python3 ./test/hello.py > 1 ; wc -l 1` <br>
`LD_PRELOAD=./mini_open.so python3 ./test/hello.py > 2 ; wc -l 2` <br>

You should see file 1 and 2 have different number of lines. We can see the hook 
based on trampoline is more reliable. 


