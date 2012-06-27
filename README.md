uhook
=====

uhook Linux kernel driver : Call kernel function from user space


1). What is uhook
------------------

uhook(userspace kernel hook) means call kernel function from
userspace. That is to say, we can write a function in kernel space 
and call it in userspace while the kernel Image is running. 

2). What we can do with uhook
-----------------------------

The most exciting feature of uhook is that we call kernel function in
userspace while kernel is running. So,

A. We can using uhook to dump value of kernel argument when some thing
goes wrong. Especially when the device driver cannot work correctly, we 
can dump some device registers to examine what is going on.

B. We can implement some switch in kernel to control if-else branch.

C. Whatever you want kernel do without recompiling kernel, reloading kernel.

3). Compare to /proc or /sys file system
---------------------------------------

Kernel has some component like /proc and /sys file system to perform
the communication between kernel and user space. But they are so complex 
that we must write some code very time we want to use them. However,
with uhook, we just need to insmod the uhook.ko, then call the kernel
function whenever and wherever.


4). TODO
--------

A. This version of uhook, can just support kernel function prototype
like int func(void). I donot know how to deal with the function that 
has many argument and variable return type.

B. The user space applicantion: uhook, is just a test, so poor, cannot
process complex command option, I am still
working on it
