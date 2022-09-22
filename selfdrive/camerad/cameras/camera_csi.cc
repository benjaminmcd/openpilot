#include "selfdrive/camerad/cameras/camera_csi.h"

#include <unistd.h>

#include <cassert>
#include <cstring>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundefined-inline"
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#pragma clang diagnostic pop

#include "selfdrive/common/clutil.h"
#include "selfdrive/common/swaglog.h"
#include "selfdrive/common/timing.h"
#include "selfdrive/common/util.h"

// id of the video capturing device
const int ROAD_CAMERA_ID = util::getenv("ROADCAM_ID", 1);
const int DRIVER_CAMERA_ID = util::getenv("DRIVERCAM_ID", 2);

#define FRAME_WIDTH  1280
#define FRAME_HEIGHT 720

extern ExitHandler do_exit;

namespace {

CameraInfo cameras_supported[CAMERA_ID_MAX] = {
  // road facing
  [CAMERA_ID_IMX219] = {
      .frame_width = FRAME_WIDTH,
      .frame_height = FRAME_HEIGHT,
      .frame_stride = FRAME_WIDTH*3,
      .bayer = false,
      .bayer_flip = false,
  }
};

std::string gstreamer_pipeline(int sensor_id, int capture_width, int capture_height, int framerate, int flip_method, int display_width, int display_height)
  {
    //    return "nvarguscamerasrc sensor_mode=4 sensor-id=" + std::to_string(sensor_id) + " ! video/x-raw(memory:NVMM), width=3264, height=2464, framerate=(fraction)" + std::to_string(framerate) + "/1, format=(string)NV12 ! nvvidconv flip-method=2 ! video/x-raw, format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! videoscale ! video/x-raw,width=" + std::to_string(width) + ",height=" + std::to_string(height) + " ! appsink";
    return "nvarguscamerasrc sensor_mode=4 sensor-id=" + std::to_string(sensor_id) + " ! video/x-raw(memory:NVMM), width=(int)" + std::to_string(capture_width) + ", height=(int)" +
           std::to_string(capture_height) + ", format=(string)NV12, framerate=(fraction)" + std::to_string(framerate) +
           "/1 ! nvvidconv flip-method=" + std::to_string(flip_method) + " ! video/x-raw, width=(int)" + std::to_string(display_width) + ", height=(int)" +
           std::to_string(display_height) + ", format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsink";
  }

void camera_open(CameraState *s, bool rear) {
  // empty
}

void camera_close(CameraState *s) {
  // empty
}

void camera_init(VisionIpcServer * v, CameraState *s, int camera_id, unsigned int fps, cl_device_id device_id, cl_context ctx, VisionStreamType rgb_type, VisionStreamType yuv_type) {
  assert(camera_id < std::size(cameras_supported));
  s->ci = cameras_supported[camera_id];
  assert(s->ci.frame_width != 0);

  s->camera_num = camera_id;
  s->fps = fps;
  s->buf.init(device_id, ctx, s, v, FRAME_BUF_COUNT, rgb_type, yuv_type);
}

/*
void run_camera(CameraState *s, cv::VideoCapture &video_cap, float *ts) {
  assert(video_cap.isOpened());

  cv::Size size(s->ci.frame_width, s->ci.frame_height);
  const cv::Mat transform = cv::Mat(3, 3, CV_32F, ts);
  uint32_t frame_id = 0;
  size_t buf_idx = 0;

  while (!do_exit) {
    cv::Mat frame_mat, transformed_mat;
    video_cap >> frame_mat;
    if (frame_mat.empty()) continue;

    cv::warpPerspective(frame_mat, transformed_mat, transform, size, cv::INTER_LINEAR, cv::BORDER_CONSTANT, 0);

    s->buf.camera_bufs_metadata[buf_idx] = {.frame_id = frame_id};

    auto &buf = s->buf.camera_bufs[buf_idx];
    int transformed_size = transformed_mat.total() * transformed_mat.elemSize();
    CL_CHECK(clEnqueueWriteBuffer(buf.copy_q, buf.buf_cl, CL_TRUE, 0, transformed_size, transformed_mat.data, 0, NULL, NULL));

    s->buf.queue(buf_idx);

    ++frame_id;
    buf_idx = (buf_idx + 1) % FRAME_BUF_COUNT;
  }
}
*/

static void road_camera_thread(CameraState *s) {
  util::set_thread_name("webcam_road_camera_thread");
  std::string pipeline = gstreamer_pipeline(
      0,
      1280,
      720,
      s->fps,
      0,
      FRAME_WIDTH,
      FRAME_HEIGHT);

  cv::VideoCapture cap_road(pipeline, cv::CAP_GSTREAMER); // road

  std::cout << "Using pipeline: \n\t" << pipeline << "\n";

  std::cout << "Road Started: " << cap_road.isOpened() << "\n";

  //float ts[9] = {-0.61904762, 0.0, 909,
 //                 0.0, -0.46523517, 689,
  //                0.0, 0.0, 1.0};

  //float ts[9] = {1.06246351, 0.0, 21.01926445,
  //                0.0, 1.06246351, -137.79276124,
  //               0.0, 0.0, 1.0};

  //float ts[9] = {1.03317725, 0.0, -19.30915697,
  //                0.0, 1.03317725, -14.49845635,
  //               0.0, 0.0, 1.0};
  // if camera upside down:
  // float ts[9] = {-1.50330396, 0.0, 1223.4,
  //                 0.0, -1.50330396, 797.8,
  //                 0.0, 0.0, 1.0};
  float ts[9] = {1.50330396, 0.0, -59.40969163,
                  0.0, 1.50330396, 76.20704846,
                  0.0, 0.0, 1.0};

  assert(cap_road.isOpened());

  cv::Size size(FRAME_WIDTH, FRAME_HEIGHT);
  cv::Mat transform = cv::Mat(3, 3, CV_32F, ts);
  //std::cout << transform << "\n";
  uint32_t frame_id = 0;
  size_t buf_idx = 0;

  cv::Mat frame_mat, transformed_mat;
  //cv::cuda::GpuMat src_gpu, dst_gpu, tf_gpu;
  //tf_gpu.upload(transform);

  while (!do_exit) {
    cap_road.read(frame_mat);

    //src_gpu.upload(frame_mat);

	  //transformed_mat.create(frame_mat.size(), frame_mat.type());
	  //dst_gpu.upload(transformed_mat);

    if (frame_mat.empty()) continue;


    //
   
    //cv::warpPerspective(frame_mat, transformed_mat, transform, cv::Size(FRAME_WIDTH, FRAME_HEIGHT));
      // ... Contents of your main
    transformed_mat = frame_mat;
    
    //cv::cuda::warpPerspective(src_gpu, dst_gpu, tf_gpu, size, cv::INTER_LINEAR , cv::BORDER_CONSTANT, 0.0, cv::cuda::Stream::Null());
    
    //cv::undistort(frame_mat, transformed_mat, transform, distCoeffs)
    //dst_gpu.download(transformed_mat);

    s->buf.camera_bufs_metadata[buf_idx] = {.frame_id = frame_id};

    auto &buf = s->buf.camera_bufs[buf_idx];
    int transformed_size = transformed_mat.total() * transformed_mat.elemSize();
    CL_CHECK(clEnqueueWriteBuffer(buf.copy_q, buf.buf_cl, CL_TRUE, 0, transformed_size, transformed_mat.data, 0, NULL, NULL));

    s->buf.queue(buf_idx);

    ++frame_id;
    buf_idx = (buf_idx + 1) % FRAME_BUF_COUNT;
  }
}


void driver_camera_thread(CameraState *s) {
  std::string pipeline = gstreamer_pipeline(
        1,
        1280,
        720,
        s->fps,
        2,
        FRAME_WIDTH,
        FRAME_HEIGHT);

  cv::VideoCapture cap_driver(pipeline, cv::CAP_GSTREAMER); // road

  std::cout << "Using pipeline: \n\t" << pipeline << "\n";

  std::cout << "Driver Started: " << cap_driver.isOpened() << "\n";

  // transforms calculation see tools/webcam/warp_vis.py
  float ts[9] = {1.42070485, 0.0, -30.16740088,
                  0.0, 1.42070485, 91.030837,
                  0.0, 0.0, 1.0};
  // if camera upside down:
  // float ts[9] = {-1.42070485, 0.0, 1182.2,
  //                 0.0, -1.42070485, 773.0,
  //                 0.0, 0.0, 1.0};
  assert(cap_driver.isOpened());

  cv::Size size(s->ci.frame_width, s->ci.frame_height);
  const cv::Mat transform = cv::Mat(3, 3, CV_32F, ts);
  uint32_t frame_id = 0;
  size_t buf_idx = 0;

  while (!do_exit) {
    cv::Mat frame_mat, transformed_mat;
    cap_driver.read(frame_mat);
    if (frame_mat.empty()) continue;

    //cv::warpPerspective(frame_mat, transformed_mat, transform, size, cv::INTER_LINEAR, cv::BORDER_CONSTANT, 0);
    transformed_mat = frame_mat;

    s->buf.camera_bufs_metadata[buf_idx] = {.frame_id = frame_id};

    auto &buf = s->buf.camera_bufs[buf_idx];
    int transformed_size = transformed_mat.total() * transformed_mat.elemSize();
    CL_CHECK(clEnqueueWriteBuffer(buf.copy_q, buf.buf_cl, CL_TRUE, 0, transformed_size, transformed_mat.data, 0, NULL, NULL));

    s->buf.queue(buf_idx);

    ++frame_id;
    buf_idx = (buf_idx + 1) % FRAME_BUF_COUNT;
  }
}

}  // namespace

