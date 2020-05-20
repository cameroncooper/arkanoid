#include "arkanoid.h"
#include "images.h"
#include "syscall.h"
#include "_clib.h"
#include "pg.h"
#include "audio.h"
#include "levels.h"

#define BLOCKTOP 380

paddleData paddle;
blockData blocks[11][7];
powerupData powerups[11][7];
ballData ball[5]; // a max of 5 balls
int numPaddles;
int numPoints=0;
int numCredits=0;
int frame;
int needCredits=1;
int whichInsertCoin;
int soundsLoaded=0;
int gameOver=0;
int glued=0;
int bigPaddle=0;
int bigPaddleCount=0;
int level=0;

int exp10(int n)
{
	int result = 10;
	if (n == 0) return 1;
	while (n > 1) { result *= 10; n--; }
	return result;
}

void doBlockSound(int n)
{
	switch (n)
	{
		case 1: case 2: case 3: case 4: case 5: case 6: case 7: 
			wavoutStartPlay1(&wavinfo_se2); break;
		case 8: case 0: case -1:
			wavoutStartPlay1(&wavinfo_se1);
	}
}

void doPrintChar(char c, int x, int y)
{
	switch (c)
	{
		case 'A': pgBitBltA(x,y,12,12,1,img_char_a,1,0x7C1F); break;
		case 'B': pgBitBltA(x,y,12,12,1,img_char_b,1,0x7C1F); break;
		case 'C': pgBitBltA(x,y,12,12,1,img_char_c,1,0x7C1F); break;
		case 'D': pgBitBltA(x,y,12,12,1,img_char_d,1,0x7C1F); break;
		case 'E': pgBitBltA(x,y,12,12,1,img_char_e,1,0x7C1F); break;
		case 'F': pgBitBltA(x,y,12,12,1,img_char_f,1,0x7C1F); break;
		case 'G': pgBitBltA(x,y,12,12,1,img_char_g,1,0x7C1F); break;
		case 'H': pgBitBltA(x,y,12,12,1,img_char_h,1,0x7C1F); break;
		case 'I': pgBitBltA(x,y,12,12,1,img_char_i,1,0x7C1F); break;
		case 'J': pgBitBltA(x,y,12,12,1,img_char_j,1,0x7C1F); break;
		case 'K': pgBitBltA(x,y,12,12,1,img_char_k,1,0x7C1F); break;
		case 'L': pgBitBltA(x,y,12,12,1,img_char_l,1,0x7C1F); break;
		case 'M': pgBitBltA(x,y,12,12,1,img_char_m,1,0x7C1F); break;
		case 'N': pgBitBltA(x,y,12,12,1,img_char_n,1,0x7C1F); break;
		case 'O': pgBitBltA(x,y,12,12,1,img_char_o,1,0x7C1F); break;
		case 'P': pgBitBltA(x,y,12,12,1,img_char_p,1,0x7C1F); break;
		case 'Q': pgBitBltA(x,y,12,12,1,img_char_q,1,0x7C1F); break;
		case 'R': pgBitBltA(x,y,12,12,1,img_char_r,1,0x7C1F); break;
		case 'S': pgBitBltA(x,y,12,12,1,img_char_s,1,0x7C1F); break;
		case 'T': pgBitBltA(x,y,12,12,1,img_char_t,1,0x7C1F); break;
		case 'U': pgBitBltA(x,y,12,12,1,img_char_u,1,0x7C1F); break;
		case 'V': pgBitBltA(x,y,12,12,1,img_char_v,1,0x7C1F); break;
		case 'W': pgBitBltA(x,y,12,12,1,img_char_w,1,0x7C1F); break;
		case 'X': pgBitBltA(x,y,12,12,1,img_char_x,1,0x7C1F); break;
		case 'Y': pgBitBltA(x,y,12,12,1,img_char_y,1,0x7C1F); break;
		case 'Z': pgBitBltA(x,y,12,12,1,img_char_z,1,0x7C1F); break;
	}
}

#define SCORETOP 393
#define CHARLEFT  50

