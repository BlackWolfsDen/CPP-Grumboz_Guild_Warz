/*
-- **g****************************************s***
-- ********© Grumbo'z Guild Warz SystemT ©********
-- ********** Brought to you by Grumbo  *******l**
-- **r*************** slp13at420 ****p************
-- ********************** of *********************
-- ****************  Emudevs.com  **********1*****
-- **u******************?*********3***************
-- *********************?*************************
-- ************ This is A C++ SCRIPT **a**********
-- ***m***********  This is For  *****************
-- ************** TRINITY Core ONLY *t************
-- *?*******************4***********************?*
-- *b* Please Do Not Rem©ve any of the credits ***
-- **** and/or attempt to release as your own **2*
-- ***o******************©*************0**********
-- **************** First Public *****************
-- ********** release date 03-10-2013 ************
-- ***********************************************
Grumbo'z Guild Warz
my fucked up idea of a Guild plot system.
Guild's can own multiple locations and take locations from other Guild's via PvP.
Scripted By Slp13at420 of EmuDevs.com
Trinity Core2 WotlK C++ version.
*/
// !! On to the Madness Now !!

#include "chat.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "Common.h"
#include "Config.h"
#include <cstring>
#include "DatabaseEnv.h"
#include "GameObject.h"
#include "GameObjectAI.h"
#include "GameTime.h"
#include "GossipDef.h"
#include <GrumbozGuildWarz\Grumboz_Guild_Warz.h>
#include "Guild.h"
#include "GuildMgr.h"
#include "item.h"
#include "Language.h"
#include "Log.h"
#include "MapManager.h"
// #include "ObjectAccessor.h"
// #include "ObjectDefines.h"
// #include "Opcodes.h"
#include "ObjectMgr.h"
#include "player.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include "World.h"
#include "WorldSession.h"

std::string SERVER_GUILD_NAME = "SERVER";
std::string GUILD_RANK_COMMAND = "rank";
std::string EVIL_DOER = "!Evil Do`er!";

int GGW_MSG = -1;

bool test = false; // test mode true/false . this will cause the core to belch out data as events happen for help with bug fixes.

uint64 seconds = 1000;
uint64 minutes = 60000;
uint64 hours = 3600000;

// db address's in string value for sql updates
// std::string Guild_Warz_DB = sConfigMgr->GetStringDefault("GUILD_WARZ.DB_ADDRESS", "guild_warz_335");

//std::string commands_db = Guild_Warz_DB + ".commands";
//std::string help_db = Guild_Warz_DB + ".help";
//std::string zones_db = Guild_Warz_DB + ".zones";
//std::string ranking_db = Guild_Warz_DB + ".ranking";

GGW::GGW() { }

GGW::~GGW()
{
	for (std::unordered_map<uint32, Commands>::iterator itr = GWCOMM.begin(); itr != GWCOMM.end(); ++itr)
		delete &itr->second;
	for (std::unordered_map<uint32, Help>::iterator itr = GWHELP.begin(); itr != GWHELP.end(); ++itr)
		delete &itr->second;
	for (std::unordered_map<uint32, LocData>::iterator itr = GWARZ.begin(); itr != GWARZ.end(); ++itr)
		delete &itr->second;
	for (std::unordered_map<uint32, GGW_CreatureData>::iterator itr = GGW_Creature.begin(); itr != GGW_Creature.end(); ++itr)
		delete &itr->second;
	for (std::map<uint32, rank_info>::iterator itr = GW_Ranks.begin(); itr != GW_Ranks.end(); ++itr)
		delete &itr->second;
	for (std::map<uint32, uint32>::iterator itr = GW_RANKS.begin(); itr != GW_RANKS.end(); ++itr)
		delete &itr->second;

	GWCOMM.clear();
	GWHELP.clear();
	GWARZ.clear();
	GGW_Creature.clear();
	GW_Ranks.clear();
	GW_RANKS.clear();
}

GGW* GGW::instance()
{
	static GGW instance;
	return &instance;
}

uint64 GGW::GGW_ConvertStringToNumber(std::string arg)
{
	uint64 Value64;

	std::istringstream(arg) >> Value64;

	return Value64;
}

std::string GGW::ConvertNumberToString(uint64 numberX)
{
	auto number = numberX;
	std::stringstream convert;
	std::string number32_to_string;
	convert << number;
	number32_to_string = convert.str();

	return number32_to_string;
};

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems)
{
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

std::vector<std::string> split(const std::string &s, char delim)
{
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}

bool GGW::LoadCommands()
{
	QueryResult ComQry = WorldDatabase.Query(("SELECT * FROM " + Guild_Warz_DB + ".commands;").c_str());

	if (ComQry)
	{
		do
		{
			Field *fields = ComQry->Fetch();
			// Save the DB values to the Commands object
			std::string guild = fields[0].GetString();
			uint32 guild_id = fields[1].GetUInt32();
			uint32 team = fields[2].GetUInt32();
			std::string commands = fields[3].GetString();
			std::string info_loc = fields[4].GetString();
			std::string list_loc = fields[5].GetString();
			std::string tele = fields[6].GetString();
			std::string version = fields[7].GetString();
			uint8 GLD_lvlb = fields[8].GetUInt8();
			uint8 GLD_lvls = fields[9].GetUInt8();
			std::string respawn_flag = fields[10].GetString();
			std::string details_loc = fields[11].GetString();
			std::string table = fields[12].GetString();
			uint8 GM_admin = fields[13].GetUInt8();
			uint8 GM_minimum = fields[14].GetUInt8();
			uint64 currency = fields[15].GetUInt64();
			uint64 pig_payz = fields[16].GetUInt64();
			uint64 pig_payz_timer = fields[17].GetUInt64();
			uint8 gift_count = fields[18].GetUInt8();
			uint8 flag_require = fields[19].GetUInt8();
			std::string Server = fields[20].GetString();
			std::string command_set = fields[21].GetString();
			uint8 anarchy = fields[22].GetUInt8();
			uint8 f_timer = fields[23].GetUInt8();
			uint64 s_timer = fields[24].GetUInt64();
			uint8 guild_invite = fields[25].GetBool();
			std::string loc = fields[26].GetString();
			uint32 loc_cost = fields[27].GetUInt32();
			uint64 flag_id = fields[28].GetUInt64();
			std::string farm = fields[29].GetString();
			uint32 farm_cost = fields[30].GetUInt32();
			uint32 farm_L = fields[31].GetUInt32();
			uint64 farm_id = fields[32].GetUInt64();
			std::string barrack = fields[33].GetString();
			uint32 barrack_cost = fields[34].GetUInt32();
			uint32 barrack_L = fields[35].GetUInt32();
			uint64 barrack_id = fields[36].GetUInt64();
			std::string hall = fields[37].GetString();
			uint32 hall_cost = fields[38].GetUInt32();
			uint32 hall_L = fields[39].GetUInt32();
			uint64 hall_id = fields[40].GetUInt64();
			std::string pig = fields[41].GetString();
			uint32 pig_cost = fields[42].GetUInt32();
			uint32 pig_L = fields[43].GetUInt32();
			uint64 pig_id = fields[44].GetUInt64();
			std::string guard = fields[45].GetString();
			uint32 guard_cost = fields[46].GetUInt32();
			uint32 guard_L = fields[47].GetUInt32();
			uint64 guard_id = fields[48].GetUInt64();
			std::string vendor1 = fields[49].GetString();
			uint32 vendor1_cost = fields[50].GetUInt32();
			uint32 vendor1_L = fields[51].GetUInt32();
			uint64 vendor1_id = fields[52].GetUInt64();
			std::string vendor2 = fields[53].GetString();
			uint32 vendor2_cost = fields[54].GetUInt32();
			uint32 vendor2_L = fields[55].GetUInt32();
			uint64 vendor2_id = fields[56].GetUInt64();
			std::string vendor3 = fields[57].GetString();
			uint32 vendor3_cost = fields[58].GetUInt32();
			uint32 vendor3_L = fields[59].GetUInt32();
			uint64 vendor3_id = fields[60].GetUInt64();
			std::string cannon = fields[61].GetString();
			uint32 cannon_cost = fields[62].GetUInt32();
			uint32 cannon_L = fields[63].GetUInt32();
			uint64 cannon_id = fields[64].GetUInt64();
			std::string vault = fields[65].GetString();
			uint16 vault_cost = fields[66].GetUInt32();
			uint16 vault_L = fields[67].GetUInt32();
			uint64 vault_id = fields[68].GetUInt64();
			std::string mailbox = fields[69].GetString();
			uint32 mailbox_cost = fields[70].GetUInt32();
			uint32 mailbox_L = fields[71].GetUInt32();
			uint64 mailbox_id = fields[72].GetUInt64();
			std::string setup = fields[73].GetString();
			uint8 allowed = fields[74].GetUInt8();
			std::string color_1 = fields[75].GetString();
			std::string color_2 = fields[76].GetString();
			std::string color_3 = fields[77].GetString();
			std::string color_4 = fields[78].GetString();
			std::string color_5 = fields[79].GetString();
			std::string color_6 = fields[80].GetString();
			std::string color_7 = fields[81].GetString();
			std::string color_8 = fields[82].GetString();
			std::string color_9 = fields[83].GetString();
			std::string color_10 = fields[84].GetString();
			std::string color_11 = fields[85].GetString();
			std::string color_12 = fields[86].GetString();
			std::string color_13 = fields[87].GetString();
			std::string color_14 = fields[88].GetString();
			std::string color_15 = fields[89].GetString();

			Commands& data = sGGW->GWCOMM[guild_id]; // like Lua table GWARZ[guild_id].entry
			data.guild = guild;
			data.guild_id = guild_id;
			data.team = team;
			data.commands = commands;
			data.info_loc = info_loc;
			data.list_loc = list_loc;
			data.tele = tele;
			data.version = version;
			data.GLD_lvlb = GLD_lvlb;
			data.GLD_lvls = GLD_lvls;
			data.respawn_flag = respawn_flag;
			data.details_loc = details_loc;
			data.table = table;
			data.GM_admin = GM_admin;
			data.GM_minimum = GM_minimum;
			data.currency = currency;
			data.pig_payz = pig_payz;
			data.pig_payz_timer = pig_payz_timer;
			data.gift_count = gift_count;
			data.flag_require = flag_require;
			data.Server = Server;
			data.command_set = command_set;
			data.anarchy = anarchy;
			data.f_timer = f_timer;
			data.s_timer = s_timer;
			data.guild_invite = guild_invite;
			data.loc = loc;
			data.loc_cost = loc_cost;
			data.flag_id = flag_id;
			data.farm = farm;
			data.farm_cost = farm_cost;
			data.farm_L = farm_L;
			data.farm_id = farm_id;
			data.barrack = barrack;
			data.barrack_cost = barrack_cost;
			data.barrack_L = barrack_L;
			data.barrack_id = barrack_id;
			data.hall = hall;
			data.hall_cost = hall_cost;
			data.hall_L = hall_L;
			data.hall_id = hall_id;
			data.pig = pig;
			data.pig_cost = pig_cost;
			data.pig_L = pig_L;
			data.pig_id = pig_id;
			data.guard = guard;
			data.guard_cost = guard_cost;
			data.guard_L = guard_L;
			data.guard_id = guard_id;
			data.vendor1 = vendor1;
			data.vendor1_cost = vendor1_cost;
			data.vendor1_L = vendor1_L;
			data.vendor1_id = vendor1_id;
			data.vendor2 = vendor2;
			data.vendor2_cost = vendor2_cost;
			data.vendor2_L = vendor2_L;
			data.vendor2_id = vendor2_id;
			data.vendor3 = vendor3;
			data.vendor3_cost = vendor3_cost;
			data.vendor3_L = vendor3_L;
			data.vendor3_id = vendor3_id;
			data.cannon = cannon;
			data.cannon_cost = cannon_cost;
			data.cannon_L = cannon_L;
			data.cannon_id = cannon_id;
			data.vault = vault;
			data.vault_cost = vault_cost;
			data.vault_L = vault_L;
			data.vault_id = vault_id;
			data.mailbox = mailbox;
			data.mailbox_cost = mailbox_cost;
			data.mailbox_L = mailbox_L;
			data.mailbox_id = mailbox_id;
			data.setup = setup;
			data.allowed = allowed;
			data.color_1 = color_1;
			data.color_2 = color_2;
			data.color_3 = color_3;
			data.color_4 = color_4;
			data.color_5 = color_5;
			data.color_6 = color_6;
			data.color_7 = color_7;
			data.color_8 = color_8;
			data.color_9 = color_9;
			data.color_10 = color_10;
			data.color_11 = color_11;
			data.color_12 = color_12;
			data.color_13 = color_13;
			data.color_14 = color_14;
			data.color_15 = color_15;

		} while (ComQry->NextRow());

		return true;
	}
	return false;
};

bool GGW::LoadHelp()
{
	QueryResult HlpQry = WorldDatabase.Query(("SELECT * FROM " + Guild_Warz_DB + ".help;").c_str());

	if (HlpQry)
	{
		do
		{
			Field *fields = HlpQry->Fetch();
			// Save the DB values to the Help object
			uint32 entry = fields[0].GetUInt32();
			std::string name = fields[1].GetString();
			std::string description = fields[2].GetString();
			std::string example = fields[3].GetString();
			uint8 command_level = fields[4].GetUInt8();

			Help& data = sGGW->GWHELP[entry]; // like Lua table GWHELP[entry].entry
			data.entry = entry;
			data.name = name;
			data.description = description;
			data.example = example;
			data.command_level = command_level;
		} while (HlpQry->NextRow());

		return true;
	}
	return false;
};

bool GGW::LoadLoc()
{
	QueryResult LocQry = WorldDatabase.Query(("SELECT * FROM " + Guild_Warz_DB + ".zones").c_str());

	if (LocQry)
	{
		do
		{
			Field *fields = LocQry->Fetch();
			// Save the DB values to the LocData object
			uint32 entry = fields[0].GetUInt32();
			uint32 map_id = fields[1].GetUInt32();
			uint32 area_id = fields[2].GetUInt32();
			uint32 zone_id = fields[3].GetUInt32();
			std::string guild_name = fields[4].GetString();
			uint8 team = fields[5].GetUInt8();
			float x = fields[6].GetFloat();
			float y = fields[7].GetFloat();
			float z = fields[8].GetFloat();
			uint64 farm_count = fields[9].GetUInt64();
			uint64 barrack_count = fields[10].GetUInt64();
			uint64 hall_count = fields[11].GetUInt64();
			uint64 pig_count = fields[12].GetUInt64();
			uint64 guard_count = fields[13].GetUInt64();
			uint64 vendor1_count = fields[14].GetUInt64();
			uint64 vendor2_count = fields[15].GetUInt64();
			uint64 vendor3_count = fields[16].GetUInt64();
			uint64 cannon_count = fields[17].GetUInt64();
			uint16 vault_count = fields[18].GetUInt64();
			uint64 mailbox_count = fields[19].GetUInt64();
			uint64 flag_id = fields[20].GetUInt64();
			uint64 fs_time = fields[21].GetUInt64();
			uint32 guild_id = fields[22].GetUInt32();

			LocData& data = sGGW->GWARZ[entry]; // like Lua table GWARZ[guild_id].entry
			data.entry = entry;
			data.map_id = map_id;
			data.area_id = area_id;
			data.zone_id = zone_id;
			data.guild_name = guild_name;
			data.team = team;
			data.x = x;
			data.y = y;
			data.z = z;
			data.farm_count = farm_count;
			data.barrack_count = barrack_count;
			data.hall_count = hall_count;
			data.pig_count = pig_count;
			data.guard_count = guard_count;
			data.vendor1_count = vendor1_count;
			data.vendor2_count = vendor2_count;
			data.vendor3_count = vendor3_count;
			data.cannon_count = cannon_count;
			data.vault_count = vault_count;
			data.mailbox_count = mailbox_count;
			data.flag_id = flag_id;
			data.fs_time = fs_time;
			data.guild_id = guild_id;

		} while (LocQry->NextRow());

		return true;
	}
	return false;
};

uint32 GGW::GetTotalPigs(uint32 guild_id)
{
	uint32 loc_id = 0;
	uint32 pig_total = 0;

	for (loc_id = 1; loc_id < GWARZ.size(); loc_id++)
	{
		if (GWARZ[loc_id].guild_id == guild_id) { pig_total = pig_total + GWARZ[loc_id].pig_count; };
	}
	return pig_total;
};

uint32 GGW::CalculateTotalLocations(uint32 guild_id)
{
	uint32 loc_id = 0;
	uint32 loc_total = 0;

	for (loc_id = 1; loc_id < GWARZ.size(); loc_id++)
	{
		if (GWARZ[loc_id].guild_id == guild_id) { loc_total = loc_total + 1; };
	}
	return loc_total;
};

uint32 GGW::CalculateLocationValue(uint32 loc_id)
{
uint32 loc_value = 0;

	if (GWARZ[loc_id].entry == loc_id)
	{
		loc_value += GWCOMM[SERVER_GUILD_ID].loc_cost;
		loc_value += (GWARZ[loc_id].farm_count * GWCOMM[SERVER_GUILD_ID].farm_cost);
		loc_value += (GWARZ[loc_id].pig_count * GWCOMM[SERVER_GUILD_ID].pig_cost);
		loc_value += (GWARZ[loc_id].barrack_count * GWCOMM[SERVER_GUILD_ID].barrack_cost);
//		loc_value += (GWARZ[loc_id].guard_count * GWCOMM[SERVER_GUILD_ID].guard_cost); // guards are non-refundable so no value added
		loc_value += (GWARZ[loc_id].hall_count * GWCOMM[SERVER_GUILD_ID].hall_cost);
		loc_value += (GWARZ[loc_id].vendor1_count * GWCOMM[SERVER_GUILD_ID].vendor1_cost);
		loc_value += (GWARZ[loc_id].vendor2_count * GWCOMM[SERVER_GUILD_ID].vendor2_cost);
		loc_value += (GWARZ[loc_id].vendor3_count * GWCOMM[SERVER_GUILD_ID].vendor3_cost);
		loc_value += (GWARZ[loc_id].vault_count * GWCOMM[SERVER_GUILD_ID].vault_cost);
		loc_value += (GWARZ[loc_id].mailbox_count * GWCOMM[SERVER_GUILD_ID].mailbox_cost);
		loc_value += (GWARZ[loc_id].cannon_count * GWCOMM[SERVER_GUILD_ID].cannon_cost);

	};
return loc_value;
};

uint64 GGW::CalculateTotalLocationsValue(uint32 guild_id)
{
	uint32 loc_id = 0;
	uint32 loc_value = 0;
	uint64 loc_total_value = 0;

	for (loc_id = 1; loc_id < GWARZ.size(); loc_id++)
	{
		if (GWARZ[loc_id].guild_id == guild_id) 
		{
			loc_value = CalculateLocationValue(loc_id);
			loc_total_value = uint64(loc_total_value + loc_value);
		};
	}
	return loc_total_value;
};

uint32 GGW::GetRank(uint32 guild_id)
{
	uint32 rank;
	uint32 guild_rank;
	uint32 size = sGGW->GW_Ranks.size();

	for (rank = 1; rank <= size; rank++)
	{
		if (sGGW->GW_Ranks[rank].guild_id == guild_id)
		{ 
			guild_rank = rank;
			break; 
		};
	}
	return guild_rank;
};

bool GGW::UpdateRankEntry(uint32 guild_id, uint32 total_gross_worth, uint8 team)
{
	if (test) { TC_LOG_INFO("server.loading", "UPDATE RANK ENRTY:: GUILD:%u WORTH:%u TEAM:%u", guild_id, total_gross_worth, team); };

	if (sGGW->GWCOMM[guild_id].guild_id = guild_id)
	{
		WorldDatabase.PExecute("REPLACE INTO %s (`guild_id`, `team`, `total_gross_worth`) VALUES('%u', '%u', '%u');", ranking_db.c_str(), guild_id, team, total_gross_worth);

		uint32 rank;

		if (sGGW->GetRank(guild_id))
			rank = sGGW->GetRank(guild_id);
		else
		{
			rank = GW_Ranks.size() + 1;
		}

		GW_Ranks[rank].guild_id = guild_id;
		GW_Ranks[rank].team = team;
		GW_Ranks[rank].total_gross_worth = total_gross_worth;
	}
	return true;
};

void GGW::BuildRankList()
{
	sGGW->GW_Ranks.clear();

	uint32 rank = 1;

	QueryResult RankQry = WorldDatabase.Query(("SELECT `guild_id`,`team`,`total_gross_worth`  FROM " + sGGW->ranking_db + " WHERE guild_id != '0' ORDER BY `total_gross_worth` DESC;").c_str());

		if (RankQry)
		{
			do
			{
				Field *fields = RankQry->Fetch();
				// Save the DB values to the LocData object
				uint32 guild_id = fields[0].GetUInt32();
				uint8 team = fields[1].GetUInt32();
				uint32 total_gross_worth = fields[2].GetUInt32();
	
				if (sGGW->GWCOMM[guild_id].guild != "")
				{
					if(test){TC_LOG_INFO("server.loading", "<RANK>%u <NAME>%s <GUILD_ID>%u <TEAM>%u", rank, sGGW->GWCOMM[guild_id].guild, guild_id, team); };

					sGGW->GW_Ranks[rank].guild_id = guild_id;
					sGGW->GW_Ranks[rank].team = team;
					sGGW->GW_Ranks[rank].total_gross_worth = total_gross_worth;
					sGGW->GW_Ranks[rank].name = sGGW->GWCOMM[guild_id].guild;

					rank = rank + 1;
				}
			} while (RankQry->NextRow());
		}
}

bool GGW::CreateRankList()
{
	uint32 guildid;

		for (std::unordered_map<uint32, Commands>::iterator itr1 = sGGW->GWCOMM.begin(); itr1 != sGGW->GWCOMM.end(); itr1++)
		{
			guildid = itr1->first;

			if (guildid != 0)
			{
				uint32 total_gross_worth = sGGW->CalculateTotalLocationsValue(guildid);
				bool dummyUpdateRankEntry = sGGW->UpdateRankEntry(guildid, total_gross_worth, sGGW->GWCOMM[guildid].team);
			}
		}
		BuildRankList();

		return true;
};

bool rank_binary_ticker = false;

class GGW_RankTimer : public WorldScript
{
public:
	GGW_RankTimer() : WorldScript("GGW_RankTimer")
	{
		events.ScheduleEvent(1, sGGW->GUILDWARZ_RANKING_TIMER);
	};

	void OnUpdate(uint32 diff) override
	{
		events.Update(diff);

		uint8 id = events.ExecuteEvent();

		if (rank_binary_ticker == true)
			rank_binary_ticker = false;
		else
		{
			switch (id)
			{
			case 1:
				events.CancelEvent(1);
				sGGW->CreateRankList();

					if (sGGW->GUILDWARZ_RANK_TYPE < 2)
					{ 
						events.ScheduleEvent(1, sGGW->GUILDWARZ_RANKING_TIMER);

						if (test) { TC_LOG_INFO("server.loading", "GUILD_WARZ_RANK_TIMER : TICKED"); };
					};

				rank_binary_ticker = true;
				break;
			};
		};
	};

EventMap events;
};

class GGW_LoadGWtable : public WorldScript
{
	public: GGW_LoadGWtable() : WorldScript("GGW_LoadGWtable"){ };

		virtual void OnConfigLoad(bool /*reload*/)
		{
			TC_LOG_INFO("server.loading", "______________________________________");
			TC_LOG_INFO("server.loading", "-      Grumbo'z Guild Warz CPP       -");
			TC_LOG_INFO("server.loading", "-                                    -");
			TC_LOG_INFO("server.loading", "-    For Trinity Core WotlK 3.3.5a   -");
			TC_LOG_INFO("server.loading", "______________________________________");
			TC_LOG_INFO("server.loading", "-               LOADING              -");
            TC_LOG_INFO("server.loading", "______________________________________");

            // db address's in string value for sql updates
            sGGW->Guild_Warz_DB = sConfigMgr->GetStringDefault("GUILD_WARZ.DB_ADDRESS", "guild_warz_335");
            sGGW->commands_db = sGGW->Guild_Warz_DB + ".commands";
            sGGW->help_db = sGGW->Guild_Warz_DB + ".help";
            sGGW->zones_db = sGGW->Guild_Warz_DB + ".zones";
            sGGW->ranking_db = sGGW->Guild_Warz_DB + ".ranking";

            TC_LOG_INFO("server.loading", "- DB ADDRS : %s", sGGW->Guild_Warz_DB);

			bool com_table = sGGW->LoadCommands();

			if (com_table)
			{
				TC_LOG_INFO("server.loading", "- commands loaded                    -", sGGW->GWCOMM.size());
			}

			bool help_table = sGGW->LoadHelp();

			if (help_table)
			{
				TC_LOG_INFO("server.loading", "- help system loaded                 -");
			}

			bool loc_table = sGGW->LoadLoc();

			if (loc_table)
			{
				TC_LOG_INFO("server.loading", "- %u locations loaded                   ", sGGW->GWARZ.size());
			}

			sGGW->GUILDWARZ_PIGPAYZ_VALUE = sConfigMgr->GetIntDefault("GUILD_WARZ.PIGPAYZ_REWARD", 1000);
			sGGW->GUILDWARZ_PIGPAYZ_TIMER = (sConfigMgr->GetIntDefault("GUILD_WARZ.PIGPAYZ_TIMER", 30)) * minutes;
			sGGW->GUILDWARZ_RANKING_TIMER = (sConfigMgr->GetIntDefault("GUILD_WARZ.RANKING_TIMER", 10)) * minutes;
			sGGW->GUILDWARZ_RANK_TYPE		= sConfigMgr->GetIntDefault("GUILD_WARZ.RANK_TYPE", 1);
			sGGW->GUILDWARZ_RANKING_MAX	= sConfigMgr->GetIntDefault("GUILD_WARZ.RANK_MAX", 100);

			if (test){ TC_LOG_INFO("server.loading", "LOADING :: PIG_PAYZ :: PAYZ:%u TIMER:%u", sGGW->GUILDWARZ_PIGPAYZ_VALUE, sGGW->GUILDWARZ_PIGPAYZ_TIMER); };

			sGGW->Currencyid = sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency;

			if (!sObjectMgr->GetItemTemplate(sGGW->Currencyid))
			{
				TC_LOG_INFO("server.loading", "- Error Loading Currency ID:%d . Item May NOT exsist in the sql DB.", sGGW->Currencyid);
				TC_LOG_INFO("server.loading", "- Make sure you are using a valid item entry id in the sql DB.");
				TC_LOG_INFO("server.loading", "- Prepare for crash.");
			}
	
			sGGW->Currencyname = sObjectMgr->GetItemTemplate(sGGW->Currencyid)->Name1;

			TC_LOG_INFO("server.loading", "- Core version:%.2f                  -", sGGW->core_version);
			TC_LOG_INFO("server.loading", "- tables version:%.2f                -", sGGW->table_version);
			TC_LOG_INFO("server.loading", "- Pig Payz version:%.2f              -", sGGW->pigpayz_version);
			TC_LOG_INFO("server.loading", "- Teleporter version:%.2f            -", sGGW->tele_version);
			TC_LOG_INFO("server.loading", "- PvP version:%.2f                   -", sGGW->pvp_version);

			if (sGGW->vendor1) { TC_LOG_INFO("server.loading", "- Vendor1 loaded                     -"); };
			if (sGGW->vendor2) { TC_LOG_INFO("server.loading", "- Vendor2 loaded                     -"); };
			if (sGGW->vendor3) { TC_LOG_INFO("server.loading", "- Vendor3 loaded                     -"); };

			TC_LOG_INFO("server.loading", "- Vendor Core:%.2f                   -", sGGW->tele_version);


			if (sGGW->GUILDWARZ_RANK_TYPE < 2)
			{
				TC_LOG_INFO("server.loading", "- Guild Ranking type:Timer           -", sGGW->GW_version);
				sGGW->CreateRankList();
				events.ScheduleEvent(1, sGGW->GUILDWARZ_RANKING_TIMER);
			};

			if (sGGW->GUILDWARZ_RANK_TYPE > 1) TC_LOG_INFO("server.loading", "- Guild Ranking type:OnEvent         -", sGGW->GW_version);

			TC_LOG_INFO("server.loading", "______________________________________");
			TC_LOG_INFO("server.loading", "-          Goliath Online            -");
			TC_LOG_INFO("server.loading", "-                                    -");
			TC_LOG_INFO("server.loading", "-        Guild Warz Ver:%.2fc        -", sGGW->GW_version);
			TC_LOG_INFO("server.loading", "______________________________________");
		};
	EventMap events;
};

void GGW::SendGuildMessage(uint32 guild_id, std::string msg)
{
	SessionMap sessions = sWorld->GetAllSessions();

	msg = "[" + sGGW->GWCOMM[guild_id].color_8 + "GuildWarz" + "|r]:" + msg;

	for (SessionMap::iterator itr = sessions.begin(); itr != sessions.end(); ++itr)
	{

		if (!itr->second)
			continue;

		Player *player = itr->second->GetPlayer();

		if (player->GetGuildId() == guild_id)
		{
			ChatHandler(player->GetSession()).PSendSysMessage(msg.c_str());
		}
	}
};

uint32 GGW::CreateGuildLocation(uint32 map_id, uint32 area_id, uint32 zone_id, float pX, float pY, float pZ)
{
	uint32 CLentry = (GWARZ.size()); // + 1

	if (GWARZ[CLentry].entry == CLentry)
		do
		{
			CLentry++;
		} while (GWARZ[CLentry].entry == CLentry);

	WorldDatabase.PExecute("INSERT INTO %s (entry, map_id, area_id, zone_id, x, y, z) VALUES('%u', '%u', '%u', '%u', '%f', '%f', '%f');", zones_db.c_str(), CLentry, map_id, area_id, zone_id, pX, pY, pZ);

	LocData& data = GWARZ[CLentry]; // like Lua table GWARZ[guild_id].entry
	data.entry = CLentry;
	data.map_id = map_id;
	data.area_id = area_id;
	data.zone_id = zone_id;
	data.guild_name = SERVER_GUILD_NAME;
	data.team = SERVER_GUILD_TEAM;
	data.x = pX;
	data.y = pY;
	data.z = pZ;
	data.farm_count = 0;
	data.barrack_count = 0;
	data.hall_count = 0;
	data.pig_count = 0;
	data.guard_count = 0;
	data.vendor1_count = 0;
	data.vendor2_count = 0;
	data.vendor3_count = 0;
	data.cannon_count = 0;
	data.vault_count = 0;
	data.mailbox_count = 0;
	data.flag_id = 0;
	data.fs_time = 0;
	data.guild_id = SERVER_GUILD_ID;

	return CLentry;
};

void GGW::UpdateGuildLocData(std::string column_target, std::string new_data, uint32 loc_id)
{
	WorldDatabase.PExecute("UPDATE %s SET `%s` = '%s' WHERE `entry` = '%u';", sGGW->zones_db.c_str(), column_target.c_str(), new_data.c_str(), loc_id);

	if (column_target == "guild_name")
	{
		sGGW->GWARZ[loc_id].guild_name = new_data.c_str();
	}
	if (column_target == "team")
	{
		sGGW->GWARZ[loc_id].team = uint8(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "farm_count")
	{
		sGGW->GWARZ[loc_id].farm_count = uint32(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "barrack_count")
	{
		sGGW->GWARZ[loc_id].barrack_count = uint32(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "hall_count")
	{
		sGGW->GWARZ[loc_id].hall_count = uint32(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "pig_count")
	{
		sGGW->GWARZ[loc_id].pig_count = uint32(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "guard_count")
	{
		sGGW->GWARZ[loc_id].guard_count = uint32(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "vendor1_count")
	{
		sGGW->GWARZ[loc_id].vendor1_count = uint32(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "vendor2_count")
	{
		sGGW->GWARZ[loc_id].vendor2_count = uint32(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "vendor3_count")
	{
		sGGW->GWARZ[loc_id].vendor3_count = uint32(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "cannon_count")
	{
		sGGW->GWARZ[loc_id].cannon_count = uint32(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "vault_count")
	{
		sGGW->GWARZ[loc_id].vault_count = uint32(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "mailbox_count")
	{
		sGGW->GWARZ[loc_id].mailbox_count = uint32(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "flag_id")
	{
		sGGW->GWARZ[loc_id].flag_id = uint32(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "fs_time")
	{
		sGGW->GWARZ[loc_id].fs_time = sGGW->GGW_ConvertStringToNumber(new_data); // sWorld->GetGameTime();
	}
	if (column_target == "guild_id")
	{
		sGGW->GWARZ[loc_id].guild_id = uint32(sGGW->GGW_ConvertStringToNumber(new_data));
	}
};

void GGW::UpdateGuildLocFloat(float x, float y, float z, uint32 loc_id)
{
	WorldDatabase.PExecute("UPDATE %s SET x='%f', y='%f', z='%f' WHERE entry='%u';", sGGW->zones_db.c_str(), x, y, z, loc_id);

	sGGW->GWARZ[loc_id].x = x;
	sGGW->GWARZ[loc_id].y = y;
	sGGW->GWARZ[loc_id].z = z;
};

uint32 GGW::GetLocationID(uint32 map_id, uint32 area_id, uint32 zone_id)
{
	uint32 loc_id = NULL;

	for (loc_id = 1; loc_id < sGGW->GWARZ.size(); loc_id++)
	{
		if (sGGW->GWARZ[loc_id].map_id == map_id && sGGW->GWARZ[loc_id].area_id == area_id && sGGW->GWARZ[loc_id].zone_id == zone_id)
		{
			return loc_id;
			break;
		}
	}
	return false;
};

uint32 GGW::CreateGuildCommands(std::string guild_name, uint32 guild_id)
{
	WorldDatabase.PExecute("INSERT INTO %s (guild_id, guild) VALUES('%u', '%s');", commands_db.c_str(), guild_id, guild_name.c_str());

		Commands& data = GWCOMM[guild_id]; // like Lua table GWCOMM[guild_id].commands
		data.guild = guild_name;
		data.guild_id = guild_id;
		data.commands = GWCOMM[SERVER_GUILD_ID].commands;
		data.info_loc = GWCOMM[SERVER_GUILD_ID].info_loc;
		data.list_loc = GWCOMM[SERVER_GUILD_ID].list_loc;
		data.tele = GWCOMM[SERVER_GUILD_ID].tele;
		data.version = GWCOMM[SERVER_GUILD_ID].version;
		data.GLD_lvlb = GWCOMM[SERVER_GUILD_ID].GLD_lvlb;
		data.GLD_lvls = GWCOMM[SERVER_GUILD_ID].GLD_lvls;
		data.respawn_flag = GWCOMM[SERVER_GUILD_ID].respawn_flag;
		data.details_loc = GWCOMM[SERVER_GUILD_ID].details_loc;
		data.table = GWCOMM[SERVER_GUILD_ID].table;
		data.GM_admin = GWCOMM[SERVER_GUILD_ID].GM_admin;
		data.GM_minimum = GWCOMM[SERVER_GUILD_ID].GM_minimum;
		data.currency = GWCOMM[SERVER_GUILD_ID].currency;
		data.pig_payz = GWCOMM[SERVER_GUILD_ID].pig_payz;
		data.pig_payz_timer = GWCOMM[SERVER_GUILD_ID].pig_payz_timer;
		data.gift_count = GWCOMM[SERVER_GUILD_ID].gift_count;
		data.flag_require = GWCOMM[SERVER_GUILD_ID].flag_require;
		data.Server = GWCOMM[SERVER_GUILD_ID].Server;
		data.command_set = GWCOMM[SERVER_GUILD_ID].command_set;
		data.anarchy = GWCOMM[SERVER_GUILD_ID].anarchy;
		data.f_timer = GWCOMM[SERVER_GUILD_ID].f_timer;
		data.s_timer = GWCOMM[SERVER_GUILD_ID].s_timer;
		data.guild_invite = GWCOMM[SERVER_GUILD_ID].guild_invite;
		data.loc = GWCOMM[SERVER_GUILD_ID].loc;
		data.loc_cost = GWCOMM[SERVER_GUILD_ID].loc_cost;
		data.flag_id = GWCOMM[SERVER_GUILD_ID].flag_id;
		data.farm = GWCOMM[SERVER_GUILD_ID].farm;
		data.farm_cost = GWCOMM[SERVER_GUILD_ID].farm_cost;
		data.farm_L = GWCOMM[SERVER_GUILD_ID].farm_L;
		data.farm_id = GWCOMM[SERVER_GUILD_ID].farm_id;
		data.barrack = GWCOMM[SERVER_GUILD_ID].barrack;
		data.barrack_cost = GWCOMM[SERVER_GUILD_ID].barrack_cost;
		data.barrack_L = GWCOMM[SERVER_GUILD_ID].barrack_L;
		data.barrack_id = GWCOMM[SERVER_GUILD_ID].barrack_id;
		data.hall = GWCOMM[SERVER_GUILD_ID].hall;
		data.hall_cost = GWCOMM[SERVER_GUILD_ID].hall_cost;
		data.hall_L = GWCOMM[SERVER_GUILD_ID].hall_L;
		data.hall_id = GWCOMM[SERVER_GUILD_ID].hall_id;
		data.pig = GWCOMM[SERVER_GUILD_ID].pig;
		data.pig_cost = GWCOMM[SERVER_GUILD_ID].pig_cost;
		data.pig_L = GWCOMM[SERVER_GUILD_ID].pig_L;
		data.pig_id = GWCOMM[SERVER_GUILD_ID].pig_id;
		data.guard = GWCOMM[SERVER_GUILD_ID].guard;
		data.guard_cost = GWCOMM[SERVER_GUILD_ID].guard_cost;
		data.guard_L = GWCOMM[SERVER_GUILD_ID].guard_L;
		data.guard_id = GWCOMM[SERVER_GUILD_ID].guard_id;
		data.vendor1 = GWCOMM[SERVER_GUILD_ID].vendor1;
		data.vendor1_cost = GWCOMM[SERVER_GUILD_ID].vendor1_cost;
		data.vendor1_L = GWCOMM[SERVER_GUILD_ID].vendor1_L;
		data.vendor1_id = GWCOMM[SERVER_GUILD_ID].vendor1_id;
		data.vendor2 = GWCOMM[SERVER_GUILD_ID].vendor2;
		data.vendor2_cost = GWCOMM[SERVER_GUILD_ID].vendor2_cost;
		data.vendor2_L = GWCOMM[SERVER_GUILD_ID].vendor2_L;
		data.vendor2_id = GWCOMM[SERVER_GUILD_ID].vendor2_id;
		data.vendor3 = GWCOMM[SERVER_GUILD_ID].vendor3;
		data.vendor3_cost = GWCOMM[SERVER_GUILD_ID].vendor3_cost;
		data.vendor3_L = GWCOMM[SERVER_GUILD_ID].vendor3_L;
		data.vendor3_id = GWCOMM[SERVER_GUILD_ID].vendor3_id;
		data.cannon = GWCOMM[SERVER_GUILD_ID].cannon;
		data.cannon_cost = GWCOMM[SERVER_GUILD_ID].cannon_cost;
		data.cannon_L = GWCOMM[SERVER_GUILD_ID].cannon_L;
		data.cannon_id = GWCOMM[SERVER_GUILD_ID].cannon_id;
		data.vault = GWCOMM[SERVER_GUILD_ID].vault;
		data.vault_cost = GWCOMM[SERVER_GUILD_ID].vault_cost;
		data.vault_L = GWCOMM[SERVER_GUILD_ID].vault_L;
		data.vault_id = GWCOMM[SERVER_GUILD_ID].vault_id;
		data.mailbox = GWCOMM[SERVER_GUILD_ID].mailbox;
		data.mailbox_cost = GWCOMM[SERVER_GUILD_ID].mailbox_cost;
		data.mailbox_L = GWCOMM[SERVER_GUILD_ID].mailbox_L;
		data.mailbox_id = GWCOMM[SERVER_GUILD_ID].mailbox_id;
		data.setup = GWCOMM[SERVER_GUILD_ID].setup;
		data.color_1 = GWCOMM[SERVER_GUILD_ID].color_1;
		data.color_2 = GWCOMM[SERVER_GUILD_ID].color_2;
		data.color_3 = GWCOMM[SERVER_GUILD_ID].color_3;
		data.color_4 = GWCOMM[SERVER_GUILD_ID].color_4;
		data.color_5 = GWCOMM[SERVER_GUILD_ID].color_5;
		data.color_6 = GWCOMM[SERVER_GUILD_ID].color_6;
		data.color_7 = GWCOMM[SERVER_GUILD_ID].color_7;
		data.color_8 = GWCOMM[SERVER_GUILD_ID].color_8;
		data.color_9 = GWCOMM[SERVER_GUILD_ID].color_9;
		data.color_10 = GWCOMM[SERVER_GUILD_ID].color_10;
		data.color_11 = GWCOMM[SERVER_GUILD_ID].color_11;
		data.color_12 = GWCOMM[SERVER_GUILD_ID].color_12;
		data.color_13 = GWCOMM[SERVER_GUILD_ID].color_13;
		data.color_14 = GWCOMM[SERVER_GUILD_ID].color_14;
		data.color_15 = GWCOMM[SERVER_GUILD_ID].color_15;

		return GWCOMM[guild_id].guild_id;
};

void GGW::UpdateGuildCommandData(std::string column_target, std::string new_data, uint32 guild_id)
{
	WorldDatabase.PExecute("UPDATE %s SET `%s`='%s' WHERE `guild_id` = '%u';", sGGW->commands_db.c_str(), column_target.c_str(), new_data.c_str(), guild_id);

	// GUILD MASTER
	if (column_target == "commands")
	{
		sGGW->GWCOMM[guild_id].commands = new_data.c_str();
	}
	if (column_target == "info_loc")
	{
		sGGW->GWCOMM[guild_id].info_loc = new_data.c_str();
	}
	if (column_target == "list_loc")
	{
		sGGW->GWCOMM[guild_id].list_loc = new_data.c_str();
	}
	if (column_target == "tele")
	{
		sGGW->GWCOMM[guild_id].tele = new_data.c_str();
	}
	if (column_target == "loc")
	{
		sGGW->GWCOMM[guild_id].loc = new_data.c_str();
	}
	if (column_target == "farm")
	{
		sGGW->GWCOMM[guild_id].farm = new_data.c_str();
	}
	if (column_target == "barrack")
	{
		sGGW->GWCOMM[guild_id].barrack = new_data.c_str();
	}
	if (column_target == "hall")
	{
		sGGW->GWCOMM[guild_id].hall = new_data.c_str();
	}
	if (column_target == "pig")
	{
		sGGW->GWCOMM[guild_id].pig = new_data.c_str();
	}
	if (column_target == "guard")
	{
		sGGW->GWCOMM[guild_id].guard = new_data.c_str();
	}
	if (column_target == "vendor1")
	{
		sGGW->GWCOMM[guild_id].vendor1 = new_data.c_str();
	}
	if (column_target == "vendor2")
	{
		sGGW->GWCOMM[guild_id].vendor2 = new_data.c_str();
	}
	if (column_target == "vendor3")
	{
		sGGW->GWCOMM[guild_id].vendor3 = new_data.c_str();
	}
	if (column_target == "cannon")
	{
		sGGW->GWCOMM[guild_id].cannon = new_data.c_str();
	}
	if (column_target == "vault")
	{
		sGGW->GWCOMM[guild_id].vault = new_data.c_str();
	}
	if (column_target == "mailbox")
	{
		sGGW->GWCOMM[guild_id].vault = new_data.c_str();
	}
	if (column_target == "color_1")
	{
		sGGW->GWCOMM[guild_id].color_1 = new_data.c_str();
	}
	if (column_target == "color_2")
	{
		sGGW->GWCOMM[guild_id].color_2 = new_data.c_str();
	}
	if (column_target == "color_3")
	{
		sGGW->GWCOMM[guild_id].color_3 = new_data.c_str();
	}
	if (column_target == "color_4")
	{
		sGGW->GWCOMM[guild_id].color_4 = new_data.c_str();
	}
	if (column_target == "color_5")
	{
		sGGW->GWCOMM[guild_id].color_5 = new_data.c_str();
	}
	if (column_target == "color_6")
	{
		sGGW->GWCOMM[guild_id].color_6 = new_data.c_str();
	}
	if (column_target == "color_7")
	{
		sGGW->GWCOMM[guild_id].color_7 = new_data.c_str();
	}
	if (column_target == "color_8")
	{
		sGGW->GWCOMM[guild_id].color_8 = new_data.c_str();
	}
	if (column_target == "color_9")
	{
		sGGW->GWCOMM[guild_id].color_9 = new_data.c_str();
	}
	if (column_target == "color_10")
	{
		sGGW->GWCOMM[guild_id].color_10 = new_data.c_str();
	}
	if (column_target == "color_11")
	{
		sGGW->GWCOMM[guild_id].color_11 = new_data.c_str();
	}
	if (column_target == "color_12")
	{
		sGGW->GWCOMM[guild_id].color_12 = new_data.c_str();
	}
	if (column_target == "color_13")
	{
		sGGW->GWCOMM[guild_id].color_13 = new_data.c_str();
	}
	if (column_target == "color_14")
	{
		sGGW->GWCOMM[guild_id].color_14 = new_data.c_str();
	}
	if (column_target == "color_15")
	{
		sGGW->GWCOMM[guild_id].color_15 = new_data.c_str();
	}
	if (column_target == "guild_invite")
	{
		sGGW->GWCOMM[guild_id].guild_invite = uint8(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "GLD_lvlb")
	{
		sGGW->GWCOMM[guild_id].GLD_lvlb = uint8(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "GLD_lvls")
	{
		sGGW->GWCOMM[guild_id].GLD_lvls = uint8(sGGW->GGW_ConvertStringToNumber(new_data));
	}

// ADMIN
	if (column_target == "GM_admin")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].GM_admin = uint8(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "GM_minimum")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].GM_minimum = uint8(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "currency")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency = uint64(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "respawn_flag")
	{
		sGGW->GWCOMM[guild_id].respawn_flag = new_data.c_str();
	}
	if (column_target == "details_loc")
	{
		sGGW->GWCOMM[guild_id].details_loc = new_data.c_str();
	}
	if (column_target == "table")
	{
		sGGW->GWCOMM[guild_id].table = new_data.c_str();
	}
	if (column_target == "Server")
	{
		sGGW->GWCOMM[guild_id].Server = new_data.c_str();
	}
	if (column_target == "version")
	{
		sGGW->GWCOMM[guild_id].version = new_data.c_str();
	}
	if (column_target == "command_set")
	{
		sGGW->GWCOMM[guild_id].command_set = new_data.c_str();
	}
	if (column_target == "pig_payz")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_payz = uint64(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "pig_payz_timer")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_payz_timer = uint64(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "gift_count")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].gift_count = uint32(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "flag_require")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].flag_require = uint8(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "anarchy")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].anarchy = uint8(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "f_timer")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].f_timer = uint8(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "s_timer")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].s_timer = uint64(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "loc_cost")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].loc_cost = uint32(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "flag_id")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].flag_id = uint64(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "farm_cost")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].farm_cost = uint16(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "farm_L")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].farm_L = uint16(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "farm_id")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].farm_id = uint64(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "barrack_cost")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].barrack_cost = uint16(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "barrack_L")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].barrack_L = uint16(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "barrack_id")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].barrack_id = uint64(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "hall_cost")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].hall_cost = uint16(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "hall_L")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].hall_L = uint16(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "hall_id")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].hall_id = uint64(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "pig_cost")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_cost = uint16(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "pig_L")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_L = uint16(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "pig_id")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_id = uint64(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "guard_cost")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].guard_cost = uint16(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "guard_L")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].barrack_L = uint16(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "guard_id")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].guard_id = uint64(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "vendor1_cost")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor1_cost = uint16(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "vendor1_L")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor1_L = uint16(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "vendor1_id")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor1_id = uint64(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "vendor2_cost")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor2_cost = uint16(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "vendor2_L")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor2_L = uint16(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "vendor2_id")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor2_id = uint64(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "vendor3_cost")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor3_cost = uint16(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "vendor3_L")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor3_L = uint16(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "vendor3_id")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor3_id = uint64(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "cannon_cost")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].cannon_cost = uint16(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "cannon_L")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].cannon_L = uint16(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "cannon_id")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].cannon_id = uint64(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "vault_cost")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vault_cost = uint16(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "vault_L")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vault_L = uint16(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "vault_id")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vault_id = uint64(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "mailbox_cost")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vault_cost = uint16(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "mailbox_L")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].mailbox_L = uint16(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "mailbox_id")
	{
		sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vault_id = uint64(sGGW->GGW_ConvertStringToNumber(new_data));
	}
	// System
	if (column_target == "allowed")
	{
		sGGW->GWCOMM[guild_id].allowed = uint8(sGGW->GGW_ConvertStringToNumber(new_data));
	}
};

