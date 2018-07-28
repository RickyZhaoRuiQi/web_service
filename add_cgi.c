#include <stdio.h>
#include <unistd.h>
#include <string.h>

int getargv(char data[],char argv[]);

int main(int argc,char *argv[])
{
	if(argc != 2)
		return 1;
	
	char data[100] = {0};
	strcpy(data,argv[1]);
	int ad1 = getargv(data,"a1=");
	int ad2 = getargv(data,"a2=");
	int result = ad1 + ad2;
	printf("<html>\n<head>\n<meta charset=\"utf-8\">\n<title> 计算器</title>\n</head>\n<body>\n<center>\n网页加法计算器 结果\n<form action=\"\" method=POST name=\"form2\" target=\"_parent\">\n<input type=\"TEXT\" size=10 maxlength=10 value=%d name=\"a1\"> + \n<input type=\"TEXT\" size=10 maxlength=10 value=%d name=\"a2\"> = \n<input type=\"TEXT\" size=10 maxlength=10 value=%d name=\"result\">\n<input type=\"SUBMIT\" value=\"计算\" name=\"calculate result\">\n</form>\n</center>\n</body>\n</html>",ad1,ad2,result);

	return 0;
}


int getargv(char data[],char argv[])
{
	char *tmp = NULL;
	tmp = strstr(data,argv);
	tmp+=strlen(argv);
	char res[10] = {0};
	int i = 0;
	while(*tmp != '&')
	{
		res[i++] = *tmp++;
	}
	return atoi(res);
}
