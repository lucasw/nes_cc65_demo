// this example shows metasprite use, two pads polling,
// and simple collision detection that changes palette

// screen resolution is 256 x 240, or 32 x 30 tiles 8 pixels size

#include "cc65_nes/neslib.h"

// variables
static unsigned char i, j, k;
static unsigned char x, y;
static unsigned char pad;
static unsigned char spr, old_spr;
static unsigned char touch, vel;
static unsigned char frame;

// two players coords
static unsigned char player_x[2];
static unsigned char player_y[2];

static unsigned int scroll_y = 0;

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

// explosions
#define NUM_EXPLOSIONS 2
static unsigned char explosion_x[NUM_EXPLOSIONS];
static unsigned char explosion_y[NUM_EXPLOSIONS];

#define IN_BOUNDS(x1, x2, margin) ( \
    (((x1) < (margin)) && ((x2) < (margin))) || \
    (((x1) > (255 - (margin))) && ((x2) > (255 - (margin)))) || \
    (((x1) < ((x2) + (margin))) && ((x1) > ((x2) - (margin)))) \
    )

#define LIMIT(lvalue, min, max) \
  if (lvalue < (min)) \
  { \
    lvalue = (min); \
  } \
  else if (lvalue > (max)) \
  { \
    lvalue = (max); \
  }

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
  0,  0,  0x52,  3,
  8,  0,  0x52,  3 | (1 << 6),
  128   // TODO(lucasw) what is this?
};