void doHighScores()
{
	unsigned long pad;
	int fd;
	int i,j,count=0;
	int scores[16][6];
	char buf[144];
	int highScoreIndex;
		
	wavoutClearWavinfoPtr(&wavinfo_bg);
	int ret=wavoutLoadWav("credits.wav",&wavinfo_bg,wavdata_bg,sizeof(wavdata_bg));
	if (ret!=0) halt("credits.wav load error");
	wavinfo_bg.playloop=1;
	wavoutStartPlay0(&wavinfo_bg);

	fd=pgfOpen("scores.dat",OF_RDONLY);
	if (fd<0) { halt("unable to open scores.dat"); }
	pgfRead(fd,buf,144);
	pgfClose(fd);
		
	// rip out the scores
	// print out the names
	for (i=0,count=0; i < 16; i++) {
		count += 3;
		for (j=0; j < 6; j++)
		{
			scores[i][j] = buf[count];
			count++;
		}
	}
	
	// see if we have a new high score
	for (i =0; i < 16; i++)
		if (numPoints > (scores[i][0]*100000+scores[i][1]*10000+scores[i][2]*1000+scores[i][3]*100+scores[i][4]*10+scores[i][5]))
			break;
		
	if (numPoints > (scores[i][0]*100000+scores[i][1]*10000+scores[i][2]*1000+scores[i][3]*100+scores[i][4]*10+scores[i][5]))
	{
		highScoreIndex=i;
		// get their name, we've got a new high score		
		int done=0;
		char c1,c2,c3;
		c1=c2=c3='A';
		int selChar=1;
		int flashCount=5;
		int flashShow=1;

		while (!done)
		{
			frame=(frame?0:1);
			pgFillvram(frame);
			pgBitBlt(0,0,480,272,1,img_highscores);
			pgBitBltA(310,26,70,220,1,img_enter_name,1,0x7C1F); 		
		
			// print out the chars
			switch (selChar)
			{
				case 1:	if (flashShow)	doPrintChar(c1, 280, 118); 
					doPrintChar(c2, 280, 132); doPrintChar(c3, 280, 146); break;
				case 2:	if (flashShow)	doPrintChar(c2, 280, 132); 
					doPrintChar(c1, 280, 118); doPrintChar(c3, 280, 146); break;
				case 3:	if (flashShow)	doPrintChar(c3, 280, 146); 
					doPrintChar(c2, 280, 132); doPrintChar(c1, 280, 118); break;
			}
			
			flashCount--;
			if (flashCount <= 0)
			{
				flashCount = 5;
				flashShow=(flashShow?0:1);
			}
			
			pgWaitV();
			
			pgScreenFlipV();
			
			pad=pgiGetpad();
	
			if ((pad&CTRL_UP)!=0)
				if (selChar > 1)
					selChar--;
			if ((pad&CTRL_DOWN)!=0)
				if (selChar < 3)
					selChar++;
			if ((pad&CTRL_RIGHT)!=0)
			{
				switch (selChar)
				{
					case 1:
						c1++;
						if (c1 > 'Z') c1 = 'A';
						break;
					case 2:
						c2++;
						if (c2 > 'Z') c2 = 'A';
						break;
					case 3:
						c3++;
						if (c3 > 'Z') c3 = 'A';
						break;												
				}
			}
			if ((pad&CTRL_LEFT)!=0)
			{
				switch (selChar)
				{
					case 1:
						c1--;
						if (c1 < 'A') c1 = 'Z';
						break;
					case 2:
						c2--;
						if (c2 < 'A') c2 = 'Z';
						break;
					case 3:
						c3--;
						if (c3 < 'A') c3 = 'Z';
						break;
				}
			}
			if ((pad&CTRL_LTRIGGER)!=0)
			{
				// put the info in buffer and save it back to the file
				// copy everything from highScoreIndex down one
				for (i=15,count=143; i >= highScoreIndex;i--)
					for (j=0; j < 9; j++,count--)
						buf[count] = buf[count-9];
					
						
				buf[highScoreIndex*9] = c1;
				buf[highScoreIndex*9+1] = c2;
				buf[highScoreIndex*9+2] = c3;
				buf[highScoreIndex*9+3] = ((numPoints / 100000) % 10);
				buf[highScoreIndex*9+4] = ((numPoints / 10000) % 10);
				buf[highScoreIndex*9+5] = ((numPoints / 1000) % 10);
				buf[highScoreIndex*9+6] = ((numPoints / 100) % 10);
				buf[highScoreIndex*9+7] = ((numPoints / 10) % 10);
				buf[highScoreIndex*9+8] = (numPoints  % 10);
				
				fd=pgfOpen("scores.dat",OF_WRONLY);
				if (fd<0) { halt("unable to open scores.dat"); }
				pgfWrite(fd,buf,144);
				pgfClose(fd);
				
				done = 1;
			}
			pgWaitVn(3);
		}
	}
	
	// rip out the scores again, since it might be updated
	for (i=0,count=0; i < 16; i++) {
		count += 3;
		for (j=0; j < 6; j++)
		{
			scores[i][j] = buf[count];
			count++;
		}
	}

	
	while (1)
	{
		pad=pgiGetpad();
	
		if (pad !=0)
		{
			wavoutStopPlay0();
			wavout_snd0_playend=1;
		
			doArkanoid();
		}
		
		frame=(frame?0:1);
		pgFillvram(frame);
		
		pgBitBlt(0,0,480,272,1,img_highscores);
		
		// print out the numbers
		{
		pgBitBltA(SCORETOP,22,12,12,1,img_num_1,1,0x7C1F);
		pgBitBltA(SCORETOP - 14,22,12,12,1,img_num_2,1,0x7C1F);
		pgBitBltA(SCORETOP - 28,22,12,12,1,img_num_3,1,0x7C1F);
		pgBitBltA(SCORETOP - 42,22,12,12,1,img_num_4,1,0x7C1F);
		pgBitBltA(SCORETOP - 56,22,12,12,1,img_num_5,1,0x7C1F);
		pgBitBltA(SCORETOP - 70,22,12,12,1,img_num_6,1,0x7C1F);
		pgBitBltA(SCORETOP - 84,22,12,12,1,img_num_7,1,0x7C1F);
		pgBitBltA(SCORETOP - 98,22,12,12,1,img_num_8,1,0x7C1F);
		pgBitBltA(SCORETOP - 112,22,12,12,1,img_num_9,1,0x7C1F);
		pgBitBltA(SCORETOP - 126,22,12,12,1,img_num_1,1,0x7C1F); pgBitBltA(SCORETOP - 126 ,34,12,12,1,img_num_0,1,0x7C1F);
		pgBitBltA(SCORETOP - 140,22,12,12,1,img_num_1,1,0x7C1F); pgBitBltA(SCORETOP - 140 ,34,12,12,1,img_num_1,1,0x7C1F);
		pgBitBltA(SCORETOP - 154,22,12,12,1,img_num_1,1,0x7C1F); pgBitBltA(SCORETOP - 154 ,34,12,12,1,img_num_2,1,0x7C1F);
		pgBitBltA(SCORETOP - 168,22,12,12,1,img_num_1,1,0x7C1F); pgBitBltA(SCORETOP - 168 ,34,12,12,1,img_num_3,1,0x7C1F);
		pgBitBltA(SCORETOP - 182,22,12,12,1,img_num_1,1,0x7C1F); pgBitBltA(SCORETOP - 182 ,34,12,12,1,img_num_4,1,0x7C1F);
		pgBitBltA(SCORETOP - 196,22,12,12,1,img_num_1,1,0x7C1F); pgBitBltA(SCORETOP - 196 ,34,12,12,1,img_num_5,1,0x7C1F);
		pgBitBltA(SCORETOP - 210,22,12,12,1,img_num_1,1,0x7C1F); pgBitBltA(SCORETOP - 210 ,34,12,12,1,img_num_6,1,0x7C1F);
		}
		
		// print out the names
		for (i=0,count=0; i < 16; i++) {
			for (j=0; j < 3; j++)
			{
				doPrintChar(buf[count], SCORETOP - 14*i , 50+12*j);
				
				count++;
			}
			count += 6;
		}
		
		for (j=0; j < 16; j++)
			doPrintNum((scores[j][0]*100000+scores[j][1]*10000+scores[j][2]*1000+scores[j][3]*100+scores[j][4]*10+scores[j][5]), SCORETOP - 14*j, 90);

		pgScreenFlipV();
	}	
	level=0;
	numPoints=0;
}

