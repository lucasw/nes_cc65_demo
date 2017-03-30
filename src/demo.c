// this example shows metasprite use, two pads polling,
// and simple collision detection that changes palette

#include "cc65_nes/neslib.h"

// variables
static unsigned char i;
static unsigned char pad, spr;
static unsigned char touch;
static unsigned char frame;

// two players coords
static unsigned char cat_x[2];
static unsigned char cat_y[2];

// first player metasprite, data structure explained in neslib.h
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
const unsigned char meta_cat1[]={
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
const unsigned char meta_cat2[]={
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

void main(void)
{
  // enable rendering
  ppu_on_all();

  // set initial coords
  cat_x[0] = 52;
  cat_y[0] = 100;
  cat_x[1] = 180;
  cat_y[1] = 100;

  // init other vars

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

    // process players
    spr = 0;

    for (i = 0; i < 2; ++i)
    {
      // display metasprite
      spr = oam_meta_spr(cat_x[i], cat_y[i], spr, i ? meta_cat2 : meta_cat1);

      // poll pad and change coordinates
      pad = pad_poll(i);
      if (pad & PAD_LEFT && cat_x[i] > 0) cat_x[i] -= 2;
      if (pad & PAD_RIGHT && cat_x[i] < 232) cat_x[i] += 2;
      if (pad & PAD_UP && cat_y[i] > 0) cat_y[i] -= 2;
      if (pad & PAD_DOWN && cat_y[i] < 212) cat_y[i] += 2;
    }

    // bullet
    // set sprite in OAM buffer, chrnum is tile, attr is attribute, sprid is offset in OAM in bytes
    // returns sprid+4, which is offset for a next sprite
    // oam_spr(x, y, chrnum, attr, sprid);
    spr = oam_spr(25, 70, 0x40, 2, spr);


    // check for collision for a smaller bounding box
    // metasprite is 24x24, collision box is 20x20

    if (!(cat_x[0] + 22 < cat_x[1] + 2 ||
        cat_x[0] + 2 >= cat_x[1] + 22 ||
        cat_y[0] + 22 < cat_y[1] + 2 ||
        cat_y[0] + 2 >= cat_y[1] + 22))
    {
      touch = 1;
    }
    else
    {
      touch = 0;
    }

    frame++;
  }  // game loop
}  // main
