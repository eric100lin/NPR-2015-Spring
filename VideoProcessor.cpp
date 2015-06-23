// Yet anther C++ implementation of EVM, based on OpenCV and Qt.
// Copyright (C) 2014  Joseph Pan <cs.wzpan@gmail.com>
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301 USA
//

#include "VideoProcessor.h"

VideoProcessor::VideoProcessor(QObject *parent)
  : QObject(parent)
  , delay(-1)
  , rate(0)
  , fnumber(0)
  , length(0)
  , stop(true)
  , modify(false)
  , curPos(0)
  , curIndex(0)
  , curLevel(0)
  , digits(0)
  , extension(".avi")
  , levels(4)
  , alpha(10)
  , lambda_c(80)
  , fl(0.05)
  , fh(0.4)
  , chromAttenuation(0.1)
  , delta(0)
  , exaggeration_factor(2.0)
  , lambda(0)
  , _loop(false)
{
    connect(this, SIGNAL(revert()), this, SLOT(revertVideo()));
}

/**
 * setDelay	-	 set a delay between each frame
 *
 * 0 means wait at each frame,
 * negative means no delay
 * @param d	-	delay param
 */
void VideoProcessor::setDelay(int d)
{
    delay = d;
}

/**
 * getNumberOfProcessedFrames	-	a count is kept of the processed frames
 *
 *
 * @return the number of processed frames
 */
long VideoProcessor::getNumberOfProcessedFrames()
{
    return fnumber;
}

/**
 * getNumberOfPlayedFrames	-	get the current playing progress
 *
 * @return the number of played frames
 */
long VideoProcessor::getNumberOfPlayedFrames()
{
    return curPos;
}

cv::VideoCapture *VideoProcessor::getClonedCapture()
{
    cv::VideoCapture *cloneCapture = new cv::VideoCapture();
    if (cloneCapture->isOpened()){
        cloneCapture->release();
    }
    if(cloneCapture->open(inputFile)){
        return cloneCapture;
    } else {
        return NULL;
    }
}

/**
 * getFrameSize	-	return the size of the video frame
 *
 *
 * @return the size of the video frame
 */
cv::Size VideoProcessor::getFrameSize()
{
    int w = static_cast<int>(capture.get(CV_CAP_PROP_FRAME_WIDTH));
    int h = static_cast<int>(capture.get(CV_CAP_PROP_FRAME_HEIGHT));

    return cv::Size(w,h);
}

/**
 * getFrameNumber	-	return the frame number of the next frame
 *
 *
 * @return the frame number of the next frame
 */
long VideoProcessor::getFrameNumber()
{
    long f = static_cast<long>(capture.get(CV_CAP_PROP_POS_FRAMES));

    return f;
}

/**
 * getPositionMS	-	return the position in milliseconds
 *
 * @return the position in milliseconds
 */
double VideoProcessor::getPositionMS()
{
    double t = capture.get(CV_CAP_PROP_POS_MSEC);

    return t;
}

/**
 * getFrameRate	-	return the frame rate
 *
 *
 * @return the frame rate
 */
double VideoProcessor::getFrameRate()
{
    double r = capture.get(CV_CAP_PROP_FPS);

    return r;
}

/**
 * getLength	-	return the number of frames in video
 *
 * @return the number of frames
 */
long VideoProcessor::getLength()
{
    return length;
}


/**
 * getLengthMS	-	return the video length in milliseconds
 *
 *
 * @return the length of length in milliseconds
 */
double VideoProcessor::getLengthMS()
{
    double l = 1000.0 * length / rate;
    return l;
}

/**
 * calculateLength	-	recalculate the number of frames in video
 *
 * normally doesn't need it unless getLength()
 * can't return a valid value
 */
void VideoProcessor::calculateLength()
{
    long l = 0;
    cv::Mat img;
    cv::VideoCapture tempCapture(tempFile);
    while(tempCapture.read(img)){
        ++l;
    }
    length = l;
    tempCapture.release();
}

/**
 * concat	-	concat all the frames into a single large Mat
 *              where each column is a reshaped single frame
 *
 * @param frames	-	frames of the video sequence
 * @param dst		-	destinate concatnate image
 */
void VideoProcessor::concat(const std::vector<cv::Mat> &frames,
                            cv::Mat &dst)
{
    cv::Size frameSize = frames.at(0).size();
    cv::Mat temp(frameSize.width*frameSize.height, length-1, CV_32FC3);
    for (int i = 0; i < length-1; ++i) {
        // get a frame if any
        cv::Mat input = frames.at(i);
        // reshape the frame into one column
        cv::Mat reshaped = input.reshape(3, input.cols*input.rows).clone();
        cv::Mat line = temp.col(i);
        // save the reshaped frame to one column of the destinate big image
        reshaped.copyTo(line);
    }
    temp.copyTo(dst);
}

/**
 * deConcat	-	de-concat the concatnate image into frames
 *
 * @param src       -   source concatnate image
 * @param framesize	-	frame size
 * @param frames	-	destinate frames
 */
