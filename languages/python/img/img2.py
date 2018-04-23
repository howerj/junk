from cv import *

storage = CreateMemStorage(0)
img = LoadImage("img")  # or read from camera

found = list(HOGDetectMultiScale(img, storage, win_stride=(8,8),
                padding=(32,32), scale=1.05, group_threshold=2))

print found
