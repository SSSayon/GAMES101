//
// Created by LEI XU on 4/27/19.
//

#ifndef RASTERIZER_TEXTURE_H
#define RASTERIZER_TEXTURE_H
#include "global.hpp"
#include <eigen3/Eigen/Eigen>
#include <opencv2/opencv.hpp>
#include <ostream>
class Texture{
private:
    cv::Mat image_data;
    inline float lerp(float x, float y, float t) {
        return x + t * (y - x);
    }

public:
    Texture(const std::string& name)
    {
        image_data = cv::imread(name);
        cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
        width = image_data.cols;
        height = image_data.rows;
    }

    int width, height;

    Eigen::Vector3f getColor(float u, float v)
    {
        auto u_img = u * width;
        auto v_img = (1 - v) * height;
        auto color = image_data.at<cv::Vec3b>(v_img, u_img);
        return Eigen::Vector3f(color[0], color[1], color[2]);
    }

    Eigen::Vector3f getColorBilinear(float u, float v) {
        auto u_img = u * width;
        auto v_img = (1 - v) * height;
        int round_u = std::round(u_img);
        int round_v = std::round(v_img);
        int ceil_u = round_u < width - 1 ? round_u + 1 : round_u;
        int ceil_v = round_v < height - 1 ? round_v + 1 : round_v;

        // TL TR
        // BL BR
        auto BL_color = image_data.at<cv::Vec3b>(round_v, round_u);
        auto BR_color = image_data.at<cv::Vec3b>(round_v, ceil_u);
        auto TL_color = image_data.at<cv::Vec3b>(ceil_v, round_u);
        auto TR_color = image_data.at<cv::Vec3b>(ceil_v, ceil_u); 

        return Eigen::Vector3f(lerp(lerp(static_cast<float>(BL_color[0]), static_cast<float>(TL_color[0]), v_img - round_v), 
                                    lerp(static_cast<float>(BR_color[0]), static_cast<float>(TR_color[0]), v_img - round_v), 
                                    u_img - round_u),
                               lerp(lerp(static_cast<float>(BL_color[1]), static_cast<float>(TL_color[1]), v_img - round_v), 
                                    lerp(static_cast<float>(BR_color[1]), static_cast<float>(TR_color[1]), v_img - round_v), 
                                    u_img - round_u),
                               lerp(lerp(static_cast<float>(BL_color[2]), static_cast<float>(TL_color[2]), v_img - round_v), 
                                    lerp(static_cast<float>(BR_color[2]), static_cast<float>(TR_color[2]), v_img - round_v), 
                                    u_img - round_u));
    }

};
#endif //RASTERIZER_TEXTURE_H
