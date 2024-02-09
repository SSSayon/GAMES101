#include "opencv2/core/matx.hpp"
#include "opencv2/core/types.hpp"
#include <chrono>
#include <cmath>
#include <complex>
#include <cstddef>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

std::vector<cv::Point2f> control_points;
const int N = 6;

void mouse_handler(int event, int x, int y, int flags, void *userdata) 
{
    if (event == cv::EVENT_LBUTTONDOWN && control_points.size() < N) 
    {
        std::cout << "Left button of the mouse is clicked - position (" << x << ", "
        << y << ")" << '\n';
        control_points.emplace_back(x, y);
    }     
}

void naive_bezier(const std::vector<cv::Point2f> &points, cv::Mat &window) 
{
    auto &p_0 = points[0];
    auto &p_1 = points[1];
    auto &p_2 = points[2];
    auto &p_3 = points[3];

    for (double t = 0.0; t <= 1.0; t += 0.001) 
    {
        auto point = std::pow(1 - t, 3) * p_0 + 3 * t * std::pow(1 - t, 2) * p_1 +
                 3 * std::pow(t, 2) * (1 - t) * p_2 + std::pow(t, 3) * p_3;

        window.at<cv::Vec3b>(point.y, point.x)[2] = 255;
    }
}

cv::Point2f recursive_bezier(const std::vector<cv::Point2f> &control_points, float t) 
{
    // Implement de Casteljau's algorithm
    if (control_points.size() == 1) {
        return control_points[0];
    }

    size_t size = control_points.size() - 1;
    std::vector<cv::Point2f> intermediate_points;
    intermediate_points.reserve(size);

    for (size_t i = 0; i < size; ++i) {
        intermediate_points.push_back(control_points[i] + t * (control_points[i + 1] - control_points[i]));
    }

    return recursive_bezier(intermediate_points, t);
}

inline float dist(float x1, float x2, float y1, float y2) {  // suppose x1 <= x2, y1 <= y2
    return std::max(x2 - x1, y2 - y1);
}

const float TOLERENCE = 0.25f;
inline int lerp_color(float dist) {
    return static_cast<int>(std::min(1.f, (1.f - dist) / (1.f - TOLERENCE)) * 255.f);
}

void bezier(const std::vector<cv::Point2f> &control_points, cv::Mat &window) 
{
    // Iterate through all t = 0 to t = 1 with small steps, and call de Casteljau's 
    // recursive Bezier algorithm.
    for (float t = 0.0; t <= 1.0; t += 0.0001) {
        auto point = recursive_bezier(control_points, t);
        float x = point.x;
        float y = point.y;
        int round_x = round(x);
        int round_y = round(y);

        window.at<cv::Vec3b>(round_y, round_x)[1] = std::max(static_cast<int>(window.at<cv::Vec3b>(round_y, round_x)[1]), 
                                                                    lerp_color(dist(round_x, x, round_y, y)));
        window.at<cv::Vec3b>(round_y + 1, round_x)[1] = std::max(static_cast<int>(window.at<cv::Vec3b>(round_y + 1, round_x)[1]), 
                                                                        lerp_color(dist(round_x, x, y, round_y + 1)));
        window.at<cv::Vec3b>(round_y, round_x + 1)[1] = std::max(static_cast<int>(window.at<cv::Vec3b>(round_y, round_x + 1)[1]), 
                                                                        lerp_color(dist(x, round_x + 1, round_y, y)));
        window.at<cv::Vec3b>(round_y + 1, round_x + 1)[1] = std::max(static_cast<int>(window.at<cv::Vec3b>(round_y + 1, round_x + 1)[1]), 
                                                                            lerp_color(dist(x, round_x + 1, y, round_y + 1)));
    }
}

int main() 
{
    cv::Mat window = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0));
    cv::cvtColor(window, window, cv::COLOR_BGR2RGB);
    cv::namedWindow("Bezier Curve", cv::WINDOW_AUTOSIZE);

    cv::setMouseCallback("Bezier Curve", mouse_handler, nullptr);

    int key = -1;
    while (key != 27) 
    {
        for (auto &point : control_points) 
        {
            cv::circle(window, point, 3, {255, 255, 255}, 3);
        }

        if (control_points.size() == N) 
        {
            // naive_bezier(control_points, window);
            bezier(control_points, window);

            cv::imshow("Bezier Curve", window);
            cv::imwrite("my_bezier_curve.png", window);
            key = cv::waitKey(0);

            return 0;
        }

        cv::imshow("Bezier Curve", window);
        key = cv::waitKey(20);
    }

return 0;
}
