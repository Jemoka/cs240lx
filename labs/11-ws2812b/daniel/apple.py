import cv2
import numpy as np

cap = cv2.VideoCapture("bitmap.mp4")
frames = []
while True:
    ret, frame = cap.read()
    if not ret:
        break
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    binary = (gray > 128).astype(np.uint8)  # shape (7, 10), values 0 or 1
    frames.append(binary)
cap.release()

bitmap = np.stack(frames)  # shape (n_frames, 7, 10)

# we want to stick everything into n_frames*10 uint8s
(frames, w, h) = bitmap.shape

vals = []
for x in range(frames):
    for y in range(h):
        arr = bitmap[x, :, y]  # shape (7,)
        val = 0
        for i in range(7):
            val |= (arr[i] << i)
        vals.append(val)
vals = np.array(vals)
vals.astype(np.uint8).tofile('data.bin')

vals.max()

# with open('data.bin', 'ab') as f:
#     f.write(arr.astype(np.uint8).tobytes())


