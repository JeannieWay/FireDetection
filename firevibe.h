#ifndef FIREVIBE_H
#define FIREVIBE_H

#include <vector>
#include <opencv2/core/core.hpp>

class FireVibe
{
public:
    //! 配置参数, 结构体中的参数的含义参见论文
    struct Config
    {
        //! 获取处理彩色图的参数
        static Config getRGBConfig(void)
        {
            return Config("[rgb]", 20, 60, 2, 16);
        }
        //! 获取处理灰度图的参数
        static Config getGrayConfig(void)
        {
            return Config("[gray]", 20, 20, 2, 16);
        }
        //! 构造函数
        Config(const std::string &label_, int numOfSamples_, int minMatchDist_, int minNumOfMatchCount_, int subSampleInterval_)
            : label(label_), numOfSamples(numOfSamples_), minMatchDist(minMatchDist_),
              minNumOfMatchCount(minNumOfMatchCount_), subSampleInterval(subSampleInterval_)
        {}
        std::string label;       ///< 标签
        int numOfSamples;        ///< 每个像素保存的样本数量-N
        int minMatchDist;        ///< 处理图片的高度
        int minNumOfMatchCount;  ///< 判定为背景的最小匹配成功次数-#min
        int subSampleInterval;   ///< 它的倒数等于更新保存像素值的概率-更新速率
    };
    //! 初始化模型
    /*!
        传入第一帧画面 image, 给定配置参数 config
        image 必须是 CV_8UC1 或者 CV_8UC3 格式, 否则会抛出 std::exception 类型的异常
     */
    void init(const cv::Mat &image, const Config &config);
    //! 提取前景, 更新模型
    /*!
        结合现有模型参数, 检测输入图片 image 中的前景, 输出到 foregroundImage 中
        image 的尺寸和格式必须和 init 函数中进行初始化的图片的尺寸和格式完全相同, 否则会抛出 cv::exception 类型的异常
        foregroundImage 的尺寸和 image 相同, 格式为 CV_8UC1, 前景像素值等于 255, 背景像素值等于 0
     */
    void update(const cv::Mat &image, cv::Mat &foregroundImage);

private:
    int imageWidth;                         ///< 处理图片的宽度
    int imageHeight;                        ///< 处理图片的高度
    int imageChannels;                      ///< 处理图片的通道数, 支持 1 和 3
    int imageType;                          ///< 处理图片的类型, 支持 CV_8UC1 和 CV_8UC3

    cv::Mat samples;                        ///< 背景模型
    std::vector<unsigned char *> rowSamples; ///< 样本的行首地址, 使用 vector 方便管理内存
    unsigned char **ptrSamples;             ///< &rowSamples[0], 使用数组的下标而不是 vector 的 [] 运算符, 加快程序运行速度

    void fill8UC3(const cv::Mat &image);
    void fill8UC1(const cv::Mat &image);
    void proc8UC3(const cv::Mat &image, cv::Mat &foreImage);
    void proc8UC1(const cv::Mat &image, cv::Mat &foreImage);

    int numOfSamples;                       ///< 每个像素保存的样本数量
    int minMatchDist;                       ///< 判定前景背景的距离
    int minNumOfMatchCount;                 ///< 判定为背景的最小匹配成功次数
    int subSampleInterval;

};

#endif // FIREVIBE_H
