#include "examples.hpp"
#include "video.hpp"
#include "features.hpp"

using namespace cv;
using namespace std;
using namespace FaceLandmarking;

enum class ProcessType
{
	Video,
	Example,
	Features,
	Autoencoder
};

int main(int argc, char** argv)
{
	const int N = 69;
	const int N_in = 194;

	ProcessType processType = ProcessType::Video;
	string dataPath = "./../Data";
	string videoPath = "./../Data/video-examples/ja6.mp4";
	string mask = "all";
	int steps = 100;

	bool transform = true;
	int transformRotate = cv::ROTATE_90_COUNTERCLOCKWISE;
	int transformWidth = 350;
	int transformHeight = 600;

	int regressionSize = 5;
	bool debug = false;

	for (int i = 1; i < argc;)
	{
		string param(argv[i++]);

		if (param == "-type")
		{
			string type(argv[i++]);

			if (type == "video")
				processType = ProcessType::Video;
			if (type == "example")
				processType = ProcessType::Example;
			if (type == "features")
				processType = ProcessType::Features;
			if (type == "autoencoder")
				processType = ProcessType::Autoencoder;
		}
		if (param == "-data")
			dataPath = argv[i++];
		if (param == "-video")
			videoPath = argv[i++];
		if (param == "-mask")
			mask = argv[i++];
		if (param == "-steps")
			steps = stoi(argv[i++]);
		if (param == "-transform")
			transform = true;
		if (param == "-transform-rotate")
			transformRotate = stoi(argv[i++]);
		if (param == "-transform-width")
			transformWidth = stoi(argv[i++]);
		if (param == "-transform-height")
			transformHeight = stoi(argv[i++]);
		if (param == "-regression-size")
			regressionSize = stoi(argv[i++]);
		if (param == "-debug")
			debug = false;
	}

	switch (processType)
	{
	case ProcessType::Video:
		video_test<N, N_in>(dataPath, videoPath, mask, steps, transform, transformRotate, transformWidth, transformHeight, regressionSize, debug);
		break;
	case ProcessType::Example:
		example_test<N, N_in>(dataPath, mask);
		break;
	case ProcessType::Features:
		features_test<N, N_in>(dataPath);
		break;
	case ProcessType::Autoencoder:
		autoencoder_test<N, N_in>(dataPath);
		break;
	}
}
