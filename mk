echo "Compiling main.c" && \
/usr/local/ps2dev/ee/bin/ee-gcc -march=r4000 -O3 -g -mgp32 -mlong32 -c main.c && \
echo "Compiling pg.c" && \
/usr/local/ps2dev/ee/bin/ee-gcc -march=r4000 -O3 -g -mgp32 -mlong32 -c pg.c && \
echo "Compiling _clib.c" && \
/usr/local/ps2dev/ee/bin/ee-gcc -march=r4000 -O3 -g -mgp32 -mlong32 -c _clib.c && \
echo "Compiling arkanoid.c" && \
/usr/local/ps2dev/ee/bin/ee-gcc -march=r4000 -O3 -g -mgp32 -mlong32 -c arkanoid.c && \
echo "Assembling startup.s" && \
/usr/local/ps2dev/ee/bin/ee-gcc -march=r4000 -g -mgp32 -c -xassembler -O -a=startup.lst -o startup.o startup.s && \
echo "Linking" && \
/usr/local/ps2dev/ee/bin/ee-ld startup.o main.o pg.o _clib.o arkanoid.o images/loading/splash.o images/loading/rotate.o images/ball.o images/paddle.o images/lifepaddle.o images/insert_coin/insert_coin_1.o images/insert_coin/insert_coin_2.o images/blocks/block_1.o images/blocks/block_2.o images/blocks/block_3.o images/blocks/block_4.o images/blocks/block_5.o images/blocks/block_6.o images/blocks/block_7.o images/blocks/block_8.o images/level1.o images/num/num_0.o images/num/num_1.o images/num/num_2.o images/num/num_3.o images/num/num_4.o images/num/num_5.o images/num/num_6.o images/num/num_7.o images/num/num_8.o images/num/num_9.o images/gameover.o images/highscores.o images/text/char_a.o images/text/char_b.o images/text/char_c.o images/text/char_d.o images/text/char_e.o images/text/char_f.o images/text/char_g.o images/text/char_h.o images/text/char_i.o images/text/char_j.o images/text/char_k.o images/text/char_l.o images/text/char_m.o images/text/char_n.o images/text/char_o.o images/text/char_p.o images/text/char_q.o images/text/char_r.o images/text/char_s.o images/text/char_t.o images/text/char_u.o images/text/char_v.o images/text/char_w.o images/text/char_x.o images/text/char_y.o images/text/char_z.o images/powerup/exp/exp_1.o images/powerup/exp/exp_2.o images/powerup/exp/exp_3.o images/powerup/exp/exp_4.o images/powerup/exp/exp_5.o images/powerup/exp/exp_6.o images/powerup/exp/exp_7.o images/powerup/exp/exp_8.o images/powerup/exp/exp_9.o images/powerup/exp/exp_10.o images/powerup/exp/exp_11.o images/powerup/glue/glue_1.o images/powerup/glue/glue_2.o images/powerup/glue/glue_3.o images/powerup/glue/glue_4.o images/powerup/glue/glue_5.o images/powerup/glue/glue_6.o images/powerup/glue/glue_7.o images/powerup/glue/glue_8.o images/powerup/glue/glue_9.o images/powerup/glue/glue_10.o images/powerup/glue/glue_11.o images/powerup/ball/ball_1.o images/powerup/ball/ball_2.o images/powerup/ball/ball_3.o images/powerup/ball/ball_4.o images/powerup/ball/ball_5.o images/powerup/ball/ball_6.o images/powerup/ball/ball_7.o images/powerup/ball/ball_8.o images/powerup/ball/ball_9.o images/powerup/ball/ball_10.o images/powerup/ball/ball_11.o images/paddle_big.o levels.o images/enter_name.o images/level.o -M -Ttext 8900000 -q -o out > sound.map && \
echo "Patching" && \
./outpatch && \
echo "Packing" && \
../elf2pbp/elf2pbp outp "Arkanoid" && \
echo "Copying" && \
mount -t msdos /dev/sda1 /mnt/psp && \
cp EBOOT.PBP /mnt/psp/psp/game/Arkanoid/ && \
umount /mnt/psp 

