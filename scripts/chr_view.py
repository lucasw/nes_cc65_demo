#!/usr/bin/env python
# Lucas Walter
# March 2017

# load a nes .chr or .nes and view the graphics

import cv2
import numpy as np
import sys


if __name__ == '__main__':
    image = np.zeros((8 * 5, 8 * 8, 3), np.uint8)

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
    loop = True
    for ind in range(0, len(data), 16):
        if not loop:
            break
        print 'ind', ind
        if ind >= 32:
            break
        if y >= image.shape[0]:
            print 'y ', y, image.shape[0]
            break

        # get 8 pixels from 16 bytes
        plane0 = data[ind:ind + 8]
        plane1 = data[ind + 8:ind + 16]
        pind = [0, 0, 0, 0, 0, 0, 0, 0]

        # go through each row in current sprite
        for j in range(8):
            for i in range(len(pind)):
                # first plane, lower bit
                plane0_bit = (plane0[j] & (0x8 >> i)) >> (7 - i)
                plane1_bit = (plane1[j] & (0x8 >> i)) >> (7 - i)
                # second plane, higher bit
                pind[i] = plane0_bit | (plane1_bit << 1)

                x1 = x + i
                y1 = y + j
                # print 'x1', x1, ', x', x, 'i', i, ', y', y
                if x1 >= image.shape[1]:
                    print 'x ', x1, image.shape[1]
                    loop = False
                    break
                # print 'pind', pind[i], i
                for k in range(3):
                    image[y1, x1, k] = colors[pind[i]][k]
                    # print y, x1, pind[xo], image[y, x1, :]
        x += 8
        if x > 8 * 8:
            x = 0
            y += 8
            print x, y
        # go to next sprite
        # TODO store sprites as individual numpy images
        # then tile them in final image for visualization
        if False:  # if y % 8 == 0:
            y -= 8
            x += 8
            # go to next row of tiles
            if x >= image.shape[1]:
                y += 8
                x = 0
                # print x, y

    scale = 16
    scaled_image = cv2.resize(image, (0, 0), fx = scale, fy = scale,
                              interpolation = cv2.INTER_NEAREST)
    # pixel boundary grid
    scaled_image[0::scale, :, 0] = 88
    scaled_image[0::scale, :, 1] = 78
    scaled_image[0::scale, :, 2] = 0
    scaled_image[:, 0::scale, 0] = 66
    scaled_image[:, 0::scale, 1] = 56
    scaled_image[:, 0::scale, 2] = 0

    # sprint boundary grid
    scale *= 8
    scaled_image[0::scale, :, 0] = 168
    scaled_image[0::scale, :, 1] = 168
    scaled_image[0::scale, :, 2] = 30
    scaled_image[:, 0::scale, 0] = 106
    scaled_image[:, 0::scale, 1] = 136
    scaled_image[:, 0::scale, 2] = 30


    while True:
        key = cv2.waitKey(10)
        if key == ord('q'):
            print key, ord('q')
            break
        cv2.imshow("image", scaled_image)