const unsigned char explosion_meta[] = {
  0,  0,  0x45,  2,
  8,  0,  0x45,  2 | (1 << 6),
  0,  8,  0x45,  2 | (1 << 7),
  8,  8,  0x45,  2 | (3 << 6),
  128
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
// TODO(lucasw) instead just allocate the memory and memcpy player_meta1
// into it and change the palette number
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


static unsigned char list[6 * 3];

// init data for the update list, it contains MSB and LSB of a tile address
// in the nametable, then the tile number

const unsigned char list_init[6 * 3]={
  MSB(NTADR_A(2, 2)), LSB(NTADR_A(2, 2)), 0x42,
  MSB(NTADR_A(4, 2)), LSB(NTADR_A(4, 2)), 0x44,
  MSB(NTADR_A(6, 2)), LSB(NTADR_A(6, 2)), 0x42,
  MSB(NTADR_A(8, 2)), LSB(NTADR_A(8, 2)), 0x42,
  MSB(NTADR_A(10, 2)), LSB(NTADR_A(10, 2)), 0x43,
  MSB(NTADR_A(12, 2)), LSB(NTADR_A(12, 2)), 0x44
};

// TODO(lucasw) neslib has rand8
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
  // the first 16 colors are for the background (?)
  pal_col(0, 0x0f);
  pal_col(1, 0x12);
  pal_col(2, 0x3D);
  pal_col(3, 0x30);

  pal_col(4, 0x0f);
  pal_col(5, 0x15);
  pal_col(6, 0x17);
  pal_col(7, 0x39);

  // sprite palette 0
  // 16-31 are for the sprites (?)
  pal_col(16, 0x0f);  // sprite color 0
  // 17 - 18 are changed in the game loop
  pal_col(19, 0x15);  // sprite color 3

  // sprite palette 1
  pal_col(20, 0x0f);  // sprite color 0
  // 21 - 23 are changed in the game loop

  // sprite palette 2
  pal_col(24, 0x0f);  // set third sprite color 0
  pal_col(25, 0x27);  // set third sprite color 1
  pal_col(26, 0x3A);  // set third sprite color 2
  pal_col(27, 0x30);  // set third sprite color 3

  // sprite palette 3
  pal_col(28, 0x0f);  // set fourth sprite color 1
  pal_col(29, 0x13);  // set fourth sprite color 1
  pal_col(30, 0x23);  // set fourth sprite color 2
  pal_col(31, 0x33);  // set fourth sprite color 3

  memcpy(list, list_init, sizeof(list_init));
  set_vram_update(6, list);

  set_rand(0x3412);
  for (y = 0; y < 30; ++y)
  {
    for (x = 0; x < 32; ++x)
    {
      vram_adr(NTADR_A(x, y));
      i = rand8();
      vram_put((i > 180) ? (0x42 + rand8() % 3) : 0);
      // vram_put(0x42);

      if ((x % 4 == 0) && (y % 4 == 0))
      {
        // can only change background tile palette,
        // but can't rotate/flip like sprites.
        vram_adr(0x23C0 + (y >> 2) * 8 + (x >> 2));
        i = rand8();
        vram_put(i);
        /*
        for (j = 0; j < 8; j += 2)
        {
          k |= (1 << j);
        }*/
      }

      vram_adr(NTADR_C(x, y));
      i = rand8();
      vram_put((i > 230) ? (0x42 + rand8() % 3) : 0);
    }
  }

  // populate the first two screens

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

  for (i = 0; i < NUM_EXPLOSIONS; ++i)
  {
    explosion_x[i] = 0;
    explosion_y[i] = 255;
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

  // spr from last cycle
  old_spr = 0;

  // collision flag
  touch = 0;
  // frame counter
  frame = 0;

  // now the main loop

  while (1)
  {
    ppu_waitnmi();  // wait for next TV frame

    scroll(0, scroll_y);

    // the first 16 indices are for the background tiles?
    pal_col(17, touch ? i : 0x10);  // set first sprite color 1
    pal_col(18, touch ? i : 0x2D);  // set first sprite color 2

    pal_col(21, touch ? i : 0x10);  // set second sprite color 1
    pal_col(22, touch ? i : 0x0C);  // set second sprite color 2
    pal_col(23, touch ? i : 0x17);  // set second sprite color 3

    spr = 0;

    for (i = 0; i < 2; ++i)
    {
      spr = oam_meta_spr(player_x[i], player_y[i], spr, i ? player_meta2 : player_meta1);
    }

    for (i = 0; i < NUM_ENEMIES; ++i)
    {
      // flicker
      if (frame % 2 == 0)
        j = i;
      else
        j = NUM_ENEMIES - i - 1;

      if (enemy_y[j] < 255)
      {
        spr = oam_meta_spr(enemy_x[j], enemy_y[j], spr, enemy_meta);
      }
    }

    // player cannon blasts
    // set sprite in OAM buffer, chrnum is tile, attr is attribute, sprid is offset in OAM in bytes
    // returns sprid+4, which is offset for a next sprite
    // oam_spr(x, y, chrnum, attr, sprid);
    for (i = 0; i < 2; ++i)
    {
      for (j = 0; j < NUM_BULLETS; ++j)
      {
        if (by[i][j] < 255)
        // spr = oam_spr(b1x[i], b1y[i], 0x40, 2 | ((frame % 4 > 2) ? (1 << 6) : 0), spr);
          spr = oam_spr(bx[i][j], by[i][j], 0x40, 2, spr);
      }
    }

    for (i = 0; i < NUM_EXPLOSIONS; ++i)
    {
      if (explosion_y[i] < 255)
      {
        spr = oam_meta_spr(explosion_x[i], explosion_y[i], spr, explosion_meta);
        explosion_y[i] = 255;
      }
    }

    // clear all the old sprites
    // for (i = spr; i < old_spr; i += 4)
    for (i = spr; i < old_spr; i += 4)
    {
      oam_spr(0, 255, 0x0, 0, i);
    }
    old_spr = spr;

    ///////////////////////////////////////////////////////////////////////////
    // non graphics stuff - can happen after blank period

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
      vel = 2;
      if (pad & PAD_LEFT && player_x[i] > vel) player_x[i] -= vel;
      if (pad & PAD_RIGHT && player_x[i] < 255 - vel) player_x[i] += vel;
      if (pad & PAD_UP && player_y[i] > vel) player_y[i] -= vel;
      if (pad & PAD_DOWN && player_y[i] < 255 - vel) player_y[i] += vel;

      LIMIT(player_x[i], 0, 255 - 24);
      LIMIT(player_y[i], 6, 240 - 32);
    }

    // check for collision for a smaller bounding box
    // metasprite is 24x24, collision box is 20x20

    if (IN_BOUNDS(player_x[0] + 12, player_x[1] + 12, 22) &&
        IN_BOUNDS(player_y[0] + 8, player_y[1] + 8, 18))
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

      if (enemy_y[i] == 255)
      {

      }
      else if (frame % 2 == 0)
      {
        enemy_y[i] += 1;
        vel = enemy_r[i] >> 7;
        if (enemy_r[i] > 150)
          enemy_x[i] += vel;
        else
          enemy_x[i] -= vel;

        LIMIT(enemy_x[i], 8, 240);
      }

      for (j = 0; j < 2; ++j)
      {
        if (IN_BOUNDS(enemy_x[i] + 8, player_x[j] + 12, 16) &&
            IN_BOUNDS(enemy_y[i] + 4, player_y[j] + 8, 10))
        {
          explosion_x[0] = player_x[j] + 4;
          explosion_y[0] = player_y[j];
          enemy_y[i] = 255;
          player_y[j] = 255;
        }
      }
    }  // loop through enemies

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
          if (enemy_y[k] > 250)
            continue;

          // TODO(lucasw) handle overflow when +4, or +8 > 255
          if (IN_BOUNDS(bx[i][j] + 4, enemy_x[k] + 8, 8) &&
              IN_BOUNDS(by[i][j] + 4, enemy_y[k] + 4, 4))
          {
            by[i][j] = 255;
            explosion_x[0] = enemy_x[k];
            explosion_y[0] = enemy_y[k] - 4;
            enemy_y[k] = 255;
          }
        }  // collision detection
      }
    }  // bullet update

    // reset enemies
    for (i = 0; i < NUM_ENEMIES; ++i)
    {
      if (enemy_y[i] < 250)
        continue;

      enemy_x[i] = rand8();
      enemy_y[i] = 0;
    }

    // flashing color for touch
    i = frame & 1 ? 0x30 : 0x2a;

    scroll_y -= 1;
    if (scroll_y >= 240 * 2)
      scroll_y = 240 * 2 - 1;  // needs to be within height of 2 nametables

    frame++;
  }  // game loop
}  // main
