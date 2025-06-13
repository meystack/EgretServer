#pragma once
//一件套装的属性
typedef struct tagOneSuit
{
	WORD circle;
	WORD level;
	DataList<WORD>		itmList;
	WORD	 ring[2];	 //戒指
	WORD  bangle[2]; //手镯
	DataList<GAMEATTR>  attrs;					   //套装的属性列表
	tagOneSuit()
	{
		memset(this, 0, sizeof(*this));
	}
}ONESUIT,*PONESUIT;

typedef DataList<ONESUIT> SUITCONFIG,*PSUITCONFIG; //套装列表
