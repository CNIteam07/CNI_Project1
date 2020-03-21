#include<opencv2/opencv.hpp>
#include<iostream>

using namespace std;
using namespace cv;

#define _DST_P "e:/try/x41.png"
class userdata {
public:
	vector<Point2f> points;
};



int main()
{
	Mat Image = imread(_DST_P);
	imshow("Image", Image);
	Mat result = Mat::zeros(400, 400, CV_8UC3);

	vector<Point2f> obj;
	obj.push_back(Point2f(0, 0));
	obj.push_back(Point2f(400, 0));
	obj.push_back(Point2f(400, 400));
	obj.push_back(Point2f(0, 400));

	Mat dst = Image.clone();

	userdata data;
	data.points.push_back(Point2f(100, 100));//此四个点应该修改为二维码最外层轮廓四个顶点，顺序为顺时针
	data.points.push_back(Point2f(100, 800));
	data.points.push_back(Point2f(800, 100));
	data.points.push_back(Point2f(800, 800));


	Mat H = findHomography(data.points, obj, RANSAC);;
	warpPerspective(Image, result, H, result.size());
	namedWindow("result", 0);
	imshow("result", result);
	imwrite("e:/result.png", result);
	waitKey(0);
}
