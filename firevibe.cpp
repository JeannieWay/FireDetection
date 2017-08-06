#include "firevibe.h"
#include <cmath>
#include <fstream>
#include <sstream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
using namespace std;
using namespace cv;

// 用于表示当前位置像素八邻域的所有位置的所在行和所在列相对于当前行和当前列的偏移量
// 举个例子, 假设当前像素所在行和所在列是 y 和 x,
// 那么该位置上方的点使用的偏移量是 adjPositions[1][],
// 即行和列是 y + adjPositions[1][0], x + adjPositions[1][1]
// 其他位置的点一次类推
// 8 个邻域点的排列顺序是从左上角开始按顺时针的方向旋转一周
const static int adjPositions[8][2] = {{ -1, -1}, { -1, 0}, { -1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};
const static int numOfNeighbors = 8;
const static int mask = 65535;

void FireVibe::init(const Mat &image, const Config &config)
{
    CV_Assert(image.cols > 0 && image.rows > 0 && (image.type() == CV_8UC3 || image.type() == CV_8UC1));

    imageWidth = image.cols;
    imageHeight = image.rows;
    imageChannels = image.channels();
    imageType = image.type();

    numOfSamples = config.numOfSamples;
    minMatchDist = config.minMatchDist;
    minNumOfMatchCount = config.minNumOfMatchCount;
    subSampleInterval = config.subSampleInterval;

    // 分配保存样本空间
    samples = Mat::zeros(imageWidth * imageHeight * imageChannels * numOfSamples, 1, CV_8UC1);

    rowSamples.resize(imageHeight, 0);
    // 标记图片每一行首个像素存储样本的地址
    for (int i = 0; i < imageHeight; i++)
        rowSamples[i] = samples.data + imageWidth * numOfSamples * imageChannels * i;
    ptrSamples = &rowSamples[0];

    // 填充背景模型
    if (imageChannels == 3)
        fill8UC3(image);
    else if (imageChannels == 1)
        fill8UC1(image);
}

void FireVibe::fill8UC3(const Mat &image)
{
    // 用 CPU 时钟计数模拟随机数
    int index = getTickCount() & mask;
    // 输入图片的相邻行首地址, 用于快速定位当前行邻域所在行
    const unsigned char *ptrImageAdjRows[3];
    for (int i = 1; i < imageHeight - 1; i++)
    {
        ptrImageAdjRows[0] = image.ptr<unsigned char>(i - 1);
        ptrImageAdjRows[1] = image.ptr<unsigned char>(i);
        ptrImageAdjRows[2] = image.ptr<unsigned char>(i + 1);
        // ptrCenterRow[0] 指向要处理的那一行, 即当前行,
        // ptrCenterRow[-1] 指向当前行的上一行
        // ptrCenterRow[1] 指向当前行的下一行
        const unsigned char **ptrCenterRow = &ptrImageAdjRows[1];
        for (int j = 1; j < imageWidth - 1; j++)
        {
            for (int k = 0; k < numOfSamples; k++)
            {
                // index 值自增后取模, 确保 index 的值落在 0 到 numOfNeighbors - 1 之间
                index = (++index) % numOfNeighbors;
                // 当前像素所在行是 i, 所在列是 j
                // ptrSamples[i] + (j * numOfSamples + k) * 3 指向当前像素第 k 个存储样本的第 0 个字节
                // 第 k 个样本存储下标为 index 的邻域的像素值
                memcpy(ptrSamples[i] + (j * numOfSamples + k) * 3,
                       ptrCenterRow[adjPositions[index][0]] + (j + adjPositions[index][1]) * 3,
                       sizeof(unsigned char) * 3);
            }
        }
    }
}

void FireVibe::fill8UC1(const Mat &image)
{
    int index = getTickCount() & mask;
    const unsigned char *ptrImageAdjRows[3];
    for (int i = 1; i < imageHeight - 1; i++)
    {
        ptrImageAdjRows[0] = image.ptr<unsigned char>(i - 1);
        ptrImageAdjRows[1] = image.ptr<unsigned char>(i);
        ptrImageAdjRows[2] = image.ptr<unsigned char>(i + 1);
        const unsigned char **ptrCenterRow = &ptrImageAdjRows[1];
        for (int j = 1; j < imageWidth - 1; j++)
        {
            for (int k = 0; k < numOfSamples; k++)
            {
                index = (++index) % numOfNeighbors;
                ptrSamples[i][j * numOfSamples + k] =
                    ptrCenterRow[adjPositions[index][0]][j + adjPositions[index][1]];
            }
        }
    }
}

void FireVibe::update(const Mat &image, Mat &foreImage)
{
    CV_Assert(image.type() == imageType && image.cols == imageWidth && image.rows == imageHeight);

    foreImage.create(imageHeight, imageWidth, CV_8UC1);

    // 画面最外面一圈的像素默认为背景
    // 本算法的前景检测和背景模型更新过程涉及到邻域操作
    // 最外面一圈像素的邻域是不完整的, 如果要进行处理, 需要额外的判断
    // 但是在实际的应用中, 画面不会太小, 最外面一圈像素的识别结果不会对最终的结果产生显著的影响
    // 所以直接把这一圈像素忽略掉了
    foreImage.row(0).setTo(0);
    foreImage.row(imageHeight - 1).setTo(0);
    foreImage.col(0).setTo(0);
    foreImage.col(imageWidth - 1).setTo(0);

    // 按照实际的图片类型进行处理
    if (imageChannels == 3)
        proc8UC3(image, foreImage);
    else if (imageChannels == 1)
        proc8UC1(image, foreImage);
}

void FireVibe::proc8UC3(const Mat &image, Mat &foreImage)
{
    // 匹配成功后, 确定是否替换当前位置背景模型的某个样本值
    int rndReplaceCurr = getTickCount() & mask;
    // 要替换的背景模型样本值的下标
    int rndIndexCurr = getTickCount() & mask;
    // 匹配成功后, 确定是否替换当前位置某个邻域的某个样本值
    int rndReplaceAdj = getTickCount() & mask;
    // 要替换的邻域的下标
    int rndPositionAdj = getTickCount() & mask;
    // 要替换的背景模型样本值的下标
    int rndIndexAdj = getTickCount() & mask;

    double avgY = mean(image)[0];//取出亮度均值

    // 首尾两行默认为背景, 不做处理
    for (int i = 1; i < imageHeight - 1; i++)
    {
        const unsigned char *ptrRow = image.ptr<unsigned char>(i);
        unsigned char *ptrFore = foreImage.ptr<unsigned char>(i);
        // 首尾两列默认为背景, 不做处理
        for (int j = 1; j < imageWidth - 1; j++)
        {
            // 统计当前像素能够和多少个已存储的样本匹配
            int matchCount = 0;

            const unsigned char *ptrInput = ptrRow + j * 3;
            unsigned char *ptrCurrSamples = ptrSamples[i] + j * numOfSamples * 3;

            if(int(ptrInput[0]) > avgY+20)
            {


                for (int k = 0; k < numOfSamples && matchCount < minNumOfMatchCount; k++)
                {
                    int dist = abs(int(ptrInput[0]) - int(ptrCurrSamples[k * 3])) +
                               abs(int(ptrInput[1]) - int(ptrCurrSamples[k * 3 + 1])) +
                               abs(int(ptrInput[2]) - int(ptrCurrSamples[k * 3 + 2]));

                    if (dist < minMatchDist)
                        matchCount++;


                }
            }
            else
            {
                matchCount = minNumOfMatchCount;
            }

            // 是前景
            if (matchCount < minNumOfMatchCount)
            {
                ptrFore[j] = 255;
                continue;
            }


            // 是背景
            ptrFore[j] = 0;

            // 更新当前像素的存储样本, 以 1 / subSampleInterval 为概率更新背景模型的某个样本值
            rndReplaceCurr = (++rndReplaceCurr) % subSampleInterval;
            if (rndReplaceCurr == 0)
            {
                // 确定要替换的样本值的下标
                rndIndexCurr = (++rndIndexCurr) % numOfSamples;
                // 赋值
                memcpy(ptrCurrSamples + rndIndexCurr * 3,
                       ptrInput, sizeof(unsigned char) * 3);
            }

            // 更新某个邻域像素的存储样本, , 以 1 / subSampleInterval 为概率更新背景模型的某个样本值
            rndReplaceAdj = (++rndReplaceAdj) % subSampleInterval;
            if (rndReplaceAdj == 0)
            {
                // 确定要替换的邻域的下标
                rndPositionAdj = (++rndPositionAdj) % numOfNeighbors;
                // 确定要替换的样本值的下标
                rndIndexAdj = (++rndIndexAdj) % numOfSamples;
                // 邻域所在行
                int y = i + adjPositions[rndPositionAdj][0];
                // 邻域所在列
                int x = j + adjPositions[rndPositionAdj][1];
                // 赋值
                memcpy(ptrSamples[y] + (x * numOfSamples + rndIndexAdj) * 3,
                       ptrInput, sizeof(unsigned char) * 3);
            }
        }
    }
}

void FireVibe::proc8UC1(const Mat &image, Mat &foreImage)
{
    int rndReplaceCurr = getTickCount() & mask;
    int rndIndexCurr = getTickCount() & mask;
    int rndReplaceAdj = getTickCount() & mask;
    int rndPositionAdj = getTickCount() & mask;
    int rndIndexAdj = getTickCount() & mask;
    for (int i = 1; i < imageHeight - 1; i++)
    {
        const unsigned char *ptrRow = image.ptr<unsigned char>(i);
        unsigned char *ptrFore = foreImage.ptr<unsigned char>(i);
        for (int j = 1; j < imageWidth - 1; j++)
        {
            // 统计当前像素能够和多少个已存储的样本匹配
            int matchCount = 0;
            const unsigned char *ptrInput = ptrRow + j;
            unsigned char *ptrCurrSamples = ptrSamples[i] + j * numOfSamples;
            for (int k = 0; k < numOfSamples && matchCount < minNumOfMatchCount; k++)
            {
                int dist = abs(int(ptrInput[0]) - int(ptrCurrSamples[k]));
                if (dist < minMatchDist)
                    matchCount++;
            }

            // 是前景
            if (matchCount < minNumOfMatchCount)
            {
                ptrFore[j] = 255;
                continue;
            }

            // 是背景
            ptrFore[j] = 0;

            // 更新当前像素的存储样本
            rndReplaceCurr = (++rndReplaceCurr) % subSampleInterval;
            if (rndReplaceCurr == 0)
            {
                rndIndexCurr = (++rndIndexCurr) % numOfSamples;
                ptrCurrSamples[rndIndexCurr] = *ptrInput;
            }

            // 更新邻域像素的存储样本
            rndReplaceAdj = (++rndReplaceAdj) % subSampleInterval;
            if (rndReplaceAdj == 0)
            {
                rndPositionAdj = (++rndPositionAdj) % numOfNeighbors;
                rndIndexAdj = (++rndIndexAdj) % numOfSamples;
                int y = i + adjPositions[rndPositionAdj][0];
                int x = j + adjPositions[rndPositionAdj][1];
                ptrSamples[y][x * numOfSamples + rndIndexAdj] = *ptrInput;
            }
        }
    }
}