void cameras_init(VisionIpcServer *v, MultiCameraState *s, cl_device_id device_id, cl_context ctx) {
  camera_init(v, &s->road_cam, CAMERA_ID_IMX219, 20, device_id, ctx,
              VISION_STREAM_RGB_BACK, VISION_STREAM_ROAD);
  camera_init(v, &s->driver_cam, CAMERA_ID_IMX219, 10, device_id, ctx,
              VISION_STREAM_RGB_FRONT, VISION_STREAM_DRIVER);
  s->pm = new PubMaster({"roadCameraState", "driverCameraState", "thumbnail"});
}

void camera_autoexposure(CameraState *s, float grey_frac) {}

void cameras_open(MultiCameraState *s) {
  // LOG("*** open driver camera ***");
  camera_open(&s->driver_cam, false);
  // LOG("*** open road camera ***");
  camera_open(&s->road_cam, true);
}

void cameras_close(MultiCameraState *s) {
  camera_close(&s->road_cam);
  camera_close(&s->driver_cam);
  delete s->pm;
}

void process_driver_camera(MultiCameraState *s, CameraState *c, int cnt) {
  MessageBuilder msg;
  auto framed = msg.initEvent().initDriverCameraState();
  framed.setFrameType(cereal::FrameData::FrameType::FRONT);
  fill_frame_data(framed, c->buf.cur_frame_data);
  s->pm->send("driverCameraState", msg);
}

void process_road_camera(MultiCameraState *s, CameraState *c, int cnt) {
  const CameraBuf *b = &c->buf;
  MessageBuilder msg;
  auto framed = msg.initEvent().initRoadCameraState();
  fill_frame_data(framed, b->cur_frame_data);
  framed.setImage(kj::arrayPtr((const uint8_t *)b->cur_yuv_buf->addr, b->cur_yuv_buf->len));
  framed.setTransform(b->yuv_transform.v);
  s->pm->send("roadCameraState", msg);
}

void cameras_run(MultiCameraState *s) {
  std::vector<std::thread> threads;
  threads.push_back(start_process_thread(s, &s->road_cam, process_road_camera));
  threads.push_back(start_process_thread(s, &s->driver_cam, process_driver_camera));

  std::thread t_rear = std::thread(road_camera_thread, &s->road_cam);
  util::set_thread_name("webcam_thread");
  driver_camera_thread(&s->driver_cam);

  t_rear.join();

  for (auto &t : threads) t.join();

  cameras_close(s);
}