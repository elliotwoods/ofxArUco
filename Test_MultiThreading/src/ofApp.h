#pragma once

#include "ofMain.h"
#include "ofxCvMin.h"
#include "ofxCvGui.h"
#include <aruco/aruco.h>
#include "NanoMulti.h"

struct Image {
	string filename;
	cv::Mat image;
	ofImage preview;
	ofxCvGui::PanelPtr panel;

	vector<aruco::Marker> markers;
};

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		
		void load();
		void detectSingleThreaded();
		void detectOneDetector();
		void detectNano();
		void detectCopyDetector();
		void detectMultiPyrNano();

		void setParams(aruco::MarkerDetector&);
		void printTotalMarkersFound();

		ofxCvGui::Builder gui;
		vector<shared_ptr<Image>> images;

		struct Parameters : ofParameterGroup {
			ofParameter<int> maxFiles{ "Max files", 16 };
			ofParameter<int> iterations{ "Iterations", 1};
			ofParameter<bool> paramsEnabled{ "Params enabled", false };
			ofParameter<int> thresholdAttempts{"Threshold attempts", 3 };
			ofParameter<int> windowSize{ "Window Size", 15 };
			ofParameter<int> windowSizeRange{ "Window Size Range", 30 };
			ofParameter<int> threshold{ "Threshold", 7 };
			ofParameter<int> numThreadsAruco{ "Num threads aruco", 0 };

			struct NanoParameters : ofParameterGroup {
				struct PyramidRange : ofParameterGroup {
					ofParameter<int> min{ "Min", -2 };
					ofParameter<int> max{ "Max", 1 };
					PyramidRange() {
						this->setName("Pyramid range");
						this->add(this->min);
						this->add(this->max);
					}
				} pyramidRange;

				NanoParameters() {
					this->setName("Nano");
					this->add(this->pyramidRange);
				}
			} nanoParameters;

			Parameters() {
				this->setName("Detector");
				this->add(this->maxFiles);
				this->add(this->iterations);
				this->add(this->paramsEnabled);
				this->add(this->thresholdAttempts);
				this->add(this->windowSize);
				this->add(this->windowSizeRange);
				this->add(this->threshold);
				this->add(this->numThreadsAruco);
				this->add(this->nanoParameters);
			}
		} parameters;
};