uint32 SpawnGuildObjects(uint8 type, uint32 flat_id, uint32 team_id, uint32 guild_id, Map *map, float x, float y, float z, float o, Player *player, uint32 LocId)
{
	// type 0 = Creature :: 1 = GameObject :: 2 = GameObject Flag.

	uint32 id = (flat_id + team_id);

	if (type == 0)
	{
		Creature* GGW_NPC = new Creature();

		ObjectGuid::LowType db_guid = map->GenerateLowGuid<HighGuid::GameObject>();
//		uint32 db_guid = sObjectMgr->GenerateLowGuid(HIGHGUID_UNIT);

		if (!GGW_NPC->Create(db_guid, map, PHASEMASK_NORMAL, id, (x, y, z, o)))
		{
			ChatHandler(player->GetSession()).PSendSysMessage("%sNPC create error..", sGGW->GWCOMM[guild_id].color_15.c_str());
			delete GGW_NPC;
			return false;
		}

		CreatureData& data = sObjectMgr->NewOrExistCreatureData(db_guid); // Filling in the needed data to save
		data.id = id;
		data.phaseMask = player->GetPhaseMaskForSpawn();
        data.spawnPoint.Relocate(player->GetTransOffsetX(), player->GetTransOffsetY(), player->GetTransOffsetZ(), player->GetTransOffsetO());
        
		GGW_NPC->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()), 1); 

		if (!GGW_NPC->LoadFromDB(db_guid, map, 1, 0))
		{
			delete GGW_NPC;
			return false;
		}

		sObjectMgr->AddCreatureToGrid(db_guid, &data);

		if (!GGW_NPC->IsVisible())
		{
			ChatHandler(player->GetSession()).PSendSysMessage("%sNPC add to grid error..", sGGW->GWCOMM[guild_id].color_15.c_str());
			delete GGW_NPC;
			return false;
		}

		if (GGW_NPC->IsVisible())
		{
			GGW_NPC->SetFullHealth();
			return GGW_NPC->GetSpawnId();
		}
	return false;
	}

	if (type == 1 || type == 2)
	{
		const GameObjectTemplate* objectInfo = sObjectMgr->GetGameObjectTemplate(id);

			if (!objectInfo)
			{
				return false;
			}

		GameObject* GGW_GO = new GameObject(); // creating an empty GameObject shell.

		ObjectGuid::LowType guidLow = map->GenerateLowGuid<HighGuid::GameObject>();
        QuaternionData rot = QuaternionData::fromEulerAnglesZYX(player->GetOrientation(), 0.f, 0.f);

			if (!GGW_GO->Create(guidLow, id, map, PHASEMASK_NORMAL, *player, rot, 255, GO_STATE_ACTIVE))
			{
				ChatHandler(player->GetSession()).PSendSysMessage("%sFlag build error..", sGGW->GWCOMM[guild_id].color_15.c_str());
				delete GGW_GO;
				return false;
			};

		map->AddToMap(GGW_GO); // visual responce of spawning flag.

			if (!GGW_GO->isSpawned())
			{
				ChatHandler(player->GetSession()).PSendSysMessage("%sFlag spawn error..", sGGW->GWCOMM[guild_id].color_15.c_str());
				delete GGW_GO;
				return false;
			}

			if (GGW_GO->isSpawned())
			{
				GGW_GO->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()), player->GetPhaseMaskForSpawn()); // finally passes all checks, so save it to DB.

				return GGW_GO->GetSpawnId();
			}
		return false;
	}
	return false;
};

class GGW_GuildEngine : public GuildScript
{
public: GGW_GuildEngine() : GuildScript("GGW_GuildEngine"){ };

		virtual void OnCreate(Guild* guild, Player* leader, const std::string& name)
		{
			uint32 guild_id = guild->GetId();
			std::string gName = guild->GetName();
			auto team = leader->GetTeamId();

			sGGW->CreateGuildCommands(name, guild_id);

			sGGW->UpdateGuildCommandData("team", sGGW->ConvertNumberToString(team), guild_id);

			uint32 total_gross_worth = sGGW->CalculateTotalLocationsValue(guild_id);
			bool dummyUpdateRankEntry = sGGW->UpdateRankEntry(guild_id, total_gross_worth, team);

			leader->AddItem( 65000 + team, 1);
		}

		virtual void OnDisband(Guild* guild)
		{
			uint32 guild_id = guild->GetId();

			WorldDatabase.PExecute("DELETE FROM %s WHERE `guild_id`='%u';", sGGW->commands_db.c_str(), guild_id);

			for (uint32 pos = 1; pos <= sGGW->GWARZ.size(); pos++)
			{
				if (sGGW->GWARZ[pos].guild_id == guild_id)
				{
					sGGW->UpdateGuildLocData("guild_name", SERVER_GUILD_NAME, pos);
					sGGW->UpdateGuildLocData("team", "" + sGGW->ConvertNumberToString(sGGW->SERVER_GUILD_TEAM), pos);
					sGGW->UpdateGuildLocData("flag_id", "" + sGGW->ConvertNumberToString(0), pos);
					sGGW->UpdateGuildLocData("guild_id", "" + sGGW->ConvertNumberToString(sGGW->SERVER_GUILD_ID), pos);
				}
			}

			WorldDatabase.PExecute("DELETE FROM %s WHERE `guild_id` = '%u';", sGGW->ranking_db.c_str(), guild_id);

			sGGW->GWCOMM.erase(guild_id);
			sGGW->GWARZ.erase(guild_id);
		}

		virtual void OnEvent(Guild* guild, uint8 eventType, ObjectGuid::LowType playerGuid1, ObjectGuid::LowType playerGuid2, uint8 newRank) 
		{ 
			std::string GuildEventMap[5] = { "Unknown1", "Unknown2", "member_rank_up", "Member_rank_down", "Unknown5"};

			TC_LOG_INFO("server.loading", "GUILD_NAME:%s EVENT_ID:%u PLAYER1_GUID:%u PLAYER2_GUID:%u", guild->GetName().c_str(), GuildEventMap[eventType], playerGuid1, playerGuid2);
		}
};

