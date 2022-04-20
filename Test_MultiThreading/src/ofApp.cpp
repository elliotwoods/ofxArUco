#include "ofApp.h"
#include <future>
#include "aruco_nano.h"
#include "ofxProfiler.h"

//-----------
void
ofApp::setup()
{
	this->gui.init();
	{
		auto widgets = this->gui.addWidgets();
		widgets->addButton("Load images in data folder", [this]() {
			ofxProfiler::Activity::Root().clear();
			this->load();
			cout << ofxProfiler::getResults() << endl;
			}, ' ');
		widgets->addButton("Detect single threaded", [this]() {
			ofxProfiler::Activity::Root().clear();
			for (int i = 0; i < this->parameters.iterations; i++) {
				this->detectSingleThreaded();
			}
			cout << ofxProfiler::getResults() << endl;
			}, '1');
		widgets->addButton("Detect one detector", [this]() {
			ofxProfiler::Activity::Root().clear();
			for (int i = 0; i < this->parameters.iterations; i++) {
				this->detectOneDetector();
			}
			cout << ofxProfiler::getResults() << endl;
			}, '2');
		widgets->addButton("Detect nano", [this]() {
			ofxProfiler::Activity::Root().clear();
			for (int i = 0; i < this->parameters.iterations; i++) {
				this->detectNano();
			}
			cout << ofxProfiler::getResults() << endl;
			}, '3');

		widgets->addButton("Detect copy", [this]() {
			ofxProfiler::Activity::Root().clear();
			for (int i = 0; i < this->parameters.iterations; i++) {
				this->detectCopyDetector();
			}
			cout << ofxProfiler::getResults() << endl;
			}, '4');

		widgets->addButton("Detect multi pyramid nano", [this]() {
			ofxProfiler::Activity::Root().clear();
			for (int i = 0; i < this->parameters.iterations; i++) {
				this->detectMultiPyrNano();
			}
			cout << ofxProfiler::getResults() << endl;
			}, '5');

		widgets->addParameterGroup(this->parameters);
	}

}

//-----------
void ofApp::update()
{

}

//-----------
void ofApp::draw()
{
	
}

//-----------
void ofApp::load(){
	PROFILE_SCOPE("Loading images");

	auto folder = ofDirectory(".");
	auto count = folder.listDir();
	
	for (const auto& file : folder) {
		if (this->images.size() >= this->parameters.maxFiles) {
			break;
		}

		auto image = make_shared<Image>();
		image->filename = file.getAbsolutePath();

		PROFILE_SCOPE("Loading " + image->filename);

		try {
			// Load the image
			{
				PROFILE_SCOPE("Read file");
				image->image = cv::imread(file.getAbsolutePath());
				if (image->image.empty()) {
					continue;
				}
				image->image.convertTo(image->image, CV_8UC1);
				cv::cvtColor(image->image, image->image, cv::COLOR_RGB2GRAY);
				cout << "Loaded : " << image->filename << endl;
			}

			// Upload texture
			{
				PROFILE_SCOPE("Upload texture");
				ofxCv::copy(image->image, image->preview);
				image->preview.update();
			}

			// Setup panel
			{
				image->panel = this->gui.add(image->preview, image->filename);
				auto imagePanel = dynamic_pointer_cast<ofxCvGui::Panels::Image>(image->panel);
				imagePanel->onDrawImage += [image](ofxCvGui::DrawImageArguments&) {
					ofPushStyle();
					{
						ofSetColor(200, 0, 0);
						ofSetLineWidth(3.0f);
						for (const auto& marker : image->markers) {
							ofPolyline line;
							for (const auto& point : marker) {
								line.addVertex(point.x, point.y);
							}
							line.close();
							line.draw();
							ofDrawBitmapStringHighlight(ofToString(marker.id), line.getVertices().front());
						}
					}
					ofPopStyle();
				};
				imagePanel->onDraw += [image](ofxCvGui::DrawArguments&) {
					ofxCvGui::Utils::drawText(ofToString(image->markers.size()) + " markers found", 30, 90);
				};
			}

			this->images.push_back(image);
		}
		catch (...) {

		}
	}
}