void doPrintNum(int n, int x, int y)
{
	int i;
	
	// Print out the numbers
	for (i = 0; i < 6; i++)
		switch((n / exp10(5-i)) % 10)
		{
			case 1: pgBitBltA(x,y+12*i,12,12,1,img_num_1,1,0x7C1F); break;
			case 2: pgBitBltA(x,y+12*i,12,12,1,img_num_2,1,0x7C1F); break;
			case 3: pgBitBltA(x,y+12*i,12,12,1,img_num_3,1,0x7C1F); break;
			case 4: pgBitBltA(x,y+12*i,12,12,1,img_num_4,1,0x7C1F); break;
			case 5: pgBitBltA(x,y+12*i,12,12,1,img_num_5,1,0x7C1F); break;
			case 6: pgBitBltA(x,y+12*i,12,12,1,img_num_6,1,0x7C1F); break;
			case 7: pgBitBltA(x,y+12*i,12,12,1,img_num_7,1,0x7C1F); break;
			case 8: pgBitBltA(x,y+12*i,12,12,1,img_num_8,1,0x7C1F); break;
			case 9: pgBitBltA(x,y+12*i,12,12,1,img_num_9,1,0x7C1F); break;
			default: pgBitBltA(x,y+12*i,12,12,1,img_num_0,1,0x7C1F);
		}
}