void GGW::PigPayz(Player *player)
{
	uint32 guild_id = player->GetGuildId();
	uint32 pig_count = 0;
	uint32 pMoney = player->GetMoney();
	std::string coinage;
	std::string coin_types;

	pig_count = sGGW->GetTotalPigs(guild_id);

	uint32 tally = (pig_count * sGGW->GUILDWARZ_PIGPAYZ_VALUE);

	if (test){ TC_LOG_INFO("server.loading", "PIG_PAYZ PIG_COUNT:%u PAYZ:%u TALLY:%u", pig_count, sGGW->GUILDWARZ_PIGPAYZ_VALUE, tally); };

	player->ModifyMoney(tally, true);

	uint32 Gold = tally / 10000;
	uint32 Silver = (tally - (Gold * 10000)) / 100;
	uint32 Copper = (tally - ((Gold * 10000) + (Silver * 100))) / 1;

	if (Gold > 0){ coin_types = sGGW->ConvertNumberToString(Gold) + ":Gold"; };
	if (Silver > 0){ coin_types = coin_types + " " + sGGW->ConvertNumberToString(Silver) + ":Silver"; };
	if (Copper > 0){ coin_types = coin_types + " " + sGGW->ConvertNumberToString(Copper) + ":Copper"; };

	if (pig_count == 0)
	{
		ChatHandler(player->GetSession()).PSendSysMessage("[%sGuild Warz|r]:%sYour Guild needs Guild Pig's to start receiving profit.|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_15, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_10);
		ChatHandler(player->GetSession()).PSendSysMessage("[%sGuild Warz|r]:%sTell your Guild Leader to start adding Guild Pig's to your Guild Locations.|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_15, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_10);
	}

	if (pig_count > 0)
	{
		ChatHandler(player->GetSession()).PSendSysMessage("[%sGuild Warz|r]:%sYour %u Guild Pig's pay off.|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_15, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_10, pig_count);
		ChatHandler(player->GetSession()).PSendSysMessage("%s+%s.", sGGW->GWCOMM[guild_id].color_2.c_str(), coin_types.c_str());
	}
};

class GGW_PigPayz : public BasicEvent
{
public:
	GGW_PigPayz(Player* _player) : player(_player)
	{
		_player->m_Events.AddEvent(this, _player->m_Events.CalculateTime(sGGW->GUILDWARZ_PIGPAYZ_TIMER)); // 1000 = 1 second  // 60000 = 1 minute
	}

	bool Execute(uint64, uint32) override
	{
		sGGW->PigPayz(player);

		new GGW_PigPayz(player);

		return true;
	}

	Player* player;
};

void FactionReset(Player* player)
{
	uint32 pGuid = player->GetGUID();

	uint32 map_id = player->GetMapId();
	uint32 area_id = player->GetAreaId();
	uint32 zone_id = player->GetZoneId();
	float pX = player->GetPositionX();
	float pY = player->GetPositionY();
	float pZ = player->GetPositionZ();

	uint32 LocId = sGGW->GetLocationID(map_id, area_id, zone_id);

	auto pTeam_id = player->GetTeamId();
	uint32 guild_id = player->GetGuildId();

	uint32 loc_guild = sGGW->GWARZ[LocId].guild_id;
	uint8 LocTeam = sGGW->GWARZ[LocId].team;

	uint32 loc_faction = (84 - LocTeam);
	uint32 invade_faction = (83 + LocTeam);

	if (!LocId)
	{
		LocId = sGGW->CreateGuildLocation(map_id, area_id, zone_id, pX, pY, pZ);
	}

	if (LocId)
	{
		if (sGGW->GWARZ[LocId].team > 1)
		{
			player->RestoreFaction();
		};

		if (sGGW->GWARZ[LocId].team < 2)
		{
			if (sGGW->GWARZ[LocId].guild_id == guild_id)
			{
				player->SetPvP(true);
				player->SetFaction(loc_faction);
			}

			if (guild_id == 0)
			{
				if (sGGW->GWARZ[LocId].team == pTeam_id){ player->SetFaction(loc_faction); };
				if (sGGW->GWARZ[LocId].team != pTeam_id){ player->SetFaction(invade_faction); };
			}

			if (guild_id > 0)
			{
				if (sGGW->GWARZ[LocId].guild_id != guild_id)
				{
					if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].anarchy == 1)
					{
						player->SetFaction(invade_faction);
					}

					if (sGGW->GWARZ[LocId].team != pTeam_id)
					{
						player->SetPvP(true);
						player->SetFaction(invade_faction);
					}

					if (sGGW->GWARZ[LocId].team == pTeam_id)
					{
						if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].anarchy == 0)
						{
							player->SetPvP(true);
							player->SetFaction(loc_faction);
						}

						if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].anarchy == 1)
						{
							player->SetPvP(true);
							player->SetFaction(invade_faction);
						}
					}
				}
			}
		}
	}
};

class GGW_FactionReset : public BasicEvent
{
public:
	GGW_FactionReset(Player* _player) : player(_player)
	{
		_player->m_Events.AddEvent(this, _player->m_Events.CalculateTime(sGGW->GUILDWARZ_PLAYER_CHECK_TIMER)); // 1000 = 1 second  // 60000 = 1 minute
	}

	bool Execute(uint64, uint32) override
	{
		FactionReset(player);
	}
	Player* player;
};

class GGW_GuildPlayerOnLogin : public PlayerScript
{
public: GGW_GuildPlayerOnLogin() : PlayerScript("GGW_GuildPlayerOnLogin"){ };

	virtual void OnLogin(Player* player, bool /*firstLogin*/)
	{
		if (player->GetGuildId() > 0)
		{
			uint32 pGuid = player->GetGUID();
			uint32 guild_id = player->GetGuildId();
			auto pTeam = player->GetTeamId();
			
			new GGW_PigPayz(player);

			ChatHandler(player->GetSession()).PSendSysMessage("%s<%sGrumbo|r%s> Says:Type `/g commands` for a list of Your Guild Warz Commands.|r", sGGW->GWCOMM[guild_id].color_10.c_str(), sGGW->GWCOMM[guild_id].color_6.c_str(), sGGW->GWCOMM[guild_id].color_10.c_str());
		}
	}
};

/*
-- ****************************************************
-- ******************* CORE ENGINE ********************
-- ****************************************************
*/

class GGW_commands : public PlayerScript
{
public: GGW_commands() : PlayerScript("GGW_commands"){ };