void VideoProcessor::deConcat(const cv::Mat &src,
                              const cv::Size &frameSize,
                              std::vector<cv::Mat> &frames)
{
    for (int i = 0; i < length-1; ++i) {    // get a line if any
        cv::Mat line = src.col(i).clone();
        cv::Mat reshaped = line.reshape(3, frameSize.height).clone();
        frames.push_back(reshaped);
    }
}

/**
 * createIdealBandpassFilter	-	create a 1D ideal band-pass filter
 *
 * @param filter    -	destinate filter
 * @param fl        -	low cut-off
 * @param fh		-	high cut-off
 * @param rate      -   sampling rate(i.e. video frame rate)
 */
void VideoProcessor::createIdealBandpassFilter(cv::Mat &filter, double fl, double fh, double rate)
{
    int width = filter.cols;
    int height = filter.rows;

    fl = 2 * fl * width / rate;
    fh = 2 * fh * width / rate;

    double response;

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            // filter response
            if (j >= fl && j <= fh)
                response = 1.0f;
            else
                response = 0.0f;
            filter.at<float>(i, j) = response;
        }
    }
}

/**
 * getCodec	-	get the codec of input video
 *
 * @param codec	-	the codec arrays
 *
 * @return the codec integer
 */
int VideoProcessor::getCodec(char codec[])
{
    union {
        int value;
        char code[4]; } returned;

    returned.value = static_cast<int>(capture.get(CV_CAP_PROP_FOURCC));

    codec[0] = returned.code[0];
    codec[1] = returned.code[1];
    codec[2] = returned.code[2];
    codec[3] = returned.code[3];

    return returned.value;
}


/**
 * getTempFile	-	temp file lists
 *
 * @param str	-	the reference of the output string
 */
void VideoProcessor::getTempFile(std::string &str)
{
    if (!tempFileList.empty()){
        str = tempFileList.back();
        tempFileList.pop_back();
    } else {
        str = "";
    }
}

/**
 * getCurTempFile	-	get current temp file
 *
 * @param str	-	the reference of the output string
 */
void VideoProcessor::getCurTempFile(std::string &str)
{
    str = tempFile;
}

/**
 * setInput	-	set the name of the expected video file
 *
 * @param fileName	-	the name of the video file
 *
 * @return True if success. False otherwise
 */
bool VideoProcessor::setInput(const std::string &fileName)
{
    fnumber = 0;
    inputFile = fileName;
    tempFile = fileName;

    // In case a resource was already
    // associated with the VideoCapture instance
    if (isOpened()){
        capture.release();
    }

    // Open the video file
    if(capture.open(fileName)){
        // read parameters
        length = capture.get(CV_CAP_PROP_FRAME_COUNT);
        rate = getFrameRate();
        return true;
    } else {
        return false;
    }
}

/**
 * setOutput	-	set the output video file
 *
 * by default the same parameters than input video will be used
 *
 * @param filename	-	filename prefix
 * @param codec		-	the codec
 * @param framerate -	frame rate
 * @param isColor	-	is the video colorful
 *
 * @return True if successful. False otherwise
 */
bool VideoProcessor::setOutput(const std::string &filename, int codec, double framerate, bool isColor)
{
    outputFile = filename;
    extension.clear();

    if (framerate==0.0)
        framerate = getFrameRate(); // same as input

    char c[4];
    // use same codec as input
    if (codec==0) {
        codec = getCodec(c);
    }

    // Open output video
    return writer.open(outputFile, // filename
                       codec, // codec to be used
                       framerate,      // frame rate of the video
                       getFrameSize(), // frame size
                       isColor);       // color video?
}

/**
 * set the output as a series of image files
 *
 * extension must be ".jpg", ".bmp" ...
 *
 * @param filename	-	filename prefix
 * @param ext		-	image file extension
 * @param numberOfDigits	-	number of digits
 * @param startIndex	-	start index
 *
 * @return True if successful. False otherwise
 */
bool VideoProcessor::setOutput(const std::string &filename, const std::string &ext, int numberOfDigits, int startIndex)
{
    // number of digits must be positive
    if (numberOfDigits<0)
        return false;

    // filenames and their common extension
    outputFile = filename;
    extension = ext;

    // number of digits in the file numbering scheme
    digits = numberOfDigits;
    // start numbering at this index
    curIndex = startIndex;

    return true;
}

/**
 * setTemp	-	set the temp video file
 *
 * by default the same parameters to the input video
 *
 * @param codec	-	video codec
 * @param framerate	-	frame rate
 * @param isColor	-	is the video colorful
 *
 * @return True if successful. False otherwise
 */
bool VideoProcessor::createTemp(double framerate, bool isColor)
{
    std::stringstream ss;
    ss << "temp_" << QDateTime::currentDateTime().toTime_t() << ".avi";
    tempFile = ss.str();

    tempFileList.push_back(tempFile);

    if (framerate==0.0)
        framerate = getFrameRate(); // same as input

    // Open output video
    return tempWriter.open(tempFile, // filename
                       CV_FOURCC('M', 'J', 'P', 'G'), // codec to be used
                       framerate,      // frame rate of the video
                       getFrameSize(), // frame size
                       isColor);       // color video?
}

