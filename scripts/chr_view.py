#!/usr/bin/env python
# Lucas Walter
# March 2017

# load a nes .chr or .nes and view the graphics

import cv2
import numpy as np
import sys


if __name__ == '__main__':
    image = np.zeros((8 * 5, 8 * 10, 3), np.uint8)

    colors = []
    colors.append((0, 0, 0))
    colors.append((60, 90, 180))
    colors.append((100, 150, 120))
    colors.append((250, 200, 250))

    image[:,:,0] = 228

    if False:  # for i in range(3):
        image[1::4, 3::7, i] = colors[1][i]
        # image[2::7, :, i] = colors[2][i]
        # image[3::11, :, i] = colors[3][i]

    # 16 bytes of contiguous memory makes a
    # 8 x 8 sprite x 2bpp = 128
    # 16 x 8 = 128
    # So the first row of 8 pixels is
    # 8pixels x 2bpp = 16 bits = 2 bytes
    # with open(sys.argv[1], "rb") as fl:
    chr_name = sys.argv[1]
    data = np.fromfile(chr_name, dtype=np.uint8)
    print len(data), (len(data) / 64), chr_name
    x = 0
    y = 0
    for ind in range(0, len(data), 2):
        if ind >= 8 * 2:
            break
        # tile_x = 0

        # get 8 pixels from 2 bytes
        tile = data[ind:ind+2]
        pind = [0, 0, 0, 0]
        for i in range(2):
            pind[0] = (tile[i] & (0x3 << 0))
            pind[1] = (tile[i] & (0x3 << 2)) >> 2
            pind[2] = (tile[i] & (0x3 << 4)) >> 4
            pind[3] = (tile[i] & (0x3 << 6)) >> 6
            for xo in range(len(pind)):
                x1 = x + xo + 4 * i
                print 'x1', x1, ', x', x, 'xo', xo, 'i ', i
                if x1 >= image.shape[1]:
                    print 'x ', x1, image.shape[1]
                    break
                if y >= image.shape[0]:
                    print 'y ', y, image.shape[0]
                    break
                for j in range(3):
                    image[y, x1, j] = colors[pind[xo]][j]
                # print y, x1, pind[xo], image[y, x1, :]
        # go to next row in current sprite
        # x += 8
        # if x % 8 == 0:
        if True:
            # x -= 8
            y += 1
            # go to next sprite
            # TODO store sprites as individual numpy images
            # then tile them in final image for visualization
            if y % 8 == 0:
                y -= 0
                x += 8
                # go to next row of tiles
                if x >= image.shape[1] - 8:
                    y += 8
                    x = 0
                    print x, y

    scale = 16
    scaled_image = cv2.resize(image, (0, 0), fx = scale, fy = scale,
                              interpolation = cv2.INTER_NEAREST)
    # pixel boundaries
    scaled_image[0::scale, :, 0] = 128
    scaled_image[0::scale, :, 1] = 128
    scaled_image[0::scale, :, 2] = 0
    scaled_image[:, 0::scale, 0] = 96
    scaled_image[:, 0::scale, 1] = 96
    scaled_image[:, 0::scale, 2] = 0

    while True:
        key = cv2.waitKey(10)
        if key == ord('q'):
            print key, ord('q')
            break
        cv2.imshow("image", scaled_image)
