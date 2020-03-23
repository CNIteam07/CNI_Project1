#include "decoder.h"
int main(char argc,char* argv[])
{
	SearchQrcodeFunctions SearchQrcodeFunction;
	DecodingFunctions Decodefunction;
	string File;//������ʱ�洢�ļ���
	char filename[20];
	int codenum = 0;
	int imnum = 1;//��¼�����ͼƬ����
	int Begin_flag = 0;//0��ʾǰͬ���룬1��ʾ������
	string Path = _DST_PATH;

	//���������зֽ���Ƶ
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
				if (SearchQrcodeFunction.Is_empty(img, qrPoint))//�ж�λ����Ϊ��
				{
					if (Begin_flag == 0)//�ǿ�ʼ��
						continue;//����һ��	
					else//�ǽ�����
						return 0;//�˳�
				}
				else//�ж�λ���Ҳ���
				{
					Begin_flag = 1;//��������
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