void doRenderBlocks()
{
	int i,j;
	// start at 380,14
	for (i=0; i <11; i++)
		for (j=0; j < 7; j++)
			switch (blocks[i][j].type)
			{
				case 1: pgBitBlt(BLOCKTOP-i*13,14+j*35,13,35,1,img_block_1); break;
				case 2: pgBitBlt(BLOCKTOP-i*13,14+j*35,13,35,1,img_block_2); break;
				case 3: pgBitBlt(BLOCKTOP-i*13,14+j*35,13,35,1,img_block_3); break;
				case 4: pgBitBlt(BLOCKTOP-i*13,14+j*35,13,35,1,img_block_4); break;
				case 5: pgBitBlt(BLOCKTOP-i*13,14+j*35,13,35,1,img_block_5); break;
				case 6: pgBitBlt(BLOCKTOP-i*13,14+j*35,13,35,1,img_block_6); break;
				case 7: pgBitBlt(BLOCKTOP-i*13,14+j*35,13,35,1,img_block_7); break;
				case 8: pgBitBlt(BLOCKTOP-i*13,14+j*35,13,35,1,img_block_8); break;
			}
}

void doRenderPowerups()
{
	int i,j;
	
	for (i=0; i < 11; i++)
		for (j=0; j < 7; j++)
			if (powerups[i][j].type != 0)
			{
				switch (powerups[i][j].type)
				{
					case 1: // exp
						switch (powerups[i][j].frame)
						{
							case 1: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_exp_1,1,0x7C1F); break;
							case 2: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_exp_2,1,0x7C1F); break;
							case 3: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_exp_3,1,0x7C1F); break;
							case 4: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_exp_4,1,0x7C1F); break;
							case 5: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_exp_5,1,0x7C1F); break;
							case 6: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_exp_6,1,0x7C1F); break;
							case 7: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_exp_7,1,0x7C1F); break;
							case 8: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_exp_8,1,0x7C1F); break;
							case 9: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_exp_9,1,0x7C1F); break;
							case 10: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_exp_10,1,0x7C1F); break;
							case 11: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_exp_11,1,0x7C1F); break;
						} break;				
					case 2: // glue
						switch (powerups[i][j].frame)
						{
							case 1: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_glue_1,1,0x7C1F); break;
							case 2: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_glue_2,1,0x7C1F); break;
							case 3: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_glue_3,1,0x7C1F); break;
							case 4: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_glue_4,1,0x7C1F); break;
							case 5: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_glue_5,1,0x7C1F); break;
							case 6: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_glue_6,1,0x7C1F); break;
							case 7: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_glue_7,1,0x7C1F); break;
							case 8: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_glue_8,1,0x7C1F); break;
							case 9: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_glue_9,1,0x7C1F); break;
							case 10: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_glue_10,1,0x7C1F); break;
							case 11: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_glue_11,1,0x7C1F); break;
						} break;	
					case 3: // ball
						switch (powerups[i][j].frame)
						{
							case 1: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_ball_1,1,0x7C1F); break;
							case 2: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_ball_2,1,0x7C1F); break;
							case 3: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_ball_3,1,0x7C1F); break;
							case 4: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_ball_4,1,0x7C1F); break;
							case 5: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_ball_5,1,0x7C1F); break;
							case 6: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_ball_6,1,0x7C1F); break;
							case 7: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_ball_7,1,0x7C1F); break;
							case 8: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_ball_8,1,0x7C1F); break;
							case 9: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_ball_9,1,0x7C1F); break;
							case 10: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_ball_10,1,0x7C1F); break;
							case 11: pgBitBltA(powerups[i][j].y,14+j*35,10,37,1,img_ball_11,1,0x7C1F); break;
						} break;
				}
				powerups[i][j].y += powerups[i][j].dy;
				if (powerups[i][j].y <= 10) powerups[i][j].type = 0;
				powerups[i][j].frame++;
				if (powerups[i][j].frame == 12) powerups[i][j].frame = 1;
			}
}

void doRenderBalls()
{
	int i;
	for (i=0; i < 3; i++)
		if (ball[i].x >= 0)
			pgBitBltA((unsigned long)ball[i].x,(unsigned long)ball[i].y,13,14,1,img_ball,1,0x7C1F);
}

