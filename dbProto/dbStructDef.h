/*
db table 嵌套的 struct 定义
*/

#pragma once

#include <string>
#include <vector>
#include <array>
#include "svr_util/include/typedef.h"
#include "dbTable.h"

namespace db
{
	enum class Attr
	{
		Hp,
		Str,
		Dec,
		Intl,
		MinDam,
		MaxDam,

	};
	struct ActorBrief  
	{
		uint64_t uin;
		std::string name;
	};
	struct DbAttr 
	{
		Attr type;
		int32_t v;
	};

	struct DbItemBase
	{
		uint16_t cfgId;
		uint16_t x = 0, y = 0, z = 0;
		uint32 num = 1;
	};

	struct DbItem
	{
		DbItemBase dbItemBase;
		std::string data;//不同类物品的抽象数据。 比如：DbContainer， DbEquipItem
	};

	struct DbContainer //为 DbItem::data内容
	{
		uint8 maxItemNum = 5; //最大物品数
		uint8 maxNestNum = 3; //最大嵌套容器数
		std::vector<DbItem> vecItem;
	};

	//装备，武器等穿戴物
	struct DbEquipItem //为 DbItem::data内容
	{
		std::string name;//独特名
		//其他可变属性
		std::vector<DbAttr> attrs;
	};

	struct DbEquips 
	{
		std::vector<DbItem> vecItem;//穿戴物, idx is Layer type
	};



	struct DbActorAttr
	{
		uint32_t str = 0;
		uint32_t dex = 0;
		uint32_t intl = 0;
		uint32_t hits = 0;;
		uint32_t stam = 0;
		uint32_t mana = 0;
	};

	struct DbActorBase 
	{
		std::string name;
		uint16_t x = 0, y = 0, z = 0;
		uint8 dir=0;
		int body=0;
		bool female = false;
		uint8_t race = 0;
		uint32 sceneId=0;
		int hue=0;
		uint16 faceItemID = 0;
		int16 faceHue = 0;
		uint16 hairItemID = 0;
		int16 hairHue = 0;
		uint16 facialHairItemID = 0;//Beard
		int16 facialHairHue = 0;
	};

	struct DbPlayerBase
	{
		int16_t hairVal = 0;
		int16_t hairHue = 0;
		int16_t hairValf = 0;
		int16_t hairHuef = 0;
	};

	struct DbSkill
	{
		uint32 idx=0;
		uint16 base = 0; //GM 1000
		uint16 cap = 1000; //GM 1000
		uint32 lock = 0;
	};

	struct DbSkills
	{
		std::vector<DbSkill> vecSkill;
	};

	struct DbActor //monster ,npc, playeractor com use
	{
		DbActorBase actorBase;
		DbActorAttr actorAttr;
		DbPlayerBase playerBase;
		DbEquips equips;
		DbItem bag;
		DbSkills skills;
	};


//	
//先定义宏参数信息，以后再具体实现
//域的顺序 和 遗漏，都会初始化的时候检查出来
#define DB_ALL_STRUCT_INFO \
	DB_CLASS_NAME(ActorBrief)\
	DB_FIELD(uin)\
	DB_FIELD(name)\
	DB_CLASS_END\
\
	DB_CLASS_NAME(DbAttr)\
	DB_FIELD(type)\
	DB_FIELD(v)\
	DB_CLASS_END\
\
	DB_CLASS_NAME(DbEquipItem)\
	DB_FIELD(name)\
	DB_FIELD(attrs)\
	DB_CLASS_END\
\
	DB_CLASS_NAME(DbEquips)\
	DB_FIELD(vecItem)\
	DB_CLASS_END\
\
	DB_CLASS_NAME(DbActorAttr)\
	DB_FIELD(str)\
	DB_FIELD(dex)\
	DB_FIELD(intl)\
	DB_FIELD(hits)\
	DB_FIELD(stam)\
	DB_FIELD(mana)\
	DB_CLASS_END\
\
	DB_CLASS_NAME(DbActorBase)\
	DB_FIELD(name)\
	DB_FIELD(x)\
	DB_FIELD(y)\
	DB_FIELD(z)\
	DB_FIELD(dir)\
	DB_FIELD(body)\
	DB_FIELD(female)\
	DB_FIELD(race)\
	DB_FIELD(sceneId)\
	DB_FIELD(hue)\
	DB_FIELD(faceItemID)\
	DB_FIELD(faceHue)\
	DB_FIELD(hairItemID)\
	DB_FIELD(hairHue)\
	DB_FIELD(facialHairItemID)\
	DB_FIELD(facialHairHue)\
	DB_CLASS_END\
\
	DB_CLASS_NAME(DbPlayerBase)\
	DB_FIELD(hairVal)\
	DB_FIELD(hairHue)\
	DB_FIELD(hairValf)\
	DB_FIELD(hairHuef)\
	DB_CLASS_END\
\
	DB_CLASS_NAME(DbItemBase)\
	DB_FIELD(cfgId)\
	DB_FIELD(x)\
	DB_FIELD(y)\
	DB_FIELD(z)\
	DB_FIELD(num)\
	DB_CLASS_END\
\
	DB_CLASS_NAME(DbItem)\
	DB_FIELD(dbItemBase)\
	DB_FIELD(data)\
	DB_CLASS_END\
\
	DB_CLASS_NAME(DbContainer)\
	DB_FIELD(maxItemNum)\
	DB_FIELD(maxNestNum)\
	DB_FIELD(vecItem)\
	DB_CLASS_END\
\
	DB_CLASS_NAME(DbSkill)\
	DB_FIELD(idx)\
	DB_FIELD(base)\
	DB_FIELD(cap)\
	DB_FIELD(lock)\
	DB_CLASS_END\
\
	DB_CLASS_NAME(DbSkills)\
	DB_FIELD(vecSkill)\
	DB_CLASS_END\
\
	DB_CLASS_NAME(DbActor)\
	DB_FIELD(actorBase)\
	DB_FIELD(actorAttr)\
	DB_FIELD(playerBase)\
	DB_FIELD(equips)\
	DB_FIELD(bag)\
	DB_FIELD(skills)\
	DB_CLASS_END\
\





}