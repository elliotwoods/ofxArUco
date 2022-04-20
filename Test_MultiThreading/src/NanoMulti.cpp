#include "NanoMulti.h"

void
scaleResults(std::vector<aruconano::Marker>& results, float scale)
{
	for (auto& marker : results) {
		for (auto& corner : marker) {
			corner.x *= scale;
			corner.y *= scale;
		}
	}
}

std::vector<aruconano::Marker>
mergeResults(const std::vector<aruconano::Marker> & highPriority, const std::vector<aruconano::Marker>& lowPriority)
{
	auto merged = highPriority;
	for (const auto& lowPriorityMarker : lowPriority) {
		// first check if it already exists in set
		bool alreadyExists = false;
		for (const auto& priorResult : merged) {
			if (priorResult.id == lowPriorityMarker.id) {
				alreadyExists = true;
				break;
			}
		}

		if (!alreadyExists) {
			merged.push_back(lowPriorityMarker);
		}
	}
	return merged;
}

std::vector<aruco::Marker>
multiPyrDetectNano(const cv::Mat& image, int minPyramid, int maxPyramid)
{
	aruconano::MarkerDetector markerDetector;
	// Start with result from base pyramid
	auto results = markerDetector.detect(image);

	// Go down the pyramid
	{
		cv::Mat pyramidImage = image;

		for (int i = -1; i >= minPyramid; i--) {
			cv::pyrDown(pyramidImage, pyramidImage);
			auto subResults = markerDetector.detect(pyramidImage);
			scaleResults(subResults, pow(2, -i));
			results = mergeResults(results, subResults);
		}
	}
	

	// Go up the pyramid
	{
		cv::Mat pyramidImage = image;

		for (int i = 1; i <= maxPyramid; i++) {
			cv::pyrUp(pyramidImage, pyramidImage);
			auto subResults = markerDetector.detect(pyramidImage);
			scaleResults(subResults, pow(2, -i));
			results = mergeResults(subResults, results); // prioritise the higher res find
		}
	}

	// return merged results
	std::vector<aruco::Marker> resultsTyped;
	for (const auto& result : results) {
		resultsTyped.push_back(aruco::Marker());
		(std::vector<cv::Point2f>&)resultsTyped.back() = (std::vector<cv::Point2f>&)result;
		resultsTyped.back().id = result.id;
	}
	return resultsTyped;
}