void doRenderLevel()
{
	pgBitBltA(460,10,12,60,1,img_level,1,0x7C1F);
	
	int x,y;
	
	if ((level+1) > 10)
	{
		x = 446; y = 28;
			switch(((level+1) / 10) % 10)
			{
				case 1: pgBitBltA(x,y+12,12,12,1,img_num_1,1,0x7C1F); break;
				case 2: pgBitBltA(x,y+12,12,12,1,img_num_2,1,0x7C1F); break;
				case 3: pgBitBltA(x,y+12,12,12,1,img_num_3,1,0x7C1F); break;
				case 4: pgBitBltA(x,y+12,12,12,1,img_num_4,1,0x7C1F); break;
				case 5: pgBitBltA(x,y+12,12,12,1,img_num_5,1,0x7C1F); break;
				case 6: pgBitBltA(x,y+12,12,12,1,img_num_6,1,0x7C1F); break;
				case 7: pgBitBltA(x,y+12,12,12,1,img_num_7,1,0x7C1F); break;
				case 8: pgBitBltA(x,y+12,12,12,1,img_num_8,1,0x7C1F); break;
				case 9: pgBitBltA(x,y+12,12,12,1,img_num_9,1,0x7C1F); break;
				default: pgBitBltA(x,y+12,12,12,1,img_num_0,1,0x7C1F);
			}
			switch((level+1) % 10)
			{
				case 1: pgBitBltA(x,y,12,12,1,img_num_1,1,0x7C1F); break;
				case 2: pgBitBltA(x,y,12,12,1,img_num_2,1,0x7C1F); break;
				case 3: pgBitBltA(x,y,12,12,1,img_num_3,1,0x7C1F); break;
				case 4: pgBitBltA(x,y,12,12,1,img_num_4,1,0x7C1F); break;
				case 5: pgBitBltA(x,y,12,12,1,img_num_5,1,0x7C1F); break;
				case 6: pgBitBltA(x,y,12,12,1,img_num_6,1,0x7C1F); break;
				case 7: pgBitBltA(x,y,12,12,1,img_num_7,1,0x7C1F); break;
				case 8: pgBitBltA(x,y,12,12,1,img_num_8,1,0x7C1F); break;
				case 9: pgBitBltA(x,y,12,12,1,img_num_9,1,0x7C1F); break;
				default: pgBitBltA(x,y,12,12,1,img_num_0,1,0x7C1F);
			}
	}
	else
	{
		x = 446; y = 34;	
		switch((level+1) % 10)
			{
				case 1: pgBitBltA(x,y,12,12,1,img_num_1,1,0x7C1F); break;
				case 2: pgBitBltA(x,y,12,12,1,img_num_2,1,0x7C1F); break;
				case 3: pgBitBltA(x,y,12,12,1,img_num_3,1,0x7C1F); break;
				case 4: pgBitBltA(x,y,12,12,1,img_num_4,1,0x7C1F); break;
				case 5: pgBitBltA(x,y,12,12,1,img_num_5,1,0x7C1F); break;
				case 6: pgBitBltA(x,y,12,12,1,img_num_6,1,0x7C1F); break;
				case 7: pgBitBltA(x,y,12,12,1,img_num_7,1,0x7C1F); break;
				case 8: pgBitBltA(x,y,12,12,1,img_num_8,1,0x7C1F); break;
				case 9: pgBitBltA(x,y,12,12,1,img_num_9,1,0x7C1F); break;
				default: pgBitBltA(x,y,12,12,1,img_num_0,1,0x7C1F);
			}
	}
}

void doRender()
{
	int i;
	frame=(frame?0:1);
	pgFillvram(frame);
	
	// draw the board
        pgBitBlt(0,0,480,272,1,img_level1);
	
	// render the level
	doRenderLevel();
        
	// draw the paddle
	if (numPaddles > 0 && needCredits == 0)
	{
		if (bigPaddle)	
			pgBitBltA(15,paddle.pos,10,50,1,img_paddle_big,1,0x7C1F);
		else 
			pgBitBltA(15,paddle.pos,10,38,1,img_paddle,1,0x7C1F);
		for (i=1; i < numPaddles; i++)
		{
			pgBitBltA(0,(i-1)*38,10,38,1,img_lifepaddle,1,0x7C1F);
		} 
	}
	
	// render the blocks
	doRenderBlocks();
	
	// render powerups
	doRenderPowerups();
	
	// render the balls
	if (needCredits == 0)
		doRenderBalls();	
	else
	{
		if (needCredits != 0)
		{
			if (whichInsertCoin == 0)
				pgBitBltA(100,91,10,95,1,img_insert_coin_1,1,0x7C1F);
		
			whichInsertCoin=(whichInsertCoin?0:1);	
			pgWaitVn(20);
		}
	}
	
	// render points
	doPrintNum(numPoints, 445, 101);
	
	pgScreenFlipV();
}

void doCheckLevelOver()
{
	int i,j,done=1;
	for (i = 0; i < 11; i++)
		for (j = 0; j < 7; j++)
			if ((blocks[i][j].type != 0) && (blocks[i][j].type != 8))
				done = 0;			// This one still isn't broken
				
	if (done)
	{
		// Do whatever we have do to to load the new level
		level++;
		wavoutStartPlay1(&wavinfo_se4);
		if ((level % 9) == 0)
			numPoints += 1000;
		
		doArkanoid();
	}
}