	virtual void OnChat(Player* player, uint32 type, uint32 lang, std::string& msg, Guild* guild)
	{
		if (player->GetGuildId() > 0)
		{
			Map* Pmap = player->GetMap();
			uint32 map_id = player->GetMapId();
			uint32 area_id = player->GetAreaId();
			uint32 zone_id = player->GetZoneId();

			float pX = player->GetPositionX();
			float pY = player->GetPositionY();
			float pZ = player->GetPositionZ();
			float pO = player->GetOrientation();

			uint32 GGW_map_spawn_mode = Pmap->GetSpawnMode();
			uint32 GGW_player_phase_mask_for_spawn = player->GetPhaseMaskForSpawn();
			std::string pName = player->GetName();
			uint32 pGuid = player->GetGUID();
			auto pTeam_id = player->GetTeamId();
			uint32 guild_id = player->GetGuildId();
			std::string guild_name = guild->GetName();
			uint8 pGuildRank = player->GetRank();
			uint8 pGMRank = player->GetSession()->GetSecurity();
			uint32 GuildLeaderGUID = guild->GetLeaderGUID();

			uint32 gGuildID = sGGW->GWCOMM[guild_id].guild_id;

			uint32 LocId = sGGW->GetLocationID(map_id, area_id, zone_id);

			if (!LocId)
			{
				LocId = sGGW->CreateGuildLocation(map_id, area_id, zone_id, pX, pY, pZ);
			}

			if (guild_name != sGGW->GWCOMM[guild_id].guild)
			{
				ChatHandler(player->GetSession()).PSendSysMessage("<Grumbo>:CREATING Your Guild Warz Commands for Guild %s.", guild_name.c_str());
				sGGW->CreateGuildCommands(guild_name, guild_id);
			}

			std::string str_LocId = sGGW->ConvertNumberToString(LocId);

			if (!pGMRank)
			{
				pGMRank = 0;
			}

			if (sGGW->GWARZ[LocId].x == 0 && sGGW->GWARZ[LocId].y == 0 && sGGW->GWARZ[LocId].z == 0) // just a temp catch-22.
			{
				sGGW->UpdateGuildLocFloat(pX, pY, pZ, LocId);
			}

			uint32 Zoneprice = (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].loc_cost) + (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].farm_cost*sGGW->GWARZ[LocId].farm_count) + (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].barrack_cost*sGGW->GWARZ[LocId].barrack_count) + (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].hall_cost*sGGW->GWARZ[LocId].hall_count) + (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_cost*sGGW->GWARZ[LocId].pig_count) + (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor1_cost*sGGW->GWARZ[LocId].vendor1_count) + (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor2_cost*sGGW->GWARZ[LocId].vendor2_count) + (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor3_cost*sGGW->GWARZ[LocId].vendor3_count) + (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].cannon_cost*sGGW->GWARZ[LocId].cannon_count) + (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vault_cost*sGGW->GWARZ[LocId].vault_count) + (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].mailbox_cost*sGGW->GWARZ[LocId].mailbox_count);
			uint8 k = 0;

			std::vector<std::string> words = split(msg, ' ');
			std::map<uint8, std::string>ChatCache;

			for (auto it = words.begin(); it != words.end(); ++it)
			{
				k++;
				ChatCache[k] = *it;
			}

			/*
			-- ****************
			-- Commands Command
			-- ****************
			*/

			if (ChatCache[1] == sGGW->GWCOMM[guild_id].commands)
			{
				ChatHandler(player->GetSession()).PSendSysMessage("%s************************************************************|r", sGGW->GWCOMM[guild_id].color_9.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s*    -Grumbo'z Guild Warz Commands:-     *|r", sGGW->GWCOMM[guild_id].color_8.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s************************************************************|r", sGGW->GWCOMM[guild_id].color_9.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("(Guild: %s)", guild_name.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("(Name: %s) (Guild Rank: %u) (GAME Rank: %u)", pName.c_str(), pGuildRank, pGMRank);
				ChatHandler(player->GetSession()).PSendSysMessage("%s************************************************************|r", sGGW->GWCOMM[guild_id].color_9.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s Guild Member Commands:", sGGW->GWCOMM[guild_id].color_8.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s------------------------------------------------------------|r", sGGW->GWCOMM[guild_id].color_9.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s %s |r          %s list guild commands.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].commands.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s %s|r %s                     lists area info.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].info_loc.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s %s|r                      %s lists areas owned", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].list_loc.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s   |r                        %s by your guild.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s %s `location_id`|r  %s teleport to area", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].tele.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s |r                          %s by location id.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s %s|r                     %s -displays Core versions.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].version.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s %s|r                     %s -displays Core setup and settings.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].setup.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s %s|r                     %s -Displays Guild Ranking.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), GUILD_RANK_COMMAND.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s************************************************************|r", sGGW->GWCOMM[guild_id].color_9.c_str());

				if (pGuildRank <= sGGW->GWCOMM[guild_id].GLD_lvlb)
				{
					ChatHandler(player->GetSession()).PSendSysMessage("%s buy %sGuild Master level Commands:Rank:%s%u%s access.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_8.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[guild_id].GLD_lvlb, sGGW->GWCOMM[guild_id].color_8.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s------------------------------------------------------------|r", sGGW->GWCOMM[guild_id].color_9.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sbuy %s       %s- purchase area.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].loc.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sbuy %s       %s- purchase a guild farm.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].farm.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sbuy %s       %s- purchase a pig.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].pig.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sbuy %s       %s- purchase a barracks for guards.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].barrack.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sbuy %s       %s- purchase a disposable guard.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].guard.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sbuy %s       %s- purchase a guild hall.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].hall.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());

					if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].cannon_id > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%sbuy %s      %s- purchase a cannon.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].cannon.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					}

					if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor1_id > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%sbuy %s      %s- purchase vendor1.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].vendor1.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					}

					if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor2_id > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%sbuy %s      %s- purchase vendor2.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].vendor2.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					}

					if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor3_id > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%sbuy %s      %s- purchase vendor3.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].vendor3.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					}

					ChatHandler(player->GetSession()).PSendSysMessage("%sbuy %s       %s- purchase a Guild Vault.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].vault.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sbuy %s       %s- purchase a mailbox.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].mailbox.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s************************************************************|r", sGGW->GWCOMM[guild_id].color_9.c_str());

				}

				if (pGuildRank <= sGGW->GWCOMM[guild_id].GLD_lvls)
				{
					ChatHandler(player->GetSession()).PSendSysMessage("%ssell %sGuild Master level Commands:Rank:%s%u%s access.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_8.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[guild_id].GLD_lvls, sGGW->GWCOMM[guild_id].color_8.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s------------------------------------------------------------|r", sGGW->GWCOMM[guild_id].color_9.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%ssell %s        %s- sell area for its TOTAL value.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].loc.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%ssell %s        %s- sell farm.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].farm.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%ssell %s        %s- sell a pig to market.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].pig.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%ssell %s        %s- sell barracks.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].barrack.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%ssell %s        %s- removes a selected guard.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].guard.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s                 guards are disposable.|r", sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s                 no cash back.|r", sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%ssell %s        %s- sell Guild hall.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].hall.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());

					if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].cannon_id > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%ssell %s|r  %s- sell a selected cannon.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].cannon.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					}

					if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor1_id > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%ssell %s|r  %s- sells a selected vendor1.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].vendor1.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					}

					if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor2_id > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%ssell %s|r  %s- sells a selected vendor2.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].vendor2.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					}

					if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor3_id > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%ssell %s|r  %s- sells a selected vendor3.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].vendor3.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					}

					ChatHandler(player->GetSession()).PSendSysMessage("%ssell %s|r  %s- sells a guild vault.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].vault.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%ssell %s|r  %s- sells a guild mailbox.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].mailbox.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s************************************************************|r", sGGW->GWCOMM[guild_id].color_9.c_str());
				}

				if (pGuildRank == 0)
				{
					ChatHandler(player->GetSession()).PSendSysMessage("%sGuild Master settings:", sGGW->GWCOMM[guild_id].color_8.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s------------------------------------------------------------|r", sGGW->GWCOMM[guild_id].color_9.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sGuild Buy. minimum required guild rank|r:%s%u.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[guild_id].GLD_lvlb);
					ChatHandler(player->GetSession()).PSendSysMessage("%sGuild Sell. minimum required guild rank|r:%s%u.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[guild_id].GLD_lvls);
					ChatHandler(player->GetSession()).PSendSysMessage("%sGuild invite system|r:%s%u|r         %s0 = off 1 = on.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[guild_id].guild_invite, sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s------------------------------------------------------------|r", sGGW->GWCOMM[guild_id].color_9.c_str());
				}

				if (pGuildRank == 0 || (pGMRank == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].GM_admin && player->IsGameMaster()))
				{
					ChatHandler(player->GetSession()).PSendSysMessage("%sGuild Master/Game Master/Admin special Commands:", sGGW->GWCOMM[guild_id].color_8.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s------------------------------------------------------------|r", sGGW->GWCOMM[guild_id].color_9.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s%s|r     %s used to modify commands and settings.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].command_set.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%shelp %s|r    %slists commands and settings you may change.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].command_set.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s************************************************************|r", sGGW->GWCOMM[guild_id].color_9.c_str());
				}

				if (pGuildRank <= sGGW->GWCOMM[guild_id].GLD_lvlb || (pGMRank >= sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].GM_minimum && player->IsGameMaster()))
				{
					ChatHandler(player->GetSession()).PSendSysMessage("%sPrices in %s%s's:|r", sGGW->GWCOMM[guild_id].color_8.c_str(), sGGW->Currencyname.c_str(), sGGW->GWCOMM[guild_id].color_8.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s------------------------------------------------------------|r", sGGW->GWCOMM[guild_id].color_9.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sZone price|r: %s%u|r %sbase location price.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].loc_cost, sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sFarm price|r: %s%u.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].farm_cost);
					ChatHandler(player->GetSession()).PSendSysMessage("%sPig price|r: %s%u|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_cost);
					ChatHandler(player->GetSession()).PSendSysMessage("%sBarracks price|r: %s%u.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].barrack_cost);
					ChatHandler(player->GetSession()).PSendSysMessage("%sGuard price|r: %s%u|r - %sThese Dont increase location price.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].guard_cost, sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sHall price|r: %s%u.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].hall_cost);

					if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].cannon_id > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%scannon price|r: %s%u.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].cannon_cost);
					}

					if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor1_id > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%svendor1 price|r: %s%u.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor1_cost);
					}

					if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor2_id > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%svendor2 price|r: %s%u|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor2_cost);
					}

					if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor3_id > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%svendor3 price|r: %s%u|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor3_cost);
					}

					ChatHandler(player->GetSession()).PSendSysMessage("%sVault price|r: %s%u.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vault_cost);
					ChatHandler(player->GetSession()).PSendSysMessage("%sMailbox price|r: %s%u.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].mailbox_cost);
					ChatHandler(player->GetSession()).PSendSysMessage("%s************************************************************|r", sGGW->GWCOMM[guild_id].color_9.c_str());
				}

				if (pGuildRank <= sGGW->GWCOMM[guild_id].GLD_lvlb || (pGMRank >= sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].GM_minimum && player->IsGameMaster()))
				{
					ChatHandler(player->GetSession()).PSendSysMessage("%sLocation Limits:", sGGW->GWCOMM[guild_id].color_8.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s------------------------------------------------------------|r", sGGW->GWCOMM[guild_id].color_9.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sFarm limit|r: %s%u|r %sper location.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].farm_L, sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sPig limit|r: %s%u|r %sper farm. Require a farm.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_L, sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sBarrack limit|r: %s%u|r %sper farm. Require a farm.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].barrack_L, sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sGuard limit|r: %s%u|r %sper barrack. Require a barrack.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].guard_L, sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sHall limit|r: %s%u|r %sper location. Require a barrack.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].hall_L, sGGW->GWCOMM[guild_id].color_3.c_str());

					if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor1_id > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%sVendor1 limit|r: %s%u|r %sper location. Require a hall.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor1_L, sGGW->GWCOMM[guild_id].color_3.c_str());
					}
					if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor2_id > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%sVendor2 limit|r: %s%u|r %sper location. Require a hall.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor2_L, sGGW->GWCOMM[guild_id].color_3.c_str());
					}
					if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor3_id > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%sVendor3 limit|r: %s%u|r %sper location. Require a hall.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor3_L, sGGW->GWCOMM[guild_id].color_3.c_str());
					}
					if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].cannon_id > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%sCannon limit|r: %s%u|r %sper hall. Require a hall.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].cannon_L, sGGW->GWCOMM[guild_id].color_3.c_str());
					}
					ChatHandler(player->GetSession()).PSendSysMessage("%sGuild Vault limit|r: %s%u|r %sper location. Require a hall.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vault_L, sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sGuild MailBox limit|r: %s%u|r %sper location. Require a hall.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].mailbox_L, sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s************************************************************|r", sGGW->GWCOMM[guild_id].color_9.c_str());
				}

				if (pGMRank >= sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].GM_minimum && player->IsGameMaster())
				{
					ChatHandler(player->GetSession()).PSendSysMessage("Game Master Commands:", sGGW->GWCOMM[guild_id].color_8.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s------------------------------------------------------------|r", sGGW->GWCOMM[guild_id].color_9.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s%s|r                %s-location info.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].details_loc.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sreset %s|r          %s- Resets location to server.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].loc.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sreload %s|r         %s- Reloads GW tables.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].table.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sreset %s|r          %s- Resets location farm count to 0.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].farm.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());

					ChatHandler(player->GetSession()).PSendSysMessage("%sreset %s|r          %s- Resets location barrack count to 0.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].barrack.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sreset %s|r          %s- Resets location hall count to 0.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].hall.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sreset %s|r          %s- Resets location pig count to 0.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sreset %s|r          %s- Resets location guard count to 0.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].guard.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());

					ChatHandler(player->GetSession()).PSendSysMessage("%sreset %s|r          %s- Resets location vendor 1 count to 0.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor1.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sreset %s|r          %s- Resets location vendor 2 count to 0.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor2.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sreset %s|r          %s- Resets location vendor 3 count to 0.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor3.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sreset %s|r          %s- Resets location cannon count to 0.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].cannon.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sreset %s|r          %s- Resets location vault count to 0.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vault.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sreset %s|r          %s- Resets location mailbox count to 0.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].mailbox.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());

					ChatHandler(player->GetSession()).PSendSysMessage("%sspawn %s|r         %s- Spawns new flag if current|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].respawn_flag.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s                       flag is missing or corrupt.|r", sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%slock %s|r         %s- locks a location from purchase.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].loc.c_str(), sGGW->GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s************************************************************|r", sGGW->GWCOMM[guild_id].color_9.c_str());
				}

				if (pGMRank == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].GM_admin && player->IsGameMaster())
				{
					ChatHandler(player->GetSession()).PSendSysMessage("%sADMIN settings:", sGGW->GWCOMM[guild_id].color_8.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s------------------------------------------------------------|r", sGGW->GWCOMM[guild_id].color_9.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sADMIN Level Access|r:%s%u |r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].GM_admin);
					ChatHandler(player->GetSession()).PSendSysMessage("%sMinimum GM Level Access|r: %s%u|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].GM_minimum);
					ChatHandler(player->GetSession()).PSendSysMessage("%sPig Payz:%s%u copper.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_payz); // , GWCOMM[guild_id].color_3.c_str()
					ChatHandler(player->GetSession()).PSendSysMessage("%sNew Guild Gift amount|r:%s%u %s's|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].gift_count, sGGW->Currencyname.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sFlag require|r: %s%u|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].flag_require);
					ChatHandler(player->GetSession()).PSendSysMessage("%sAnarchy|r: %s%u|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].anarchy);

					ChatHandler(player->GetSession()).PSendSysMessage("%sNo-Tag-Flag|r:%s%u|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].f_timer);
					ChatHandler(player->GetSession()).PSendSysMessage("%sNo-Tag-Flag-Timer:|r%s%u|r - in seconds", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].s_timer); //  , GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s************************************************************|r", sGGW->GWCOMM[guild_id].color_9.c_str());
				}
				msg = GGW_MSG;
			};

/*
-- *************
-- Info Commands
-- *************
*/
			if ((ChatCache[1] == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].setup)) // && (ChatCache[2] != ""))
			{
				ChatHandler(player->GetSession()).PSendSysMessage("%s****************************", sGGW->GWCOMM[guild_id].color_9);
				ChatHandler(player->GetSession()).PSendSysMessage("%sSystem setup details:", sGGW->GWCOMM[guild_id].color_8);
				ChatHandler(player->GetSession()).PSendSysMessage("%s-------------------------------------------", sGGW->GWCOMM[guild_id].color_9);
				ChatHandler(player->GetSession()).PSendSysMessage("%sCosts:|r", sGGW->GWCOMM[guild_id].color_8);
				ChatHandler(player->GetSession()).PSendSysMessage("%sPrices in %s%s's:|r", sGGW->GWCOMM[guild_id].color_8, sGGW->Currencyname, sGGW->GWCOMM[guild_id].color_8);
				ChatHandler(player->GetSession()).PSendSysMessage("%s---------------------------------", sGGW->GWCOMM[guild_id].color_9);

				ChatHandler(player->GetSession()).PSendSysMessage("%sZone price|r:%s%u %sbase location price.|r", sGGW->GWCOMM[guild_id].color_1, sGGW->GWCOMM[guild_id].color_2, sGGW->GWCOMM[guild_id].loc_cost, sGGW->GWCOMM[guild_id].color_3);
				ChatHandler(player->GetSession()).PSendSysMessage("%sFarm price|r:%s%u ", sGGW->GWCOMM[guild_id].color_1, sGGW->GWCOMM[guild_id].color_2, sGGW->GWCOMM[guild_id].farm_cost);
				ChatHandler(player->GetSession()).PSendSysMessage("%sBarracks price|r:%s%u ", sGGW->GWCOMM[guild_id].color_1, sGGW->GWCOMM[guild_id].color_2, sGGW->GWCOMM[guild_id].barrack_cost);
				ChatHandler(player->GetSession()).PSendSysMessage("%sHall price:%s%u ", sGGW->GWCOMM[guild_id].color_1, sGGW->GWCOMM[guild_id].color_2, sGGW->GWCOMM[guild_id].hall_cost);
				ChatHandler(player->GetSession()).PSendSysMessage("%sPig price|r:%s%u ", sGGW->GWCOMM[guild_id].color_1, sGGW->GWCOMM[guild_id].color_2, sGGW->GWCOMM[guild_id].pig_cost);
				ChatHandler(player->GetSession()).PSendSysMessage("%sGuard price|r:%s%u |r - %sThese Dont increase location price.", sGGW->GWCOMM[guild_id].color_1, sGGW->GWCOMM[guild_id].color_2, sGGW->GWCOMM[guild_id].guard_cost, sGGW->GWCOMM[guild_id].color_14);


				if (sGGW->GWCOMM[guild_id].cannon_id > 0) { ChatHandler(player->GetSession()).PSendSysMessage("%scannon price|r:%s%u", sGGW->GWCOMM[guild_id].color_1, sGGW->GWCOMM[guild_id].color_2, sGGW->GWCOMM[guild_id].cannon_cost); };
				if (sGGW->GWCOMM[guild_id].vendor1_id > 0) { ChatHandler(player->GetSession()).PSendSysMessage("%svendor1 price|r:%s%u", sGGW->GWCOMM[guild_id].color_1, sGGW->GWCOMM[guild_id].color_2, sGGW->GWCOMM[guild_id].vendor1_cost); };
				if (sGGW->GWCOMM[guild_id].vendor2_id > 0) { ChatHandler(player->GetSession()).PSendSysMessage("%svendor2 price|r:%s%u", sGGW->GWCOMM[guild_id].color_1, sGGW->GWCOMM[guild_id].color_2, sGGW->GWCOMM[guild_id].vendor2_cost); };
				if (sGGW->GWCOMM[guild_id].vendor3_id > 0) { ChatHandler(player->GetSession()).PSendSysMessage("%svendor3 price|r:%s%u", sGGW->GWCOMM[guild_id].color_1, sGGW->GWCOMM[guild_id].color_2, sGGW->GWCOMM[guild_id].vendor3_cost); };

				ChatHandler(player->GetSession()).PSendSysMessage("%sVault price|r:%s%u ", sGGW->GWCOMM[guild_id].color_1, sGGW->GWCOMM[guild_id].color_2, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vault_cost);
				ChatHandler(player->GetSession()).PSendSysMessage("%sMailbox price|r:%s%u ", sGGW->GWCOMM[guild_id].color_1, sGGW->GWCOMM[guild_id].color_2, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].mailbox_cost);
				ChatHandler(player->GetSession()).PSendSysMessage("%s-------------------------------------------", sGGW->GWCOMM[guild_id].color_9);
				ChatHandler(player->GetSession()).PSendSysMessage("%sLocation Limits:", sGGW->GWCOMM[guild_id].color_8);
				ChatHandler(player->GetSession()).PSendSysMessage("%sFarm limit|r:%s%u %sper location.|r", sGGW->GWCOMM[guild_id].color_1, sGGW->GWCOMM[guild_id].color_2, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].farm_L, sGGW->GWCOMM[guild_id].color_3);
				ChatHandler(player->GetSession()).PSendSysMessage("%sPig limit|r:%s%u %sper farm. Require a farm.|r", sGGW->GWCOMM[guild_id].color_1, sGGW->GWCOMM[guild_id].color_2, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_L, sGGW->GWCOMM[guild_id].color_3);
				ChatHandler(player->GetSession()).PSendSysMessage("%sBarrack limit|r:%s%u %sper farm. Require a farm.|r", sGGW->GWCOMM[guild_id].color_1, sGGW->GWCOMM[guild_id].color_2, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].barrack_L, sGGW->GWCOMM[guild_id].color_3);
				ChatHandler(player->GetSession()).PSendSysMessage("%sGuard limit|r:%s%u %sper barrack. Require a barrack.|r", sGGW->GWCOMM[guild_id].color_1, sGGW->GWCOMM[guild_id].color_2, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].guard_L, sGGW->GWCOMM[guild_id].color_3);
				ChatHandler(player->GetSession()).PSendSysMessage("%sHall limit|r:%s%u %sper location. Require a barrack.|r", sGGW->GWCOMM[guild_id].color_1, sGGW->GWCOMM[guild_id].color_2, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].hall_L, sGGW->GWCOMM[guild_id].color_3);

					if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor1_id > 0) { ChatHandler(player->GetSession()).PSendSysMessage("%sVendor1 limit|r:%s%u %sper location. Require a hall.|r", sGGW->GWCOMM[guild_id].color_1, sGGW->GWCOMM[guild_id].color_2, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor1_L, sGGW->GWCOMM[guild_id].color_3); };
					if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor2_id > 0) { ChatHandler(player->GetSession()).PSendSysMessage("%sVendor2 limit|r:%s%u %sper location. Require a hall.|r", sGGW->GWCOMM[guild_id].color_1, sGGW->GWCOMM[guild_id].color_2, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor2_L, sGGW->GWCOMM[guild_id].color_3); };
					if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor3_id > 0) { ChatHandler(player->GetSession()).PSendSysMessage("%sVendor3 limit|r:%s%u %sper location. Require a hall.|r", sGGW->GWCOMM[guild_id].color_1, sGGW->GWCOMM[guild_id].color_2, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor3_L, sGGW->GWCOMM[guild_id].color_3); };
					if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].cannon_id > 0) { ChatHandler(player->GetSession()).PSendSysMessage("%sCannon limit|r:%s%u %sper hall. Require a hall.|r", sGGW->GWCOMM[guild_id].color_1, sGGW->GWCOMM[guild_id].color_2, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].cannon_L, sGGW->GWCOMM[guild_id].color_3); };

				ChatHandler(player->GetSession()).PSendSysMessage("%sGuild Vault limit|r:%s%u %sper location. Require a hall.|r", sGGW->GWCOMM[guild_id].color_1, sGGW->GWCOMM[guild_id].color_2, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vault_L, sGGW->GWCOMM[guild_id].color_3);
				ChatHandler(player->GetSession()).PSendSysMessage("%sGuild MailBox limit|r:%s%u %sper location. Require a hall.|r", sGGW->GWCOMM[guild_id].color_1, sGGW->GWCOMM[guild_id].color_2, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].mailbox_L, sGGW->GWCOMM[guild_id].color_3);
				ChatHandler(player->GetSession()).PSendSysMessage("%s-------------------------------------------", sGGW->GWCOMM[guild_id].color_9);

				ChatHandler(player->GetSession()).PSendSysMessage("%sRewards:|r", sGGW->GWCOMM[guild_id].color_8);
				ChatHandler(player->GetSession()).PSendSysMessage("%sPig Payz|r:%s%u  %scopper every %s%u %sminutes.", sGGW->GWCOMM[guild_id].color_1, sGGW->GWCOMM[guild_id].color_2, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_payz, sGGW->GWCOMM[guild_id].color_3, sGGW->GWCOMM[guild_id].color_2, ((sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_payz_timer / 1000) / 60), sGGW->GWCOMM[guild_id].color_3);
				ChatHandler(player->GetSession()).PSendSysMessage("%sNew Guild Gift amount|r:%s%u %s%s%s's.", sGGW->GWCOMM[guild_id].color_1, sGGW->GWCOMM[guild_id].color_2, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].gift_count, sGGW->GWCOMM[guild_id].color_8, sGGW->Currencyname, sGGW->GWCOMM[guild_id].color_8);
				ChatHandler(player->GetSession()).PSendSysMessage("%s-------------------------------------------", sGGW->GWCOMM[guild_id].color_9);
		
				ChatHandler(player->GetSession()).PSendSysMessage("%sSystem features:|r", sGGW->GWCOMM[guild_id].color_8);

					if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].flag_require != 0) { ChatHandler(player->GetSession()).PSendSysMessage("%sFlag require|r: %sEnabled. %sAll guards MUST be cleared before a flag can be tagged.|r", sGGW->GWCOMM[guild_id].color_1, sGGW->GWCOMM[guild_id].color_2, sGGW->GWCOMM[guild_id].color_3); };
					if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].anarchy != 0) { ChatHandler(player->GetSession()).PSendSysMessage("%sAnarchy|r: %sEnabled. |r%sSame faction invasions allowed.|r", sGGW->GWCOMM[guild_id].color_1, sGGW->GWCOMM[guild_id].color_2, sGGW->GWCOMM[guild_id].color_3); };

					if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].f_timer != 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%sNo-Tag-Flag:|r Enabled%s. un-taggable flag Cooldown timer.", sGGW->GWCOMM[guild_id].color_1, sGGW->GWCOMM[guild_id].color_3);
						ChatHandler(player->GetSession()).PSendSysMessage("%sNo-Tag-Flag-Timer Duration|r:%s%u|r - %sin seconds.", sGGW->GWCOMM[guild_id].color_1, sGGW->GWCOMM[guild_id].color_2, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].s_timer, sGGW->GWCOMM[guild_id].color_3);
					}

				ChatHandler(player->GetSession()).PSendSysMessage("%s-------------------------------------------", sGGW->GWCOMM[guild_id].color_9);
				ChatHandler(player->GetSession()).PSendSysMessage("%s****************************", sGGW->GWCOMM[guild_id].color_9);

				msg = GGW_MSG;
			}

			if (ChatCache[1] == sGGW->GWCOMM[guild_id].info_loc)
			{
				ChatHandler(player->GetSession()).PSendSysMessage("%s---------------------------------", sGGW->GWCOMM[guild_id].color_9.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%sLocation details:", sGGW->GWCOMM[guild_id].color_8.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s---------------------------------", sGGW->GWCOMM[guild_id].color_9.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%sLoc ID:|r %s%u|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWARZ[LocId].entry);

				if (sGGW->GWARZ[LocId].team == 0) ChatHandler(player->GetSession()).PSendSysMessage("%sOwner|r: %s %s|r.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_4.c_str(), sGGW->GWARZ[LocId].guild_name.c_str());
				if (sGGW->GWARZ[LocId].team == 1) ChatHandler(player->GetSession()).PSendSysMessage("%sOwner|r: %s %s|r.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_5.c_str(), sGGW->GWARZ[LocId].guild_name.c_str());
				if (sGGW->GWARZ[LocId].team == 2) ChatHandler(player->GetSession()).PSendSysMessage("%sOwner|r: %s %s|r.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_6.c_str(), sGGW->GWARZ[LocId].guild_name.c_str());
				if (sGGW->GWARZ[LocId].team == 3) ChatHandler(player->GetSession()).PSendSysMessage("%sOwner|r: %s %s|r.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_7.c_str(), sGGW->GWARZ[LocId].guild_name.c_str());
					
				if (sGGW->GWARZ[LocId].team == 0) ChatHandler(player->GetSession()).PSendSysMessage("%sFaction|r: %sAlliance.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_4.c_str());
				if (sGGW->GWARZ[LocId].team == 1) ChatHandler(player->GetSession()).PSendSysMessage("%sFaction|r: %sHorde.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_5.c_str());
				if (sGGW->GWARZ[LocId].team == 2) ChatHandler(player->GetSession()).PSendSysMessage("%sFaction|r: %sFor Sale.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_6.c_str());
				if (sGGW->GWARZ[LocId].team == 3) ChatHandler(player->GetSession()).PSendSysMessage("%sFaction|r: %sLOCKED|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_7.c_str());

				if (sGGW->GWARZ[LocId].team < 2) ChatHandler(player->GetSession()).PSendSysMessage("%sGuild Rank|r: %s#%u", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_7.c_str(), sGGW->GetRank(sGGW->GWARZ[LocId].guild_id));
				if (sGGW->GWARZ[LocId].team < 2) ChatHandler(player->GetSession()).PSendSysMessage("%s---------------------------------", sGGW->GWCOMM[guild_id].color_9.c_str());

				ChatHandler(player->GetSession()).PSendSysMessage("%sFarms|r: %s%u|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWARZ[LocId].farm_count);
				ChatHandler(player->GetSession()).PSendSysMessage("%sHall|r: %s%u|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWARZ[LocId].hall_count);
				ChatHandler(player->GetSession()).PSendSysMessage("%spigs|r: %s%u|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWARZ[LocId].pig_count);
				ChatHandler(player->GetSession()).PSendSysMessage("%sValue:%u %s's:|r", sGGW->GWCOMM[guild_id].color_1.c_str(), uint32(Zoneprice), sGGW->Currencyname.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s---------------------------------", sGGW->GWCOMM[guild_id].color_9.c_str());

				msg = GGW_MSG;
			}

/*
-- ************
-- List Command
-- ************
*/

				if (ChatCache[1] == sGGW->GWCOMM[guild_id].list_loc)
				{
					uint32 Gloc;
					uint32 yentry = 0;
					uint32 ypigcnt = 0;
					uint32 yvalue = 0;
					uint32 Xzoneprice = 0;

					ChatHandler(player->GetSession()).PSendSysMessage("%s**********************************************************************", sGGW->GWCOMM[guild_id].color_9.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sLoc ID:     farm:     barracks:     Hall:     Guards:     Pigs:          Zone value:|r", sGGW->GWCOMM[guild_id].color_1.c_str());

					for (Gloc = 0; Gloc < sGGW->GWARZ.size(); Gloc++)do
					{
						if (sGGW->GWARZ[Gloc].guild_id == guild_id)
						{
							Xzoneprice = sGGW->CalculateLocationValue(Gloc);

							ChatHandler(player->GetSession()).PSendSysMessage("%s%u             %u             %u             %u             %u             %u                  %u|r", sGGW->GWCOMM[guild_id].color_2.c_str(), Gloc, sGGW->GWARZ[Gloc].farm_count, sGGW->GWARZ[Gloc].barrack_count, sGGW->GWARZ[Gloc].hall_count, sGGW->GWARZ[Gloc].guard_count, sGGW->GWARZ[Gloc].pig_count, Xzoneprice);

							yentry++;
							ypigcnt = ypigcnt + sGGW->GWARZ[Gloc].pig_count;
							yvalue = yvalue + Xzoneprice;
						}
						Gloc = Gloc + 1;
					} while (Gloc < sGGW->GWARZ.size());

					if (yentry == 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("*                 - Your guild does not own any land -               *");
						ChatHandler(player->GetSession()).PSendSysMessage("%s**********************************************************************", sGGW->GWCOMM[guild_id].color_9.c_str());
					}

					if (yentry > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%s**********************************************************************", sGGW->GWCOMM[guild_id].color_9.c_str());
						ChatHandler(player->GetSession()).PSendSysMessage("%stotal locations: %s%u|r      %stotal pigs:|r %s%u|r      %sTotal value:|r %s%u|r %s%s's.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), uint32(yentry), sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), uint32(ypigcnt), sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), uint32(yvalue), sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->Currencyname.c_str());
						ChatHandler(player->GetSession()).PSendSysMessage("%s**********************************************************************", sGGW->GWCOMM[guild_id].color_9.c_str());
					}
					msg = GGW_MSG;
				}

/*
-- ************
-- Rank Command
-- ************
*/

				if (ChatCache[1] == GUILD_RANK_COMMAND)
				{
					uint32 PGR = sGGW->GetRank(guild_id);
					uint32 rank = 1;
					uint32 size = sGGW->GW_Ranks.size();
					uint32 max;

					if (size <= sGGW->GUILDWARZ_RANKING_MAX){ max = size; };
					if (size > sGGW->GUILDWARZ_RANKING_MAX){ max = sGGW->GUILDWARZ_RANKING_MAX; };

					if (test){ TC_LOG_INFO("server.loading", "<GUILDWARZ_RANKING_MAX>%u <SIZE>%u <SET_SIZE>%u", sGGW->GUILDWARZ_RANKING_MAX, size, max); };

					ChatHandler(player->GetSession()).PSendSysMessage("%s****************************************", sGGW->GWCOMM[guild_id].color_9.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sRank   :   Score   :   Guild|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_1.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s****************************************", sGGW->GWCOMM[guild_id].color_9.c_str());

					for (rank = 1; rank <= max; rank++)do
					{
						uint32 GID = sGGW->GW_Ranks[rank].guild_id;
						uint8 GTID = sGGW->GW_Ranks[rank].team;
						uint32 GR = rank; //  GetRank(GID);

						if (GTID == 0) { ChatHandler(player->GetSession()).PSendSysMessage("%s%u             %s%u            %s%s", sGGW->GWCOMM[guild_id].color_1.c_str(), GR, sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GW_Ranks[rank].total_gross_worth, sGGW->GWCOMM[GID].color_4.c_str(), sGGW->GW_Ranks[rank].name.c_str()); };
						if (GTID == 1) { ChatHandler(player->GetSession()).PSendSysMessage("%s%u             %s%u            %s%s", sGGW->GWCOMM[guild_id].color_1.c_str(), GR, sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GW_Ranks[rank].total_gross_worth, sGGW->GWCOMM[GID].color_5.c_str(), sGGW->GW_Ranks[rank].name.c_str()); };

						rank = rank + 1;

					} while (rank <= max);

					ChatHandler(player->GetSession()).PSendSysMessage("%s****************************************", sGGW->GWCOMM[guild_id].color_9.c_str());
	
					if (pTeam_id == 0) { ChatHandler(player->GetSession()).PSendSysMessage("%sYour Guild|r `%s%s|r`%s is Ranked|r #%u.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_4.c_str(), guild_name.c_str(), sGGW->GWCOMM[guild_id].color_1.c_str(), PGR); };
					if (pTeam_id == 1) { ChatHandler(player->GetSession()).PSendSysMessage("%sYour Guild|r `%s%s|r`%s is Ranked|r #%u.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_5.c_str(), guild_name.c_str(), sGGW->GWCOMM[guild_id].color_1.c_str(), PGR); };
					
					ChatHandler(player->GetSession()).PSendSysMessage("%s****************************************", sGGW->GWCOMM[guild_id].color_9.c_str());
					msg = GGW_MSG;
				}
				
/*
-- ***************
-- Version Command
-- ***************
*/

				if (ChatCache[1] == sGGW->GWCOMM[guild_id].version)
				{
					ChatHandler(player->GetSession()).PSendSysMessage("%s****************************************", sGGW->GWCOMM[guild_id].color_9.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sGrumbo'z Guild Warz. VER :|r %.2fc|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GW_version);
					ChatHandler(player->GetSession()).PSendSysMessage("%sCore ::|r %.2f.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->core_version);
					ChatHandler(player->GetSession()).PSendSysMessage("%sPigPayz ::|r %.2f.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->pigpayz_version);
					ChatHandler(player->GetSession()).PSendSysMessage("%sTeleport ::|r %.2f.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->tele_version);
					ChatHandler(player->GetSession()).PSendSysMessage("%sPvP ::|r %.2f.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->pvp_version);
					ChatHandler(player->GetSession()).PSendSysMessage("%sVendor ::|r %.2f.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->vendor_version);
					ChatHandler(player->GetSession()).PSendSysMessage("%s****************************************", sGGW->GWCOMM[guild_id].color_9.c_str());
					msg = GGW_MSG;
				}

/*
-- ***********
-- Set Command
-- ***********
*/

				if (ChatCache[1] == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].command_set)
				{
					if (ChatCache[3] == "")
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%sCMD ERROR:|r %s %s NO data entered.", sGGW->GWCOMM[guild_id].color_15.c_str(), ChatCache[1].c_str(), sGGW->GWCOMM[guild_id].color_15.c_str());
	
						if (ChatCache[2] == "") 
							ChatHandler(player->GetSession()).PSendSysMessage("%sType|r `/g help set` %sfor a list of settings you can adjust.|r", sGGW->GWCOMM[guild_id].color_10.c_str(), sGGW->GWCOMM[guild_id].color_10.c_str());
						else
						{
							ChatHandler(player->GetSession()).PSendSysMessage("%sType `/g help set %s for help with changing the value.", sGGW->GWCOMM[guild_id].color_10.c_str(), ChatCache[2].c_str());
						};

						return;
					}

					if ((ChatCache[2] != "")&&(ChatCache[3] != ""))
					{
						for (uint64 name2 = 0; name2 < sGGW->GWHELP.size(); ++name2) do
						{
							if (ChatCache[2] == sGGW->GWHELP[name2].name)
							{
								if ((sGGW->GWHELP[name2].command_level <= 4)&&(pGuildRank == 0))
								{
									if (test){ TC_LOG_INFO("server.loading", "<GUILD-MASTER_SET_COMMAND> %s %s %s %u", ChatCache[1].c_str(), ChatCache[2].c_str(), ChatCache[3].c_str(), guild_id); };
										
									sGGW->UpdateGuildCommandData(ChatCache[2], ChatCache[3].c_str(), guild_id);

									ChatHandler(player->GetSession()).PSendSysMessage("guild cmd %s set to `%s`.", ChatCache[2].c_str(), ChatCache[3].c_str());

									msg = GGW_MSG;
									break;
								}

								if (sGGW->GWHELP[name2].command_level <= 4 && pGuildRank > 0)
								{
									ChatHandler(player->GetSession()).PSendSysMessage("%sGLD CMD ACCESS ERROR.:|r %u", sGGW->GWCOMM[guild_id].color_15.c_str(), pGuildRank);
									ChatHandler(player->GetSession()).PSendSysMessage("%sCMD ERROR:|r %s %s %s", sGGW->GWCOMM[guild_id].color_15.c_str(), ChatCache[1].c_str(), ChatCache[2].c_str(), ChatCache[3].c_str());

									msg = GGW_MSG;
									break;
								}

								if (pGMRank >= sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].GM_admin && ((sGGW->GWHELP[name2].command_level == 5) || (sGGW->GWHELP[name2].command_level == 6)))
								{
									if (test){ TC_LOG_INFO("server.loading", "<GUILD-MASTER_SET_COMMAND> %s %s %s", ChatCache[1].c_str(), ChatCache[2].c_str(), ChatCache[3].c_str()); };

									sGGW->UpdateGuildCommandData(ChatCache[2].c_str(), ChatCache[3].c_str(), sGGW->SERVER_GUILD_ID);
										
									ChatHandler(player->GetSession()).PSendSysMessage("%sGM cmd %s set to `%s`|r.", sGGW->GWCOMM[guild_id].color_10.c_str(), ChatCache[2].c_str(), ChatCache[3].c_str());

									msg = GGW_MSG;
									break;
								}

								if (pGMRank != sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].GM_admin && (sGGW->GWHELP[name2].command_level == 5 || sGGW->GWHELP[name2].command_level == 6))
								{
									ChatHandler(player->GetSession()).PSendSysMessage("%sGM CMD ACCESS ERROR.: |r%u", sGGW->GWCOMM[guild_id].color_15.c_str(), pGMRank);
									ChatHandler(player->GetSession()).PSendSysMessage("%sCMD ERROR:|r %s %s %s %s.", sGGW->GWCOMM[guild_id].color_15.c_str(), ChatCache[1].c_str(), ChatCache[2].c_str(), ChatCache[3].c_str(), ChatCache[4].c_str());
									ChatHandler(player->GetSession()).PSendSysMessage("<%sGrumbo|r>%sIt's really no big deal.. you just dont have access to some features of `GGWz`.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_1.c_str());
									break;
								}
							}
							name2++;
						} while (name2 < sGGW->GWHELP.size());
					}
				}

/*
-- ****************
-- Help Set Command
-- ****************
*/

				if (ChatCache[1] == "help")
				{
					if (ChatCache[2] == "")
					{
						ChatHandler(player->GetSession()).PSendSysMessage("err...");
					}

					if (ChatCache[2] == sGGW->GWCOMM[guild_id].command_set && ChatCache[3]=="")
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%scommand id      -     description", sGGW->GWCOMM[guild_id].color_10.c_str());

						for (uint64 hlpsub = 0;  hlpsub < sGGW->GWHELP.size(); ++hlpsub) do
						{
							if (pGuildRank == 0 && sGGW->GWHELP[hlpsub].command_level <= 4)
							{
								ChatHandler(player->GetSession()).PSendSysMessage("%s%s     -     %s%s|r", sGGW->GWCOMM[guild_id].color_10.c_str(), sGGW->GWHELP[hlpsub].name.c_str(), sGGW->GWCOMM[guild_id].color_11.c_str(), sGGW->GWHELP[hlpsub].description.c_str());
							}

							if (pGMRank == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].GM_admin && sGGW->GWHELP[hlpsub].command_level >= 5 && sGGW->GWHELP[hlpsub].command_level <= 6)
							{
								ChatHandler(player->GetSession()).PSendSysMessage("%s%s - %s%s|r", sGGW->GWCOMM[guild_id].color_10.c_str(), sGGW->GWHELP[hlpsub].name.c_str(), sGGW->GWCOMM[guild_id].color_11.c_str(), sGGW->GWHELP[hlpsub].description.c_str());
							}
							hlpsub++;
						} while (hlpsub < sGGW->GWHELP.size());
					}

					if (ChatCache[2] == sGGW->GWCOMM[guild_id].command_set && ChatCache[3] != "")
					{
						for (uint8 name = 1; name < sGGW->GWHELP.size(); ++name) do
						{
							if (ChatCache[3] == sGGW->GWHELP[name].name)
							{
								if (pGuildRank == 0 && sGGW->GWHELP[name].command_level <= 4)
								{
									ChatHandler(player->GetSession()).PSendSysMessage("%s%s      -      %s|r", sGGW->GWCOMM[guild_id].color_10.c_str(), sGGW->GWHELP[name].name.c_str(), sGGW->GWHELP[name].description.c_str());
									ChatHandler(player->GetSession()).PSendSysMessage("%sExample:|r /g %s %s", sGGW->GWCOMM[guild_id].color_10.c_str(), sGGW->GWCOMM[guild_id].color_11.c_str(), sGGW->GWHELP[name].example.c_str());
									break;
								}

								if (pGMRank == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].GM_admin && sGGW->GWHELP[name].command_level >= 5 && sGGW->GWHELP[name].command_level <= 6)
								{
									ChatHandler(player->GetSession()).PSendSysMessage("%s%s      -      %s|r", sGGW->GWCOMM[guild_id].color_10.c_str(), sGGW->GWHELP[name].name.c_str(), sGGW->GWHELP[name].description.c_str());
									ChatHandler(player->GetSession()).PSendSysMessage("%sExample:|r /g %s%s", sGGW->GWCOMM[guild_id].color_10.c_str(), sGGW->GWCOMM[guild_id].color_11.c_str(), sGGW->GWHELP[name].example.c_str());
									break;
								}
							}
							name++;
						} while (name < sGGW->GWHELP.size());
					}
				}

// ****************************************************
// ************ CORE: Guild Master Commands ***********
// ****************************************************

// ****************************************************
// ******************* Buy commands *******************

				if (pGuildRank <= sGGW->GWCOMM[guild_id].GLD_lvlb && ChatCache[1] == "buy")
				{
					if(sGGW->GWCOMM[guild_id].allowed >= 1)
					{
						if (player->IsGameMaster())
						{
							ChatHandler(player->GetSession()).PSendSysMessage("%sYou Must exit GM mode first to use this command.", sGGW->GWCOMM[guild_id].color_15.c_str());
						}
					
						if (sGGW->GWARZ[LocId].team == sGGW->SERVER_GUILD_TEAM_LOCKED_ID)
						{
							ChatHandler(player->GetSession()).PSendSysMessage("%sTHIS AREA IS OFF LIMITS.|r", sGGW->GWCOMM[guild_id].color_15.c_str());
						};

						if (!player->IsGameMaster())
						{

	/*
	-- **** Location ****
	*/

							if (ChatCache[2] == sGGW->GWCOMM[guild_id].loc)
							{
								if ((sGGW->GWARZ[LocId].guild_id != sGGW->SERVER_GUILD_ID) || (sGGW->GWARZ[LocId].guild_id == guild_id))
								{
									ChatHandler(player->GetSession()).PSendSysMessage("%sYou cannot purchase this area.", sGGW->GWCOMM[guild_id].color_15.c_str());
									ChatHandler(player->GetSession()).PSendSysMessage("%s%s|r owns this area.", sGGW->GWCOMM[guild_id].color_14.c_str(), sGGW->GWARZ[LocId].guild_name.c_str());
								}

								if (sGGW->GWARZ[LocId].guild_id == sGGW->SERVER_GUILD_ID)
								{
									if (sGGW->GWARZ[LocId].team == 2)
									{
										if (player->GetItemCount(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency) < Zoneprice)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou do not have enough %s's.", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->Currencyname.c_str());
										};

										if (player->GetItemCount(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency) >= Zoneprice) // Flag spawn is handled uniquely since i need to store its GUIDLow();
										{
											uint32 spawnflagGUID = SpawnGuildObjects(2, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].flag_id, pTeam_id, guild_id, Pmap, pX, pY, pZ, pO, player, LocId);

											if (!spawnflagGUID)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%sflag spawn error..|r Try again.", sGGW->GWCOMM[guild_id].color_15.c_str());
											};

											if (spawnflagGUID)
											{

												player->DestroyItemCount(uint32(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency), Zoneprice, true);

			
												ChatHandler(player->GetSession()).PSendSysMessage("-%u %s's.|r", uint32(Zoneprice), sGGW->Currencyname.c_str());

												if (test){ TC_LOG_INFO("server.loading", "GUILD_ID:%u FLAG_GUID:%u TEAM_ID:%u LOC_ID:%u", guild_id, spawnflagGUID, pTeam_id, LocId); };
												if (test){ TC_LOG_INFO("server.loading", "GUILD_ID:%u FLAG_GUID:%u TEAM_ID:%u LOC_ID:%u", guild_id, spawnflagGUID, pTeam_id, sGGW->GWARZ[LocId].entry); };

											
												sGGW->UpdateGuildLocData("guild_name", guild->GetName(), LocId);
												sGGW->UpdateGuildLocData("team", sGGW->ConvertNumberToString(pTeam_id), LocId);
												sGGW->UpdateGuildLocData("flag_id", sGGW->ConvertNumberToString(spawnflagGUID), LocId);
												sGGW->UpdateGuildLocData("fs_time", sGGW->ConvertNumberToString(GameTime::GetGameTime()), LocId);
												sGGW->UpdateGuildLocData("guild_id", sGGW->ConvertNumberToString(guild_id), LocId);

												sGGW->UpdateGuildLocFloat(pX, pY, pZ, LocId);

												std::string ann = sGGW->GWCOMM[guild_id].color_1 + pName + " has added location:" + str_LocId + " to your Guild Lands.";

												sGGW->SendGuildMessage(sGGW->GWARZ[LocId].guild_id, ann);

													if (sGGW->GUILDWARZ_RANK_TYPE > 1) { sGGW->CreateRankList(); };
											
													if (player->GetGender() == 0)
													{
														ChatHandler(player->GetSession()).PSendSysMessage("%sCongratulations King %s. you have expanded %s's land.|r", sGGW->GWCOMM[guild_id].color_14.c_str(), pName.c_str(), guild_name.c_str());
													};

													if (player->GetGender() == 1)
													{
														ChatHandler(player->GetSession()).PSendSysMessage("%sCongratulations Queen %s. you have expanded %s's land.|r", sGGW->GWCOMM[guild_id].color_14.c_str(), pName.c_str(), guild_name.c_str());
													};

													msg = GGW_MSG;
											}
										}
									}
								}
							};

	/*
	-- **** Farm ****
	*/

							if (sGGW->GWARZ[LocId].guild_id != guild_id)
							{
								ChatHandler(player->GetSession()).PSendSysMessage("%sYour Guild does not own this land.", sGGW->GWCOMM[guild_id].color_15.c_str());
							};

							if (sGGW->GWARZ[LocId].guild_id == guild_id)
							{
								if (ChatCache[2] == sGGW->GWCOMM[guild_id].farm)
								{
									if (player->GetItemCount(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency) < sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].farm_cost)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sYou require more %s's.", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->Currencyname.c_str());
									}

									if (player->GetItemCount(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency) >= sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].farm_cost)
									{
										if (sGGW->GWARZ[LocId].farm_count >= sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].farm_L)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou have %u farm`s at this location.", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->GWARZ[LocId].farm_count);
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou can only purchase %u farm`s per location.", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].farm_L);
										};

										if (sGGW->GWARZ[LocId].farm_count < sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].farm_L)
										{
											if (sGGW->GWARZ[LocId].pig_count < (sGGW->GWARZ[LocId].farm_count * sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_L))
											{
												ChatHandler(player->GetSession()).PSendSysMessage("you still need to finish populating your other farms.", sGGW->GWCOMM[guild_id].color_15.c_str());
											};

											if (sGGW->GWARZ[LocId].pig_count >= (sGGW->GWARZ[LocId].farm_count * sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_L))
											{
												uint32 spawnfarm = SpawnGuildObjects(1, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].farm_id, pTeam_id, guild_id, Pmap, pX, pY, pZ, pO, player, LocId);

												if (!spawnfarm)
												{
													ChatHandler(player->GetSession()).PSendSysMessage("%sFarm spawn error..|r Try again.", sGGW->GWCOMM[guild_id].color_15.c_str());
												};

												if (spawnfarm)
												{
													player->DestroyItemCount(uint32(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].farm_cost, true);

													ChatHandler(player->GetSession()).PSendSysMessage("-%u %s's.|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].farm_cost, sGGW->Currencyname.c_str());

													sGGW->UpdateGuildLocData("farm_count", sGGW->ConvertNumberToString(sGGW->GWARZ[LocId].farm_count + 1), LocId);

													std::string ann = pName + " has added a farm to location:" + str_LocId + ".";

													sGGW->SendGuildMessage(sGGW->GWARZ[LocId].guild_id, ann);
													msg = GGW_MSG;
												}
											}
										}
									}
								}

	/*
	-- **** Barrack ****
	*/

								if (ChatCache[2] == sGGW->GWCOMM[guild_id].barrack)
								{
									if (player->GetItemCount(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency) < sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].barrack_cost)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sYou require more %s's.", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->Currencyname.c_str());
									}

									if (player->GetItemCount(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency) >= sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].barrack_cost)
									{
										if (sGGW->GWARZ[LocId].barrack_count >= uint32(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].farm_L * sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].barrack_L))
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou have %u barrack\'s at this location.", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->GWARZ[LocId].barrack_count);
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou can only purchase %u barrack\'s per location.", sGGW->GWCOMM[guild_id].color_15.c_str(), (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].farm_L * sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].barrack_L));
										};

										if (sGGW->GWARZ[LocId].barrack_count < uint32(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].farm_L * sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].barrack_L))
										{
											if (sGGW->GWARZ[LocId].barrack_count >= sGGW->GWARZ[LocId].farm_count * sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].barrack_L)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%sEach farm supports %u barracks. You need another farm.", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].barrack_L);
											};

											if (sGGW->GWARZ[LocId].barrack_count < (sGGW->GWARZ[LocId].farm_count * sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].barrack_L))
											{
												uint32 spawnbarrack = SpawnGuildObjects(1, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].barrack_id, pTeam_id, guild_id, Pmap, pX, pY, pZ, pO, player, LocId);

												if (!spawnbarrack)
												{
													ChatHandler(player->GetSession()).PSendSysMessage("%sBarrack spawn error..|r Try again.", sGGW->GWCOMM[guild_id].color_15.c_str());
												};

												if (spawnbarrack)
												{
													player->DestroyItemCount(uint32(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].barrack_cost, true);

													ChatHandler(player->GetSession()).PSendSysMessage("-%u %s's.|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].barrack_cost, sGGW->Currencyname.c_str());

													sGGW->UpdateGuildLocData("barrack_count", sGGW->ConvertNumberToString(sGGW->GWARZ[LocId].barrack_count + 1), LocId);

													std::string ann = pName + " has added a bararck's to location:" + str_LocId + ".";

													sGGW->SendGuildMessage(sGGW->GWARZ[LocId].guild_id, ann);
													msg = GGW_MSG;
												}
											}
										}
									}
								}

	/*
	-- **** Hall ****
	*/

								if (ChatCache[2] == sGGW->GWCOMM[guild_id].hall)
								{
									if (player->GetItemCount(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency) < sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].hall_cost)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sEach hall costs %u %s. You dont have eough.", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].hall_cost, sGGW->Currencyname.c_str());
									}

									if (player->GetItemCount(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency) >= sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].hall_cost)
									{
										if (sGGW->GWARZ[LocId].hall_count >= sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].hall_L)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou have %u hall at this location.", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->GWARZ[LocId].hall_count);
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou can only have %u hall per area.", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].hall_L);
										}

										if (sGGW->GWARZ[LocId].hall_count < sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].hall_L)
										{
											if (sGGW->GWARZ[LocId].barrack_count == 0)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%sEach Hall require's 1 barracks per location to provide guards for defensive support.", sGGW->GWCOMM[guild_id].color_15.c_str());
												ChatHandler(player->GetSession()).PSendSysMessage("%sGotta protect your HQ.", sGGW->GWCOMM[guild_id].color_15.c_str());
											}

											if (sGGW->GWARZ[LocId].barrack_count > 0)
											{
												uint32 spawnhall = SpawnGuildObjects(1, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].hall_id, pTeam_id, guild_id, Pmap, pX, pY, pZ, pO, player, LocId);

												if (!spawnhall)
												{
													ChatHandler(player->GetSession()).PSendSysMessage("%sHall spawn error..|r", sGGW->GWCOMM[guild_id].color_15.c_str());
												};

												if (spawnhall)
												{
													player->DestroyItemCount(uint32(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].hall_cost, true);

													ChatHandler(player->GetSession()).PSendSysMessage("-%u %s's.|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].hall_cost, sGGW->Currencyname.c_str());

													sGGW->UpdateGuildLocData("hall_count", sGGW->ConvertNumberToString(sGGW->GWARZ[LocId].hall_count + 1), LocId);

													std::string ann = pName + " has added a hall to location:" + str_LocId + ".";

													sGGW->SendGuildMessage(sGGW->GWARZ[LocId].guild_id, ann);
													msg = GGW_MSG;
												}
											}
										}
									}
								} // hall

	/*
	-- **** Pigs ****
	*/

								if (ChatCache[2] == sGGW->GWCOMM[guild_id].pig)
								{
									if (player->GetItemCount(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency) < sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_cost)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sEach pig costs %u %s.", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_cost, sGGW->Currencyname.c_str());
									}

									if (player->GetItemCount(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency) >= sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_cost)
									{
										if (sGGW->GWARZ[LocId].pig_count >= uint32(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_L * sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].farm_L))
										{
											ChatHandler(player->GetSession()).PSendSysMessage("You have %u pigs at this location.", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->GWARZ[LocId].pig_count * sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].farm_L);
											ChatHandler(player->GetSession()).PSendSysMessage("You can only have %u pig's per farm and %u farm's per location.", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_L, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].farm_L);
										}

										if (sGGW->GWARZ[LocId].farm_count == 0)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must first have a farm here before you can add pigs.", sGGW->GWCOMM[guild_id].color_15.c_str());
											ChatHandler(player->GetSession()).PSendSysMessage("%sPiggies gotta live somewhere...", sGGW->GWCOMM[guild_id].color_15.c_str());
										}

										if (sGGW->GWARZ[LocId].pig_count >= (sGGW->GWARZ[LocId].farm_count * sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_L))
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou require another farm before you can add any more pigs.", sGGW->GWCOMM[guild_id].color_15.c_str());
										}

										if (sGGW->GWARZ[LocId].pig_count < uint32(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_L * sGGW->GWARZ[LocId].farm_count) && sGGW->GWARZ[LocId].farm_count != 0) // (sGGW->GWARZ[LocId].pig_count < uint32(sGGW->GWCOMM[SERVER_GUILD_ID].pig_L * GWCOMM[SERVER_GUILD_ID].farm_L) && GWARZ[LocId].farm_count != 0)
										{
											uint32 spawnpig = SpawnGuildObjects(0, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_id, pTeam_id, guild_id, Pmap, pX, pY, pZ, pO, player, LocId);

											if (!spawnpig)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%spig spawn error..|r", sGGW->GWCOMM[guild_id].color_15.c_str());
											}

											if (spawnpig)
											{
												player->DestroyItemCount(uint32(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency), uint32(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_cost), true);

												ChatHandler(player->GetSession()).PSendSysMessage("-%u %s's.|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_cost, sGGW->Currencyname.c_str());

												sGGW->UpdateGuildLocData("pig_count", sGGW->ConvertNumberToString(sGGW->GWARZ[LocId].pig_count + 1), LocId);

												std::string ann = pName + " has added a pig to location:" + str_LocId + ".";

												sGGW->SendGuildMessage(sGGW->GWARZ[LocId].guild_id, ann);
												msg = GGW_MSG;
											}
										}
									}
								}

	/*
	-- **** Guard ****
	*/

								if (ChatCache[2] == sGGW->GWCOMM[guild_id].guard)
								{
									if (player->GetItemCount(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency) < sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].guard_cost)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%aEach guard costs %u %s.", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].guard_cost, sGGW->Currencyname.c_str());
									}

									if (player->GetItemCount(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency) >= sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].guard_cost)
									{
										if (sGGW->GWARZ[LocId].guard_count >= uint16(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].guard_L * (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].barrack_L * sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].farm_L)))
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou have %u guards at this location.", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->GWARZ[LocId].guard_count);
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou can only have %u per location.", sGGW->GWCOMM[guild_id].color_15.c_str(), (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].guard_L * sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].farm_L));
										}

										if (sGGW->GWARZ[LocId].guard_count < uint16(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].guard_L * (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].barrack_L * sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].farm_L)))
										{
											if (sGGW->GWARZ[LocId].guard_count >= (sGGW->GWARZ[LocId].barrack_count * sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].guard_L))
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%sYou must add another barracks to produce more guards.", sGGW->GWCOMM[guild_id].color_15.c_str());
											}

											if (sGGW->GWARZ[LocId].guard_count < (sGGW->GWARZ[LocId].barrack_count * sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].guard_L))
											{
												uint32 spawnguard = SpawnGuildObjects(0, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].guard_id, pTeam_id, guild_id, Pmap, pX, pY, pZ, pO, player, LocId);

												if (!spawnguard)
												{
													ChatHandler(player->GetSession()).PSendSysMessage("%sguard spawn error..|r", sGGW->GWCOMM[guild_id].color_15.c_str());
												}

												if (spawnguard)
												{
													player->DestroyItemCount(uint32(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency), uint32(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].guard_cost), true);

													ChatHandler(player->GetSession()).PSendSysMessage("-%u %s's.|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].guard_cost, sGGW->Currencyname.c_str());

													sGGW->UpdateGuildLocData("guard_count", sGGW->ConvertNumberToString(sGGW->GWARZ[LocId].guard_count + 1), LocId);

													std::string ann = pName + " has added a guard to location:" + str_LocId + ".";

													sGGW->SendGuildMessage(sGGW->GWARZ[LocId].guild_id, ann);
													msg = GGW_MSG;
												}
											}
										}
									}
								} // guard

	/*
	-- **** Vendor 1 ****
	*/

								if (ChatCache[2] == sGGW->GWCOMM[guild_id].vendor1)
								{
									if (!sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor1_id)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sVendor1 is NOT enabled.. Contact your Admin to enable Vendor 1", sGGW->GWCOMM[guild_id].color_15.c_str());
									};

									if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor1_id)
									{
										if (player->GetItemCount(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency) < sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor1_cost)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sEach vendor1 costs %u %s.", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor1_cost, sGGW->Currencyname.c_str());
										}

										if (sGGW->GWARZ[LocId].vendor1_count >= (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor1_L * sGGW->GWARZ[LocId].hall_count))
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou have %u vendor1's at this location.", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->GWARZ[LocId].vendor1_count);
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou can only have %u per Hall.", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor1_L);
										}

										if (sGGW->GWARZ[LocId].vendor1_count < (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor1_L * sGGW->GWARZ[LocId].hall_count))
										{
											uint32 spawnvendor1 = SpawnGuildObjects(0, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor1_id, pTeam_id, guild_id, Pmap, pX, pY, pZ, pO, player, LocId);

											if (!spawnvendor1)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%svendor1 spawn error..|r", sGGW->GWCOMM[guild_id].color_15.c_str());
											}

											if (spawnvendor1)
											{
												player->DestroyItemCount(uint32(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency), uint32(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor1_cost), true);

												ChatHandler(player->GetSession()).PSendSysMessage("-%u %s's.|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor1_cost, sGGW->Currencyname.c_str());

												sGGW->UpdateGuildLocData("vendor1_count", sGGW->ConvertNumberToString(sGGW->GWARZ[LocId].vendor1_count + 1), LocId);

												std::string ann = pName + " has added a vendor1 to location:" + str_LocId + ".";

												sGGW->SendGuildMessage(sGGW->GWARZ[LocId].guild_id, ann);
												msg = GGW_MSG;
											}
										}
									}
								} // vendor 1

	/*
	-- **** Vendor 2 ****
	*/

								if (ChatCache[2] == sGGW->GWCOMM[guild_id].vendor2)
								{
									if (!sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor2_id)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sVendor2 is NOT enabled..", sGGW->GWCOMM[guild_id].color_15.c_str());
									}

									if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor2_id)
									{
										if (player->GetItemCount(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency) < sGGW->GWCOMM[sGGW->sGGW->SERVER_GUILD_ID].vendor2_cost)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sEach vendor2 costs %u %s.", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor2_cost, sGGW->Currencyname.c_str());
										};

										if (sGGW->GWARZ[LocId].vendor2_count >= (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor2_L * sGGW->GWARZ[LocId].hall_count))
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou have %u vendor2's at this location.", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->GWARZ[LocId].vendor1_count);
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou can only have %u per location.", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->GWCOMM[sGGW->sGGW->SERVER_GUILD_ID].vendor2_L);
										};

										if (sGGW->GWARZ[LocId].vendor2_count < (sGGW->GWCOMM[sGGW->sGGW->SERVER_GUILD_ID].vendor2_L * sGGW->GWARZ[LocId].hall_count))
										{
											uint32 spawnvendor2 = SpawnGuildObjects(0, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor2_id, pTeam_id, guild_id, Pmap, pX, pY, pZ, pO, player, LocId);

											if (!spawnvendor2)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%svendor2 spawn error..|r", sGGW->GWCOMM[guild_id].color_15.c_str());
											}

											if (spawnvendor2)
											{
												player->DestroyItemCount(uint32(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency), uint32(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor2_cost), true);

												sGGW->UpdateGuildLocData("vendor2_count", sGGW->ConvertNumberToString(sGGW->GWARZ[LocId].vendor2_count + 1), LocId);

												ChatHandler(player->GetSession()).PSendSysMessage("-%u %s's.|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor2_cost, sGGW->Currencyname.c_str());

												std::string ann = pName + " has added a vendor2 to location:" + str_LocId + ".";
												msg = GGW_MSG;
											}
										}
									}
								} // vendor 2

	/*
	-- **** Vendor 3 ****
	*/

								if (ChatCache[2] == sGGW->GWCOMM[guild_id].vendor3)
								{
									if (!sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor3_id)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sVendor3 is NOT enabled..", sGGW->GWCOMM[guild_id].color_15.c_str());
									}

									if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor3_id)
									{
										if (player->GetItemCount(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency) < sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor3_cost)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sEach vendor3 costs %u %s.", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor3_cost, sGGW->Currencyname.c_str());
										};

										if (sGGW->GWARZ[LocId].vendor3_count >= (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor3_L * sGGW->GWARZ[LocId].hall_count))
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou have %u vendor3's at this location.", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->GWARZ[LocId].vendor1_count);
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou can only have %u per location.", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor2_L);
										};

										if (sGGW->GWARZ[LocId].vendor3_count < sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor3_L)
										{
											uint32 spawnvendor3 = SpawnGuildObjects(0, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor3_id, pTeam_id, guild_id, Pmap, pX, pY, pZ, pO, player, LocId);

											if (!spawnvendor3)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%svendor3 spawn error..|r", sGGW->GWCOMM[guild_id].color_15.c_str());
											};

											if (spawnvendor3)
											{
												player->DestroyItemCount(uint32(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency), uint32(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor3_cost), true);

												sGGW->UpdateGuildLocData("vendor3_count", sGGW->ConvertNumberToString(sGGW->GWARZ[LocId].vendor3_count + 1), LocId);

												ChatHandler(player->GetSession()).PSendSysMessage("-%u %s's.|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor3_cost, sGGW->Currencyname.c_str());

												std::string ann = pName + " has added a vendor3 to location:" + str_LocId + ".";

												sGGW->SendGuildMessage(sGGW->GWARZ[LocId].guild_id, ann);
												msg = GGW_MSG;
											}
										}
									}
								} // vendor 3

	/*
	-- **** Cannon ****
	*/

								if (ChatCache[2] == sGGW->GWCOMM[guild_id].cannon)
								{

									if (!sGGW->GWCOMM[sGGW->sGGW->SERVER_GUILD_ID].cannon_id)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%scannon's NOT enabled..", sGGW->GWCOMM[guild_id].color_15.c_str());
									}

									if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].cannon_id)
									{
										if (player->GetItemCount(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency) < sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].cannon_cost)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sEach cannon costs %u %s.", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].cannon_cost, sGGW->Currencyname.c_str());
										}

										if (player->GetItemCount(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency) >= sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].cannon_cost)
										{
											if (sGGW->GWARZ[LocId].cannon_count >= (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].cannon_L * sGGW->GWARZ[LocId].hall_count))
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%sYou have %u %s's at this location.|r", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->GWARZ[LocId].cannon_count, sGGW->GWCOMM[guild_id].cannon.c_str());
												ChatHandler(player->GetSession()).PSendSysMessage("%sYou can only have %u per Hall.|r", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].cannon_L);
											}

											if (sGGW->GWARZ[LocId].cannon_count < (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].cannon_L * sGGW->GWARZ[LocId].hall_count))
											{
												if (sGGW->GWARZ[LocId].hall_count == 0)
												{
													ChatHandler(player->GetSession()).PSendSysMessage("%sCannons require a Hall per location.", sGGW->GWCOMM[guild_id].color_15.c_str());
												}

												if (sGGW->GWARZ[LocId].hall_count > 0)
												{
													uint32 spawncannon = SpawnGuildObjects(0, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].cannon_id, pTeam_id, guild_id, Pmap, pX, pY, pZ, pO, player, LocId);

													if (!spawncannon)
													{
														ChatHandler(player->GetSession()).PSendSysMessage("%scannon spawn error..|r", sGGW->GWCOMM[guild_id].color_15.c_str());
													}

													if (spawncannon)
													{
														player->DestroyItemCount(uint32(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency), uint32(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].cannon_cost), true);

														ChatHandler(player->GetSession()).PSendSysMessage("-%u %s's.|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].cannon_cost, sGGW->Currencyname.c_str());

														sGGW->UpdateGuildLocData("cannon_count", sGGW->ConvertNumberToString(sGGW->GWARZ[LocId].cannon_count + 1), LocId);

														std::string ann = pName + " has added a cannon to location:" + str_LocId + ".";

														sGGW->SendGuildMessage(sGGW->GWARZ[LocId].guild_id, ann);

														ChatHandler(player->GetSession()).PSendSysMessage("%s!! Its a cannon !!|r", sGGW->GWCOMM[guild_id].color_14.c_str());
														msg = GGW_MSG;

													}
												}
											}
										}
									}
								} // cannon

	/*
	-- **** Vault ****
	*/

								if (ChatCache[2] == sGGW->GWCOMM[guild_id].vault)
								{
									if (player->GetItemCount(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency) < sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vault_cost)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sEach vault costs %u %s.", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vault_cost, sGGW->Currencyname.c_str());
									}

									if (player->GetItemCount(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency) >= sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vault_cost)
									{
										if (sGGW->GWARZ[LocId].vault_count >= (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vault_cost * sGGW->GWARZ[LocId].hall_count))
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou have %u %s's at this location.", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->GWARZ[LocId].vault_count, sGGW->GWCOMM[guild_id].vault.c_str());
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou can only have %u per hall.", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vault_L);
										}

										if (sGGW->GWARZ[LocId].vault_count < sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vault_L * sGGW->GWARZ[LocId].hall_count)
										{
											if (sGGW->GWARZ[LocId].hall_count == 0)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%sVaults require a Hall.", sGGW->GWCOMM[guild_id].color_15.c_str());
											}

											if (sGGW->GWARZ[LocId].hall_count > 0)
											{
												uint32 spawnvault = SpawnGuildObjects(1, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vault_id, pTeam_id, guild_id, Pmap, pX, pY, pZ, pO, player, LocId);

												if (!spawnvault)
												{
													ChatHandler(player->GetSession()).PSendSysMessage("%svault spawn error..|r", sGGW->GWCOMM[guild_id].color_15.c_str());
												}

												if (spawnvault)
												{
													player->DestroyItemCount(uint32(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency), uint32(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vault_cost), true);

													sGGW->UpdateGuildLocData("vault_count", sGGW->ConvertNumberToString(sGGW->GWARZ[LocId].vault_count + 1), LocId);

													ChatHandler(player->GetSession()).PSendSysMessage("-%u %s's.|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vault_cost, sGGW->Currencyname.c_str());

													std::string ann = pName + " has added a vault to location:" + str_LocId + ".";

													sGGW->SendGuildMessage(sGGW->GWARZ[LocId].guild_id, ann);
													msg = GGW_MSG;
												}
											}
										}
									}
								} // vault

	/*
	-- **** MailBox ****
	*/

								if (ChatCache[2] == sGGW->GWCOMM[guild_id].mailbox)
								{
									if (player->GetItemCount(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency) < sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].mailbox_cost)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sEach mailbox costs %u %s.", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].mailbox_cost, sGGW->Currencyname.c_str());
									}

									if (player->GetItemCount(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency) >= (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].mailbox_cost * sGGW->GWARZ[LocId].vault_count))
									{
										if (sGGW->GWARZ[LocId].mailbox_count >= sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].mailbox_L * sGGW->GWARZ[LocId].hall_count)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou have %u %s's at this location.", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->GWARZ[LocId].mailbox_count, sGGW->GWCOMM[guild_id].mailbox.c_str());
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou can only have %u per hall.", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].mailbox_L);
										};

										if (sGGW->GWARZ[LocId].mailbox_count < (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].mailbox_cost * sGGW->GWARZ[LocId].vault_count))
										{
											if (sGGW->GWARZ[LocId].hall_count == 0)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%smailbox require a Hall.", sGGW->GWCOMM[guild_id].color_15.c_str());
											};

											if (sGGW->GWARZ[LocId].hall_count > 0)
											{
												uint32 spawnmailbox = SpawnGuildObjects(1, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].mailbox_id, pTeam_id, guild_id, Pmap, pX, pY, pZ, pO, player, LocId);

												if (!spawnmailbox)
												{
													ChatHandler(player->GetSession()).PSendSysMessage("%smailbox spawn error..|r", sGGW->GWCOMM[guild_id].color_15.c_str());
												};

												if (spawnmailbox)
												{
													player->DestroyItemCount(uint32(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency), uint32(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].mailbox_cost), true);

													sGGW->UpdateGuildLocData("mailbox_count", sGGW->ConvertNumberToString(sGGW->GWARZ[LocId].mailbox_count + 1), LocId);

													ChatHandler(player->GetSession()).PSendSysMessage("-%u %s's.|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].mailbox_cost, sGGW->Currencyname.c_str());

													std::string ann = pName + " has added a mailbox to location:" + str_LocId + ".";

													sGGW->SendGuildMessage(sGGW->GWARZ[LocId].guild_id, ann);
													msg = GGW_MSG;
												}
											}
										}
									}
								} // mailbox
							} // is owned by player guild
						} // NOT in GM mode
					}// allowed to buy/sell
					else
					{
						if (GuildLeaderGUID == pGuid)
							ChatHandler(player->GetSession()).PSendSysMessage("<%sGrumbo|r>:%sSpeak With Your faction Leader to gain access to ownership and development.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_1.c_str());
						else
							ChatHandler(player->GetSession()).PSendSysMessage("<%sGrumbo|r>:%sRemind your Guild Leader to Speak With Your faction Leader to gain access to ownership and development.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_1.c_str());
					}
					msg = GGW_MSG;
				} // buy commands

// ******************* Sell commands *******************

				if ((pGuildRank <= sGGW->GWCOMM[guild_id].GLD_lvlb) && ChatCache[1] == "sell")
				{
					if (sGGW->GWCOMM[guild_id].allowed >= 1)
					{
						if (player->IsGameMaster())
						{
							ChatHandler(player->GetSession()).PSendSysMessage("%sYou Must exit GM mode first to use this command.", sGGW->GWCOMM[guild_id].color_15.c_str());
						};

						if (!player->IsGameMaster())
						{
							if (sGGW->GWARZ[LocId].guild_id != guild_id)
							{
								ChatHandler(player->GetSession()).PSendSysMessage("%sYour Guild does not own this land.", sGGW->GWCOMM[guild_id].color_15.c_str());
							};

							if (sGGW->GWARZ[LocId].guild_id == guild_id)
							{

	/*
	-- **** Location ****
	*/

								if (ChatCache[2] == sGGW->GWCOMM[guild_id].loc)
								{
									GameObject *object = player->FindNearestGameObject(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].flag_id + pTeam_id, 5);

									if (!object)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sYou must stand closer to your flag.|r", sGGW->GWCOMM[guild_id].color_15.c_str());
									}

									if (object)
									{
										if (player->AddItem(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency, Zoneprice))
										{
											object->RemoveFromWorld();
											object->DeleteFromDB(object->GetSpawnId());

											sGGW->UpdateGuildLocData("guild_name", SERVER_GUILD_NAME, LocId);
											sGGW->UpdateGuildLocData("team", "2", LocId);
											sGGW->UpdateGuildLocData("guild_id", sGGW->ConvertNumberToString(sGGW->SERVER_GUILD_ID), LocId);

											std::string ann = pName + " has sold location:" + str_LocId + ".";

											sGGW->SendGuildMessage(sGGW->GWARZ[LocId].guild_id, ann);

											if (sGGW->GUILDWARZ_RANK_TYPE > 1) { sGGW->CreateRankList(); };

											ChatHandler(player->GetSession()).PSendSysMessage("+%u %s's.|r", Zoneprice, sGGW->Currencyname.c_str());
											msg = GGW_MSG;
										} // can add currency
									}
								} // location

	/*
	-- **** Farm ****
	*/

								if (ChatCache[2] == sGGW->GWCOMM[guild_id].farm)
								{
									if (sGGW->GWARZ[LocId].farm_count == 0)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sYour guild does not own a farm at this location.", sGGW->GWCOMM[guild_id].color_15.c_str());
									}

									if (sGGW->GWARZ[LocId].farm_count > 0)
									{
										if (sGGW->GWARZ[LocId].pig_count > ((sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_L) * (sGGW->GWARZ[LocId].farm_count - 1)))
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must sell |r %u%s pigs  before removing there housing.|r", sGGW->GWCOMM[guild_id].color_15.c_str(), (sGGW->GWARZ[LocId].pig_count - (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_L * (sGGW->GWARZ[LocId].farm_count - 1))), sGGW->GWCOMM[guild_id].color_15.c_str());
										};

										if (sGGW->GWARZ[LocId].pig_count <= ((sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_L) * (sGGW->GWARZ[LocId].farm_count - 1)))
										{
											GameObject *go = player->FindNearestGameObject(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].farm_id, 5);

											if (!go)
											{
												go = player->FindNearestGameObject(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].farm_id + 1, 5);
											}

											if (!go)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%sYou must stand closer to a farm.|r", sGGW->GWCOMM[guild_id].color_15.c_str());
											}

											if (go)
											{
												if (player->AddItem(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].farm_cost))
												{
													go->RemoveFromWorld();
													go->DeleteFromDB(go->GetSpawnId());

													sGGW->UpdateGuildLocData("farm_count", sGGW->ConvertNumberToString(sGGW->GWARZ[LocId].farm_count - 1), LocId);

													std::string ann = pName + " has sold a farm from location:" + str_LocId + ".";

													sGGW->SendGuildMessage(sGGW->GWARZ[LocId].guild_id, ann);

													ChatHandler(player->GetSession()).PSendSysMessage("+%u %s's.|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].farm_cost, sGGW->Currencyname.c_str());
													msg = GGW_MSG;
												} // can add currency
											}
										} // enough pigs
									} // has farms
								} // farm

	/*
	-- **** Barrack ****
	*/

								if (ChatCache[2] == sGGW->GWCOMM[guild_id].barrack)
								{
									if (sGGW->GWARZ[LocId].barrack_count == 0)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sYour guild does not own a barrack at this location.", sGGW->GWCOMM[guild_id].color_15.c_str());
									}

									if (sGGW->GWARZ[LocId].barrack_count > 0)
									{
										if (sGGW->GWARZ[LocId].guard_count > ((sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].guard_L) * (sGGW->GWARZ[LocId].barrack_count - 1)))
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must sell|r %u %sguards before removing there Barrack's.|r", sGGW->GWCOMM[guild_id].color_15.c_str(), sGGW->GWARZ[LocId].guard_count - (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].guard_L * (sGGW->GWARZ[LocId].barrack_count - 1)), sGGW->GWCOMM[guild_id].color_15.c_str());
										}

										if (sGGW->GWARZ[LocId].guard_count <= ((sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].guard_L) * (sGGW->GWARZ[LocId].barrack_count - 1)))
										{
											GameObject *go = player->FindNearestGameObject(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].barrack_id, 5);

											if (!go)
											{
												go = player->FindNearestGameObject(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].barrack_id + 1, 5);
											}

											if (!go)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%sYou must stand closer to a barrack.|r", sGGW->GWCOMM[guild_id].color_15.c_str());
											}

											if (go)
											{
												if (player->AddItem(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].barrack_cost))
												{
													go->ToGameObject()->RemoveFromWorld();
													go->ToGameObject()->DeleteFromDB(go->GetSpawnId());

													sGGW->UpdateGuildLocData("barrack_count", sGGW->ConvertNumberToString(sGGW->GWARZ[LocId].barrack_count - 1), LocId);

													std::string ann = pName + " has sold a barrack from location:" + str_LocId + ".";

													sGGW->SendGuildMessage(sGGW->GWARZ[LocId].guild_id, ann);

													ChatHandler(player->GetSession()).PSendSysMessage("+%u %s's.|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].barrack_cost, sGGW->Currencyname.c_str());
													msg = GGW_MSG;
												} // can add currency
											}
										} // enough guards
									} // has barracks
								} // barrack

	/*
	-- **** Hall ****
	*/

								if (ChatCache[2] == sGGW->GWCOMM[guild_id].hall)
								{
									if (sGGW->GWARZ[LocId].hall_count == 0)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sYour guild does not own a hall at this location.", sGGW->GWCOMM[guild_id].color_15.c_str());
									}

									if (sGGW->GWARZ[LocId].hall_count > 0)
									{
										if (sGGW->GWARZ[LocId].vendor1_count > 0)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must sell off your vendor1's before removing a Hall.|r", sGGW->GWCOMM[guild_id].color_15.c_str());
										}
										if (sGGW->GWARZ[LocId].vendor2_count > 0)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must sell off your vendor2's before removing a Hall.|r", sGGW->GWCOMM[guild_id].color_15.c_str());
										}
										if (sGGW->GWARZ[LocId].vendor3_count > 0)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must sell off your vendor3's before removing a Hall.|r", sGGW->GWCOMM[guild_id].color_15.c_str());
										}
										if (sGGW->GWARZ[LocId].cannon_count > 0)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must sell off your cannon's before removing a Hall.|r", sGGW->GWCOMM[guild_id].color_15.c_str());
										}
										if (sGGW->GWARZ[LocId].vault_count > 0)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must sell off your vault's before removing a Hall.|r", sGGW->GWCOMM[guild_id].color_15.c_str());
										}
										if (sGGW->GWARZ[LocId].mailbox_count > 0)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must sell off your mailbox's before removing a Hall.|r", sGGW->GWCOMM[guild_id].color_15.c_str());
										}

										if ((sGGW->GWARZ[LocId].vendor1_count == 0) && (sGGW->GWARZ[LocId].vendor2_count == 0) && (sGGW->GWARZ[LocId].vendor3_count == 0) && (sGGW->GWARZ[LocId].cannon_count == 0) && (sGGW->GWARZ[LocId].vault_count == 0) && (sGGW->GWARZ[LocId].mailbox_count == 0))

										{
											GameObject *go = player->FindNearestGameObject(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].hall_id, 10.0);

											if (!go)
											{
												go = player->FindNearestGameObject(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].hall_id + 1, 10.0);
											}

											if (!go)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%sYou must stand closer to your hall.|r", sGGW->GWCOMM[guild_id].color_15.c_str());
											}

											if (go)
											{
												if (player->AddItem(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].hall_cost))
												{
													go->ToGameObject()->RemoveFromWorld();
													go->ToGameObject()->DeleteFromDB(go->GetSpawnId());

													sGGW->UpdateGuildLocData("hall_count", sGGW->ConvertNumberToString(sGGW->GWARZ[LocId].hall_count - 1), LocId);

													std::string ann = pName + " has sold a hall from location:" + str_LocId + ".";

													sGGW->SendGuildMessage(sGGW->GWARZ[LocId].guild_id, ann);

													ChatHandler(player->GetSession()).PSendSysMessage("+%u %s's.|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].hall_cost, sGGW->Currencyname.c_str());
													msg = GGW_MSG;
												} // can add currency
											} // found go
										} // find go
									} // has hall
								} // hall

	/*
	-- **** Pig ****
	*/

								if (ChatCache[2] == sGGW->GWCOMM[guild_id].pig)
								{
									if (sGGW->GWARZ[LocId].pig_count == 0)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sYou DONT have any pigs in this area.", sGGW->GWCOMM[guild_id].color_15.c_str());
									}

									if (sGGW->GWARZ[LocId].pig_count > 0)
									{
										Unit *unit = player->GetSelectedUnit();

										if (!unit)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must select a pig.|r", sGGW->GWCOMM[guild_id].color_15.c_str());
										}

										if (unit)
										{
											if (unit->GetTypeId() != 3)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%sYou must select a pig.|r", sGGW->GWCOMM[guild_id].color_15.c_str());
											}

											if (unit->GetTypeId() == 3)
											{
												if (unit->GetEntry() == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_id || unit->GetEntry() == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_id + 1)
												{
													if (player->AddItem(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_cost))
													{
														unit->RemoveFromWorld();
														unit->ToCreature()->DeleteFromDB(unit->ToCreature()->GetSpawnId());

														sGGW->UpdateGuildLocData("pig_count", sGGW->ConvertNumberToString(sGGW->GWARZ[LocId].pig_count - 1), LocId);

														std::string ann = pName + " has sold a pig from location:" + str_LocId + " to the butcher shop.";

														sGGW->SendGuildMessage(sGGW->GWARZ[LocId].guild_id, ann);

														ChatHandler(player->GetSession()).PSendSysMessage("+%u %s's.|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_cost, sGGW->Currencyname.c_str());
														msg = GGW_MSG;
													} // can add currency
												} // is pig
											} // is npc
										} // found go
									} // has pigs
								} //pig

	/*
	-- **** Guard ****
	*/

								if (ChatCache[2] == sGGW->GWCOMM[guild_id].guard)
								{
									if (sGGW->GWARZ[LocId].guard_count == 0)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sYou DONT have any guard's in this area.", sGGW->GWCOMM[guild_id].color_15.c_str());
									}

									if (sGGW->GWARZ[LocId].guard_count > 0)
									{
										Unit *unit = player->GetSelectedUnit();

										if (!unit)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must select a guard.|r", sGGW->GWCOMM[guild_id].color_15.c_str());
										}

										if (unit)
										{
											if (unit->GetTypeId() != 3)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%sYou must select a guard.|r", sGGW->GWCOMM[guild_id].color_15.c_str());
											}

											if (unit->GetTypeId() == 3)
											{
												if (unit->GetEntry() == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].guard_id || unit->GetEntry() == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].guard_id + 1)
												{
													unit->RemoveFromWorld();
													unit->ToCreature()->DeleteFromDB(unit->ToCreature()->GetSpawnId());

													sGGW->UpdateGuildLocData("guard_count", sGGW->ConvertNumberToString(sGGW->GWARZ[LocId].guard_count - 1), LocId);

													std::string ann = pName + " has released a guard from location:" + str_LocId + ".";

													sGGW->SendGuildMessage(sGGW->GWARZ[LocId].guild_id, ann);

													ChatHandler(player->GetSession()).PSendSysMessage("%sGuards are disposable and dont return %s's.|r", sGGW->GWCOMM[guild_id].color_14.c_str(), sGGW->Currencyname.c_str());
													ChatHandler(player->GetSession()).PSendSysMessage("%s%s Has released a guard.|r", sGGW->GWCOMM[guild_id].color_14.c_str(), pName.c_str());
													msg = GGW_MSG;
												} // is guard
											} // is npc
										} // found npc
									} // has guard
								} //guard


	/*
	-- **** Vendor 1 ****
	*/

								if (ChatCache[2] == sGGW->GWCOMM[guild_id].vendor1)
								{
									if (sGGW->GWARZ[LocId].vendor1_count == 0)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sYou DONT have any vendor1's in this area.", sGGW->GWCOMM[guild_id].color_15.c_str());
									}

									if (sGGW->GWARZ[LocId].vendor1_count > 0)
									{
										Unit *unit = player->GetSelectedUnit();

										if (!unit)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must select a vendor1.|r", sGGW->GWCOMM[guild_id].color_15.c_str());
										}

										if (unit)
										{
											if (unit->GetTypeId() != 3)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%sYou must select a vendor1.|r", sGGW->GWCOMM[guild_id].color_15.c_str());
											}

											if (unit->GetTypeId() == 3)
											{
												if (unit->GetEntry() == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor1_id || unit->GetEntry() == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor1_id + 1)
												{
													if (player->AddItem(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor1_cost))
													{
														unit->RemoveFromWorld();
														unit->ToCreature()->DeleteFromDB(unit->ToCreature()->GetSpawnId());

														sGGW->UpdateGuildLocData("vendor1_count", sGGW->ConvertNumberToString(sGGW->GWARZ[LocId].vendor1_count - 1), LocId);

														std::string ann = pName + " has sold a vendor1 from location:" + str_LocId + ".";

														sGGW->SendGuildMessage(sGGW->GWARZ[LocId].guild_id, ann);

														ChatHandler(player->GetSession()).PSendSysMessage("+%u %s's.|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor1_cost, sGGW->Currencyname.c_str());
														msg = GGW_MSG;
													} //add currency
												}//is id
											} //type 3
										} // found vendor1
									} // has vendor1
								} //vendor1

	/*
	-- **** Vendor 2 ****
	*/

								if (ChatCache[2] == sGGW->GWCOMM[guild_id].vendor2)
								{
									if (sGGW->GWARZ[LocId].vendor2_count == 0)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sYou DONT have any vendor2's in this area.", sGGW->GWCOMM[guild_id].color_15.c_str());
									}

									if (sGGW->GWARZ[LocId].vendor2_count > 0)
									{
										Unit *unit = player->GetSelectedUnit();

										if (!unit)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must select a vendor2.|r", sGGW->GWCOMM[guild_id].color_15.c_str());
										}

										if (unit)
										{
											if (unit->GetTypeId() != 3)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%sYou must select a vendor2.|r", sGGW->GWCOMM[guild_id].color_15.c_str());
											}

											if (unit->GetTypeId() == 3)
											{
												if (unit->GetEntry() == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor2_id || unit->GetEntry() == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor2_id + 1)
												{
													if (player->AddItem(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor2_cost))
													{
														unit->RemoveFromWorld();
														unit->ToCreature()->DeleteFromDB(unit->ToCreature()->GetSpawnId());

														sGGW->UpdateGuildLocData("vendor2_count", sGGW->ConvertNumberToString(sGGW->GWARZ[LocId].vendor2_count - 1), LocId);

														std::string ann = pName + " has sold a vendor2 from location:" + str_LocId + ".";

														sGGW->SendGuildMessage(sGGW->GWARZ[LocId].guild_id, ann);

														ChatHandler(player->GetSession()).PSendSysMessage("+%u %s's.|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor2_cost, sGGW->Currencyname.c_str());
														msg = GGW_MSG;
													} //add currency
												} // is id
											} // type 3
										} // found vendor2
									} // has vendor2
								} //vendor2

	/*
	-- **** Vendor 3 ****
	*/

								if (ChatCache[2] == sGGW->GWCOMM[guild_id].vendor3)
								{
									if (sGGW->GWARZ[LocId].vendor3_count == 0)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sYou DONT have any vendor3's in this area.", sGGW->GWCOMM[guild_id].color_15.c_str());
									}

									if (sGGW->GWARZ[LocId].vendor3_count > 0)
									{
										Unit *unit = player->GetSelectedUnit(); //  FindNearestCreature((GWCOMM[SERVER_GUILD_ID].pig_id || GWCOMM[SERVER_GUILD_ID].pig_id + 1), 0.5);

										if (!unit)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must select a vendor3.|r", sGGW->GWCOMM[guild_id].color_15.c_str());
										}

										if (unit)
										{
											if (unit->GetTypeId() != 3)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%sYou must select a vendor3.|r", sGGW->GWCOMM[guild_id].color_15.c_str());
											}

											if (unit->GetTypeId() == 3)
											{
												if (unit->GetEntry() == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor3_id || unit->GetEntry() == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor3_id + 1)
												{
													if (player->AddItem(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor3_cost))
													{
														unit->RemoveFromWorld();
														unit->ToCreature()->DeleteFromDB(unit->ToCreature()->GetSpawnId());

														sGGW->UpdateGuildLocData("vendor3_count", sGGW->ConvertNumberToString(sGGW->GWARZ[LocId].vendor3_count - 1), LocId);

														std::string ann = pName + " has sold a vendor3 from location:" + str_LocId + ".";

														sGGW->SendGuildMessage(sGGW->GWARZ[LocId].guild_id, ann);

														ChatHandler(player->GetSession()).PSendSysMessage("+%u %s's.|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor3_cost, sGGW->Currencyname.c_str());
														msg = GGW_MSG;
													} //add currency
												}
											} // can add currency
										} // found vendor3
									} // has vendor3
								} //vendor3

	/*
	-- **** Cannon ****
	*/

								if (ChatCache[2] == sGGW->GWCOMM[guild_id].cannon)
								{
									if (sGGW->GWARZ[LocId].cannon_count == 0)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sYou DONT have any cannon's in this area.", sGGW->GWCOMM[guild_id].color_15.c_str());
									}

									if (sGGW->GWARZ[LocId].cannon_count > 0)
									{
										Unit *unit = player->GetSelectedUnit();

										if (!unit)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must select a cannon.|r", sGGW->GWCOMM[guild_id].color_15.c_str());
										}

										if (unit)
										{
											if (unit->GetTypeId() != 3)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%sYou must select a cannon.|r", sGGW->GWCOMM[guild_id].color_15.c_str());
											}

											if (unit->GetTypeId() == 3)
											{
												if (unit->GetEntry() == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].cannon_id || unit->GetEntry() == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].cannon_id + 1)
												{
													if (player->AddItem(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].cannon_cost))
													{
														unit->RemoveFromWorld();
														unit->ToCreature()->DeleteFromDB(unit->ToCreature()->GetSpawnId());

														sGGW->UpdateGuildLocData("cannon_count", sGGW->ConvertNumberToString(sGGW->GWARZ[LocId].cannon_count - 1), LocId);

														std::string ann = pName + " has sold a cannon from location:" + str_LocId + ".";

														sGGW->SendGuildMessage(sGGW->GWARZ[LocId].guild_id, ann);

														ChatHandler(player->GetSession()).PSendSysMessage("+%u %s's.|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].cannon_cost, sGGW->Currencyname.c_str());
														msg = GGW_MSG;
													} //add currency
												}
											} // can add currency
										} // found cannon
									} // has cannon
								} //cannon

	/*
	-- **** Vault ****
	*/

								if (ChatCache[2] == sGGW->GWCOMM[guild_id].vault)
								{
									if (sGGW->GWARZ[LocId].vault_count == 0)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sYour guild does not own a vault at this location.", sGGW->GWCOMM[guild_id].color_15.c_str());
									}

									if (sGGW->GWARZ[LocId].vault_count > 0)
									{
										GameObject *go = player->FindNearestGameObject(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vault_id, 5);

										if (!go)
										{
											go = player->FindNearestGameObject(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vault_id + 1, 5);
										}

										if (!go)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must stand closer to a vault.|r", sGGW->GWCOMM[guild_id].color_15.c_str());
										}

										if (go)
										{
											if (player->AddItem(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vault_cost))
											{
												go->ToGameObject()->RemoveFromWorld();
												go->ToGameObject()->DeleteFromDB(go->ToGameObject()->GetSpawnId());

												sGGW->UpdateGuildLocData("vault_count", sGGW->ConvertNumberToString(sGGW->GWARZ[LocId].vault_count - 1), LocId);

												std::string ann = pName + " has sold a vault from location:" + str_LocId + ".";

												sGGW->SendGuildMessage(sGGW->GWARZ[LocId].guild_id, ann);

												ChatHandler(player->GetSession()).PSendSysMessage("+%u %s's.|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vault_cost, sGGW->Currencyname.c_str());
												msg = GGW_MSG;
											} // can add currency
										}
									} // has vault
								} // vault

	/*
	-- **** MailBox ****
	*/

								if (ChatCache[2] == sGGW->GWCOMM[guild_id].mailbox)
								{
									if (sGGW->GWARZ[LocId].mailbox_count == 0)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sYour guild does not own a mailbox at this location.", sGGW->GWCOMM[guild_id].color_15.c_str());
									}

									if (sGGW->GWARZ[LocId].mailbox_count > 0)
									{
										GameObject *go = player->FindNearestGameObject(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].mailbox_id, 5);

										if (!go)
										{
											go = player->FindNearestGameObject(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].mailbox_id + 1, 5);
										}

										if (!go)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must stand closer to a mailbox.|r", sGGW->GWCOMM[guild_id].color_15.c_str());
										}

										if (go)
										{
											if (player->AddItem(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].mailbox_cost))
											{
												go->ToGameObject()->RemoveFromWorld();
												go->ToGameObject()->DeleteFromDB(go->ToGameObject()->GetSpawnId());

												sGGW->UpdateGuildLocData("mailbox_count", sGGW->ConvertNumberToString(sGGW->GWARZ[LocId].mailbox_count - 1), LocId);

												std::string ann = pName + " has sold a mailbox from location:" + str_LocId + ".";

												sGGW->SendGuildMessage(sGGW->GWARZ[LocId].guild_id, ann);

												ChatHandler(player->GetSession()).PSendSysMessage("+%u %s's.|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].mailbox_cost, sGGW->Currencyname.c_str());
												msg = GGW_MSG;
											} // can add currency
										}
									} // has mailbox
								} // mailbox
							} // is owned by player's guild
						} // NOT in GM mode
					}// allowed to buy/sell
					else
					{
						if (GuildLeaderGUID == pGuid)
							ChatHandler(player->GetSession()).PSendSysMessage("<%sGrumbo|r>:%sSpeak With Your faction Leader to gain access to ownership and development.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_1.c_str());
						else 
							ChatHandler(player->GetSession()).PSendSysMessage("<%sGrumbo|r>:%sRemind your Guild Leader to Speak With Your faction Leader to gain access to ownership and development.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_1.c_str());
					}
					msg = GGW_MSG;
				} // is sell command
/*
-- ****************************************************
-- **************** Game Master Commands **************
-- ****************************************************
*/
				if (pGMRank >= sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].GM_minimum && player->IsGameMaster())
				{

/*
-- **** Loc Command ****
*/

					if ((ChatCache[1] == "lock") && (ChatCache[2] == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].loc))
					{
						sGGW->UpdateGuildLocData("guild_name", SERVER_GUILD_NAME, LocId);
						sGGW->UpdateGuildLocData("guild_id", sGGW->ConvertNumberToString(sGGW->SERVER_GUILD_ID), LocId);
						sGGW->UpdateGuildLocData("team", sGGW->ConvertNumberToString(sGGW->SERVER_GUILD_TEAM_LOCKED_ID), LocId);
						sGGW->UpdateGuildLocData("flag_id", "0", LocId);
						sGGW->UpdateGuildLocData("fs_time", "0", LocId);

						ChatHandler(player->GetSession()).PSendSysMessage("%sLocation:%u succesfully|r %sLOCKED|r.", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_14.c_str(), sGGW->GWARZ[LocId].entry, sGGW->GWCOMM[guild_id].color_7.c_str());
						msg = GGW_MSG;
					};

