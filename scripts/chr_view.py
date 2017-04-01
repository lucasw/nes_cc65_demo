#!/usr/bin/env python
# Lucas Walter
# March 2017

# load a nes .chr or .nes and view the graphics

import cv2
import numpy as np


if __name__ == '__main__':
    image = np.zeros((240, 320, 3), np.uint8)

    colors = []
    colors.append((0, 0, 0))
    colors.append((60, 60, 60))
    colors.append((120, 120, 120))
    colors.append((250, 250, 250))

    image[:,:,0] = 128

    for i in range(3):
        image[1::4, 3::7, i] = colors[1][i]
        # image[2::7, :, i] = colors[2][i]
        # image[3::11, :, i] = colors[3][i]

    while True:
        key = cv2.waitKey(10)
        if key == ord('q'):
            print key, ord('q')
            break
        cv2.imshow("image", image)
