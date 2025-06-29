﻿#pragma once

/************************************************************************

                    线程locale自动设置的功能库

   由于libc中的setlocale的设定是仅影响当前线程的，但每次创建线程均手动设置比较麻烦，且
 无法再更底层的库中进行通用化管理，因此提供一套代码补丁，按需在工程中进行设置。

   通过向CreateThread函数打代码补丁从而实现对创建线程函数的Hook，并在创建线程后立刻为
 线程设置locale以及mbcp。

************************************************************************/

int InstallThreadLocalePath(const char *lc);