/*
-- **** Reset Command ****
*/

					if (ChatCache[1] == "reset")
					{
						if (ChatCache[2] == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].loc)
						{
							sGGW->UpdateGuildLocData("guild_name", SERVER_GUILD_NAME, LocId);
							sGGW->UpdateGuildLocData("guild_id", sGGW->ConvertNumberToString(sGGW->SERVER_GUILD_ID), LocId);
							sGGW->UpdateGuildLocData("team", sGGW->ConvertNumberToString(sGGW->SERVER_GUILD_TEAM), LocId);
							sGGW->UpdateGuildLocData("flag_id", "0", LocId);
							sGGW->UpdateGuildLocData("fs_time", "0", LocId);

							ChatHandler(player->GetSession()).PSendSysMessage("%sLocation:%u succesfully reset.|r", sGGW->GWCOMM[guild_id].color_14.c_str(), sGGW->GWARZ[LocId].entry);
						}

						if (ChatCache[2] == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].farm)
						{
							sGGW->UpdateGuildLocData("farm_count", "0", LocId);

							ChatHandler(player->GetSession()).PSendSysMessage("%sLocation:%u farm count reset.|r", sGGW->GWCOMM[guild_id].color_14.c_str(), sGGW->GWARZ[LocId].entry);
						}

						if (ChatCache[2] == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].barrack)
						{
							sGGW->UpdateGuildLocData("barrack_count", "0", LocId);

							ChatHandler(player->GetSession()).PSendSysMessage("%sLocation:%u barrack count reset.|r", sGGW->GWCOMM[guild_id].color_14.c_str(), sGGW->GWARZ[LocId].entry);
						}

						if (ChatCache[2] == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].hall)
						{
							sGGW->UpdateGuildLocData("hall_count", "0", LocId);

							ChatHandler(player->GetSession()).PSendSysMessage("%sArea:%u hall count reset.|r", sGGW->GWCOMM[guild_id].color_14.c_str(), sGGW->GWARZ[LocId].entry);
						}

						if (ChatCache[2] == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig)
						{
							sGGW->UpdateGuildLocData("pig_count", "0", LocId);

							ChatHandler(player->GetSession()).PSendSysMessage("%sArea:%u pig count reset.|r", sGGW->GWCOMM[guild_id].color_14.c_str(), sGGW->GWARZ[LocId].entry);
						}

						if (ChatCache[2] == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].guard)
						{
							sGGW->UpdateGuildLocData("guard_count", "0", LocId);

							ChatHandler(player->GetSession()).PSendSysMessage("%sArea:%u guard count reset.|r", sGGW->GWCOMM[guild_id].color_14.c_str(), sGGW->GWARZ[LocId].entry);
						}

						if (ChatCache[2] == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor1)
						{
							sGGW->UpdateGuildLocData("vendor1_count", "0", LocId);

							ChatHandler(player->GetSession()).PSendSysMessage("%sArea:%u vendor1 count reset.|r", sGGW->GWCOMM[guild_id].color_14.c_str(), sGGW->GWARZ[LocId].entry);
						}

						if (ChatCache[2] == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor2)
						{
							sGGW->UpdateGuildLocData("vendor2_count", "0", LocId);

							ChatHandler(player->GetSession()).PSendSysMessage("%sArea:%u vendor2 count reset.|r", sGGW->GWCOMM[guild_id].color_14.c_str(), sGGW->GWARZ[LocId].entry);
						}

						if (ChatCache[2] == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor3)
						{
							sGGW->UpdateGuildLocData("vendor3_count", "0", LocId);
							ChatHandler(player->GetSession()).PSendSysMessage("%sArea:%u vendor3 count reset.|r", sGGW->GWCOMM[guild_id].color_14.c_str(), sGGW->GWARZ[LocId].entry);
						}

						if (ChatCache[2] == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].cannon)
						{
							sGGW->UpdateGuildLocData("cannon_count", "0", LocId);

							ChatHandler(player->GetSession()).PSendSysMessage("%sArea:%u cannon count reset.|r", sGGW->GWCOMM[guild_id].color_14.c_str(), sGGW->GWARZ[LocId].entry);
						}

						if (ChatCache[2] == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vault)
						{
							sGGW->UpdateGuildLocData("vault_count", "0", LocId);

							ChatHandler(player->GetSession()).PSendSysMessage("%sArea:%u vault count reset.|r", sGGW->GWCOMM[guild_id].color_14.c_str(), sGGW->GWARZ[LocId].entry);
						}

						if (ChatCache[2] == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].mailbox)
						{
							sGGW->UpdateGuildLocData("mailbox_count", "0", LocId);

							ChatHandler(player->GetSession()).PSendSysMessage("%sArea:%u mailbox count reset.|r", sGGW->GWCOMM[guild_id].color_14.c_str(), sGGW->GWARZ[LocId].entry);
						}

						msg = GGW_MSG;
					} // reset commands

