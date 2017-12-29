#pragma once

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>

#include "../FaceLandmarking.Reader/dataset-reader.hpp"
#include "../FaceLandmarking.Reader/mask-io.hpp"
#include "../FaceLandmarking.Reader/validation/image-color-test.hpp"
#include "../FaceLandmarking.Reader/mask-description-io.hpp"
#include "../FaceLandmarking/mask-transformation/mask-normalizer.hpp"
#include "../FaceLandmarking/mask-transformation/mask-fixer.hpp"
#include "../FaceLandmarking.Learning/average-mask-processing.hpp"
#include "../FaceLandmarking.Learning/feature-processing.hpp"
#include "../FaceLandmarking.Learning/mask-regression.hpp"
#include "../FaceLandmarking.Learning/regressors/tree-regressor.hpp"
#include "../FaceLandmarking.Learning/mask-limits-processing.hpp"
#include "../FaceLandmarking.FeatureExtraction/image-feature-extractor.hpp"
#include "../FaceLandmarking.FeatureExtraction/test/FilterApplier.hpp"
#include "../FaceLandmarking.FeatureExtraction/feature-extractor.hpp"
#include "ui/mask-ui.hpp"

using namespace cv;
using namespace std;
using namespace FaceLandmarking;

int example_test()
{
	auto dataPath = experimental::filesystem::path("D:\\Programy\\FaceLandmarking\\Data");

	//Learning::FeatureProcessing featureExtraction(dataPath);
	//featureExtraction.compute();
	//
	//return 0;

	Reader::MaskDescriptionIO maskDescriptionIO(dataPath / "mask" / "mask-description.mask");
	MaskInfo::MaskDescription maskDescription = maskDescriptionIO.load();

	Learning::AverageMaskProcessing averageMaskProcessing(dataPath);
	FaceMask averageMask = averageMaskProcessing.load();

	Learning::MaskLimitsProcessing maskLimitsProcessing(maskDescription, dataPath);
	MaskInfo::MaskLimits maskLimits = maskLimitsProcessing.load();

	std::vector<float> features;
	std::vector<float> decisions;

	FeatureExtraction::ImageFeatureExtractor featureExtractor;
	Learning::Regressors::MaskTreeRegressor treeRegressor(dataPath / "regressors" / "trees");

	Learning::MaskRegression<FeatureExtraction::ImageFeatureExtractor, Learning::Regressors::MaskTreeRegressor> maskRegression(maskDescription, featureExtractor, treeRegressor);
	MaskTransformation::MaskFixer maskFixer(maskDescription, maskLimits);

	namedWindow("example", WINDOW_AUTOSIZE);
	//namedWindow("h", WINDOW_AUTOSIZE);
	//namedWindow("s", WINDOW_AUTOSIZE);
	//namedWindow("v", WINDOW_AUTOSIZE);
	//namedWindow("filter", WINDOW_AUTOSIZE);

	Mat imageWithMasks;

	Reader::DatasetReader reader(dataPath);
	while (true)
	{
		if (reader.hasNext()) {
			auto example = reader.loadNext();
			example.scaleFace(200, 200);
			featureExtractor.setImage(example.image);

			//if (colorTest.isBackAndWhite(example.image))
			//	continue;

			auto normalizedMask = MaskTransformation::MaskNormalizer::normalizeMask(example.mask, Math::Point<float>(50, 50), Math::Size<float>(100, 100));
			auto averageScaledMask = MaskTransformation::MaskNormalizer::normalizeMask(averageMask, example.mask.faceCenter(), example.mask.faceSize());

			auto adjustedMask = averageScaledMask;

			while (true)
			{
				for (int i = 0; i < 1; i++) {
					maskRegression.adjust(adjustedMask);
					maskFixer.fix(adjustedMask);
				}

				example.image.copyTo(imageWithMasks);

				Test::UI::MaskUI::drawMask(imageWithMasks, example.mask, maskDescription);
				Test::UI::MaskUI::drawMask(imageWithMasks, averageScaledMask, maskDescription, cv::Scalar(0, 0, 255));
				Test::UI::MaskUI::drawMask(imageWithMasks, adjustedMask, maskDescription, cv::Scalar(255, 255, 255));
				imshow("example", imageWithMasks);
				//imshow("h", featureExtractor.hsv[0]);
				//imshow("s", featureExtractor.hsv[1]);
				//imshow("v", featureExtractor.hsv[2]);

				//filterApplier.applyFilter(filteredImage, featureSelector.hsv[0], 2);
				//imshow("filter", filteredImage);

				auto key = waitKey(250000);
				if (key == 32)
					continue;
				else if (key == 27)
					return 0;
				else
					break;
			}
		}
	}

	return 0;
}