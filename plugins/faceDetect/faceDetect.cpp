/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <opencv2/objdetect.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>

#include "IPluginProcess.h"

#define HAAR_FACE PLUGIN_MODEL_DIR"/haarcascade_frontalface_alt2.xml"
#define HAAR_EYES PLUGIN_MODEL_DIR"/haarcascade_eye.xml"

using namespace cv;
using namespace ovi;

class FaceDetect : public IPluginProcess
{
public:
	FaceDetect();
	~FaceDetect() = default;

	int setAttrs(const std::map<std::string, std::string>& attrs) override;
	Outcome process(ovi::FramePack *frame) override;

private:
	Mat toGray(Mat image, VideoFormat format);

	CascadeClassifier _faceCascade;
	CascadeClassifier _eyeCascade;

	bool _inverse {};
	double _scale { 1.5 };
	int _minNeighbors { 5 };
};

FaceDetect::FaceDetect()
{
	_faceCascade.load(HAAR_FACE);
	if (_faceCascade.empty())
		throw ovi::Exception(OVI_ERROR_INVALID_OPERATION, "face model load failed.");

	_eyeCascade.load(HAAR_EYES);
	if (_eyeCascade.empty())
		throw ovi::Exception(OVI_ERROR_INVALID_OPERATION, "eyes model load failed.");
}

int FaceDetect::setAttrs(const std::map<std::string, std::string>& attrs)
{
	if (attrs.find("inverse") != attrs.end())
		_inverse = (attrs.at("inverse") == "1");

	if (attrs.find("scale") != attrs.end())
		_scale = std::stod(attrs.at("scale"));

	if (attrs.find("minNeighbors") != attrs.end())
		_minNeighbors = std::stoi(attrs.at("minNeighbors"));

	return OVI_ERROR_NONE;
}

Mat FaceDetect::toGray(Mat image, VideoFormat format)
{
	Mat gray;
	switch(format) {
	case VIDEO_FORMAT_RGB24:
		cvtColor(image, gray, CV_RGB2GRAY);
		break;
	case VIDEO_FORMAT_BGR24:
		cvtColor(image, gray, CV_BGR2GRAY);
		break;
	case VIDEO_FORMAT_GRAY8:
		gray = image;
		break;
	default:
		throw ovi::Exception(OVI_ERROR_INVALID_OPERATION, "not supported format");
	}
	equalizeHist(gray, gray);
	return gray;
}

Outcome FaceDetect::process(ovi::FramePack *frame)
{
	auto vFrame = dynamic_cast<ovi::VideoFramePack *>(frame);
	assert(vFrame);

	const auto [width, height, format] = vFrame->videoProperties();
	Mat image;
	Outcome ret = { false, {} };

	switch(format) {
	case VIDEO_FORMAT_RGB24:
	case VIDEO_FORMAT_BGR24:
		image = Mat(Size(width, height), CV_8UC3, const_cast<void*>(vFrame->data()));
		break;
	case VIDEO_FORMAT_GRAY8:
		image = Mat(Size(width, height), CV_8UC1, const_cast<void*>(vFrame->data()));
		break;
	default:
		throw ovi::Exception(OVI_ERROR_INVALID_OPERATION, "not supported format");
	}

	if (!image.data)
		throw ovi::Exception(OVI_ERROR_INVALID_OPERATION, "image.data is empty");

	Mat gray = toGray(image, format);

	std::vector<Rect> faces;
	_faceCascade.detectMultiScale(gray, faces, _scale, _minNeighbors, 0, Size(30, 30));

	if (faces.empty()) {
		ret.detect = (_inverse) ? true : false;
		return ret;
	} else if (_inverse) {
		return ret;
	}

	for (size_t i = 0; i < faces.size(); ++i) {
		std::vector<Rect> eyes;
		_eyeCascade.detectMultiScale(gray(faces[i]), eyes);
		if (eyes.size() > 0 && eyes.size() < 3) {
			ret.detect = true;
			OVIRect r = {
				static_cast<double>(faces[i].x),
				static_cast<double>(faces[i].y),
				static_cast<double>(faces[i].width),
				static_cast<double>(faces[i].height)
			};
			ret.list.push_back(r);
		}
	}

	return ret;
}

extern "C" class IPlugin *createPlugin(void)
{
	return new FaceDetect();
}

extern "C" void destroyPlugin(class IPlugin *plugin)
{
	delete plugin;
}

extern "C" const char *name()
{
	return "FaceDetect";
}

extern "C" PluginType type()
{
	return PLUGIN_TYPE_VIDEO_DETECT;
}

extern "C" void *supportFormat()
{
	static std::vector<int> formats {
		VIDEO_FORMAT_GRAY8, VIDEO_FORMAT_RGB24, VIDEO_FORMAT_BGR24
	};

	return &formats;
}

extern "C" MetaForm supportMetaForm()
{
	return METAFORM_RECT;
}

extern "C" const char *description()
{
	return "Detecting faces";
}

extern "C" void *attributeList()
{
	static std::vector<Attribute> attrs {
		{ "inverse", "0 or 1", "Faceless detect" },
		{ "scale", "double", "Parameter specifying how much the image size is reduced at each image scale." },
		{ "minNeighbors", "int", "Parameter specifying how many neighbors each candidate rectangle should have to retain it." },
	};
	return &attrs;
}