void doUpdateBalls()
{
	int i;
	for (i=0; i < 3; i++)
		if (ball[i].x >= 0 && !glued)
		{
			ball[i].x += ball[i].dx;
			ball[i].y += ball[i].dy;
		}	
}

void doUpdateCheckCollisions()
{
	int i,j,k;
	for (k=0; k < 3; k++)
	{
		// See if ball has hit a block from the front
		if (ball[k].dx > 0)
		{
			for (i=0; i < 11; i++)
				for (j=0; j < 7; j++)
					if (blocks[i][j].type != 0)
						if (((ball[k].x + 13) >= (BLOCKTOP - (i+1)*13)) && ((ball[k].x + 13) < (BLOCKTOP - i*13)))
							if (((ball[k].y + 14) >= (j*35 + 14)) && ((ball[k].y - 14) < ((j+1)*35)))
								{
									if (blocks[i][j].type > 0 && blocks[i][j].type < 8)
									{
										wavoutStartPlay1(&wavinfo_se1);
										blocks[i][j].health--;
										if (blocks[i][j].health == 0)
										{
											doBlockSound(blocks[i][j].type);
											blocks[i][j].type = 0;
											numPoints += blocks[i][j].points;
											doCheckLevelOver();
											powerups[i][j].type = blocks[i][j].drop;
											powerups[i][j].y = BLOCKTOP - (i+1)*13;
											powerups[i][j].dy = -1;
											powerups[i][j].frame = 1;
										} else doBlockSound(-1); // Play a thud sound		
									}
									if (ball[k].dx > 0)
										ball[k].dx *= -1; // ball bounces back.	
									i = 12; j = 8; // make it stop
								}
		}
		// See if ball has hit a block from the back
		else
			for (i=0; i < 11; i++)
				for (j=0; j < 7; j++)
					if (blocks[i][j].type != 0)
						if ((ball[k].x <= (BLOCKTOP - i*13)) && ( ball[k].x >= (BLOCKTOP - (i+1)*13)))
							if (((ball[k].y + 14) >= (j*35 + 14)) && ((ball[k].y - 14) < ((j+1)*35)))
								{
									if (blocks[i][j].type > 0 && blocks[i][j].type < 8)
									{
										blocks[i][j].health--;
										if (blocks[i][j].health == 0)
										{
											doBlockSound(blocks[i][j].type);
											blocks[i][j].type = 0;
											numPoints += blocks[i][j].points;
											doCheckLevelOver();
											powerups[i][j].type = blocks[i][j].drop;
											powerups[i][j].y = BLOCKTOP - (i+1)*13;
											powerups[i][j].dy = -1;
											powerups[i][j].frame = 1;
										} else doBlockSound(-1); // Play a thud sound
									}
									if (ball[k].dx < 0)
										ball[k].dx *= -1; // ball bounces back.	
									i = 12; j = 8; // make it stop
								}
			
		// See if the ball has hit the paddle
		if ((ball[k].x <= 25) && (ball[k].dx < 0))
			if ((ball[k].y + 13 >= paddle.pos) && (ball[k].y <= paddle.pos + paddle.size))
			{
				ball[k].dx *= -1;
				int mag;
				if (ball[k].dy < 0) mag = -1; else mag = 1;
				ball[k].dy = (paddle.pos - ball[k].y + 12) / -3;
				if ( mag < 0 && ball[k].dy > 0 ) ball[k].dy *= mag; // make sure it stays going the same direction
				if ( mag > 0 && ball[k].dy < 0 ) ball[k].dy *= mag;
				ball[k].dx = 7-(paddle.pos - ball[k].y + 12) / -6;
				if (ball[k].dx < 0) ball[k].dx *= -1;
			}
			else
			{
				// The ball has gone too far
				ball[k].x = -1; // put it out of bounds
				
				int l,action=0;
				for (l = 0; l < 3; l++)
					if (ball[l].x != -1)
						action = 1;
				
				if (!action)
				{				
					// See if there are no balls left
					
					ball[0].x = 25;
					ball[0].y = 130;
					ball[0].dx = 0;
					ball[0].dy = 0;
					paddle.pos = 117; // Reset the paddle
					numPaddles--;
					
					paddle.size = 38;
					bigPaddle = 0;
					
					if (numPaddles == 0)
					{
						numCredits--;
						if (numCredits == 0)
						{
							// Show "Game Over" for a few seconds, then doArkanoid
							needCredits = 1;
							
							wavoutStopPlay0();
							wavout_snd0_playend=1;
							wavoutStartPlay1(&wavinfo_se3);
							
							for (i=0; i < 16; i++)
							{
								frame=(frame?0:1);
								pgFillvram(frame);
				
								// draw the board
								pgBitBlt(0,0,480,272,1,img_level1);
								doRenderBlocks();
								
								if (i%2)
									pgBitBltA(150,89,12,96,1,img_gameover,1,0x7C1F);
									
								doPrintNum(numPoints, 445, 101);
								doRenderLevel();
								
								pgScreenFlipV();
								pgWaitVn(20);
							}
							
							doHighScores();
							
						}
						else
							doArkanoid();	
					}
				}
			}
		
		// See if the ball hit the side	
		if (ball[k].y <= 14)
		{
			if (ball[k].dy < 0)
				ball[k].dy *= -1;
			ball[k].y = 14;
		} 
		if (ball[k].y >= 245)
		{
			if (ball[k].dy > 0)
				ball[k].dy *= -1;
			ball[k].y = 245;
		}
		
		// See if the ball has hit the end
		if (ball[k].x >= 410)
		{
			ball[k].x = 410;
			ball[k].dx *= -1;
		}
		
		// Make sure nothing bad happened to dx
		if (ball[k].dy != 0 && ball[k].dx == 0)
			ball[k].dx = 5;
	}
	
	// check on the powerups
	for (i=0; i < 11; i++)
		for (j=0; j < 7; j++)
			if (powerups[i][j].type != 0)
				if (powerups[i][j].y <= 25 && powerups[i][j].y >= 15)
					if (((j*35 + 14+ 37) >= paddle.pos) && ((j*35 + 14) <= (paddle.pos + paddle.size)))
					{
						// Caught the item
						switch (powerups[i][j].type)
						{
							case 1: 
								// Expand the paddle
								paddle.size = 50;
								bigPaddle = 1;
								if (paddle.pos > (272 - 13 - paddle.size)) paddle.pos = (272 - 13 - paddle.size);
								bigPaddleCount = 500;
								break;
							case 2: 
								// We're glued
								ball[0].y = paddle.pos + (paddle.size / 2) - 7;
								ball[0].x = 25;
								glued = 1;
								break;
							case 3: 
								// We've got balls
								if (ball[1].x == -1)
								{
									ball[1].y = 130;
									ball[1].x = BLOCKTOP - 156;
									ball[1].dx = -5;
									ball[1].dy = -3;
								}
								if (ball[2].x == -1)
								{
									ball[2].y = paddle.pos + (paddle.size / 2) - 7;
									ball[2].x = 25;
									ball[2].dx = 6;
									ball[2].dy = 2;
								}
								break;
						}
						powerups[i][j].type = 0;	
					}	
}

