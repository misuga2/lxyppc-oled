/*
一个Intel HEX文件可以包含任意多的十六进制记录,每条记录有五个域,下面是一个记录的格式. 

:llaaaatt[dd...]cc 

每一组字母是独立的一域,每一个字母是一个十六进制数字,每一域至少由两个十六进制数字组成,下面是字节的描述. 

:冒号 是每一条Intel HEX记录的开始 

ll 是这条记录的长度域,他表示数据(dd)的字节数目. 

aaaa 是地址域,他表示数据的起始地址 
<如果是数据记录,这表示将要烧录的这条记录中的数据在EPROM中的偏移地址, 
对于不支持扩展段地址和扩展线性地址的,如89C51,这就是此条记录的起始地址> 

tt 这个域表示这条HEX记录的类型,他有可能是下面这几种类型 
00 ----数据记录 
01 ----文件结束记录 
02 ----扩展段地址记录 
04 ----扩展线性地址记录 

dd 是数据域,表示一个字节的数据,一个记录可能有多个数据字节,字节数目可以 
查看ll域的说明 

cc 是效验和域,表示记录的效验和,计算方法是将本条记录冒号开始的所有字母对
<不包括本效验字和冒号> 所表示的十六进制数字 
<一对字母表示一个十六进制数,这样的一个十六进制数为一个字节> 

都加起来然后模除256得到的余数最后求出余数的补码即是本效验字节cc. 
<例如: 
:0300000002005E9D 
cc=0x01+NOT((0x03+0x00+0x00+0x00+0x02+0x00+0x5E)%0x100)=0x01+0x9C=0x9D 

C语言描述: 
UCHAR cc; 
cc=(UCHAR)~(0x03+0x00+0x00+0x00+0x02+0x00+0x5E); 
cc++;
//*/
//一行HEX文件的记录数据结构


#ifndef	HEXFORMAT_H
#define HEXFORMAT_H

const UCHAR	t_Data=0;
const UCHAR	t_EOF=1;
const UCHAR	t_Segement=2;
const UCHAR	t_Linear=4;

class	CHexLine
{
public:
	CHexLine();
	CHexLine(const CHexLine&);
	CHexLine	operator=(CHexLine);
	~CHexLine();
	UCHAR	RecordNumber;
	int	RecordAddress;
	UCHAR	RecordType;
	UCHAR	Records[16];
	UCHAR	SumAll;
private:
};


#endif