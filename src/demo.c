// this example shows metasprite use, two pads polling,
// and simple collision detection that changes palette

#include "cc65_nes/neslib.h"

// variables
static unsigned char i;
static unsigned char j;
static unsigned char k;
static unsigned char pad, spr;
static unsigned char touch;
static unsigned char frame;

// two players coords
static unsigned char player_x[2];
static unsigned char player_y[2];

#define NUM_BULLETS 4
static unsigned char bx[2][NUM_BULLETS];
static unsigned char by[2][NUM_BULLETS];
static unsigned char b_ind[2];

// enemies
#define NUM_ENEMIES 4
static unsigned char enemy_x[NUM_ENEMIES];
static unsigned char enemy_y[NUM_ENEMIES];
// random number for this enemy
static unsigned char enemy_r[NUM_ENEMIES];

#define IN_BOUNDS(x1, x2, margin) ( \
    (((x1) < (margin)) && ((x2) < (margin))) || \
    (((x1) > (255 - (margin))) && ((x2) > (255 - (margin)))) || \
    (((x1) < ((x2) + (margin))) && ((x1) > ((x2) - (margin)))) \
    )

// x offset, y offset, tile, attribute
/*
76543210
||||||||
||||||++- Palette (4 to 7) of sprite
|||+++--- Unimplemented
||+------ Priority (0: in front of background; 1: behind background)
|+------- Flip sprite horizontally
+-------- Flip sprite vertically
*/

const unsigned char enemy_meta[] = {
  8,  0,  0x52,  3,
  16,  0,  0x52,  3 | (1 << 6),
  128   // TODO(lucasw) what is this?
};


// first player metasprite
const unsigned char player_meta1[] = {
  0,  0,  0x50,  0,  // upper left corner ear
  8,  0,  0x51,  0,
  16,  0,  0x50,  1 << 6,
  0,  8,  0x60,  0,
  8,  8,  0x61,  0,
  16,  8,  0x60,  0 | (1 << 6),
  0,  16,  0x70,  0,
  8,  16,  0x71,  0,
  16,  16,  0x70,  1 << 6,
  128
};

// second player metasprite, the only difference is palette number
const unsigned char player_meta2[] = {
  0,  0,  0x50,  1,
  8,  0,  0x51,  1,
  16,  0,  0x50,  1 | (1 << 6),
  0,  8,  0x60,  1,
  8,  8,  0x61,  1,
  16,  8,  0x60,  1 | (1 << 6),
  0,  16,  0x70,  1,
  8,  16,  0x71,  1,
  16,  16,  0x70,  1 | (1 << 6),
  128
};

// http://www.arklyffe.com/main/2010/08/29/xorshift-pseudorandom-number-generator/
unsigned char __fastcall__ xorshift8(unsigned char y8)
{
  y8 ^= (y8 << 7);
  y8 ^= (y8 >> 5);
  return y8 ^= (y8 << 3);
}

// unsigned char __fastcall__ xorshift8(unsigned char x1, unsigned char y1,
//     unsigned char x2, unsigned char y2, unsigned char margin)