/**
 * stopIt	-	stop playing or processing
 *
 */
void VideoProcessor::stopIt()
{
    stop = true;
    emit revert();
}

/**
 * prevFrame	-	display the prev frame of the sequence
 *
 */
void VideoProcessor::prevFrame()
{
    if(isStop())
        pauseIt();
    if (curPos >= 0){
        curPos -= 1;
        jumpTo(curPos);
    }
    emit updateProgressBar();
}

/**
 * nextFrame	-	display the next frame of the sequence
 *
 */
void VideoProcessor::nextFrame()
{
    if(isStop())
        pauseIt();
    curPos += 1;
    if (curPos <= length){
        curPos += 1;
        jumpTo(curPos);
    }
    emit updateProgressBar();
}

/**
 * jumpTo	-	Jump to a position
 *
 * @param index	-	frame index
 *
 * @return True if success. False otherwise
 */
bool VideoProcessor::jumpTo(long index)
{
    if (index >= length){
        return 1;
    }

    cv::Mat frame;
    bool re = capture.set(CV_CAP_PROP_POS_FRAMES, index);

    if (re){
        capture.read(frame);
        emit showFrame(index,frame);
    }

    return re;
}


/**
 * jumpToMS	-	jump to a position at a time
 *
 * @param pos	-	time
 *
 * @return True if success. False otherwise
 *
 */
bool VideoProcessor::jumpToMS(double pos)
{
    return capture.set(CV_CAP_PROP_POS_MSEC, pos);
}


/**
 * close	-	close the video
 *
 */
void VideoProcessor::close()
{
    rate = 0;
    length = 0;
    modify = 0;
    capture.release();
    writer.release();
    tempWriter.release();
}


/**
 * isStop	-	Is the processing stop
 *
 *
 * @return True if not processing/playing. False otherwise
 */
bool VideoProcessor::isStop()
{
    return stop;
}

/**
 * isModified	-	Is the video modified?
 *
 *
 * @return True if modified. False otherwise
 */
bool VideoProcessor::isModified()
{
    return modify;
}

/**
 * isOpened	-	Is the player opened?
 *
 *
 * @return True if opened. False otherwise
 */
bool VideoProcessor::isOpened()
{
    return capture.isOpened();
}

/**
 * getNextFrame	-	get the next frame if any
 *
 * @param frame	-	the expected frame
 *
 * @return True if success. False otherwise
 */
bool VideoProcessor::getNextFrame(cv::Mat &frame)
{
    return capture.read(frame);
}

void VideoProcessor::setLoop(bool loop)
{
    _loop = loop;
}

/**
 * writeNextFrame	-	to write the output frame
 *
 * @param frame	-	the frame to be written
 */
void VideoProcessor::writeNextFrame(cv::Mat &frame)
{
    if (extension.length()) { // then we write images

        std::stringstream ss;
        ss << outputFile << std::setfill('0') << std::setw(digits) << curIndex++ << extension;
        cv::imwrite(ss.str(),frame);

    } else { // then write video file

        writer.write(frame);
    }
}

/**
 * playIt	-	play the frames of the sequence
 *
 */
void VideoProcessor::playIt()
{
    // current frame
    cv::Mat input;

    // if no capture device has been set
    if (!isOpened())
        return;

    // is playing
    stop = false;

    // update buttons
    emit updateBtn();

    while (!isStop()) {

        // read next frame if any
        if (!getNextFrame(input))
        {
            if(_loop)
            {
                jumpTo(0);
                curPos = 0;
                continue;
            }
            break;
        }

        curPos = capture.get(CV_CAP_PROP_POS_FRAMES);

        // display input frame
        emit showFrame(curPos,input);

        // update the progress bar
        emit updateProgressBar();

        // introduce a delay
        emit sleep(delay);
    }
    if (!isStop()){
        emit revert();
    }
}

/**
 * pauseIt	-	pause playing
 *
 */
void VideoProcessor::pauseIt()
{
    stop = true;
    emit updateBtn();
}

/**
 * writeOutput	-	write the processed result
 *
 */
void VideoProcessor::writeOutput()
{
    cv::Mat input;

    // if no capture device has been set
    if (!isOpened() || !writer.isOpened())
        return;

    // save the current position
    long pos = curPos;

    // jump to the first frame
    jumpTo(0);

    while (getNextFrame(input)) {

        // write output sequence
        if (outputFile.length()!=0)
            writeNextFrame(input);
    }

    // set the modify flag to false
    modify = false;

    // release the writer
    writer.release();

    // jump back to the original position
    jumpTo(pos);
}

/**
 * revertVideo	-	revert playing
 *
 */
void VideoProcessor::revertVideo()
{
    // pause the video
    jumpTo(0);
    curPos = 0;
    pauseIt();
    emit updateProgressBar();
}
