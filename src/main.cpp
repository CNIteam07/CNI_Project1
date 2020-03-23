#include "decoder.h"
int main(char argc,char* argv[])
{
	SearchQrcodeFunctions SearchQrcodeFunction;
	DecodingFunctions Decodefunction;
	string File;//用于临时存储文件名
	char filename[20];
	int codenum = 0;
	int imnum = 1;//记录解码的图片张数
	int Begin_flag = 0;//0表示前同步码，1表示结束码
	string Path = _DST_PATH;

	//调用命令行分解视频
	char command[200];
	string str;
	string first_str = "ffmpeg -i ";
	string videoname(argv[1]);
	string last_str = " -r 30 x%d.png";
	str = first_str + videoname+last_str;
	sprintf_s(command, "%s", str.c_str());
	system(command);
	
	while (1)
	{
		int* Code = new int[5000];
		sprintf_s(filename, 20, "/x%d.png", imnum++);
		File = (string)_DST_PATH+filename;
		fstream inFile(File);
		if (inFile.good())
		{
			Mat img = imread(File);

			vector<vector<Point>> qrPoint;
			if (SearchQrcodeFunction.FindQrPoint(img, qrPoint))
			{
				if (SearchQrcodeFunction.Is_empty(img, qrPoint))//有定位码且为空
				{
					if (Begin_flag == 0)//是开始码
						continue;//读下一张	
					else//是结束码
						return 0;//退出
				}
				else//有定位码且不空
				{
					Begin_flag = 1;//结束码标记
					imnum--;
					Decodefunction.Decode_new(Code,imnum,Path,argv[2],argv[3]);
				}
			}

		}
		else
			break;

		delete[]Code;
	}

	waitKey(0);
	return 0;
}