void main(void)
{
  // enable rendering
  ppu_on_all();

  // set initial coords
  player_x[0] = 52;
  player_y[0] = 200;
  player_x[1] = 180;
  player_y[1] = 200;

  for (i = 0; i < NUM_ENEMIES; ++i)
  {
    enemy_x[i] = 16 + i << 5;
    enemy_y[i] = 32;
    enemy_r[i] = 30 + i;
  }

  for (i = 0; i < 2; ++i)
  {
    b_ind[i] = 0;
    for (j = 0; j < NUM_BULLETS; ++j)
    {
      bx[i][j] = 128;
      by[i][j] = 128;
    }
  }

  // collision flag
  touch = 0;
  // frame counter
  frame = 0;

  // now the main loop

  while (1)
  {
    ppu_waitnmi();  // wait for next TV frame

    // flashing color for touch
    i = frame & 1 ? 0x30 : 0x2a;

    pal_col(16, 0x0f);  // set first sprite color 1
    pal_col(17, touch ? i : 0x10);  // set first sprite color 1
    pal_col(18, touch ? i : 0x2D);  // set first sprite color 2
    pal_col(19, 0x15);  // set first sprite color 3

    pal_col(21, touch ? i : 0x10);  // set second sprite color 1
    pal_col(22, touch ? i : 0x0C);  // set second sprite color 2
    pal_col(23, touch ? i : 0x17);  // set second sprite color 3

    pal_col(25, 0x27);  // set second sprite color 1
    pal_col(26, 0x3A);  // set second sprite color 2
    pal_col(27, 0x30);  // set second sprite color 3

    pal_col(28, 0x13);  // set second sprite color 1
    pal_col(29, 0x23);  // set second sprite color 2
    pal_col(30, 0x33);  // set second sprite color 3

    // process players
    spr = 0;

    for (i = 0; i < 2; ++i)
    {
      // display metasprite
      spr = oam_meta_spr(player_x[i], player_y[i], spr, i ? player_meta2 : player_meta1);
    }

    for (i = 0; i < NUM_ENEMIES; ++i)
    {
      spr = oam_meta_spr(enemy_x[i], enemy_y[i], spr, enemy_meta);
    }

    // player cannon blasts
    // set sprite in OAM buffer, chrnum is tile, attr is attribute, sprid is offset in OAM in bytes
    // returns sprid+4, which is offset for a next sprite
    // oam_spr(x, y, chrnum, attr, sprid);
    for (i = 0; i < 2; ++i)
    {
      for (j = 0; j < NUM_BULLETS; ++j)
      {
        // spr = oam_spr(b1x[i], b1y[i], 0x40, 2 | ((frame % 4 > 2) ? (1 << 6) : 0), spr);
        spr = oam_spr(bx[i][j], by[i][j], 0x40, 2, spr);
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    // non graphics stuff

    for (i = 0; i < 2; ++i)
    {
      // fire cannon
      pad = pad_trigger(i);
      if (pad & PAD_A)
      {
        player_y[i] += 2;
        bx[i][b_ind[i]] = player_x[i];
        by[i][b_ind[i]] = player_y[i];
        ++b_ind[i];
        b_ind[i] %= NUM_BULLETS;
      }

      // poll pad and change coordinates
      pad = pad_state(i);
      if (pad & PAD_LEFT && player_x[i] > 0) player_x[i] -= 2;
      if (pad & PAD_RIGHT && player_x[i] < 232) player_x[i] += 2;
      if (pad & PAD_UP && player_y[i] > 0) player_y[i] -= 2;
      if (pad & PAD_DOWN && player_y[i] < 212) player_y[i] += 2;
    }

    // check for collision for a smaller bounding box
    // metasprite is 24x24, collision box is 20x20

    if (!(player_x[0] + 22 < player_x[1] + 2 ||
        player_x[0] + 2 >= player_x[1] + 22 ||
        player_y[0] + 22 < player_y[1] + 2 ||
        player_y[0] + 2 >= player_y[1] + 22))
    {
      touch = 1;
    }
    else
    {
      touch = 0;
    }

    for (i = 0; i < NUM_ENEMIES; ++i)
    {
      enemy_r[i] = xorshift8(enemy_r[i]);
      if (frame % 4 == 0)
      {
        enemy_y[i] += 1;
        if (enemy_r[i] > 128)
          enemy_x[i] += enemy_r[i] >> 7;
        else
          enemy_x[i] -= enemy_r[i] >> 7;

        if (enemy_x[i] > 250)
          enemy_x[i] = 250;
        else if (enemy_x[i] < 5)
          enemy_x[i] = 5;
      }
    }

    // bullets
    for (i = 0; i < 2; ++i)
    {
      for (j = 0; j < NUM_BULLETS; ++j)
      {
        if ((by[i][j] >= 3) && (by[i][j] != 255))
          by[i][j] -= 3;
        else
          by[i][j] = 255;

        for (k = 0; k < NUM_ENEMIES; ++k)
        {
          // TODO(lucasw) handle overflow when +4, or +8 > 255
          if (IN_BOUNDS(bx[i][j] + 4, enemy_x[k] + 16, 8) &&
              IN_BOUNDS(by[i][j] + 4, enemy_y[k] + 4, 4))
          {
            by[i][j] = 255;
            enemy_y[k] = 0;
          }
        }  // collision detection
      }
    }  // bullet update

    frame++;
  }  // game loop
}  // main