/*
-- **** Reload Command ****
*/

					if (ChatCache[1] == "reload")
					{
						if (ChatCache[2] == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].table)
						{
							sGGW->LoadCommands();
							sGGW->LoadHelp();
							sGGW->LoadLoc();

							ChatHandler(player->GetSession()).PSendSysMessage("%sALL Grumbo'z Guild Warz Tables are Reloaded.|r", sGGW->GWCOMM[guild_id].color_14.c_str());
							msg = GGW_MSG;
						};
					}; // reload

/*
-- **** Spawn Flag Command ****
*/

					if (ChatCache[1] == "spawn")
					{
						if (ChatCache[2] == "flag")
						{
							if (sGGW->GWARZ[LocId].team >= sGGW->SERVER_GUILD_TEAM)
							{
								ChatHandler(player->GetSession()).PSendSysMessage("%sThe Server controls This Land. !! NO NEW FLAG SPAWNED !!|r", sGGW->GWCOMM[guild_id].color_15.c_str());
							}

							if (sGGW->GWARZ[LocId].team < sGGW->SERVER_GUILD_TEAM)
							{
								uint32 spawnflag = SpawnGuildObjects(2, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].flag_id, pTeam_id, guild_id, Pmap, pX, pY, pZ, pO, player, LocId);

								if (!spawnflag)
								{
									ChatHandler(player->GetSession()).PSendSysMessage("%sflag spawn error..|r", sGGW->GWCOMM[guild_id].color_15);
								}

								if (spawnflag)
								{
									sGGW->UpdateGuildLocFloat(pX, pY, pZ, LocId);

									sGGW->UpdateGuildLocData("flag_id", sGGW->ConvertNumberToString(spawnflag), LocId);
									sGGW->UpdateGuildLocData("fs_time", sGGW->ConvertNumberToString(GameTime::GetGameTime()), LocId);

									ChatHandler(player->GetSession()).PSendSysMessage("%sNew flag spawned for Guild Warz location:%u|r", sGGW->GWCOMM[guild_id].color_14.c_str(), sGGW->GWARZ[LocId].entry);
									msg = GGW_MSG;
								} // is spawned
							} // team check

						} // flag

						if (ChatCache[2] != "flag")
						{
							ChatHandler(player->GetSession()).PSendSysMessage("%sCMD ERROR:|r %s %s", sGGW->GWCOMM[guild_id].color_15.c_str(), ChatCache[1], ChatCache[2]);
						}
					} // spawn

