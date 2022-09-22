#!/usr/bin/env python
import numpy as np

# copied from common.transformations/camera.py
eon_dcam_focal_length = 860.0  # pixels
webcam_focal_length = 908.0  # pixels

eon_dcam_intrinsics = np.array([
  [eon_dcam_focal_length,   0,   1152/2.],
  [  0,  eon_dcam_focal_length,  864/2.],
  [  0,    0,     1]])

webcam_intrinsics = np.array([
  [webcam_focal_length,   0.,   1280/2.],
  [  0.,  webcam_focal_length,  720/2.],
  [  0.,    0.,     1.]])

cam_id = 2

def gstreamer_pipeline(sensor_id, capture_width, capture_height, framerate, flip_method, display_width, display_height):
#    return "nvarguscamerasrc sensor_mode=4 sensor-id=" + str(sensor_id) + " ! video/x-raw(memory:NVMM), width=3264, height=2464, framerate=(fraction)" + str(framerate) + "/1, format=(string)NV12 ! nvvidconv flip-method=2 ! video/x-raw, format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! videoscale ! video/x-raw,width=" + str(width) + ",height=" + str(height) + " ! appsink";
  return "nvarguscamerasrc sensor_mode=2 sensor-id=" + str(sensor_id) + " ! video/x-raw(memory:NVMM), width=(int)" + str(capture_width) + ", height=(int)" +str(capture_height) + ", format=(string)NV12, framerate=(fraction)" + str(framerate) + "/1 ! nvvidconv flip-method=" + str(flip_method) + " ! video/x-raw, width=(int)" + str(display_width) + ", height=(int)" + str(display_height) + ", format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsink";


if __name__ == "__main__":
  import cv2  # pylint: disable=import-error

  trans_webcam_to_eon_front = np.dot(eon_dcam_intrinsics, np.linalg.inv(webcam_intrinsics))

  pipeline = gstreamer_pipeline(
          0,
          1920,
          1280,
          30,
          2,
          800,
          600)

  cap = cv2.VideoCapture(pipeline, cv2.CAP_GSTREAMER)
  cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1280)
  cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 720)

  while (True):
    ret, img = cap.read()
    if ret:
      img = cv2.warpPerspective(img, trans_webcam_to_eon_front, (1152, 864), borderMode=cv2.BORDER_CONSTANT, borderValue=0)
      img = img[:, -864//2:, :]
      cv2.imshow('preview', img)
      cv2.waitKey(10)