//-----------
void ofApp::detectSingleThreaded() {

	PROFILE_SCOPE("Detect single threaded");

	aruco::MarkerDetector detector;
	this->setParams(detector);
	for (auto image : this->images) {
		PROFILE_SCOPE("Detect " + image->filename);
		cout << "Detecting : " << image->filename << endl;
		image->markers = detector.detect(image->image);
		cout << "Found " << image->markers.size() << " markers." << endl;
	}
	this->printTotalMarkersFound();
}

//-----------
void ofApp::detectOneDetector() {
	PROFILE_SCOPE("Detect with one detector");

	aruco::MarkerDetector detector;
	this->setParams(detector);

	vector<std::future<void>> futures;
	for (auto image : this->images) {
		futures.push_back (std::async([image, &detector]() {
			image->markers = detector.detect(image->image);
			}));
	}

	for (auto& future : futures) {
		future.wait();
	}
	this->printTotalMarkersFound();
}

//-----------
void ofApp::detectNano() {
	PROFILE_SCOPE("Detect with nano");
	aruconano::MarkerDetector detector;

	vector<std::future<void>> futures;
	cout << "Detecting";
	for (auto image : this->images) {
		futures.push_back(std::async([image, &detector]() {
			auto & nanoMarkers = detector.detect(image->image);

			image->markers.clear();
			for (auto nanoMarker : nanoMarkers) {
				aruco::Marker marker;
				(vector<cv::Point2f>&) marker = (vector<cv::Point2f>&)nanoMarker;
				image->markers.push_back(marker);
			}
			cout << image->markers.size() << ", ";
			}));
	}

	for (auto& future : futures) {
		future.wait();
	}
	cout << endl;
	this->printTotalMarkersFound();
}

//-----------
void ofApp::detectCopyDetector() {
	PROFILE_SCOPE("Detect with copied detector");

	aruco::MarkerDetector detector;
	this->setParams(detector);
	stringstream settingsStream;
	detector.toStream(settingsStream);

	cout << "Detecting";
	vector<std::future<void>> futures;
	for (auto image : this->images) {
		futures.push_back(std::async(std::launch::async
			, [image, &settingsStream]() {
			aruco::MarkerDetector detectorCopy;
			detectorCopy.fromStream(settingsStream);
			image->markers = detectorCopy.detect(image->image);
			cout << image->markers.size() << ", ";
			}));
	}

	for (auto& future : futures) {
		future.wait();
	}
	cout << endl;
	this->printTotalMarkersFound();
}

//-----------
void ofApp::detectMultiPyrNano() {
	PROFILE_SCOPE("Detect with multi pyramid nano");

	cout << "Detecting";
	vector<std::future<void>> futures;
	for (auto image : this->images) {
		futures.push_back(std::async(std::launch::async
			, [image, this]() {
				image->markers = multiPyrDetectNano(image->image
					, this->parameters.nanoParameters.pyramidRange.min
					, this->parameters.nanoParameters.pyramidRange.max);
				cout << image->markers.size() << ", ";
			}));
	}

	for (auto& future : futures) {
		future.wait();
	}
	cout << endl;
	this->printTotalMarkersFound();
}

//-----------
void ofApp::setParams(aruco::MarkerDetector& detector)
{
	if (!this->parameters.paramsEnabled) {
		return;
	}

	detector.setDetectionMode(aruco::DetectionMode::DM_NORMAL, 0.0f);
	{
		auto& params = detector.getParameters();
		params.AdaptiveThresWindowSize = this->parameters.windowSize;
		params.AdaptiveThresWindowSize_range = this->parameters.windowSizeRange;
		params.ThresHold = this->parameters.threshold;
		params.NAttemptsAutoThresFix = this->parameters.threshold;
		params.maxThreads = this->parameters.numThreadsAruco;
		params.error_correction_rate = 0.0f;
	}
}

//-----------
void
ofApp::printTotalMarkersFound()
{
	size_t count = 0;
	for (const auto& image : this->images) {
		count += image->markers.size();
	}
	cout << count << " markers found in total." << endl;
}