/*
-- **** Loc Details Command ****
*/

					if (ChatCache[1] == sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].details_loc)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%s*************************************", sGGW->GWCOMM[guild_id].color_9.c_str());
						ChatHandler(player->GetSession()).PSendSysMessage("%sLocation details:", sGGW->GWCOMM[guild_id].color_8.c_str());
						ChatHandler(player->GetSession()).PSendSysMessage("%sLocation ID:|r %s%u|r.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWARZ[LocId].entry);

						if (sGGW->GWARZ[LocId].team == 0) { ChatHandler(player->GetSession()).PSendSysMessage("%sGuild Name|r: %s%s|r.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_4.c_str(), sGGW->GWARZ[LocId].guild_name.c_str()); };
						if (sGGW->GWARZ[LocId].team == 1) { ChatHandler(player->GetSession()).PSendSysMessage("%sGuild Name|r: %s%s|r.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_5.c_str(), sGGW->GWARZ[LocId].guild_name.c_str()); };
						if (sGGW->GWARZ[LocId].team == 2) { ChatHandler(player->GetSession()).PSendSysMessage("%sGuild Name|r: %s%s|r.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_6.c_str(), sGGW->GWARZ[LocId].guild_name.c_str()); };
						if (sGGW->GWARZ[LocId].team == 3) { ChatHandler(player->GetSession()).PSendSysMessage("%sGuild Name|r: %s%s|r.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_7.c_str(), sGGW->GWARZ[LocId].guild_name.c_str()); };

						if (sGGW->GWARZ[LocId].team == 0) { ChatHandler(player->GetSession()).PSendSysMessage("%sFaction|r:%sAlliance.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_4.c_str()); };
						if (sGGW->GWARZ[LocId].team == 1) { ChatHandler(player->GetSession()).PSendSysMessage("%sFaction|r:%sHorde.   |r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_5.c_str()); };
						if (sGGW->GWARZ[LocId].team == 2) { ChatHandler(player->GetSession()).PSendSysMessage("%sFaction|r:%sFor Sale.|r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_6.c_str()); };
						if (sGGW->GWARZ[LocId].team == 3) { ChatHandler(player->GetSession()).PSendSysMessage("%sFaction|r:%sLOCKED.  |r", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_7.c_str()); };

						ChatHandler(player->GetSession()).PSendSysMessage("%sTeam:|r%s%u|r.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWARZ[LocId].team);
						ChatHandler(player->GetSession()).PSendSysMessage("%sFarm count|r:%s%u|r.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWARZ[LocId].farm_count);
						ChatHandler(player->GetSession()).PSendSysMessage("%sBarrack count|r:%s%u|r.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWARZ[LocId].barrack_count);
						ChatHandler(player->GetSession()).PSendSysMessage("%sHall count|r:%s%u|r.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWARZ[LocId].hall_count);
						ChatHandler(player->GetSession()).PSendSysMessage("%sPig count|r:%s%u|r.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWARZ[LocId].pig_count);
						ChatHandler(player->GetSession()).PSendSysMessage("%sguard count|r:%s%u|r.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWARZ[LocId].guard_count);
						ChatHandler(player->GetSession()).PSendSysMessage("%scannon count|r:%s%u|r.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWARZ[LocId].cannon_count);

						if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor1_id > 0){ ChatHandler(player->GetSession()).PSendSysMessage("%svendor1 count|r:%s%u|r.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWARZ[LocId].vendor1_count); };
						if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor2_id > 0){ ChatHandler(player->GetSession()).PSendSysMessage("%svendor2 count|r:%s%u|r.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWARZ[LocId].vendor2_count); };
						if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor3_id > 0){ ChatHandler(player->GetSession()).PSendSysMessage("%svendor3 count|r:%s%u|r.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWARZ[LocId].vendor3_count); };

						ChatHandler(player->GetSession()).PSendSysMessage("%svault count:|r%s%u|r.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWARZ[LocId].vault_count);
						ChatHandler(player->GetSession()).PSendSysMessage("%smailbox count:|r%s%u|r.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWARZ[LocId].mailbox_count);
						ChatHandler(player->GetSession()).PSendSysMessage("%sflag GUIDLow:|r%s%u|r.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWARZ[LocId].flag_id);
						ChatHandler(player->GetSession()).PSendSysMessage("%sflag spawn time:|r%s%u|r.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWARZ[LocId].fs_time);
						ChatHandler(player->GetSession()).PSendSysMessage("%sGuild ID:|r%s%u|r.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWARZ[LocId].guild_id);
						ChatHandler(player->GetSession()).PSendSysMessage("%sGame Time|r:%s%u|r.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_2.c_str(), GameTime::GetGameTime());
						ChatHandler(player->GetSession()).PSendSysMessage("%s*************************************|r", sGGW->GWCOMM[guild_id].color_9.c_str());
						msg = GGW_MSG;
					}
				} // is minimum GM
/*
-- ****************************************************
-- ********** Guild Warz teleporter system ************
-- ** a mild mutation of Grandelf1's guild teleporter *
-- ****************************************************
*/
				if (ChatCache[1] == sGGW->GWCOMM[guild_id].tele)
				{
					if (ChatCache[2].c_str())
					{
						uint32 loc;

						std::istringstream(ChatCache[2]) >> loc;

						if (loc > 1 && loc <= sGGW->GWARZ.size())
						{
							if (sGGW->GWARZ[loc].guild_id == guild_id || player->IsGameMaster())
							{
								player->TeleportTo(uint32(sGGW->GWARZ[loc].map_id), sGGW->GWARZ[loc].x, sGGW->GWARZ[loc].y, sGGW->GWARZ[loc].z, 0.0);

								ChatHandler(player->GetSession()).PSendSysMessage("%sDeadByDawn says:|r%sTeleport to location %u complete.|r", sGGW->GWCOMM[guild_id].color_2.c_str(), sGGW->GWCOMM[guild_id].color_1.c_str(), loc);
								msg = GGW_MSG;
							}

							if (sGGW->GWARZ[loc].guild_id != guild_id && !player->IsGameMaster())
							{
								ChatHandler(player->GetSession()).PSendSysMessage("Raider says:%sYour guild doesn't own that area.", sGGW->GWCOMM[guild_id].color_15.c_str());
								ChatHandler(player->GetSession()).PSendSysMessage("Raider says:%sYou cannot teleport there.", sGGW->GWCOMM[guild_id].color_15.c_str());
								msg = GGW_MSG;
							}
						}

						if (loc > sGGW->GWARZ.size() || loc == 1)
						{
							ChatHandler(player->GetSession()).PSendSysMessage("Grumbo says:%sLocation %u doesn't exsist.|r", sGGW->GWCOMM[guild_id].color_15.c_str(), loc);
							ChatHandler(player->GetSession()).PSendSysMessage("Grumbo says:%sTry a different id.|r", sGGW->GWCOMM[guild_id].color_15.c_str());
							msg = GGW_MSG;
						}
					} //
				} // teleporter
		 } // Is In Guild
	};
};

// ****************************************************
// ********** GUILD WARZ PvP/Anarchy System ***********
// ****************************************************

// ************* Guild Warz Flag actions **************
// ********** Guild Invite Flag Tag Actions ***********

class GGW_GUILD_FLAG : public GameObjectScript
{
public: 
	GGW_GUILD_FLAG() : GameObjectScript("GGW_GUILD_FLAG"){ }

    struct GGW_GUILD_FLAG_AI : public GameObjectAI
    {
        GGW_GUILD_FLAG_AI(GameObject* go) : GameObjectAI(go) {} 

        bool OnGossipHello(Player* player, GameObject* go) // override // virtual
        {
            Guild* guild = player->GetGuild();
            Map* Pmap = player->GetMap();

            uint32 GGW_player_phase_mask_for_spawn = player->GetPhaseMaskForSpawn();
            //uint8 GGW_map_spawn_mode = Pmap->GetSpawnMode();

            std::string pName = player->GetName();
            uint8 pTeam_id = player->GetTeamId();
            uint32 pGuid = player->GetGUID();
            uint32 map_id = player->GetMapId();
            uint32 area_id = player->GetAreaId();
            uint32 zone_id = player->GetZoneId();
            float pX = player->GetPositionX();
            float pY = player->GetPositionY();
            float pZ = player->GetPositionZ();
            float pO = player->GetOrientation();

            uint32 LocId = sGGW->GetLocationID(map_id, area_id, zone_id);

            if (!LocId)
            {
                LocId = sGGW->CreateGuildLocation(map_id, area_id, zone_id, pX, pY, pZ);
            }

            uint32 loc_guild_id = sGGW->GWARZ[LocId].guild_id;
            std::string loc_guild_name = sGGW->GWARZ[LocId].guild_name;
            uint32 loc_guild_team = sGGW->GWARZ[LocId].team;

            std::string str_LocId;
            std::stringstream convert;
            convert << LocId;
            str_LocId = convert.str();

            uint64 curr_time = GameTime::GetGameTime();
            uint64 second = 1000;
            uint64 minute = 60000;
            uint64 spawn_cooldown_timer = sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].s_timer / second;
            uint64 total_flag_spawn_time = (sGGW->GWARZ[LocId].fs_time + spawn_cooldown_timer);

            uint32 gGuid = go->GetSpawnId();

            if (test) { ChatHandler(player->GetSession()).PSendSysMessage("FLAG_TAG_EVENT triggered"); };

            if (gGuid != sGGW->GWARZ[LocId].flag_id)
            {
                go->RemoveFromWorld();
                go->DeleteFromDB(go->GetSpawnId());

                ChatHandler(player->GetSession()).PSendSysMessage("%serror.... Phantom flag removed.|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_15.c_str());
                return true;
            }

            if (player->IsGameMaster())
            {
                ChatHandler(player->GetSession()).PSendSysMessage("%sYou are in GM mode. Exit GM mode to enjoy.|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_15.c_str());
                return true;
            }
            else
            {
                if (guild)
                {
                    if (test) { TC_LOG_INFO("server.loading", "GUILD:true"); };

                    uint32 guild_id = player->GetGuildId();
                    std::string guild_name = guild->GetName();
                    uint8 pGuildRank = player->GetRank();
                    uint8 pGMRank = player->GetSession()->GetSecurity();
                    uint32 GuildLeaderGUID = guild->GetLeaderGUID();

                    if (guild_name != sGGW->GWCOMM[guild_id].guild)
                    {
                        ChatHandler(player->GetSession()).PSendSysMessage("<Grumbo>:CREATING Your Guild Warz Commands for Guild %s.", guild_name.c_str());
                        sGGW->CreateGuildCommands(guild_name, guild_id);
                    }

                    if (sGGW->GWCOMM[guild_id].allowed >= 1)
                    {
                        if (guild_id == loc_guild_id || (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].anarchy == 0 && pTeam_id == sGGW->GWARZ[LocId].team))
                        {

                            if (sGGW->GWARZ[LocId].team == 0) ChatHandler(player->GetSession()).PSendSysMessage("%s%s|r own's this location.", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_4.c_str(), sGGW->GWARZ[LocId].guild_name.c_str());
                            if (sGGW->GWARZ[LocId].team == 1) ChatHandler(player->GetSession()).PSendSysMessage("%s%s|r own's this location.", sGGW->GWCOMM[guild_id].color_5.c_str(), sGGW->GWARZ[LocId].guild_name.c_str());

                            ChatHandler(player->GetSession()).PSendSysMessage("%sGrumbo'z Guild Warz System.|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_10.c_str());
                            return true;
                        }

                        if (pTeam_id != sGGW->GWARZ[LocId].team || (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].anarchy == 1 && guild_id != loc_guild_id))
                        {
                            if (total_flag_spawn_time > curr_time && sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].f_timer == 1)
                            {
                                ChatHandler(player->GetSession()).PSendSysMessage("%s!!..Cooldown Timer in Affect..!!|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_15.c_str());
                                return true;
                            }

                            if (total_flag_spawn_time <= curr_time || sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].f_timer == 0)
                            {
                                if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].flag_require == 1 && sGGW->GWARZ[LocId].guard_count > 0)
                                {
                                    ChatHandler(player->GetSession()).PSendSysMessage("%s!!..You must clear ALL guards..!!|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_15.c_str());
                                    return true;
                                }

                                if (sGGW->GWARZ[LocId].guard_count == 0 || sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].flag_require == 0)
                                {

                                    uint32 spawnflag = SpawnGuildObjects(2, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].flag_id, pTeam_id, guild_id, Pmap, pX, pY, pZ, pO, player, LocId);

                                    if (!spawnflag)
                                    {
                                        ChatHandler(player->GetSession()).PSendSysMessage("%sflag spawn error..|r", sGGW->GWCOMM[guild_id].color_15.c_str());
                                    }

                                    if (spawnflag)
                                    {
                                        go->RemoveFromWorld();
                                        go->DeleteFromDB(go->GetSpawnId());

                                        sGGW->UpdateGuildLocData("guild_name", guild_name, LocId);
                                        sGGW->UpdateGuildLocData("guild_id", sGGW->ConvertNumberToString(guild_id), LocId);
                                        sGGW->UpdateGuildLocData("team", sGGW->ConvertNumberToString(pTeam_id), LocId);

                                        sGGW->UpdateGuildLocData("flag_id", sGGW->ConvertNumberToString(spawnflag), LocId);
                                        sGGW->UpdateGuildLocData("fs_time", sGGW->ConvertNumberToString(GameTime::GetGameTime()), LocId);

                                        sGGW->UpdateGuildLocFloat(pX, pY, pZ, LocId);

                                        std::string ann = pName + " has taken location:" + str_LocId + " from ";

                                        if (loc_guild_team == 0) ann += (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_4 + loc_guild_name + ".");
                                        if (loc_guild_team == 1) ann += (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_5 + loc_guild_name + ".");

                                        sGGW->SendGuildMessage(sGGW->GWARZ[LocId].guild_id, ann);

                                        if (sGGW->GUILDWARZ_RANK_TYPE > 1) { sGGW->CreateRankList(); };

                                        return false;
                                    } // is spawned
                                }
                            }
                        }
                    }// is allowed
                    else
                    {
                        if (GuildLeaderGUID == pGuid)
                            ChatHandler(player->GetSession()).PSendSysMessage("<%sGrumbo|r>:%sSpeak With Your faction Leader to gain access to ownership and development.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_1.c_str());
                        else
                            ChatHandler(player->GetSession()).PSendSysMessage("<%sGrumbo|r>:%sRemind your Guild Leader to Speak With Your faction Leader to gain access to ownership and development.", sGGW->GWCOMM[guild_id].color_1.c_str(), sGGW->GWCOMM[guild_id].color_1.c_str());
                    }
                }
                else
                {
                    if (test) { TC_LOG_INFO("server.loading", "GUILD:false"); };

                    if (pTeam_id != sGGW->GWARZ[LocId].team)
                    {
                        ChatHandler(player->GetSession()).PSendSysMessage("%sGrumbo'z Guild Warz System:|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_1.c_str());
                        ChatHandler(player->GetSession()).PSendSysMessage("%s%s own's this location %s.|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_1.c_str(), sGGW->GWARZ[LocId].guild_name.c_str(), pName.c_str());
                        ChatHandler(player->GetSession()).PSendSysMessage("%sJoin a Guild to participate in Grumbo'z Guild Warz System.|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_1.c_str());

                        if ((pTeam_id == sGGW->GWARZ[LocId].team) && (sGGW->GWCOMM[sGGW->GWARZ[LocId].guild_id].guild_invite == 0))
                        {
                            ChatHandler(player->GetSession()).PSendSysMessage("%sThis Guild Master has disabled the guild's auto invite system.|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_11.c_str());
                        };

                        ChatHandler(player->GetSession()).PSendSysMessage("%sBrought to you by Grumbo.|r", sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_1.c_str());

                        if (test) { ChatHandler(player->GetSession()).PSendSysMessage("TAG EVENT 0"); };
                        return false;
                    }

                    if (pTeam_id == sGGW->GWARZ[LocId].team)
                    {
                        if (test) { ChatHandler(player->GetSession()).PSendSysMessage("TAG EVENT 1"); };

                        std::string join = sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_9 + "Join the Guild `";

                        if (sGGW->GWARZ[LocId].team == 0) join += sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_4 + sGGW->GWARZ[LocId].guild_name + sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_9 + "|r`.";
                        if (sGGW->GWARZ[LocId].team == 1) join += sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_5 + sGGW->GWARZ[LocId].guild_name + sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_9 + "|r`.";

                        std::string ranked = sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_9 + "Ranked #" + sGGW->ConvertNumberToString(sGGW->GetRank(sGGW->GWARZ[LocId].guild_id)) + ".";
                        std::string total_locations = sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_9 + "Control " + sGGW->ConvertNumberToString(sGGW->CalculateTotalLocations(sGGW->GWARZ[LocId].guild_id)) + " Locations.";
                        std::string total_value = sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_9 + "Total wealth of " + sGGW->ConvertNumberToString(sGGW->CalculateTotalLocationsValue(sGGW->GWARZ[LocId].guild_id)) + " " + sGGW->Currencyname + "'s.";

                        if (sGGW->GWCOMM[loc_guild_id].guild_invite == 1) { AddGossipItemFor(player, 1, join, GOSSIP_SENDER_MAIN, 1); };

                        AddGossipItemFor(player, 1, ranked, GOSSIP_SENDER_MAIN, 3);
                        AddGossipItemFor(player, 1, total_locations, GOSSIP_SENDER_MAIN, 3);
                        AddGossipItemFor(player, 1, total_value, GOSSIP_SENDER_MAIN, 3);
                        AddGossipItemFor(player, 1, "Nevermind", GOSSIP_SENDER_MAIN, 2);

                        SendGossipMenuFor(player, 1, go->GetGUID());

                        return true;

                    }
                }
            }
            return false;
        }

        bool OnGossipSelect(Player* player, GameObject* go, uint32 /* sender */, uint32 actions)
        {
            uint32 map_id = player->GetMapId();
            uint32 area_id = player->GetAreaId();
            uint32 zone_id = player->GetZoneId();

            uint64 LocId = sGGW->GetLocationID(map_id, area_id, zone_id);

            std::string str_LocId;
            std::stringstream convert;
            convert << LocId;
            str_LocId = convert.str();

            Guild* guild = sGuildMgr->GetGuildById(sGGW->GWARZ[LocId].guild_id);
            uint32 guild_id = sGGW->GWARZ[LocId].guild_id;

            ClearGossipMenuFor(player);

            CloseGossipMenuFor(player);

            switch (actions)
            {
            case 1:
            {
                SQLTransaction trans(nullptr);
                guild->AddMember(trans, player->GetGUID(), sGGW->MAX_GUILD_RANKS);

                std::string ann = sGGW->GWCOMM[guild_id].color_1 + player->GetName() + " Has Joined the Guild via the 'Guild Warz Guild Invite System'.";

                sGGW->SendGuildMessage(sGGW->GWARZ[LocId].guild_id, ann);

                new GGW_PigPayz(player);
            }break;

            case 2:
            {
                // selected 'nevermind'-do nothing . system will close window.
            }break;

            case 3:
            {
                OnGossipHello(player, go);
            }break;
            }
            return true;
        }
    };
//    void UpdateAI(uint32 diff) override
 //   {
//        _events.Update(diff);
 //   }

    GameObjectAI* GetAI(GameObject* go) const override
    {
        return new GGW_GUILD_FLAG_AI(go);
    }
};

// ********** Guild Player Zone Change ************

class GGW_Guild_Player_zone_change : public PlayerScript
{
public: GGW_Guild_Player_zone_change() : PlayerScript("GGW_Guild_Player_zone_change"){ }

	virtual void OnUpdateZone(Player* player, uint32 newZone, uint32 newArea)
	{
		FactionReset(player);
	
		if (player->GetGuild() > 0)
		{
			uint32 map_id = player->GetMapId();
			uint32 area_id = player->GetAreaId();
			uint32 zone_id = player->GetZoneId();
			float pX = player->GetPositionX();
			float pY = player->GetPositionY();
			float pZ = player->GetPositionZ();

			uint32 LocId = sGGW->GetLocationID(map_id, area_id, zone_id);

			if (!LocId)
			{
				LocId = sGGW->CreateGuildLocation(map_id, area_id, zone_id, pX, pY, pZ);
			}

			if ((sGGW->GWARZ[LocId].guild_id != player->GetGuildId()) && (sGGW->GWARZ[LocId].team < 2))
			{
				if ((sGGW->GWARZ[LocId].team != player->GetTeamId()) || (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].anarchy == 1))
				{
					std::string Pname = player->GetName();
					std::string Pguild_name = player->GetGuild()->GetName();
					uint32 guild_id = player->GetGuild()->GetId();

					std::string ann = sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_13 + player->GetName() + sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_15 + " of the Guild " + sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_13 + "`" + player->GetGuild()->GetName() + "`" + sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_15 + " has entered location:" + sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_13 + sGGW->ConvertNumberToString(LocId) + sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].color_15 + ". !Hurry!";

					sGGW->SendGuildMessage(sGGW->GWARZ[LocId].guild_id, ann);
				}
			}
		}
	}
};

// ************ Guild Npc/Go actions **************

void UpdateGameObject(GameObject* go, uint32 base_id, uint32 LocId)
{
	uint32 LOC_NPC_ID = (base_id + sGGW->GWARZ[LocId].team);
	uint32 LOC_FACTION_ID = (84 - sGGW->GWARZ[LocId].team);

	uint32 id = go->GetEntry();

	if (sGGW->GWARZ[LocId].team >= sGGW->SERVER_GUILD_TEAM)
		go->SetFaction(35);
	else
	{
		const GameObjectTemplate* gob = sObjectMgr->GetGameObjectTemplate(LOC_NPC_ID);

		if (id != LOC_NPC_ID)
		{
			go->SetEntry(LOC_NPC_ID);
			go->SetDisplayId(gob->displayId);
		}


		if (go->GetFaction() != LOC_FACTION_ID)
		{
			go->SetFaction(LOC_FACTION_ID);
		};
	}
	go->SaveToDB();
};

class GGW_GUILD_VAULT : public GameObjectScript
{
public: GGW_GUILD_VAULT() : GameObjectScript("GGW_GUILD_VAULT"){ }

        struct GGW_GUILD_VAULTAI : public GameObjectAI
        {
            GGW_GUILD_VAULTAI(GameObject* go) : GameObjectAI(go) { }

            bool OnGossipHello(Player* player, GameObject* go) // This will show first when a player clicks on a GameObject (Gossip)
            {
                TC_LOG_INFO("server.loading", "GUILD_VAULT_TRIGGER_1");
                uint32 GoId = go->GetEntry();
                uint32 map_id = go->GetMapId();
                uint32 area_id = go->GetAreaId();
                uint32 zone_id = go->GetZoneId();

                uint32 LocId = sGGW->GetLocationID(map_id, area_id, zone_id);

                uint32 lGuild_id = sGGW->GWARZ[LocId].guild_id;
                uint32 pGuild_id = player->GetGuildId();
                auto TeamId = player->GetTeamId();

                if (GoId != (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vault_id + sGGW->GWARZ[LocId].team))
                {
                    UpdateGameObject(go, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vault_id, LocId);

                    return false;
                }
                return true;
            }
        };
        GameObjectAI* GetAI(GameObject* go) const override
        {
            return new GGW_GUILD_VAULTAI(go);
        }
};

class GGW_GUILD_MAILBOX : public GameObjectScript
{
public:
       GGW_GUILD_MAILBOX() : GameObjectScript("GGW_GUILD_MAILBOX"){ }
		
		struct GGW_GUILD_MAILBOX_AI : public GameObjectAI
		{
            GGW_GUILD_MAILBOX_AI(GameObject* go) : GameObjectAI(go) {}


		    void UpdateAI(uint32 diff) override // This will show first when a player clicks on a GameObject (Gossip)
		    {
			    uint32 map_id = me->GetMapId();
			    uint32 area_id = me->GetAreaId();
			    uint32 zone_id = me->GetZoneId();

			    uint32 LocId = sGGW->GetLocationID(map_id, area_id, zone_id);

			    uint32 GoId = me->GetEntry();

			    if (GoId != (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].mailbox_id + sGGW->GWARZ[LocId].team))
			    {
				    UpdateGameObject(me, sGGW->GWCOMM[sGGW->sGGW->SERVER_GUILD_ID].mailbox_id, LocId);
			    }
		    }
	    };

	    GameObjectAI* GetAI(GameObject* go) const override // We then return new 'go_tutAI' so the script will load and work ingame
	    {
		    return new GGW_GUILD_MAILBOX_AI(go);
	    };
};

void UpdateCreature(Creature* creature, uint32 base_id, uint32 LocId)
{
uint32 NPC_ID = creature->GetEntry();
uint32 LOC_NPC_ID = (base_id + sGGW->GWARZ[LocId].team);
uint32 LOC_FACTION_ID = (84 - sGGW->GWARZ[LocId].team);
uint32 LOC_GUILD_ID = sGGW->GWARZ[LocId].guild_id;

uint32 id = creature->GetEntry();

	if (creature->GetFaction() != LOC_FACTION_ID)
	{
		if (sGGW->GWARZ[LocId].team < sGGW->SERVER_GUILD_TEAM)
		{
			const CreatureTemplate* mob = sObjectMgr->GetCreatureTemplate(LOC_NPC_ID);

			if (id != LOC_NPC_ID)
			{
				creature->SetEntry(LOC_NPC_ID);
				creature->SetDisplayId(mob->Modelid1);
			}


			if (creature->GetFaction() != LOC_FACTION_ID)
			{
				creature->SetFaction(LOC_FACTION_ID);
			};
			creature->SaveToDB();
		}
	}
};

class GGW_GUILD_PIG : public CreatureScript
{
public:	GGW_GUILD_PIG() : CreatureScript("GGW_GUILD_PIG"){ }

		struct GuildPig_Scripts_AI : public ScriptedAI
		{
			GuildPig_Scripts_AI(Creature* creature) : ScriptedAI(creature) {}

			void MoveInLineOfSight(Unit* unit) override
			{
				Creature *creature = me;

				uint32 cGuid = creature->GetSpawnId();
				uint32 cFaction = creature->GetFaction();

				uint32 map_id = creature->GetMapId();
				uint32 area_id = creature->GetAreaId();
				uint32 zone_id = creature->GetZoneId();

				uint32 LocId = sGGW->GetLocationID(map_id, area_id, zone_id);
				uint32 guild_id = sGGW->GWARZ[LocId].guild_id;

				if (unit->ToPlayer())
					{
						Player* player = unit->ToPlayer();
						uint32 pFaction = player->GetFaction();
	
						FactionReset(player);

						if ((creature->GetEntry() + sGGW->GWARZ[LocId].team) != sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_id)
						{
							UpdateCreature(creature, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].pig_id, LocId);
						}
					}
				ScriptedAI::MoveInLineOfSight(unit);
			}
		};
	CreatureAI* GetAI(Creature* creature) const override
	{
		return new GuildPig_Scripts_AI(creature);
	}
};

class GGW_GUILD_GUARD : public CreatureScript
{
public: GGW_GUILD_GUARD() : CreatureScript("GGW_GUILD_GUARD"){ }

		struct GGW_GUILD_GUARD_Scripts : public ScriptedAI
		{
			GGW_GUILD_GUARD_Scripts(Creature* creature) : ScriptedAI(creature) { }

			void Reset() override
			{
				Creature* creature = me;

				uint32 map_id = creature->GetMapId();
				uint32 area_id = creature->GetAreaId();
				uint32 zone_id = creature->GetZoneId();

				uint32 LocId = sGGW->GetLocationID(map_id, area_id, zone_id);

				uint32 cGuid = creature->GetSpawnId();

				Map* map = creature->GetMap();

				float x, y, z, o;

				x = creature->GetPositionX();
				y = creature->GetPositionY();
				z = creature->GetPositionZ();
				o = creature->GetOrientation();

				if (!sGGW->GGW_Creature[cGuid].LocId)
				{

					GGW_CreatureData& data = sGGW->GGW_Creature[cGuid]; // Filling in the needed data to save
					data.LocId = LocId;
					data.map_id = map_id;
					data.area_id = area_id;
					data.zone_id = zone_id;
					data.x = x;
					data.y = y;
					data.z = z;
					data.o = o;
					data.map = map;
				}

				if (sGGW->GGW_Creature[cGuid].LocId != LocId)
				{
					creature->NearTeleportTo(sGGW->GGW_Creature[cGuid].x, sGGW->GGW_Creature[cGuid].y, sGGW->GGW_Creature[cGuid].z, true);
					creature->CombatStop(true);
					creature->ClearInCombat();
					creature->SetPvP(false);
				}
			}

			void MoveInLineOfSight(Unit* unit) override
			{
				Creature* creature = me;

				uint32 cGuid = creature->GetSpawnId();

				uint32 map_id = creature->GetMapId();
				uint32 area_id = creature->GetAreaId();
				uint32 zone_id = creature->GetZoneId();

				uint32 LocId = sGGW->GetLocationID(map_id, area_id, zone_id);

//				uint32 Unit_LocId = GetLocationID(unit->GetMapId(), unit->GetAreaId(), unit->GetZoneId());

				if ((creature->GetEntry() + sGGW->GWARZ[sGGW->GGW_Creature[cGuid].LocId].team) != sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].guard_id)
				{
					UpdateCreature(creature, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].guard_id, sGGW->GGW_Creature[cGuid].LocId);
				}
		
				if (unit->ToPlayer())
				{
					Player* player = unit->ToPlayer();

					if (sGGW->GGW_Creature[cGuid].LocId != LocId)
					{
						creature->NearTeleportTo(sGGW->GGW_Creature[cGuid].x, sGGW->GGW_Creature[cGuid].y, sGGW->GGW_Creature[cGuid].z, true);

						creature->Respawn(true);
						creature->CombatStop(true);
						creature->ClearInCombat();
						creature->SetPvP(false);
					}

					FactionReset(player);
				}

				ScriptedAI::MoveInLineOfSight(unit);
			}

			void UpdateAI(uint32 diff) override // This function updates every 100 or 1000
			{
				Creature* creature = me;

				uint32 cGuid = creature->GetSpawnId();

				uint32 map_id = creature->GetMapId();
				uint32 area_id = creature->GetAreaId();
				uint32 zone_id = creature->GetZoneId();

				uint32 LocId = sGGW->GetLocationID(map_id, area_id, zone_id);

				if (sGGW->GGW_Creature[cGuid].LocId != LocId)
				{
					creature->NearTeleportTo(sGGW->GGW_Creature[cGuid].x, sGGW->GGW_Creature[cGuid].y, sGGW->GGW_Creature[cGuid].z, true);
					creature->ClearInCombat();
					creature->CombatStop(true);
					creature->SetPvP(false);
				}
			}

			void EnterCombat(Unit* unit)
			{
				Creature* creature = me;

				uint32 cGuid = creature->GetSpawnId();

				uint32 map_id = creature->GetMapId();
				uint32 area_id = creature->GetAreaId();
				uint32 zone_id = creature->GetZoneId();

				uint32 LocId = sGGW->GetLocationID(map_id, area_id, zone_id);
				std::string LocName = sGGW->ConvertNumberToString(LocId);
				uint32 LocGuildId = sGGW->GWARZ[LocId].guild_id;

				if (sGGW->GGW_Creature[cGuid].LocId != LocId)
				{
					creature->NearTeleportTo(sGGW->GGW_Creature[cGuid].x, sGGW->GGW_Creature[cGuid].y, sGGW->GGW_Creature[cGuid].z, false);

					creature->Respawn(true);
					creature->CombatStop(true);
					creature->SetPvP(false);
					creature->ClearInCombat();
				}

				if (unit->ToPlayer())
				{
					Player *player = unit->ToPlayer();

					if (sGGW->GGW_Creature[cGuid].LocId != LocId)
					{
						creature->ClearInCombat();
						creature->CombatStop(true);
						creature->SetPvP(false);
					}

					if (sGGW->GGW_Creature[cGuid].LocId == LocId)
					{
						if (player->GetGuild()->GetId() > 0)
						{
							creature->Yell("You're gonna !Die! Scum!", LANG_UNIVERSAL, player);
							creature->SetPvP(true);
							creature->Attack(player, true);
							creature->SetInCombatWith(player);

							std::string ann = "<" + sGGW->GWCOMM[LocGuildId].color_8 + "Guard" + "|r>";
							ann += sGGW->GWCOMM[LocGuildId].color_15 + "I have engaged ";
							ann += sGGW->GWCOMM[LocGuildId].color_13 + player->GetName();
							ann += sGGW->GWCOMM[LocGuildId].color_15 + " of the Guild ";
							ann += sGGW->GWCOMM[LocGuildId].color_13 + "`" + player->GetGuild()->GetName() + "`";
							ann += sGGW->GWCOMM[LocGuildId].color_15 + " at location:";
							ann += sGGW->GWCOMM[LocGuildId].color_13 + LocName;
							ann += sGGW->GWCOMM[LocGuildId].color_15 + ". !Hurry!";

							sGGW->SendGuildMessage(sGGW->GWARZ[LocId].guild_id, ann);
						}
					}
				}
			}

			void KilledUnit(Unit* unit) override
			{
				Creature* creature = me;

				if (unit->ToPlayer())
				{
					creature->Yell("!You ! GoT ! SpadeD !", LANG_UNIVERSAL, unit);
				}
			}
			
			void JustDied(Unit* unit)
			{
				Creature* creature = me;

				if (unit->ToPlayer())
				{
					unit->ToPlayer()->AddItem(sGGW->Guard_Died_Drop_Reward, 3);

					uint32 LocId = sGGW->GGW_Creature[creature->GetSpawnId()].LocId;

					creature->Yell("!You Scum! .. You killed me!", LANG_UNIVERSAL, unit);

					sGGW->GGW_Creature.erase(creature->GetSpawnId());

					sGGW->UpdateGuildLocData("guard_count", sGGW->ConvertNumberToString(sGGW->GWARZ[LocId].guard_count - 1), LocId);

					creature->RemoveFromWorld();
					creature->DeleteFromDB(creature->GetSpawnId());
				}
			}
		};

		CreatureAI* GetAI(Creature* creature)const override
	{
		return new GGW_GUILD_GUARD_Scripts(creature);
	}
};

class GGW_GUILD_VENDOR1 : public CreatureScript
{
public: GGW_GUILD_VENDOR1() : CreatureScript("GGW_GUILD_VENDOR1"){ }

	bool OnGossipHello(Player* player, Creature* creature) // override // virtual 
	{
		TC_LOG_INFO("server.loading", "GUILD_VENDOR1_TRIGGER_1");

		if (sGGW->vendor1)
		{
			uint32 map_id = creature->GetMapId();
			uint32 area_id = creature->GetAreaId();
			uint32 zone_id = creature->GetZoneId();

			uint32 LocId = sGGW->GetLocationID(map_id, area_id, zone_id);

			uint32 lGuild_id = sGGW->GWARZ[LocId].guild_id;
			uint32 pGuild_id = player->GetGuildId();

				if (pGuild_id != lGuild_id)
				{
					CloseGossipMenuFor(player);

					creature->Yell("You're gonna !Die! Scum!", LANG_UNIVERSAL, player);

					return false;
				}
				uint32 int_id = 0;

				for (const auto& buffNames : sGGW->VENDOR1_BUFF_NAMES)
				{
					AddGossipItemFor(player, 1, buffNames, GOSSIP_SENDER_MAIN, int_id);
					int_id++;
				}

			AddGossipItemFor(player, 1, "Nevermind.", GOSSIP_SENDER_MAIN, int_id + 1);
			SendGossipMenuFor(player, 1, creature->GetGUID());

		}
		return true;
	}

	bool OnGossipSelect(Player* player, Creature* creature, uint32 /* sender */, uint32 actions)
	{
		if (sGGW->VENDOR1_BUFF_IDS[actions])
		{
			player->AddAura(sGGW->VENDOR1_BUFF_IDS[actions], player);
		}

		CloseGossipMenuFor(player);

		return true;
	}
	struct GGW_Guild_Buff_Vendor1 : public ScriptedAI
	{
		GGW_Guild_Buff_Vendor1(Creature* creature) : ScriptedAI(creature) { }

			void MoveInLineOfSight(Unit* unit) override
			{
				Creature* creature = me;

				uint32 map_id = creature->GetMapId();
				uint32 area_id = creature->GetAreaId();
				uint32 zone_id = creature->GetZoneId();

				uint32 LocId = sGGW->GetLocationID(map_id, area_id, zone_id);

				if (unit->ToPlayer())
				{
					Player* player = unit->ToPlayer();

					if (unit->ToPlayer()->GetGuildId())
					{
						FactionReset(player);
					}
	
					if ((creature->GetEntry() + sGGW->GWARZ[LocId].team) != sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor1_id)
					{
						UpdateCreature(creature, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor1_id, LocId);
					}
				}
				ScriptedAI::MoveInLineOfSight(unit);
			}

			void EnterCombat(Unit* unit)
			{
				Creature* creature = me;

				creature->SetPvP(false);
				creature->ClearInCombat();
				creature->SendClearTarget();
				SetCombatMovement(false);
			}

	};
	CreatureAI* GetAI(Creature* creature) const override
	{
		return new GGW_Guild_Buff_Vendor1(creature);
	}
};

class GGW_GUILD_VENDOR2 : public CreatureScript
{
public: GGW_GUILD_VENDOR2() : CreatureScript("GGW_GUILD_VENDOR2"){ }

	bool OnGossipHello(Player* player, Creature* creature)
	{
		if (test) { TC_LOG_INFO("server.loading", "GUILD_VENDOR2_TRIGGER_1"); };

		if (sGGW->vendor2)
		{
			uint32 map_id = creature->GetMapId();
			uint32 area_id = creature->GetAreaId();
			uint32 zone_id = creature->GetZoneId();

			uint32 LocId = sGGW->GetLocationID(map_id, area_id, zone_id);

			uint32 lGuild_id = sGGW->GWARZ[LocId].guild_id;
			uint32 pGuild_id = player->GetGuildId();

			if (pGuild_id != lGuild_id)
			{
				CloseGossipMenuFor(player);

				creature->Yell("You're gonna !Die! Scum!", LANG_UNIVERSAL, player);

				return false;
			}
		}
		return true;
	}

	bool OnGossipSelect(Player* player, Creature* creature, uint32 /* sender */, uint32 actions)
	{
//		sObjectMgr->AddVendorItem(1, 100, 0, 0, 0, true);
//		sObjectMgr->AddVendorItem(1, 101, 0, 0, 0, true);

		return true;
	}

	struct GGW_Guild_Vendor2_AI : public ScriptedAI
	{
		GGW_Guild_Vendor2_AI(Creature* creature) : ScriptedAI(creature) { }

		void reset()
		{
			Creature *creature = me;

			creature->SetFullHealth();

		}

		void EnterCombat(Unit* unit)
		{
			Creature *creature = me;

			creature->SetPvP(false);
			creature->ClearInCombat();
			creature->SendClearTarget();
			SetCombatMovement(false);
		}

		void MoveInLineOfSight(Unit* unit) override
		{
			Creature *creature = me;

			uint32 cGuid = creature->GetSpawnId();

			uint32 map_id = creature->GetMapId();
			uint32 area_id = creature->GetAreaId();
			uint32 zone_id = creature->GetZoneId();

			uint32 LocId = sGGW->GetLocationID(map_id, area_id, zone_id);

			if (unit->ToPlayer())
			{
				Player* player = unit->ToPlayer();

				if (unit->ToPlayer()->GetGuildId())
				{
					FactionReset(player);
				}

				if ((creature->GetEntry() + sGGW->GWARZ[LocId].team) != sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor2_id)
				{
					UpdateCreature(creature, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor2_id, LocId);
				}
			}
			ScriptedAI::MoveInLineOfSight(unit);
		}
	};
		
	CreatureAI* GetAI(Creature* creature)const override
	{
		return new GGW_Guild_Vendor2_AI(creature);
	}
};

class GGW_GUILD_VENDOR3 : public CreatureScript
{
public: GGW_GUILD_VENDOR3() : CreatureScript("GGW_GUILD_VENDOR3"){ }

		bool OnGossipHello(Player* player, Creature* creature)
		{
			TC_LOG_INFO("server.loading", "GUILD_VENDOR3_TRIGGER_1");

			if (sGGW->vendor3)
			{
				uint32 map_id = creature->GetMapId();
				uint32 area_id = creature->GetAreaId();
				uint32 zone_id = creature->GetZoneId();

				uint32 LocId = sGGW->GetLocationID(map_id, area_id, zone_id);

				uint32 lGuild_id = sGGW->GWARZ[LocId].guild_id;
				uint32 pGuild_id = player->GetGuildId();

				if (pGuild_id != lGuild_id)
				{
					CloseGossipMenuFor(player);

					creature->Yell("You're gonna !Die! Scum!", LANG_UNIVERSAL, player);

					return false;
				}
				return true;
			}
			return true;
		}

		bool OnGossipSelect(Player* player, Creature* creature, uint32 /* sender */, uint32 actions)
		{
			return true;
		}

		struct GGW_Guild_Vendor3_AI : public ScriptedAI
		{
			GGW_Guild_Vendor3_AI(Creature* creature) : ScriptedAI(creature) { }

			void reset()
			{
				Creature *creature = me;

				creature->SetFullHealth();

			}

			void MoveInLineOfSight(Unit* unit) override
			{
				Creature *creature = me;

				uint32 map_id = creature->GetMapId();
				uint32 area_id = creature->GetAreaId();
				uint32 zone_id = creature->GetZoneId();

				uint32 LocId = sGGW->GetLocationID(map_id, area_id, zone_id);

				if (unit->ToPlayer())
				{
					Player* player = unit->ToPlayer();

					if (unit->ToPlayer()->GetGuildId())
					{
						FactionReset(player);
					}

					if ((creature->GetEntry() + sGGW->GWARZ[LocId].team) != sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor3_id)
					{
						UpdateCreature(creature, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].vendor3_id, LocId);
					}
				}
				ScriptedAI::MoveInLineOfSight(unit);
			}

			void EnterCombat(Unit* unit)
			{
				Creature *creature = me;

				creature->SetPvP(false);
				creature->ClearInCombat();
				creature->SendClearTarget();
				SetCombatMovement(false);
			}
		};

		CreatureAI* GetAI(Creature* creature)const override
		{
			return new GGW_Guild_Vendor3_AI(creature);
		}
};

class GGW_GUILD_CANNON : public CreatureScript
{
public: GGW_GUILD_CANNON() : CreatureScript("GGW_GUILD_CANNON"){ }

		bool OnGossipHello(Player* player, Creature* creature)
		{

			uint32 map_id = creature->GetMapId();
			uint32 area_id = creature->GetAreaId();
			uint32 zone_id = creature->GetZoneId();

			uint32 LocId = sGGW->GetLocationID(map_id, area_id, zone_id);

			uint32 lGuild_id = sGGW->GWARZ[LocId].guild_id;
			uint32 pGuild_id = player->GetGuildId();

			if (pGuild_id != lGuild_id)
			{
				CloseGossipMenuFor(player);

				creature->Yell("You're gonna !Die! Scum!", LANG_UNIVERSAL, player);

				return false;
			}
			return true;
		}

		struct GGW_GUILD_CANNON_AI : public ScriptedAI
		{
			GGW_GUILD_CANNON_AI(Creature* creature) : ScriptedAI(creature) { }

			Creature* creature = me;

			uint32 map_id = creature->GetMapId();
			uint32 area_id = creature->GetAreaId();
			uint32 zone_id = creature->GetZoneId();

			uint32 LocId = sGGW->GetLocationID(map_id, area_id, zone_id);

			void reset()
			{
				creature->SetFullHealth();
			}

			void MoveInLineOfSight(Unit* unit) override
			{

				if (unit->ToPlayer())
				{
					Player* player = unit->ToPlayer();

					if (unit->ToPlayer()->GetGuildId())
					{
						FactionReset(player);
					}

					if ((creature->GetEntry() + sGGW->GWARZ[LocId].team) != sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].cannon_id)
					{
						UpdateCreature(creature, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].cannon_id, LocId);
					}
				}
				ScriptedAI::MoveInLineOfSight(unit);
			}

			void JustDied(Unit* unit)
			{
				if (unit->ToPlayer())
				{
					Player *player = unit->ToPlayer();

					creature->Yell("!You Scum! .. You killed me!", LANG_UNIVERSAL, player);

					sGGW->UpdateGuildLocData("cannon_count", sGGW->ConvertNumberToString(sGGW->GWARZ[LocId].cannon_count - 1), LocId);

					creature->DeleteFromDB(creature->GetSpawnId());
				}
			}
		};

		CreatureAI* GetAI(Creature* creature)const override
		{
			return new GGW_GUILD_CANNON_AI(creature);
		}
};

class GGW_GUILD_INFO_STATION : public CreatureScript
{
public: GGW_GUILD_INFO_STATION() : CreatureScript("GGW_GUILD_INFO_STATION"){ }

		bool OnGossipHello(Player* player, Creature* creature)// override // virtual 
		{
			uint32 guild_id = player->GetGuild()->GetId();
			uint32 player_guild_rank = player->GetRank();

			AddGossipItemFor(player, 1, "About Grumbo'z Guild Warz.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);

			AddGossipItemFor(player, 1, "about the flags...", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1003);

			if (player_guild_rank >= sGGW->GWCOMM[guild_id].GLD_lvlb){ AddGossipItemFor(player, 1, "buy", GOSSIP_SENDER_MAIN, 1004); };

			if (player_guild_rank >= sGGW->GWCOMM[guild_id].GLD_lvls){ AddGossipItemFor(player, 1, "sell", GOSSIP_SENDER_MAIN, 1005); };

			AddGossipItemFor(player, 1, "Nevermind.", GOSSIP_SENDER_MAIN, 1000);
			SendGossipMenuFor(player, 1, creature->GetGUID());

			return true;
		}

		bool OnGossipSelect(Player* player, Creature* creature, uint32 /* sender */, uint32 actions)
		{
			if (!player){ return false; };

			player->PlayerTalkClass->ClearMenus();

			switch (actions)
			{
				case GOSSIP_ACTION_INFO_DEF + 1000:
					CloseGossipMenuFor(player);
				break;

				case GOSSIP_ACTION_INFO_DEF + 1001:
					CloseGossipMenuFor(player);
					OnGossipHello(player, creature);
				break;

				case GOSSIP_ACTION_INFO_DEF + 1002:

					TC_LOG_INFO("server.loading", "ABOUT System");
					
					AddGossipItemFor(player, 10, "Welcome to Grumbo'z Guild Warz.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);
					AddGossipItemFor(player, 10, "An aggresive Guild Plot System.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);
					AddGossipItemFor(player, 10, "This allows Guilds to buy many plots, invade apposing Guild plots,", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);

						if (sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].anarchy == 1)
						{
							AddGossipItemFor(player, 10, "this is to include your own team Guild's.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);
							AddGossipItemFor(player, 10, "You can invade any other Guild location of either team.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);
							AddGossipItemFor(player, 10, "Becoming the most feared Guild in the realm..", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);
						}

					AddGossipItemFor(player, 10, "Expanding your Guild's land cutting a path deep ", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);
					AddGossipItemFor(player, 10, "into enemy territory. Build them up with farms to support pigs for proffit", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);
					AddGossipItemFor(player, 10, " and barracks to support guards.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);
					AddGossipItemFor(player, 10, "You can add a Guild Hall at every location for your Guildies to chill in.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1002);
					AddGossipItemFor(player, 10, "Nevermind.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1001);

					SendGossipMenuFor(player, 1, creature->GetGUID());

				break;

				case GOSSIP_ACTION_INFO_DEF + 1003:
					ClearGossipMenuFor(player);

					AddGossipItemFor(player, 1, "Welcome to Grumbo'z Guild Warz.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1010);
					AddGossipItemFor(player, 9, "Nevermind.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1001);

					SendGossipMenuFor(player, 1, creature->GetGUID());
				break;

				case GOSSIP_ACTION_INFO_DEF + 1004:
					ClearGossipMenuFor(player);
				
					AddGossipItemFor(player, 1, "Welcome to Grumbo'z Guild Warz.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1010);
					AddGossipItemFor(player, 9, "Nevermind.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1001);

					SendGossipMenuFor(player, 1, creature->GetGUID());
					break;

				case GOSSIP_ACTION_INFO_DEF + 1005:
					player->PlayerTalkClass->ClearMenus();

					AddGossipItemFor(player, 1, "Welcome to Grumbo'z Guild Warz.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1010);
					AddGossipItemFor(player, 9, "Nevermind.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1001);

					SendGossipMenuFor(player, 1, creature->GetGUID());
				break;
			break;
			}
		return true;
		}

		struct GUILD_INFO_STATION : public ScriptedAI
		{
			GUILD_INFO_STATION(Creature* creature) : ScriptedAI(creature) { }
		};

		CreatureAI* GetAI(Creature* creature)const override
		{
			return new GUILD_INFO_STATION(creature);
		}
};

class GGW_GUILD_LEADER_QUEST_ENGINE : public CreatureScript
{
	public: GGW_GUILD_LEADER_QUEST_ENGINE() : CreatureScript("GGW_GUILD_LEADER_QUEST_ENGINE") { }
		
		bool OnQuestReward(Player* player, Creature* creature, Quest const* quest, uint32 opt)
		{
			if (quest->GetQuestId() == 52000 || 52001)
			{
				uint32 PLAYER_GUILD_ID = player->GetGuildId();

				sGGW->UpdateGuildCommandData("allowed", sGGW->ConvertNumberToString(1), PLAYER_GUILD_ID);

				ChatHandler(player->GetSession()).PSendSysMessage("<%sGrumbo|r>:%sYour Guild can now aquire/develop/buy or sell any lands they may come to control during there travels.", sGGW->GWCOMM[PLAYER_GUILD_ID].color_6.c_str(), sGGW->GWCOMM[PLAYER_GUILD_ID].color_10.c_str());

				player->AddItem(sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].currency, sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].gift_count);

				ChatHandler(player->GetSession()).PSendSysMessage("<%s%s|r>:%sHere is %u %s's to get your Guild Plot started.", sGGW->GWCOMM[PLAYER_GUILD_ID].color_6.c_str(), creature->GetName(), sGGW->GWCOMM[PLAYER_GUILD_ID].color_10.c_str(), sGGW->GWCOMM[sGGW->SERVER_GUILD_ID].gift_count, sGGW->Currencyname.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("<%s%s|r>:%sNow Leave and seek out lands in the name of %s!The Horde!.", sGGW->GWCOMM[PLAYER_GUILD_ID].color_6.c_str(), creature->GetName(), sGGW->GWCOMM[PLAYER_GUILD_ID].color_10.c_str(), sGGW->GWCOMM[PLAYER_GUILD_ID].color_15.c_str());

				return true;
			}
		};

	struct Guild_Leader_QuestEngine : public ScriptedAI
	{
		Guild_Leader_QuestEngine(Creature* creature) : ScriptedAI(creature) { }
	};

	CreatureAI* GetAI(Creature* creature)const override
	{
		return new Guild_Leader_QuestEngine(creature);
	}
};

void AddSC_Grumboz_Guild_Warz()
{
	new GGW_RankTimer();
    new GGW_LoadGWtable();
	new GGW_GuildEngine();
	new GGW_GuildPlayerOnLogin();
	new GGW_commands();
	new GGW_GUILD_FLAG();
	new GGW_Guild_Player_zone_change();
	new GGW_GUILD_VAULT();
	new GGW_GUILD_MAILBOX();
	new GGW_GUILD_PIG();
	new GGW_GUILD_GUARD();
	new GGW_GUILD_VENDOR1();
	new GGW_GUILD_VENDOR2();
	new GGW_GUILD_VENDOR3();
	new GGW_GUILD_CANNON();
	new GGW_GUILD_INFO_STATION();
	new GGW_GUILD_LEADER_QUEST_ENGINE();
}
