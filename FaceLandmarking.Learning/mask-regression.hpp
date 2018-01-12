#pragma once

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include <vector>
#include <filesystem>

#include "../FaceLandmarking/mask-transformation/mask-offset.hpp"
#include "../FaceLandmarking.FeatureExtraction/hsv-image.hpp"

namespace FaceLandmarking::Learning
{
	namespace fs = std::experimental::filesystem;

	class MaskRegressionBuffer
	{
	private:
		cv::Mat bufferComputed;
		cv::Mat bufferValues;

		std::vector<Math::Point<int>> setPoints;

	public:
		void reset(int rows, int cols)
		{
			if (rows > bufferValues.rows || cols > bufferValues.cols)
				bufferValues.create(
					std::max(rows, bufferValues.rows),
					std::max(cols, bufferValues.cols),
					CV_64F);
			if (rows > bufferComputed.rows || cols > bufferComputed.cols)
				bufferComputed.create(
					std::max(rows, bufferComputed.rows),
					std::max(cols, bufferComputed.cols),
					CV_8U);

			for (auto& point : setPoints)
				bufferComputed.at<uchar>(point.y, point.x) = 0;
			setPoints.clear();
		}

		bool hasValue(int x, int y)
		{
			return bufferComputed.at<uchar>(y, x);
		}

		Math::Vector<float> getValue(int x, int y)
		{
			return bufferValues.at<Math::Vector<float>>(y, x);
		}

		void setValue(int x, int y, Math::Vector<float> offset)
		{
			bufferValues.at<Math::Vector<float>>(y, x) = offset;
			
			if (!hasValue(x, y))
			{
				bufferComputed.at<uchar>(y, x) = 1;
				setPoints.push_back(Math::Point<int>(x, y));
			}
		}
	};

	template<typename FeatureExtractor, typename Regressor>
	class MaskRegression
	{
	private:
		MaskInfo::MaskDescription maskDescription;
		FeatureExtractor featureExtractor;
		Regressor regressors;

		std::vector<float> features;
		MaskTransformation::MaskOffset maskOffset;

		int cols;
		int rows;
		std::vector<MaskRegressionBuffer> buffers;

	public:
		MaskRegression(MaskInfo::MaskDescription maskDescription, Regressor regressors) :
			maskDescription(maskDescription),
			regressors(regressors),
			buffers(maskDescription.points.size())
		{ }

		void setImage(const FeatureExtraction::HsvImage& image)
		{
			featureExtractor.setImage(image);

			cols = image.columns();
			rows = image.rows();

			for(auto& buffer : buffers)
				buffer.reset(rows, cols);
		}

		void compute(FaceMask& mask, int size = 2)
		{
			maskOffset.clear();
			maskOffset.resize(mask.size());

			for (size_t i = 0; i < mask.size(); i++)
			{
				if (!maskDescription.points[i].inUse)
					continue;

				auto& buffer = buffers[i];

				Math::Vector<float> pointOffset;
				float factor = 0;

				for (int x = -size; x <= size; x++)
				{
					for (int y = -size; y <= size; y++)
					{
						Math::Vector<float> localOffset;

						int xi = std::max(0, std::min((int)mask[i].x + x, cols - 1));
						int yi = std::max(0, std::min((int)mask[i].y + y, rows - 1));
						if (buffer.hasValue(xi, yi))
						{
							localOffset = buffer.getValue(xi, yi);
						}
						else
						{
							features.clear();
							featureExtractor.selectFeatures(xi, yi, features);

							localOffset = regressors.getOffset(i, features);

							buffer.setValue(xi, yi, localOffset);
						}

						float localFactor = 1. / (std::abs(x) + std::abs(y) + 2);
						pointOffset += localOffset * localFactor;
						factor += localFactor;
					}
				}

				pointOffset = pointOffset / factor;

				maskOffset[i] = pointOffset;
			}
		}

		void apply(FaceMask& mask, float factor = 1)
		{
			for (size_t i = 0; i < mask.size(); i++)
				mask[i] += maskOffset[i] * factor;
		}
	};
}