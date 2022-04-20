#pragma once

#include <aruco/aruco.h>
#include "aruco_nano.h"

std::vector<aruco::Marker> multiPyrDetectNano(const cv::Mat& image, int minPyramid, int maxPyramid);