void doUpdate()
{
	unsigned long pad;

	// update the balls
	doUpdateBalls();
	
	// get new keypresses
	pad=pgiGetpad();
	
	int i,action=0;
	for (i=0; i < 3; i++) // we want to see if there is action
		if ((ball[i].x >= 0) && (ball[i].dx != 0 || ball[i].dy != 0)) 
			action = 1;
	
	if (((pad&CTRL_UP)!=0) && (action || glued))
	{
		paddle.pos -= paddle.speed;
		if (glued)
			ball[0].y -= paddle.speed;
		if (paddle.pos < 0) paddle.pos = 0; // Hit the wall
	}
	if (((pad&CTRL_DOWN)!=0) && (action || glued))
	{
		paddle.pos += paddle.speed;
		if (glued)
			ball[0].y += paddle.speed;
		if (paddle.pos > (272 - 13 - paddle.size)) paddle.pos = (272 - paddle.size); // Hit the wall
	}
	if ((pad&CTRL_LTRIGGER)!=0)
		if (((ball[0].dx == 0) && (ball[0].dy == 0) && (ball[0].x == 25)) || glued)
		{
			glued = 0;
			ball[0].dx = 5;
			ball[0].dy = 2;
		}
	if ((pad&CTRL_RTRIGGER)!=0)
	{
		if (ball[0].dx == 0 && ball[0].dy == 0 && ball[0].x == 25) numCredits=1;
		if (needCredits == 1)
		{
			needCredits = 0;
			numCredits--;
			doArkanoid(); // Start over
		}
	}
	
	// check for collisions
	doUpdateCheckCollisions();
	
}

void loadSounds()
{
	int ret;
	ret=wavoutLoadWav("4.wav",&wavinfo_se0,wavdata_se4,sizeof(wavdata_se4));
	if (ret!=0) halt("4.wav load error");
	ret=wavoutLoadWav("2.wav",&wavinfo_se1,wavdata_se1,sizeof(wavdata_se1));
	if (ret!=0) halt("2.wav load error");
	ret=wavoutLoadWav("3.wav",&wavinfo_se2,wavdata_se2,sizeof(wavdata_se2));
	if (ret!=0) halt("3.wav load error");
	ret=wavoutLoadWav("1.wav",&wavinfo_se3,wavdata_se3,sizeof(wavdata_se3));
	if (ret!=0) halt("1.wav load error");
	soundsLoaded = 1;
}

