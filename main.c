#include "syscall.h"
#include "pg.h"
#include "_clib.h"
#include "callbacks.h"
#include "arkanoid.h"
#include "images.h"

void playIntroWav();

int xmain(void)
{
	SetupCallbacks();
	


	pgScreenFrame(2,0);
	wavoutInit();
	pgcCls();
        
	pgFillvram(0);
        pgBitBlt(0,0,480,272,1,img_splash);
        pgScreenFlipV();
	
	playIntroWav();
	pgWaitVn(40);

	pgFillvram(0);
        pgBitBlt(0,0,480,272,1,img_rotate);
	pgScreenFlipV();

	pgWaitVn(100);
	//pgWaitVn(10);

	doArkanoid();		// Lets get this show on the road
	
	sceKernelSleepThread();

	return 0;
}