void doPlayMusic()
{
	if (wavout_snd0_playend)
	{
		wavoutClearWavinfoPtr(&wavinfo_bg);
		int ret=wavoutLoadWav("theme.wav",&wavinfo_bg,wavdata_bg,sizeof(wavdata_bg));
		if (ret!=0) halt("theme.wav load error");
		wavinfo_bg.playloop=1;
		wavoutStartPlay0(&wavinfo_bg);
	}
}

void doLoadLevel(int l)
{
	// Fill in the blocks
	int i,j,count=0;
	for (i=0; i < 11; i++)
		for (j=0; j < 7; j++)
		{
			switch ((level%9)+1)
			{
				case 1: 
					blocks[i][j].type = level1[count];
					blocks[i][j].health = level1[count+1];
					blocks[i][j].points = level1[count+2];
					blocks[i][j].drop = level1[count+3];
					count +=4;
				break;
				case 2:
					blocks[i][j].type = level2[count];
					blocks[i][j].health = level2[count+1];
					blocks[i][j].points = level2[count+2];
					blocks[i][j].drop = level2[count+3];
					count +=4;
				break;
				case 3:
					blocks[i][j].type = level3[count];
					blocks[i][j].health = level3[count+1];
					blocks[i][j].points = level3[count+2];
					blocks[i][j].drop = level3[count+3];
					count +=4;
				break;
				case 4:
					blocks[i][j].type = level4[count];
					blocks[i][j].health = level4[count+1];
					blocks[i][j].points = level4[count+2];
					blocks[i][j].drop = level4[count+3];
					count +=4;
				break;
				case 5:
					blocks[i][j].type = level5[count];
					blocks[i][j].health = level5[count+1];
					blocks[i][j].points = level5[count+2];
					blocks[i][j].drop = level5[count+3];
					count +=4;
				break;
				case 6:
					blocks[i][j].type = level6[count];
					blocks[i][j].health = level6[count+1];
					blocks[i][j].points = level6[count+2];
					blocks[i][j].drop = level6[count+3];
					count +=4;
				break;
				case 7:
					blocks[i][j].type = level7[count];
					blocks[i][j].health = level7[count+1];
					blocks[i][j].points = level7[count+2];
					blocks[i][j].drop = level7[count+3];
					count +=4;
				break;			
				case 8:
					blocks[i][j].type = level8[count];
					blocks[i][j].health = level8[count+1];
					blocks[i][j].points = level8[count+2];
					blocks[i][j].drop = level8[count+3];
					count +=4;
				break;			
				case 9:
					blocks[i][j].type = level9[count];
					blocks[i][j].health = level9[count+1];
					blocks[i][j].points = level9[count+2];
					blocks[i][j].drop = level9[count+3];
					count +=4;
				break;			;						
			}
	

		}
}

void doArkanoid()
{
	// Set up the initial settings for the paddle
	paddle.pos = 117; // Start it out in the center
	paddle.speed = 10;
	paddle.size = 38;
	bigPaddle = 0;
	
	// 4 Paddles by default
	numPaddles = 3;
	
	// No points to start off
	// numPoints = 0;
	
	// Set a default insert coin image
	whichInsertCoin = 0;
	
	// Load up a level
	doLoadLevel(level);
	
	// Clear the powerups
	int i,j;
	for (i=0; i < 11; i++)
		for (j=0; j < 7; j++)
			powerups[i][i].type = 0;
	
	for (i=1; i < 3; i++)
		ball[i].x = -1;
			
	// Give main the ball a position
	ball[0].x = 25;
	ball[0].y = 130;
	ball[0].dx = 0;
	ball[0].dy = 0;
			
	pgScreenFrame(2,0);
	frame=1;
	
	// we want the screen up before the music starts playing
	if (wavout_snd0_playend)
		doRender();
	
	// Start the music up
	doPlayMusic();
	
	// Load up the sound effects
	if (!soundsLoaded)
		loadSounds();	
		
	while (1)
	{
		// The big paddle can only last so long
		if (bigPaddle)
		{
			bigPaddleCount--;
			if (bigPaddleCount <= 0)
			{
				bigPaddle = 0;
				paddle.size = 38;
			}
		}
		
		// paint the screen
		doRender();
		
		// perform updates
		doUpdate();
	}
}

void playIntroWav()
{
	int ret=wavoutLoadWav("intro.wav",&wavinfo_bg,wavdata_bg,sizeof(wavdata_bg));
	if (ret!=0) halt("intro.wav load error");
	wavoutStartPlay0(&wavinfo_bg);
	wavoutWaitEnd0();
	wavoutStopPlay0();
}


