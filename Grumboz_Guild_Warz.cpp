/*
-- **g****************************************s***
-- ********© Grumbo'z Guild Warz System™ ©********
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
#include "GameObject.h"
#include "GameObjectAI.h"
#include "GossipDef.h"
#include "Grumboz_Guild_Warz.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "item.h"
#include "Language.h"
#include "MapManager.h"
// #include "ObjectAccessor.h"
// #include "ObjectDefines.h"
// #include "Opcodes.h"
#include "ObjectMgr.h"
#include "player.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "mysql.h"
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include "World.h"
#include "WorldSession.h"

// defining most Globals now
// hard stored  settings //
float core_version = 6.70f;
float table_version = 2.88f;
float pigpayz_version = 2.50f;
float tele_version = 1.50f;
float pvp_version = 4.88f;
float vendor_version = 1.55f;

uint8 MAX_GUILD_RANKS = 255;
uint8 SERVER_GUILD_TEAM_LOCKED_ID = 3;
uint8 SERVER_GUILD_TEAM = 2;
uint32 SERVER_GUILD_ID = 0;
uint64 Guard_Died_Drop_Reward = 20558; // wsg's not yet added

std::string SERVER_GUILD_NAME = "SERVER";
std::string GUILD_RANK_COMMAND = "rank";
std::string EVIL_DOER = "!Evil Do`er!";

int GGW_MSG = -1;

bool test = false; // test mode true/false . this will cause the core to belch out data as events happen for help with bug fixes.

uint64 seconds = 1000;
uint64 minutes = 60000;
uint64 hours = 3600000;

// db address's in string value for sql updates
std::string Guild_Warz_DB = sConfigMgr->GetStringDefault("GUILD_WARZ.DB_ADDRESS", "guild_warz_335");

std::string commands_db = Guild_Warz_DB + ".commands";
std::string help_db = Guild_Warz_DB + ".help";
std::string zones_db = Guild_Warz_DB + ".zones";
std::string ranking_db = Guild_Warz_DB + ".ranking";

// pre-defining other Global's for later
uint32 GUILDWARZ_PLAYER_CHECK_TIMER;
uint32 GUILDWARZ_PIGPAYZ_VALUE;
uint32 GUILDWARZ_PIGPAYZ_TIMER;
uint32 GUILDWARZ_RANK_TYPE;
uint32 GUILDWARZ_RANKING_TIMER;
uint32 GUILDWARZ_RANKING_MAX;

uint32 Currencyid;
std::string Currencyname;

// ---------------------------------------------------- -
// built - in vendors operational switches and item tables
// ---------------------------------------------------- -
const std::string VENDOR1_BUFF_NAMES[10] = { "Armor + 10%", "Damage + (1 - 10)%", "Resistances + 25", "Agility + 10%", "Intelligence + 10%", "Spirit + 10%", "Strength + 10%", "Stamina + 10%", "Spell Additional Damage + 3% chance", "Heal Me" };
uint32 VENDOR1_BUFF_IDS[10] = { 23767, 23768, 23769, 23736, 23766, 23738, 23735, 23737, 30557, 25840 };
// uint32 VENDOR2_ITEMS[11] = { 7734, 6948, 49912, 34498, 46693, 34499, 35557, 37431, 17202, 21038, 46783 }; // fun items
// uint32 VENDOR3_ITEMS[11] = {32837, 32838, 22736, 19019, 51858, 24550, 2000, 50730, 50070, 34196, 30906}; // misc gear

bool vendor1 = true; // use built - in vendor 1 . false / true = no / yes.
bool vendor2 = true; // use built - in vendor 2 . false / true = no / yes.
bool vendor3 = true; // use built - in vendor 3 . false / true = no / yes.

// pre-define global core tables
std::unordered_map<uint32, Commands>GWCOMM;
std::unordered_map<uint32, Help>GWHELP;
std::unordered_map<uint32, LocData>GWARZ;
std::unordered_map<uint32, GGW_CreatureData>GGW_Creature;
std::map<uint32, rank_info>GW_Ranks;
std::map<uint32, uint32>GW_RANKS;

GuildWarz::GuildWarz() { }

GuildWarz::~GuildWarz()
{
}

uint64 GGW_ConvertStringToNumber(std::string arg)
{
	uint64 Value64;

	std::istringstream(arg) >> Value64;

	return Value64;
}

std::string ConvertNumberToString(uint64 numberX)
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

bool LoadCommands()
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

			Commands& data = GWCOMM[guild_id]; // like Lua table GWARZ[guild_id].entry
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

bool LoadHelp()
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

			Help& data = GWHELP[entry]; // like Lua table GWHELP[entry].entry
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

bool LoadLoc()
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

			LocData& data = GWARZ[entry]; // like Lua table GWARZ[guild_id].entry
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

float GW_version = ((table_version + core_version + pigpayz_version + tele_version + pvp_version + vendor_version) / 4);

uint32 GetTotalPigs(uint32 guild_id)
{
	uint32 loc_id = 0;
	uint32 pig_total = 0;

	for (loc_id = 1; loc_id < GWARZ.size(); loc_id++)
	{
		if (GWARZ[loc_id].guild_id == guild_id) { pig_total = pig_total + GWARZ[loc_id].pig_count; };
	}
	return pig_total;
};

uint32 CalculateTotalLocations(uint32 guild_id)
{
	uint32 loc_id = 0;
	uint32 loc_total = 0;

	for (loc_id = 1; loc_id < GWARZ.size(); loc_id++)
	{
		if (GWARZ[loc_id].guild_id == guild_id) { loc_total = loc_total + 1; };
	}
	return loc_total;
};

uint32 CalculateLocationValue(uint32 loc_id)
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

uint64 CalculateTotalLocationsValue(uint32 guild_id)
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

uint32 GetRank(uint32 guild_id)
{
	uint32 rank;
	uint32 guild_rank;
	uint32 size = GW_Ranks.size();

	for (rank = 1; rank <= size; rank++)
	{
		if (GW_Ranks[rank].guild_id == guild_id)
		{ 
			guild_rank = rank;
			break; 
		};
	}
	return guild_rank;
};

bool UpdateRankEntry(uint32 guild_id, uint32 total_gross_worth, uint8 team)
{
	if (test) { TC_LOG_INFO("server.loading", "UPDATE RANK ENRTY:: GUILD:%u WORTH:%u TEAM:%u", guild_id, total_gross_worth, team); };

	if (GWCOMM[guild_id].guild_id = guild_id)
	{
		WorldDatabase.PExecute("REPLACE INTO %s (`guild_id`, `team`, `total_gross_worth`) VALUES('%u', '%u', '%u');", ranking_db.c_str(), guild_id, team, total_gross_worth);

		uint32 rank;

		if (GetRank(guild_id))
			rank = GetRank(guild_id);
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

void BuildRankList()
{
	GW_Ranks.clear();

	uint32 rank = 1;

	QueryResult RankQry = WorldDatabase.Query(("SELECT `guild_id`,`team`,`total_gross_worth`  FROM " + ranking_db + " WHERE guild_id != '0' ORDER BY `total_gross_worth` DESC;").c_str());

		if (RankQry)
		{
			do
			{
				Field *fields = RankQry->Fetch();
				// Save the DB values to the LocData object
				uint32 guild_id = fields[0].GetUInt32();
				uint8 team = fields[1].GetUInt32();
				uint32 total_gross_worth = fields[2].GetUInt32();
	
				if (GWCOMM[guild_id].guild != "")
				{
					if(test){ TC_LOG_INFO("server.loading", "<RANK>%u <NAME>%s <GUILD_ID>%u <TEAM>%u", rank, GWCOMM[guild_id].guild, guild_id, team); };

					GW_Ranks[rank].guild_id = guild_id;
					GW_Ranks[rank].team = team;
					GW_Ranks[rank].total_gross_worth = total_gross_worth;
					GW_Ranks[rank].name = GWCOMM[guild_id].guild;

					rank = rank + 1;
				}
			} while (RankQry->NextRow());
		}
}

bool CreateRankList()
{
	uint32 guildid;

		for (std::unordered_map<uint32, Commands>::iterator itr1 = GWCOMM.begin(); itr1 != GWCOMM.end(); itr1++)
		{
			guildid = itr1->first;

			if (guildid != 0)
			{
				uint32 total_gross_worth = CalculateTotalLocationsValue(guildid);
				bool dummyUpdateRankEntry = UpdateRankEntry(guildid, total_gross_worth, GWCOMM[guildid].team);
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
		events.ScheduleEvent(1, GUILDWARZ_RANKING_TIMER);
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
				CreateRankList();

					if (GUILDWARZ_RANK_TYPE < 2) 
					{ 
						events.ScheduleEvent(1, GUILDWARZ_RANKING_TIMER); 

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

			bool com_table = LoadCommands();

			if (com_table)
			{
				TC_LOG_INFO("server.loading", "- commands loaded                    -", GWCOMM.size());
			}

			bool help_table = LoadHelp();

			if (help_table)
			{
				TC_LOG_INFO("server.loading", "- help system loaded                 -");
			}

			bool loc_table = LoadLoc();

			if (loc_table)
			{
				TC_LOG_INFO("server.loading", "- %u locations loaded                   ", GWARZ.size());
			}

			GUILDWARZ_PIGPAYZ_VALUE = sConfigMgr->GetIntDefault("GUILD_WARZ.PIGPAYZ_REWARD", 1000);
			GUILDWARZ_PIGPAYZ_TIMER = (sConfigMgr->GetIntDefault("GUILD_WARZ.PIGPAYZ_TIMER", 30)) * minutes;
			GUILDWARZ_RANKING_TIMER = (sConfigMgr->GetIntDefault("GUILD_WARZ.RANKING_TIMER", 10)) * minutes;
			GUILDWARZ_RANK_TYPE		= sConfigMgr->GetIntDefault("GUILD_WARZ.RANK_TYPE", 1);
			GUILDWARZ_RANKING_MAX	= sConfigMgr->GetIntDefault("GUILD_WARZ.RANK_MAX", 100);

			if (test){ TC_LOG_INFO("server.loading", "LOADING :: PIG_PAYZ :: PAYZ:%u TIMER:%u", GUILDWARZ_PIGPAYZ_VALUE, GUILDWARZ_PIGPAYZ_TIMER); };

			Currencyid = GWCOMM[SERVER_GUILD_ID].currency;

			if (!sObjectMgr->GetItemTemplate(Currencyid))
			{
				TC_LOG_INFO("server.loading", "- Error Loading Currency ID:%d . Item May NOT exsist in the sql DB.", Currencyid);
				TC_LOG_INFO("server.loading", "- Make sure you are using a valid item entry id in the sql DB.");
				TC_LOG_INFO("server.loading", "- Prepare for crash.");
			}
	
			Currencyname = sObjectMgr->GetItemTemplate(Currencyid)->Name1;

			TC_LOG_INFO("server.loading", "- Core version:%.2f                  -", core_version);
			TC_LOG_INFO("server.loading", "- tables version:%.2f                -", table_version);
			TC_LOG_INFO("server.loading", "- Pig Payz version:%.2f              -", pigpayz_version);
			TC_LOG_INFO("server.loading", "- Teleporter version:%.2f            -", tele_version);
			TC_LOG_INFO("server.loading", "- PvP version:%.2f                   -", pvp_version);

			if (vendor1) { TC_LOG_INFO("server.loading", "- Vendor1 loaded                     -"); };
			if (vendor2) { TC_LOG_INFO("server.loading", "- Vendor2 loaded                     -"); };
			if (vendor3) { TC_LOG_INFO("server.loading", "- Vendor3 loaded                     -"); };

			TC_LOG_INFO("server.loading", "- Vendor Core:%.2f                   -", tele_version);


			if (GUILDWARZ_RANK_TYPE < 2) 
			{
				TC_LOG_INFO("server.loading", "- Guild Ranking type:Timer           -", GW_version);
				CreateRankList();
				events.ScheduleEvent(1, GUILDWARZ_RANKING_TIMER);
			};

			if (GUILDWARZ_RANK_TYPE > 1) TC_LOG_INFO("server.loading", "- Guild Ranking type:OnEvent         -", GW_version);

			TC_LOG_INFO("server.loading", "______________________________________");
			TC_LOG_INFO("server.loading", "-          Goliath Online            -", GW_version);
			TC_LOG_INFO("server.loading", "-                                    -");
			TC_LOG_INFO("server.loading", "-        Guild Warz Ver:%.2fc        -", GW_version);
			TC_LOG_INFO("server.loading", "______________________________________");
		};
	EventMap events;
};

void SendGuildMessage(uint32 guild_id, std::string msg)
{
	SessionMap sessions = sWorld->GetAllSessions();

	msg = "[" + GWCOMM[guild_id].color_8 + "GuildWarz" + "|r]:" + msg;

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

uint32 CreateGuildLocation(uint32 map_id, uint32 area_id, uint32 zone_id, float pX, float pY, float pZ)
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

void UpdateGuildLocData(std::string column_target, std::string new_data, uint32 loc_id)
{
	WorldDatabase.PExecute("UPDATE %s SET `%s` = '%s' WHERE `entry` = '%u';", zones_db.c_str(), column_target.c_str(), new_data.c_str(), loc_id);

	if (column_target == "guild_name")
	{
		GWARZ[loc_id].guild_name = new_data.c_str();
	}
	if (column_target == "team")
	{
		GWARZ[loc_id].team = uint8(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "farm_count")
	{
		GWARZ[loc_id].farm_count = uint32(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "barrack_count")
	{
		GWARZ[loc_id].barrack_count = uint32(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "hall_count")
	{
		GWARZ[loc_id].hall_count = uint32(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "pig_count")
	{
		GWARZ[loc_id].pig_count = uint32(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "guard_count")
	{
		GWARZ[loc_id].guard_count = uint32(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "vendor1_count")
	{
		GWARZ[loc_id].vendor1_count = uint32(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "vendor2_count")
	{
		GWARZ[loc_id].vendor2_count = uint32(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "vendor3_count")
	{
		GWARZ[loc_id].vendor3_count = uint32(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "cannon_count")
	{
		GWARZ[loc_id].cannon_count = uint32(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "vault_count")
	{
		GWARZ[loc_id].vault_count = uint32(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "mailbox_count")
	{
		GWARZ[loc_id].mailbox_count = uint32(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "flag_id")
	{
		GWARZ[loc_id].flag_id = uint32(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "fs_time")
	{
		GWARZ[loc_id].fs_time = GGW_ConvertStringToNumber(new_data); // sWorld->GetGameTime();
	}
	if (column_target == "guild_id")
	{
		GWARZ[loc_id].guild_id = uint32(GGW_ConvertStringToNumber(new_data));
	}
};

void UpdateGuildLocFloat(float x, float y, float z, uint32 loc_id)
{
	WorldDatabase.PExecute("UPDATE %s SET x='%f', y='%f', z='%f' WHERE entry='%u';", zones_db.c_str(), x, y, z, loc_id);

	GWARZ[loc_id].x = x;
	GWARZ[loc_id].y = y;
	GWARZ[loc_id].z = z;
};

uint32 GetLocationID(uint32 map_id, uint32 area_id, uint32 zone_id)
{
	uint32 loc_id = NULL;

	for (loc_id = 1; loc_id < GWARZ.size(); loc_id++)
	{
		if (GWARZ[loc_id].map_id == map_id && GWARZ[loc_id].area_id == area_id && GWARZ[loc_id].zone_id == zone_id)
		{
			return loc_id;
			break;
		}
	}
	return false;
};

uint32 CreateGuildCommands(std::string guild_name, uint32 guild_id)
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

void UpdateGuildCommandData(std::string column_target, std::string new_data, uint32 guild_id)
{
	WorldDatabase.PExecute("UPDATE %s SET `%s`='%s' WHERE `guild_id` = '%u';", commands_db.c_str(), column_target.c_str(), new_data.c_str(), guild_id);

	// GUILD MASTER
	if (column_target == "commands")
	{
		GWCOMM[guild_id].commands = new_data.c_str();
	}
	if (column_target == "info_loc")
	{
		GWCOMM[guild_id].info_loc = new_data.c_str();
	}
	if (column_target == "list_loc")
	{
		GWCOMM[guild_id].list_loc = new_data.c_str();
	}
	if (column_target == "tele")
	{
		GWCOMM[guild_id].tele = new_data.c_str();
	}
	if (column_target == "loc")
	{
		GWCOMM[guild_id].loc = new_data.c_str();
	}
	if (column_target == "farm")
	{
		GWCOMM[guild_id].farm = new_data.c_str();
	}
	if (column_target == "barrack")
	{
		GWCOMM[guild_id].barrack = new_data.c_str();
	}
	if (column_target == "hall")
	{
		GWCOMM[guild_id].hall = new_data.c_str();
	}
	if (column_target == "pig")
	{
		GWCOMM[guild_id].pig = new_data.c_str();
	}
	if (column_target == "guard")
	{
		GWCOMM[guild_id].guard = new_data.c_str();
	}
	if (column_target == "vendor1")
	{
		GWCOMM[guild_id].vendor1 = new_data.c_str();
	}
	if (column_target == "vendor2")
	{
		GWCOMM[guild_id].vendor2 = new_data.c_str();
	}
	if (column_target == "vendor3")
	{
		GWCOMM[guild_id].vendor3 = new_data.c_str();
	}
	if (column_target == "cannon")
	{
		GWCOMM[guild_id].cannon = new_data.c_str();
	}
	if (column_target == "vault")
	{
		GWCOMM[guild_id].vault = new_data.c_str();
	}
	if (column_target == "mailbox")
	{
		GWCOMM[guild_id].vault = new_data.c_str();
	}
	if (column_target == "color_1")
	{
		GWCOMM[guild_id].color_1 = new_data.c_str();
	}
	if (column_target == "color_2")
	{
		GWCOMM[guild_id].color_2 = new_data.c_str();
	}
	if (column_target == "color_3")
	{
		GWCOMM[guild_id].color_3 = new_data.c_str();
	}
	if (column_target == "color_4")
	{
		GWCOMM[guild_id].color_4 = new_data.c_str();
	}
	if (column_target == "color_5")
	{
		GWCOMM[guild_id].color_5 = new_data.c_str();
	}
	if (column_target == "color_6")
	{
		GWCOMM[guild_id].color_6 = new_data.c_str();
	}
	if (column_target == "color_7")
	{
		GWCOMM[guild_id].color_7 = new_data.c_str();
	}
	if (column_target == "color_8")
	{
		GWCOMM[guild_id].color_8 = new_data.c_str();
	}
	if (column_target == "color_9")
	{
		GWCOMM[guild_id].color_9 = new_data.c_str();
	}
	if (column_target == "color_10")
	{
		GWCOMM[guild_id].color_10 = new_data.c_str();
	}
	if (column_target == "color_11")
	{
		GWCOMM[guild_id].color_11 = new_data.c_str();
	}
	if (column_target == "color_12")
	{
		GWCOMM[guild_id].color_12 = new_data.c_str();
	}
	if (column_target == "color_13")
	{
		GWCOMM[guild_id].color_13 = new_data.c_str();
	}
	if (column_target == "color_14")
	{
		GWCOMM[guild_id].color_14 = new_data.c_str();
	}
	if (column_target == "color_15")
	{
		GWCOMM[guild_id].color_15 = new_data.c_str();
	}
	if (column_target == "guild_invite")
	{
		GWCOMM[guild_id].guild_invite = uint8(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "GLD_lvlb")
	{
		GWCOMM[guild_id].GLD_lvlb = uint8(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "GLD_lvls")
	{
		GWCOMM[guild_id].GLD_lvls = uint8(GGW_ConvertStringToNumber(new_data));
	}

// ADMIN
	if (column_target == "GM_admin")
	{
		GWCOMM[SERVER_GUILD_ID].GM_admin = uint8(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "GM_minimum")
	{
		GWCOMM[SERVER_GUILD_ID].GM_minimum = uint8(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "currency")
	{
		GWCOMM[SERVER_GUILD_ID].currency = uint64(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "respawn_flag")
	{
		GWCOMM[guild_id].respawn_flag = new_data.c_str();
	}
	if (column_target == "details_loc")
	{
		GWCOMM[guild_id].details_loc = new_data.c_str();
	}
	if (column_target == "table")
	{
		GWCOMM[guild_id].table = new_data.c_str();
	}
	if (column_target == "Server")
	{
		GWCOMM[guild_id].Server = new_data.c_str();
	}
	if (column_target == "version")
	{
		GWCOMM[guild_id].version = new_data.c_str();
	}
	if (column_target == "command_set")
	{
		GWCOMM[guild_id].command_set = new_data.c_str();
	}
	if (column_target == "pig_payz")
	{
		GWCOMM[SERVER_GUILD_ID].pig_payz = uint64(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "pig_payz_timer")
	{
		GWCOMM[SERVER_GUILD_ID].pig_payz_timer = uint64(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "gift_count")
	{
		GWCOMM[SERVER_GUILD_ID].gift_count = uint32(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "flag_require")
	{
		GWCOMM[SERVER_GUILD_ID].flag_require = uint8(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "anarchy")
	{
		GWCOMM[SERVER_GUILD_ID].anarchy = uint8(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "f_timer")
	{
		GWCOMM[SERVER_GUILD_ID].f_timer = uint8(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "s_timer")
	{
		GWCOMM[SERVER_GUILD_ID].s_timer = uint64(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "loc_cost")
	{
		GWCOMM[SERVER_GUILD_ID].loc_cost = uint32(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "flag_id")
	{
		GWCOMM[SERVER_GUILD_ID].flag_id = uint64(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "farm_cost")
	{
		GWCOMM[SERVER_GUILD_ID].farm_cost = uint16(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "farm_L")
	{
		GWCOMM[SERVER_GUILD_ID].farm_L = uint16(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "farm_id")
	{
		GWCOMM[SERVER_GUILD_ID].farm_id = uint64(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "barrack_cost")
	{
		GWCOMM[SERVER_GUILD_ID].barrack_cost = uint16(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "barrack_L")
	{
		GWCOMM[SERVER_GUILD_ID].barrack_L = uint16(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "barrack_id")
	{
		GWCOMM[SERVER_GUILD_ID].barrack_id = uint64(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "hall_cost")
	{
		GWCOMM[SERVER_GUILD_ID].hall_cost = uint16(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "hall_L")
	{
		GWCOMM[SERVER_GUILD_ID].hall_L = uint16(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "hall_id")
	{
		GWCOMM[SERVER_GUILD_ID].hall_id = uint64(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "pig_cost")
	{
		GWCOMM[SERVER_GUILD_ID].pig_cost = uint16(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "pig_L")
	{
		GWCOMM[SERVER_GUILD_ID].pig_L = uint16(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "pig_id")
	{
		GWCOMM[SERVER_GUILD_ID].pig_id = uint64(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "guard_cost")
	{
		GWCOMM[SERVER_GUILD_ID].guard_cost = uint16(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "guard_L")
	{
		GWCOMM[SERVER_GUILD_ID].barrack_L = uint16(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "guard_id")
	{
		GWCOMM[SERVER_GUILD_ID].guard_id = uint64(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "vendor1_cost")
	{
		GWCOMM[SERVER_GUILD_ID].vendor1_cost = uint16(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "vendor1_L")
	{
		GWCOMM[SERVER_GUILD_ID].vendor1_L = uint16(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "vendor1_id")
	{
		GWCOMM[SERVER_GUILD_ID].vendor1_id = uint64(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "vendor2_cost")
	{
		GWCOMM[SERVER_GUILD_ID].vendor2_cost = uint16(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "vendor2_L")
	{
		GWCOMM[SERVER_GUILD_ID].vendor2_L = uint16(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "vendor2_id")
	{
		GWCOMM[SERVER_GUILD_ID].vendor2_id = uint64(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "vendor3_cost")
	{
		GWCOMM[SERVER_GUILD_ID].vendor3_cost = uint16(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "vendor3_L")
	{
		GWCOMM[SERVER_GUILD_ID].vendor3_L = uint16(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "vendor3_id")
	{
		GWCOMM[SERVER_GUILD_ID].vendor3_id = uint64(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "cannon_cost")
	{
		GWCOMM[SERVER_GUILD_ID].cannon_cost = uint16(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "cannon_L")
	{
		GWCOMM[SERVER_GUILD_ID].cannon_L = uint16(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "cannon_id")
	{
		GWCOMM[SERVER_GUILD_ID].cannon_id = uint64(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "vault_cost")
	{
		GWCOMM[SERVER_GUILD_ID].vault_cost = uint16(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "vault_L")
	{
		GWCOMM[SERVER_GUILD_ID].vault_L = uint16(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "vault_id")
	{
		GWCOMM[SERVER_GUILD_ID].vault_id = uint64(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "mailbox_cost")
	{
		GWCOMM[SERVER_GUILD_ID].vault_cost = uint16(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "mailbox_L")
	{
		GWCOMM[SERVER_GUILD_ID].mailbox_L = uint16(GGW_ConvertStringToNumber(new_data));
	}
	if (column_target == "mailbox_id")
	{
		GWCOMM[SERVER_GUILD_ID].vault_id = uint64(GGW_ConvertStringToNumber(new_data));
	}
	// System
	if (column_target == "allowed")
	{
		GWCOMM[guild_id].allowed = uint8(GGW_ConvertStringToNumber(new_data));
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

		if (!GGW_NPC->Create(db_guid, map, PHASEMASK_NORMAL, id, x, y, z, o))
		{
			ChatHandler(player->GetSession()).PSendSysMessage("%sNPC create error..", GWCOMM[guild_id].color_15.c_str());
			delete GGW_NPC;
			return false;
		}

		CreatureData& data = sObjectMgr->NewOrExistCreatureData(db_guid); // Filling in the needed data to save
		data.id = id;
		data.phaseMask = player->GetPhaseMaskForSpawn();
		data.posX = x;
		data.posY = y;
		data.posZ = z;
		data.orientation = o;

		GGW_NPC->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()), 1); 

		if (!GGW_NPC->LoadCreatureFromDB(db_guid, map))
		{
			delete GGW_NPC;
			return false;
		}

		sObjectMgr->AddCreatureToGrid(db_guid, &data);

		if (!GGW_NPC->IsVisible())
		{
			ChatHandler(player->GetSession()).PSendSysMessage("%sNPC add to grid error..", GWCOMM[guild_id].color_15.c_str());
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
		G3D::Quat rot = G3D::Matrix3::fromEulerAnglesZYX(player->GetOrientation(), 0.f, 0.f);

			if (!GGW_GO->Create(guidLow, id, map, PHASEMASK_NORMAL, *player, rot, 255, GO_STATE_ACTIVE))
			{
				ChatHandler(player->GetSession()).PSendSysMessage("%sFlag build error..", GWCOMM[guild_id].color_15.c_str());
				delete GGW_GO;
				return false;
			};

		map->AddToMap(GGW_GO); // visual responce of spawning flag.

			if (!GGW_GO->isSpawned())
			{
				ChatHandler(player->GetSession()).PSendSysMessage("%sFlag spawn error..", GWCOMM[guild_id].color_15.c_str());
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

			CreateGuildCommands(name, guild_id);

			UpdateGuildCommandData("team", ConvertNumberToString(team), guild_id);

			uint32 total_gross_worth = CalculateTotalLocationsValue(guild_id);
			bool dummyUpdateRankEntry = UpdateRankEntry(guild_id, total_gross_worth, team);

			leader->AddItem( 65000 + team, 1);
		}

		virtual void OnDisband(Guild* guild)
		{
			uint32 guild_id = guild->GetId();

			WorldDatabase.PExecute("DELETE FROM %s WHERE `guild_id`='%u';", commands_db.c_str(), guild_id);

			//			WorldDatabase.PExecute("UPDATE %s SET `guild_name` = '%s' AND `team` = '0' AND `flag_id` = '0' AND `guild_id` = '%u' WHERE `guild_id` = '%u';", zones_db.c_str(), SERVER_GUILD_NAME.c_str(), SERVER_GUILD_ID, guild_id); //  WHERE `guild_id` = '%u'

			for (uint32 pos = 1; pos <= GWARZ.size(); pos++)
			{
				if (GWARZ[pos].guild_id == guild_id)
				{
					UpdateGuildLocData("guild_name", SERVER_GUILD_NAME, pos);
					UpdateGuildLocData("team", "" + ConvertNumberToString(SERVER_GUILD_TEAM), pos);
					UpdateGuildLocData("flag_id", "" + ConvertNumberToString(0), pos);
					UpdateGuildLocData("guild_id", "" + ConvertNumberToString(SERVER_GUILD_ID), pos);
				}
			}

			WorldDatabase.PExecute("DELETE FROM %s WHERE `guild_id` = '%u';", ranking_db.c_str(), guild_id);

			GWCOMM.erase(guild_id);
			GWARZ.erase(guild_id);
		}

		virtual void OnEvent(Guild* guild, uint8 eventType, ObjectGuid::LowType playerGuid1, ObjectGuid::LowType playerGuid2, uint8 newRank) 
		{ 
			std::string GuildEventMap[5] = { "Unknown1", "Unknown2", "member_rank_up", "Member_rank_down", "Unknown5"};

			TC_LOG_INFO("server.loading", "GUILD_NAME:%s EVENT_ID:%u PLAYER1_GUID:%u PLAYER2_GUID:%u", guild->GetName().c_str(), GuildEventMap[eventType], playerGuid1, playerGuid2);
		}
};

void PigPayz(Player *player)
{
	uint32 guild_id = player->GetGuildId();
	uint32 pig_count = 0;
	uint32 pMoney = player->GetMoney();
	std::string coinage;
	std::string coin_types;

	pig_count = GetTotalPigs(guild_id);

	uint32 tally = (pig_count * GUILDWARZ_PIGPAYZ_VALUE);

	if (test){ TC_LOG_INFO("server.loading", "PIG_PAYZ PIG_COUNT:%u PAYZ:%u TALLY:%u", pig_count, GUILDWARZ_PIGPAYZ_VALUE, tally); };

	player->ModifyMoney(tally, true);

	uint32 Gold = tally / 10000;
	uint32 Silver = (tally - (Gold * 10000)) / 100;
	uint32 Copper = (tally - ((Gold * 10000) + (Silver * 100))) / 1;

	if (Gold > 0){ coin_types = ConvertNumberToString(Gold) + ":Gold"; };
	if (Silver > 0){ coin_types = coin_types + " " + ConvertNumberToString(Silver) + ":Silver"; };
	if (Copper > 0){ coin_types = coin_types + " " + ConvertNumberToString(Copper) + ":Copper"; };

	if (pig_count == 0)
	{
		ChatHandler(player->GetSession()).PSendSysMessage("[%sGuild Warz|r]:%sYour Guild needs Guild Pig's to start receiving profit.|r", GWCOMM[SERVER_GUILD_ID].color_15, GWCOMM[SERVER_GUILD_ID].color_10);
		ChatHandler(player->GetSession()).PSendSysMessage("[%sGuild Warz|r]:%sTell your Guild Leader to start adding Guild Pig's to your Guild Locations.|r", GWCOMM[SERVER_GUILD_ID].color_15, GWCOMM[SERVER_GUILD_ID].color_10);
	}

	if (pig_count > 0)
	{
		ChatHandler(player->GetSession()).PSendSysMessage("[%sGuild Warz|r]:%sYour %u Guild Pig's pay off.|r", GWCOMM[SERVER_GUILD_ID].color_15, GWCOMM[SERVER_GUILD_ID].color_10, pig_count);
		ChatHandler(player->GetSession()).PSendSysMessage("%s+%s.", GWCOMM[guild_id].color_2.c_str(), coin_types.c_str());
	}
};

class GGW_PigPayz : public BasicEvent
{
public:
	GGW_PigPayz(Player* _player) : player(_player)
	{
		_player->m_Events.AddEvent(this, _player->m_Events.CalculateTime(GUILDWARZ_PIGPAYZ_TIMER)); // 1000 = 1 second  // 60000 = 1 minute
	}

	bool Execute(uint64, uint32) override
	{
		PigPayz(player);

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

	uint32 LocId = GetLocationID(map_id, area_id, zone_id);

	auto pTeam_id = player->GetTeamId();
	uint32 guild_id = player->GetGuildId();

	uint32 loc_guild = GWARZ[LocId].guild_id;
	uint8 LocTeam = GWARZ[LocId].team;

	uint32 loc_faction = (84 - LocTeam);
	uint32 invade_faction = (83 + LocTeam);

	if (!LocId)
	{
		LocId = CreateGuildLocation(map_id, area_id, zone_id, pX, pY, pZ);
	}

	if (LocId)
	{
		if (GWARZ[LocId].team > 1)
		{
			player->RestoreFaction();
		};

		if (GWARZ[LocId].team < 2)
		{
			if (GWARZ[LocId].guild_id == guild_id)
			{
				player->SetPvP(true);
				player->setFaction(loc_faction);
			}

			if (guild_id == 0)
			{
				if (GWARZ[LocId].team == pTeam_id){ player->setFaction(loc_faction); };
				if (GWARZ[LocId].team != pTeam_id){ player->setFaction(invade_faction); };
			}

			if (guild_id > 0)
			{
				if (GWARZ[LocId].guild_id != guild_id)
				{
					if (GWCOMM[SERVER_GUILD_ID].anarchy == 1)
					{
						player->setFaction(invade_faction);
					}

					if (GWARZ[LocId].team != pTeam_id)
					{
						player->SetPvP(true);
						player->setFaction(invade_faction);
					}

					if (GWARZ[LocId].team == pTeam_id)
					{
						if (GWCOMM[SERVER_GUILD_ID].anarchy == 0)
						{
							player->SetPvP(true);
							player->setFaction(loc_faction);
						}

						if (GWCOMM[SERVER_GUILD_ID].anarchy == 1)
						{
							player->SetPvP(true);
							player->setFaction(invade_faction);
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
		_player->m_Events.AddEvent(this, _player->m_Events.CalculateTime(GUILDWARZ_PLAYER_CHECK_TIMER)); // 1000 = 1 second  // 60000 = 1 minute
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

			ChatHandler(player->GetSession()).PSendSysMessage("%s<%sGrumbo|r%s> Says:Type `/g commands` for a list of Your Guild Warz Commands.|r", GWCOMM[guild_id].color_10.c_str(), GWCOMM[guild_id].color_6.c_str(), GWCOMM[guild_id].color_10.c_str());
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

			uint32 gGuildID = GWCOMM[guild_id].guild_id;

			uint32 LocId = GetLocationID(map_id, area_id, zone_id);

			if (!LocId)
			{
				LocId = CreateGuildLocation(map_id, area_id, zone_id, pX, pY, pZ);
			}

			if (guild_name != GWCOMM[guild_id].guild)
			{
				ChatHandler(player->GetSession()).PSendSysMessage("<Grumbo>:CREATING Your Guild Warz Commands for Guild %s.", guild_name.c_str());
				CreateGuildCommands(guild_name, guild_id);
			}

			std::string str_LocId = ConvertNumberToString(LocId);

			if (!pGMRank)
			{
				pGMRank = 0;
			}

			if (GWARZ[LocId].x == 0 && GWARZ[LocId].y == 0 && GWARZ[LocId].z == 0) // just a temp catch-22.
			{
				UpdateGuildLocFloat(pX, pY, pZ, LocId);
			}

			uint32 Zoneprice = (GWCOMM[SERVER_GUILD_ID].loc_cost) + (GWCOMM[SERVER_GUILD_ID].farm_cost*GWARZ[LocId].farm_count) + (GWCOMM[SERVER_GUILD_ID].barrack_cost*GWARZ[LocId].barrack_count) + (GWCOMM[SERVER_GUILD_ID].hall_cost*GWARZ[LocId].hall_count) + (GWCOMM[SERVER_GUILD_ID].pig_cost*GWARZ[LocId].pig_count) + (GWCOMM[SERVER_GUILD_ID].vendor1_cost*GWARZ[LocId].vendor1_count) + (GWCOMM[SERVER_GUILD_ID].vendor2_cost*GWARZ[LocId].vendor2_count) + (GWCOMM[SERVER_GUILD_ID].vendor3_cost*GWARZ[LocId].vendor3_count) + (GWCOMM[SERVER_GUILD_ID].cannon_cost*GWARZ[LocId].cannon_count) + (GWCOMM[SERVER_GUILD_ID].vault_cost*GWARZ[LocId].vault_count) + (GWCOMM[SERVER_GUILD_ID].mailbox_cost*GWARZ[LocId].mailbox_count);
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

			if (ChatCache[1] == GWCOMM[guild_id].commands)
			{
				ChatHandler(player->GetSession()).PSendSysMessage("%s************************************************************|r", GWCOMM[guild_id].color_9.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s*    -Grumbo'z Guild Warz Commands:-     *|r", GWCOMM[guild_id].color_8.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s************************************************************|r", GWCOMM[guild_id].color_9.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("(Guild: %s)", guild_name.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("(Name: %s) (Guild Rank: %u) (GAME Rank: %u)", pName.c_str(), pGuildRank, pGMRank);
				ChatHandler(player->GetSession()).PSendSysMessage("%s************************************************************|r", GWCOMM[guild_id].color_9.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s Guild Member Commands:", GWCOMM[guild_id].color_8.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s------------------------------------------------------------|r", GWCOMM[guild_id].color_9.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s %s |r          %s list guild commands.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].commands.c_str(), GWCOMM[guild_id].color_3.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s %s|r %s                     lists area info.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].info_loc.c_str(), GWCOMM[guild_id].color_3.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s %s|r                      %s lists areas owned", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].list_loc.c_str(), GWCOMM[guild_id].color_3.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s   |r                        %s by your guild.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_3.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s %s `location_id`|r  %s teleport to area", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].tele.c_str(), GWCOMM[guild_id].color_3.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s |r                          %s by location id.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_3.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s %s|r                     %s -displays Core versions.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].version.c_str(), GWCOMM[guild_id].color_3.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s %s|r                     %s -displays Core setup and settings.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].setup.c_str(), GWCOMM[guild_id].color_3.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s %s|r                     %s -Displays Guild Ranking.|r", GWCOMM[guild_id].color_1.c_str(), GUILD_RANK_COMMAND.c_str(), GWCOMM[guild_id].color_3.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s************************************************************|r", GWCOMM[guild_id].color_9.c_str());

				if (pGuildRank <= GWCOMM[guild_id].GLD_lvlb)
				{
					ChatHandler(player->GetSession()).PSendSysMessage("%s buy %sGuild Master level Commands:Rank:%s%u%s access.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_8.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[guild_id].GLD_lvlb, GWCOMM[guild_id].color_8.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s------------------------------------------------------------|r", GWCOMM[guild_id].color_9.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sbuy %s       %s- purchase area.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].loc.c_str(), GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sbuy %s       %s- purchase a guild farm.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].farm.c_str(), GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sbuy %s       %s- purchase a pig.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].pig.c_str(), GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sbuy %s       %s- purchase a barracks for guards.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].barrack.c_str(), GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sbuy %s       %s- purchase a disposable guard.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].guard.c_str(), GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sbuy %s       %s- purchase a guild hall.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].hall.c_str(), GWCOMM[guild_id].color_3.c_str());

					if (GWCOMM[SERVER_GUILD_ID].cannon_id > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%sbuy %s      %s- purchase a cannon.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].cannon.c_str(), GWCOMM[guild_id].color_3.c_str());
					}

					if (GWCOMM[SERVER_GUILD_ID].vendor1_id > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%sbuy %s      %s- purchase vendor1.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].vendor1.c_str(), GWCOMM[guild_id].color_3.c_str());
					}

					if (GWCOMM[SERVER_GUILD_ID].vendor2_id > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%sbuy %s      %s- purchase vendor2.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].vendor2.c_str(), GWCOMM[guild_id].color_3.c_str());
					}

					if (GWCOMM[SERVER_GUILD_ID].vendor3_id > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%sbuy %s      %s- purchase vendor3.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].vendor3.c_str(), GWCOMM[guild_id].color_3.c_str());
					}

					ChatHandler(player->GetSession()).PSendSysMessage("%sbuy %s       %s- purchase a Guild Vault.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].vault.c_str(), GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sbuy %s       %s- purchase a mailbox.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].mailbox.c_str(), GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s************************************************************|r", GWCOMM[guild_id].color_9.c_str());

				}

				if (pGuildRank <= GWCOMM[guild_id].GLD_lvls)
				{
					ChatHandler(player->GetSession()).PSendSysMessage("%ssell %sGuild Master level Commands:Rank:%s%u%s access.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_8.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[guild_id].GLD_lvls, GWCOMM[guild_id].color_8.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s------------------------------------------------------------|r", GWCOMM[guild_id].color_9.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%ssell %s        %s- sell area for its TOTAL value.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].loc.c_str(), GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%ssell %s        %s- sell farm.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].farm.c_str(), GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%ssell %s        %s- sell a pig to market.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].pig.c_str(), GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%ssell %s        %s- sell barracks.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].barrack.c_str(), GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%ssell %s        %s- removes a selected guard.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].guard.c_str(), GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s                 guards are disposable.|r", GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s                 no cash back.|r", GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%ssell %s        %s- sell Guild hall.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].hall.c_str(), GWCOMM[guild_id].color_3.c_str());

					if (GWCOMM[SERVER_GUILD_ID].cannon_id > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%ssell %s|r  %s- sell a selected cannon.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].cannon.c_str(), GWCOMM[guild_id].color_3.c_str());
					}

					if (GWCOMM[SERVER_GUILD_ID].vendor1_id > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%ssell %s|r  %s- sells a selected vendor1.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].vendor1.c_str(), GWCOMM[guild_id].color_3.c_str());
					}

					if (GWCOMM[SERVER_GUILD_ID].vendor2_id > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%ssell %s|r  %s- sells a selected vendor2.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].vendor2.c_str(), GWCOMM[guild_id].color_3.c_str());
					}

					if (GWCOMM[SERVER_GUILD_ID].vendor3_id > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%ssell %s|r  %s- sells a selected vendor3.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].vendor3.c_str(), GWCOMM[guild_id].color_3.c_str());
					}

					ChatHandler(player->GetSession()).PSendSysMessage("%ssell %s|r  %s- sells a guild vault.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].vault.c_str(), GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%ssell %s|r  %s- sells a guild mailbox.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].mailbox.c_str(), GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s************************************************************|r", GWCOMM[guild_id].color_9.c_str());
				}

				if (pGuildRank == 0)
				{
					ChatHandler(player->GetSession()).PSendSysMessage("%sGuild Master settings:", GWCOMM[guild_id].color_8.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s------------------------------------------------------------|r", GWCOMM[guild_id].color_9.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sGuild Buy. minimum required guild rank|r:%s%u.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[guild_id].GLD_lvlb);
					ChatHandler(player->GetSession()).PSendSysMessage("%sGuild Sell. minimum required guild rank|r:%s%u.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[guild_id].GLD_lvls);
					ChatHandler(player->GetSession()).PSendSysMessage("%sGuild invite system|r:%s%u|r         %s0 = off 1 = on.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[guild_id].guild_invite, GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s------------------------------------------------------------|r", GWCOMM[guild_id].color_9.c_str());
				}

				if (pGuildRank == 0 || (pGMRank == GWCOMM[SERVER_GUILD_ID].GM_admin && player->IsGameMaster()))
				{
					ChatHandler(player->GetSession()).PSendSysMessage("%sGuild Master/Game Master/Admin special Commands:", GWCOMM[guild_id].color_8.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s------------------------------------------------------------|r", GWCOMM[guild_id].color_9.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s%s|r     %s used to modify commands and settings.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[SERVER_GUILD_ID].command_set.c_str(), GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%shelp %s|r    %slists commands and settings you may change.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[SERVER_GUILD_ID].command_set.c_str(), GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s************************************************************|r", GWCOMM[guild_id].color_9.c_str());
				}

				if (pGuildRank <= GWCOMM[guild_id].GLD_lvlb || (pGMRank >= GWCOMM[SERVER_GUILD_ID].GM_minimum && player->IsGameMaster()))
				{
					ChatHandler(player->GetSession()).PSendSysMessage("%sPrices in %s%s's:|r", GWCOMM[guild_id].color_8.c_str(), Currencyname.c_str(), GWCOMM[guild_id].color_8.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s------------------------------------------------------------|r", GWCOMM[guild_id].color_9.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sZone price|r: %s%u|r %sbase location price.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[SERVER_GUILD_ID].loc_cost, GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sFarm price|r: %s%u.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[SERVER_GUILD_ID].farm_cost);
					ChatHandler(player->GetSession()).PSendSysMessage("%sPig price|r: %s%u|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[SERVER_GUILD_ID].pig_cost);
					ChatHandler(player->GetSession()).PSendSysMessage("%sBarracks price|r: %s%u.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[SERVER_GUILD_ID].barrack_cost);
					ChatHandler(player->GetSession()).PSendSysMessage("%sGuard price|r: %s%u|r - %sThese Dont increase location price.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[SERVER_GUILD_ID].guard_cost, GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sHall price|r: %s%u.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[SERVER_GUILD_ID].hall_cost);

					if (GWCOMM[SERVER_GUILD_ID].cannon_id > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%scannon price|r: %s%u.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[SERVER_GUILD_ID].cannon_cost);
					}

					if (GWCOMM[SERVER_GUILD_ID].vendor1_id > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%svendor1 price|r: %s%u.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[SERVER_GUILD_ID].vendor1_cost);
					}

					if (GWCOMM[SERVER_GUILD_ID].vendor2_id > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%svendor2 price|r: %s%u|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[SERVER_GUILD_ID].vendor2_cost);
					}

					if (GWCOMM[SERVER_GUILD_ID].vendor3_id > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%svendor3 price|r: %s%u|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[SERVER_GUILD_ID].vendor3_cost);
					}

					ChatHandler(player->GetSession()).PSendSysMessage("%sVault price|r: %s%u.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[SERVER_GUILD_ID].vault_cost);
					ChatHandler(player->GetSession()).PSendSysMessage("%sMailbox price|r: %s%u.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[SERVER_GUILD_ID].mailbox_cost);
					ChatHandler(player->GetSession()).PSendSysMessage("%s************************************************************|r", GWCOMM[guild_id].color_9.c_str());
				}

				if (pGuildRank <= GWCOMM[guild_id].GLD_lvlb || (pGMRank >= GWCOMM[SERVER_GUILD_ID].GM_minimum && player->IsGameMaster()))
				{
					ChatHandler(player->GetSession()).PSendSysMessage("%sLocation Limits:", GWCOMM[guild_id].color_8.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s------------------------------------------------------------|r", GWCOMM[guild_id].color_9.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sFarm limit|r: %s%u|r %sper location.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[SERVER_GUILD_ID].farm_L, GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sPig limit|r: %s%u|r %sper farm. Require a farm.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[SERVER_GUILD_ID].pig_L, GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sBarrack limit|r: %s%u|r %sper farm. Require a farm.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[SERVER_GUILD_ID].barrack_L, GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sGuard limit|r: %s%u|r %sper barrack. Require a barrack.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[SERVER_GUILD_ID].guard_L, GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sHall limit|r: %s%u|r %sper location. Require a barrack.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[SERVER_GUILD_ID].hall_L, GWCOMM[guild_id].color_3.c_str());

					if (GWCOMM[SERVER_GUILD_ID].vendor1_id > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%sVendor1 limit|r: %s%u|r %sper location. Require a hall.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[SERVER_GUILD_ID].vendor1_L, GWCOMM[guild_id].color_3.c_str());
					}
					if (GWCOMM[SERVER_GUILD_ID].vendor2_id > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%sVendor2 limit|r: %s%u|r %sper location. Require a hall.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[SERVER_GUILD_ID].vendor2_L, GWCOMM[guild_id].color_3.c_str());
					}
					if (GWCOMM[SERVER_GUILD_ID].vendor3_id > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%sVendor3 limit|r: %s%u|r %sper location. Require a hall.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[SERVER_GUILD_ID].vendor3_L, GWCOMM[guild_id].color_3.c_str());
					}
					if (GWCOMM[SERVER_GUILD_ID].cannon_id > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%sCannon limit|r: %s%u|r %sper hall. Require a hall.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[SERVER_GUILD_ID].cannon_L, GWCOMM[guild_id].color_3.c_str());
					}
					ChatHandler(player->GetSession()).PSendSysMessage("%sGuild Vault limit|r: %s%u|r %sper location. Require a hall.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[SERVER_GUILD_ID].vault_L, GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sGuild MailBox limit|r: %s%u|r %sper location. Require a hall.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[SERVER_GUILD_ID].mailbox_L, GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s************************************************************|r", GWCOMM[guild_id].color_9.c_str());
				}

				if (pGMRank >= GWCOMM[SERVER_GUILD_ID].GM_minimum && player->IsGameMaster())
				{
					ChatHandler(player->GetSession()).PSendSysMessage("Game Master Commands:", GWCOMM[guild_id].color_8.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s------------------------------------------------------------|r", GWCOMM[guild_id].color_9.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s%s|r                %s-location info.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[SERVER_GUILD_ID].details_loc.c_str(), GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sreset %s|r          %s- Resets location to server.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[SERVER_GUILD_ID].loc.c_str(), GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sreload %s|r         %s- Reloads GW tables.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[SERVER_GUILD_ID].table.c_str(), GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sreset %s|r          %s- Resets location farm count to 0.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[SERVER_GUILD_ID].farm.c_str(), GWCOMM[guild_id].color_3.c_str());

					ChatHandler(player->GetSession()).PSendSysMessage("%sreset %s|r          %s- Resets location barrack count to 0.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[SERVER_GUILD_ID].barrack.c_str(), GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sreset %s|r          %s- Resets location hall count to 0.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[SERVER_GUILD_ID].hall.c_str(), GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sreset %s|r          %s- Resets location pig count to 0.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[SERVER_GUILD_ID].pig.c_str(), GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sreset %s|r          %s- Resets location guard count to 0.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[SERVER_GUILD_ID].guard.c_str(), GWCOMM[guild_id].color_3.c_str());

					ChatHandler(player->GetSession()).PSendSysMessage("%sreset %s|r          %s- Resets location vendor 1 count to 0.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[SERVER_GUILD_ID].vendor1.c_str(), GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sreset %s|r          %s- Resets location vendor 2 count to 0.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[SERVER_GUILD_ID].vendor2.c_str(), GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sreset %s|r          %s- Resets location vendor 3 count to 0.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[SERVER_GUILD_ID].vendor3.c_str(), GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sreset %s|r          %s- Resets location cannon count to 0.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[SERVER_GUILD_ID].cannon.c_str(), GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sreset %s|r          %s- Resets location vault count to 0.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[SERVER_GUILD_ID].vault.c_str(), GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sreset %s|r          %s- Resets location mailbox count to 0.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[SERVER_GUILD_ID].mailbox.c_str(), GWCOMM[guild_id].color_3.c_str());

					ChatHandler(player->GetSession()).PSendSysMessage("%sspawn %s|r         %s- Spawns new flag if current|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[SERVER_GUILD_ID].respawn_flag.c_str(), GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s                       flag is missing or corrupt.|r", GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%slock %s|r         %s- locks a location from purchase.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[SERVER_GUILD_ID].loc.c_str(), GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s************************************************************|r", GWCOMM[guild_id].color_9.c_str());
				}

				if (pGMRank == GWCOMM[SERVER_GUILD_ID].GM_admin && player->IsGameMaster())
				{
					ChatHandler(player->GetSession()).PSendSysMessage("%sADMIN settings:", GWCOMM[guild_id].color_8.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s------------------------------------------------------------|r", GWCOMM[guild_id].color_9.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sADMIN Level Access|r:%s%u |r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[SERVER_GUILD_ID].GM_admin);
					ChatHandler(player->GetSession()).PSendSysMessage("%sMinimum GM Level Access|r: %s%u|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[SERVER_GUILD_ID].GM_minimum);
					ChatHandler(player->GetSession()).PSendSysMessage("%sPig Payz:%s%u copper.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[SERVER_GUILD_ID].pig_payz); // , GWCOMM[guild_id].color_3.c_str()
					ChatHandler(player->GetSession()).PSendSysMessage("%sNew Guild Gift amount|r:%s%u %s's|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[SERVER_GUILD_ID].gift_count, Currencyname.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sFlag require|r: %s%u|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[SERVER_GUILD_ID].flag_require);
					ChatHandler(player->GetSession()).PSendSysMessage("%sAnarchy|r: %s%u|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[SERVER_GUILD_ID].anarchy);

					ChatHandler(player->GetSession()).PSendSysMessage("%sNo-Tag-Flag|r:%s%u|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[SERVER_GUILD_ID].f_timer);
					ChatHandler(player->GetSession()).PSendSysMessage("%sNo-Tag-Flag-Timer:|r%s%u|r - in seconds", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWCOMM[SERVER_GUILD_ID].s_timer); //  , GWCOMM[guild_id].color_3.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s************************************************************|r", GWCOMM[guild_id].color_9.c_str());
				}
				msg = GGW_MSG;
			};

/*
-- *************
-- Info Commands
-- *************
*/
			if ((ChatCache[1] == GWCOMM[SERVER_GUILD_ID].setup)) // && (ChatCache[2] != ""))
			{
				ChatHandler(player->GetSession()).PSendSysMessage("%s****************************", GWCOMM[guild_id].color_9);
				ChatHandler(player->GetSession()).PSendSysMessage("%sSystem setup details:", GWCOMM[guild_id].color_8);
				ChatHandler(player->GetSession()).PSendSysMessage("%s-------------------------------------------", GWCOMM[guild_id].color_9);
				ChatHandler(player->GetSession()).PSendSysMessage("%sCosts:|r", GWCOMM[guild_id].color_8);
				ChatHandler(player->GetSession()).PSendSysMessage("%sPrices in %s%s's:|r", GWCOMM[guild_id].color_8, Currencyname, GWCOMM[guild_id].color_8);
				ChatHandler(player->GetSession()).PSendSysMessage("%s---------------------------------", GWCOMM[guild_id].color_9);

				ChatHandler(player->GetSession()).PSendSysMessage("%sZone price|r:%s%u %sbase location price.|r", GWCOMM[guild_id].color_1, GWCOMM[guild_id].color_2, GWCOMM[guild_id].loc_cost, GWCOMM[guild_id].color_3);
				ChatHandler(player->GetSession()).PSendSysMessage("%sFarm price|r:%s%u ", GWCOMM[guild_id].color_1, GWCOMM[guild_id].color_2, GWCOMM[guild_id].farm_cost);
				ChatHandler(player->GetSession()).PSendSysMessage("%sBarracks price|r:%s%u ", GWCOMM[guild_id].color_1, GWCOMM[guild_id].color_2, GWCOMM[guild_id].barrack_cost);
				ChatHandler(player->GetSession()).PSendSysMessage("%sHall price:%s%u ", GWCOMM[guild_id].color_1, GWCOMM[guild_id].color_2, GWCOMM[guild_id].hall_cost);
				ChatHandler(player->GetSession()).PSendSysMessage("%sPig price|r:%s%u ", GWCOMM[guild_id].color_1, GWCOMM[guild_id].color_2, GWCOMM[guild_id].pig_cost);
				ChatHandler(player->GetSession()).PSendSysMessage("%sGuard price|r:%s%u |r - %sThese Dont increase location price.", GWCOMM[guild_id].color_1, GWCOMM[guild_id].color_2, GWCOMM[guild_id].guard_cost, GWCOMM[guild_id].color_14);


				if (GWCOMM[guild_id].cannon_id > 0) { ChatHandler(player->GetSession()).PSendSysMessage("%scannon price|r:%s%u", GWCOMM[guild_id].color_1, GWCOMM[guild_id].color_2, GWCOMM[guild_id].cannon_cost); };
				if (GWCOMM[guild_id].vendor1_id > 0) { ChatHandler(player->GetSession()).PSendSysMessage("%svendor1 price|r:%s%u", GWCOMM[guild_id].color_1, GWCOMM[guild_id].color_2, GWCOMM[guild_id].vendor1_cost); };
				if (GWCOMM[guild_id].vendor2_id > 0) { ChatHandler(player->GetSession()).PSendSysMessage("%svendor2 price|r:%s%u", GWCOMM[guild_id].color_1, GWCOMM[guild_id].color_2, GWCOMM[guild_id].vendor2_cost); };
				if (GWCOMM[guild_id].vendor3_id > 0) { ChatHandler(player->GetSession()).PSendSysMessage("%svendor3 price|r:%s%u", GWCOMM[guild_id].color_1, GWCOMM[guild_id].color_2, GWCOMM[guild_id].vendor3_cost); };

				ChatHandler(player->GetSession()).PSendSysMessage("%sVault price|r:%s%u ", GWCOMM[guild_id].color_1, GWCOMM[guild_id].color_2, GWCOMM[SERVER_GUILD_ID].vault_cost);
				ChatHandler(player->GetSession()).PSendSysMessage("%sMailbox price|r:%s%u ", GWCOMM[guild_id].color_1, GWCOMM[guild_id].color_2, GWCOMM[SERVER_GUILD_ID].mailbox_cost);
				ChatHandler(player->GetSession()).PSendSysMessage("%s-------------------------------------------", GWCOMM[guild_id].color_9);
				ChatHandler(player->GetSession()).PSendSysMessage("%sLocation Limits:", GWCOMM[guild_id].color_8);
				ChatHandler(player->GetSession()).PSendSysMessage("%sFarm limit|r:%s%u %sper location.|r", GWCOMM[guild_id].color_1, GWCOMM[guild_id].color_2, GWCOMM[SERVER_GUILD_ID].farm_L, GWCOMM[guild_id].color_3);
				ChatHandler(player->GetSession()).PSendSysMessage("%sPig limit|r:%s%u %sper farm. Require a farm.|r", GWCOMM[guild_id].color_1, GWCOMM[guild_id].color_2, GWCOMM[SERVER_GUILD_ID].pig_L, GWCOMM[guild_id].color_3);
				ChatHandler(player->GetSession()).PSendSysMessage("%sBarrack limit|r:%s%u %sper farm. Require a farm.|r", GWCOMM[guild_id].color_1, GWCOMM[guild_id].color_2, GWCOMM[SERVER_GUILD_ID].barrack_L, GWCOMM[guild_id].color_3);
				ChatHandler(player->GetSession()).PSendSysMessage("%sGuard limit|r:%s%u %sper barrack. Require a barrack.|r", GWCOMM[guild_id].color_1, GWCOMM[guild_id].color_2, GWCOMM[SERVER_GUILD_ID].guard_L, GWCOMM[guild_id].color_3);
				ChatHandler(player->GetSession()).PSendSysMessage("%sHall limit|r:%s%u %sper location. Require a barrack.|r", GWCOMM[guild_id].color_1, GWCOMM[guild_id].color_2, GWCOMM[SERVER_GUILD_ID].hall_L, GWCOMM[guild_id].color_3);

					if (GWCOMM[SERVER_GUILD_ID].vendor1_id > 0) { ChatHandler(player->GetSession()).PSendSysMessage("%sVendor1 limit|r:%s%u %sper location. Require a hall.|r", GWCOMM[guild_id].color_1, GWCOMM[guild_id].color_2, GWCOMM[SERVER_GUILD_ID].vendor1_L, GWCOMM[guild_id].color_3); };
					if (GWCOMM[SERVER_GUILD_ID].vendor2_id > 0) { ChatHandler(player->GetSession()).PSendSysMessage("%sVendor2 limit|r:%s%u %sper location. Require a hall.|r", GWCOMM[guild_id].color_1, GWCOMM[guild_id].color_2, GWCOMM[SERVER_GUILD_ID].vendor2_L, GWCOMM[guild_id].color_3); };
					if (GWCOMM[SERVER_GUILD_ID].vendor3_id > 0) { ChatHandler(player->GetSession()).PSendSysMessage("%sVendor3 limit|r:%s%u %sper location. Require a hall.|r", GWCOMM[guild_id].color_1, GWCOMM[guild_id].color_2, GWCOMM[SERVER_GUILD_ID].vendor3_L, GWCOMM[guild_id].color_3); };
					if (GWCOMM[SERVER_GUILD_ID].cannon_id > 0) { ChatHandler(player->GetSession()).PSendSysMessage("%sCannon limit|r:%s%u %sper hall. Require a hall.|r", GWCOMM[guild_id].color_1, GWCOMM[guild_id].color_2, GWCOMM[SERVER_GUILD_ID].cannon_L, GWCOMM[guild_id].color_3); };

				ChatHandler(player->GetSession()).PSendSysMessage("%sGuild Vault limit|r:%s%u %sper location. Require a hall.|r", GWCOMM[guild_id].color_1, GWCOMM[guild_id].color_2, GWCOMM[SERVER_GUILD_ID].vault_L, GWCOMM[guild_id].color_3);
				ChatHandler(player->GetSession()).PSendSysMessage("%sGuild MailBox limit|r:%s%u %sper location. Require a hall.|r", GWCOMM[guild_id].color_1, GWCOMM[guild_id].color_2, GWCOMM[SERVER_GUILD_ID].mailbox_L, GWCOMM[guild_id].color_3);
				ChatHandler(player->GetSession()).PSendSysMessage("%s-------------------------------------------", GWCOMM[guild_id].color_9);

				ChatHandler(player->GetSession()).PSendSysMessage("%sRewards:|r", GWCOMM[guild_id].color_8);
				ChatHandler(player->GetSession()).PSendSysMessage("%sPig Payz|r:%s%u  %scopper every %s%u %sminutes.", GWCOMM[guild_id].color_1, GWCOMM[guild_id].color_2, GWCOMM[SERVER_GUILD_ID].pig_payz, GWCOMM[guild_id].color_3, GWCOMM[guild_id].color_2, ((GWCOMM[SERVER_GUILD_ID].pig_payz_timer / 1000) / 60), GWCOMM[guild_id].color_3);
				ChatHandler(player->GetSession()).PSendSysMessage("%sNew Guild Gift amount|r:%s%u %s%s%s's.", GWCOMM[guild_id].color_1, GWCOMM[guild_id].color_2, GWCOMM[SERVER_GUILD_ID].gift_count, GWCOMM[guild_id].color_8, Currencyname, GWCOMM[guild_id].color_8);
				ChatHandler(player->GetSession()).PSendSysMessage("%s-------------------------------------------", GWCOMM[guild_id].color_9);
		
				ChatHandler(player->GetSession()).PSendSysMessage("%sSystem features:|r", GWCOMM[guild_id].color_8);

					if (GWCOMM[SERVER_GUILD_ID].flag_require != 0) { ChatHandler(player->GetSession()).PSendSysMessage("%sFlag require|r: %sEnabled. %sAll guards MUST be cleared before a flag can be tagged.|r", GWCOMM[guild_id].color_1, GWCOMM[guild_id].color_2, GWCOMM[guild_id].color_3); };
					if (GWCOMM[SERVER_GUILD_ID].anarchy != 0) { ChatHandler(player->GetSession()).PSendSysMessage("%sAnarchy|r: %sEnabled. |r%sSame faction invasions allowed.|r", GWCOMM[guild_id].color_1, GWCOMM[guild_id].color_2, GWCOMM[guild_id].color_3); };

					if (GWCOMM[SERVER_GUILD_ID].f_timer != 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%sNo-Tag-Flag:|r Enabled%s. un-taggable flag Cooldown timer.", GWCOMM[guild_id].color_1, GWCOMM[guild_id].color_3);
						ChatHandler(player->GetSession()).PSendSysMessage("%sNo-Tag-Flag-Timer Duration|r:%s%u|r - %sin seconds.", GWCOMM[guild_id].color_1, GWCOMM[guild_id].color_2, GWCOMM[SERVER_GUILD_ID].s_timer, GWCOMM[guild_id].color_3);
					}

				ChatHandler(player->GetSession()).PSendSysMessage("%s-------------------------------------------", GWCOMM[guild_id].color_9);
				ChatHandler(player->GetSession()).PSendSysMessage("%s****************************", GWCOMM[guild_id].color_9);

				msg = GGW_MSG;
			}

			if (ChatCache[1] == GWCOMM[guild_id].info_loc)
			{
				ChatHandler(player->GetSession()).PSendSysMessage("%s---------------------------------", GWCOMM[guild_id].color_9.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%sLocation details:", GWCOMM[guild_id].color_8.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s---------------------------------", GWCOMM[guild_id].color_9.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%sLoc ID:|r %s%u|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWARZ[LocId].entry);

				if (GWARZ[LocId].team == 0) ChatHandler(player->GetSession()).PSendSysMessage("%sOwner|r: %s %s|r.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_4.c_str(), GWARZ[LocId].guild_name.c_str());
				if (GWARZ[LocId].team == 1) ChatHandler(player->GetSession()).PSendSysMessage("%sOwner|r: %s %s|r.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_5.c_str(), GWARZ[LocId].guild_name.c_str());
				if (GWARZ[LocId].team == 2) ChatHandler(player->GetSession()).PSendSysMessage("%sOwner|r: %s %s|r.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_6.c_str(), GWARZ[LocId].guild_name.c_str());
				if (GWARZ[LocId].team == 3) ChatHandler(player->GetSession()).PSendSysMessage("%sOwner|r: %s %s|r.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_7.c_str(), GWARZ[LocId].guild_name.c_str());
					
				if (GWARZ[LocId].team == 0) ChatHandler(player->GetSession()).PSendSysMessage("%sFaction|r: %sAlliance.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_4.c_str());
				if (GWARZ[LocId].team == 1) ChatHandler(player->GetSession()).PSendSysMessage("%sFaction|r: %sHorde.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_5.c_str());
				if (GWARZ[LocId].team == 2) ChatHandler(player->GetSession()).PSendSysMessage("%sFaction|r: %sFor Sale.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_6.c_str());
				if (GWARZ[LocId].team == 3) ChatHandler(player->GetSession()).PSendSysMessage("%sFaction|r: %sLOCKED|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_7.c_str());

				if (GWARZ[LocId].team < 2) ChatHandler(player->GetSession()).PSendSysMessage("%sGuild Rank|r: %s#%u", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_7.c_str(), GetRank(GWARZ[LocId].guild_id));
				if (GWARZ[LocId].team < 2) ChatHandler(player->GetSession()).PSendSysMessage("%s---------------------------------", GWCOMM[guild_id].color_9.c_str());

				ChatHandler(player->GetSession()).PSendSysMessage("%sFarms|r: %s%u|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWARZ[LocId].farm_count);
				ChatHandler(player->GetSession()).PSendSysMessage("%sHall|r: %s%u|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWARZ[LocId].hall_count);
				ChatHandler(player->GetSession()).PSendSysMessage("%spigs|r: %s%u|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWARZ[LocId].pig_count);
				ChatHandler(player->GetSession()).PSendSysMessage("%sValue:%u %s's:|r", GWCOMM[guild_id].color_1.c_str(), uint32(Zoneprice), Currencyname.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s---------------------------------", GWCOMM[guild_id].color_9.c_str());

				msg = GGW_MSG;
			}

/*
-- ************
-- List Command
-- ************
*/

				if (ChatCache[1] == GWCOMM[guild_id].list_loc)
				{
					uint32 Gloc;
					uint32 yentry = 0;
					uint32 ypigcnt = 0;
					uint32 yvalue = 0;
					uint32 Xzoneprice = 0;

					ChatHandler(player->GetSession()).PSendSysMessage("%s**********************************************************************", GWCOMM[guild_id].color_9.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sLoc ID:     farm:     barracks:     Hall:     Guards:     Pigs:          Zone value:|r", GWCOMM[guild_id].color_1.c_str());

					for (Gloc = 0; Gloc < GWARZ.size(); Gloc++)do
					{
						if (GWARZ[Gloc].guild_id == guild_id)
						{
							Xzoneprice = CalculateLocationValue(Gloc);

							ChatHandler(player->GetSession()).PSendSysMessage("%s%u             %u             %u             %u             %u             %u                  %u|r", GWCOMM[guild_id].color_2.c_str(), Gloc, GWARZ[Gloc].farm_count, GWARZ[Gloc].barrack_count, GWARZ[Gloc].hall_count, GWARZ[Gloc].guard_count, GWARZ[Gloc].pig_count, Xzoneprice);

							yentry++;
							ypigcnt = ypigcnt + GWARZ[Gloc].pig_count;
							yvalue = yvalue + Xzoneprice;
						}
						Gloc = Gloc + 1;
					} while (Gloc < GWARZ.size());

					if (yentry == 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("*                 - Your guild does not own any land -               *");
						ChatHandler(player->GetSession()).PSendSysMessage("%s**********************************************************************", GWCOMM[guild_id].color_9.c_str());
					}

					if (yentry > 0)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%s**********************************************************************", GWCOMM[guild_id].color_9.c_str());
						ChatHandler(player->GetSession()).PSendSysMessage("%stotal locations: %s%u|r      %stotal pigs:|r %s%u|r      %sTotal value:|r %s%u|r %s%s's.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), uint32(yentry), GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), uint32(ypigcnt), GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), uint32(yvalue), GWCOMM[guild_id].color_1.c_str(), Currencyname.c_str());
						ChatHandler(player->GetSession()).PSendSysMessage("%s**********************************************************************", GWCOMM[guild_id].color_9.c_str());
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
					uint32 PGR = GetRank(guild_id);
					uint32 rank = 1;
					uint32 size = GW_Ranks.size();
					uint32 max;

					if (size <= GUILDWARZ_RANKING_MAX){ max = size; };
					if (size > GUILDWARZ_RANKING_MAX){ max = GUILDWARZ_RANKING_MAX; };

					if (test){ TC_LOG_INFO("server.loading", "<GUILDWARZ_RANKING_MAX>%u <SIZE>%u <SET_SIZE>%u", GUILDWARZ_RANKING_MAX, size, max); };

					ChatHandler(player->GetSession()).PSendSysMessage("%s****************************************", GWCOMM[guild_id].color_9.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sRank   :   Score   :   Guild|r", GWCOMM[SERVER_GUILD_ID].color_1.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%s****************************************", GWCOMM[guild_id].color_9.c_str());

					for (rank = 1; rank <= max; rank++)do
					{
						uint32 GID = GW_Ranks[rank].guild_id;
						uint8 GTID = GW_Ranks[rank].team;
						uint32 GR = rank; //  GetRank(GID);

						if (GTID == 0) { ChatHandler(player->GetSession()).PSendSysMessage("%s%u             %s%u            %s%s", GWCOMM[guild_id].color_1.c_str(), GR, GWCOMM[guild_id].color_1.c_str(), GW_Ranks[rank].total_gross_worth, GWCOMM[GID].color_4.c_str(), GW_Ranks[rank].name.c_str()); };
						if (GTID == 1) { ChatHandler(player->GetSession()).PSendSysMessage("%s%u             %s%u            %s%s", GWCOMM[guild_id].color_1.c_str(), GR, GWCOMM[guild_id].color_1.c_str(), GW_Ranks[rank].total_gross_worth, GWCOMM[GID].color_5.c_str(), GW_Ranks[rank].name.c_str()); };

						rank = rank + 1;

					} while (rank <= max);

					ChatHandler(player->GetSession()).PSendSysMessage("%s****************************************", GWCOMM[guild_id].color_9.c_str());
	
					if (pTeam_id == 0) { ChatHandler(player->GetSession()).PSendSysMessage("%sYour Guild|r `%s%s|r`%s is Ranked|r #%u.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_4.c_str(), guild_name.c_str(), GWCOMM[guild_id].color_1.c_str(), PGR); };
					if (pTeam_id == 1) { ChatHandler(player->GetSession()).PSendSysMessage("%sYour Guild|r `%s%s|r`%s is Ranked|r #%u.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_5.c_str(), guild_name.c_str(), GWCOMM[guild_id].color_1.c_str(), PGR); };
					
					ChatHandler(player->GetSession()).PSendSysMessage("%s****************************************", GWCOMM[guild_id].color_9.c_str());
					msg = GGW_MSG;
				}
				
/*
-- ***************
-- Version Command
-- ***************
*/

				if (ChatCache[1] == GWCOMM[guild_id].version)
				{
					ChatHandler(player->GetSession()).PSendSysMessage("%s****************************************", GWCOMM[guild_id].color_9.c_str());
					ChatHandler(player->GetSession()).PSendSysMessage("%sGrumbo'z Guild Warz. VER :|r %.2fc|r", GWCOMM[guild_id].color_1.c_str(), GW_version);
					ChatHandler(player->GetSession()).PSendSysMessage("%sCore ::|r %.2f.", GWCOMM[guild_id].color_1.c_str(), core_version);
					ChatHandler(player->GetSession()).PSendSysMessage("%sPigPayz ::|r %.2f.", GWCOMM[guild_id].color_1.c_str(), pigpayz_version);
					ChatHandler(player->GetSession()).PSendSysMessage("%sTeleport ::|r %.2f.", GWCOMM[guild_id].color_1.c_str(), tele_version);
					ChatHandler(player->GetSession()).PSendSysMessage("%sPvP ::|r %.2f.", GWCOMM[guild_id].color_1.c_str(), pvp_version);
					ChatHandler(player->GetSession()).PSendSysMessage("%sVendor ::|r %.2f.", GWCOMM[guild_id].color_1.c_str(), vendor_version);
					ChatHandler(player->GetSession()).PSendSysMessage("%s****************************************", GWCOMM[guild_id].color_9.c_str());
					msg = GGW_MSG;
				}

/*
-- ***********
-- Set Command
-- ***********
*/

				if (ChatCache[1] == GWCOMM[SERVER_GUILD_ID].command_set)
				{
					if (ChatCache[3] == "")
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%sCMD ERROR:|r %s %s NO data entered.", GWCOMM[guild_id].color_15.c_str(), ChatCache[1].c_str(), GWCOMM[guild_id].color_15.c_str());
	
						if (ChatCache[2] == "") 
							ChatHandler(player->GetSession()).PSendSysMessage("%sType|r `/g help set` %sfor a list of settings you can adjust.|r", GWCOMM[guild_id].color_10.c_str(), GWCOMM[guild_id].color_10.c_str());
						else
						{
							ChatHandler(player->GetSession()).PSendSysMessage("%sType `/g help set %s for help with changing the value.", GWCOMM[guild_id].color_10.c_str(), ChatCache[2].c_str());
						};

						return;
					}

					if ((ChatCache[2] != "")&&(ChatCache[3] != ""))
					{
						for (uint64 name2 = 0; name2 < GWHELP.size(); ++name2) do
						{
							if (ChatCache[2] == GWHELP[name2].name)
							{
								if ((GWHELP[name2].command_level <= 4)&&(pGuildRank == 0))
								{
									if (test){ TC_LOG_INFO("server.loading", "<GUILD-MASTER_SET_COMMAND> %s %s %s %u", ChatCache[1].c_str(), ChatCache[2].c_str(), ChatCache[3].c_str(), guild_id); };
										
									UpdateGuildCommandData(ChatCache[2], ChatCache[3].c_str(), guild_id);

									ChatHandler(player->GetSession()).PSendSysMessage("guild cmd %s set to `%s`.", ChatCache[2].c_str(), ChatCache[3].c_str());

									msg = GGW_MSG;
									break;
								}

								if (GWHELP[name2].command_level <= 4 && pGuildRank > 0)
								{
									ChatHandler(player->GetSession()).PSendSysMessage("%sGLD CMD ACCESS ERROR.:|r %u", GWCOMM[guild_id].color_15.c_str(), pGuildRank);
									ChatHandler(player->GetSession()).PSendSysMessage("%sCMD ERROR:|r %s %s %s", GWCOMM[guild_id].color_15.c_str(), ChatCache[1].c_str(), ChatCache[2].c_str(), ChatCache[3].c_str());

									msg = GGW_MSG;
									break;
								}

								if (pGMRank >= GWCOMM[SERVER_GUILD_ID].GM_admin && ((GWHELP[name2].command_level == 5) || (GWHELP[name2].command_level == 6)))
								{
									if (test){ TC_LOG_INFO("server.loading", "<GUILD-MASTER_SET_COMMAND> %s %s %s", ChatCache[1].c_str(), ChatCache[2].c_str(), ChatCache[3].c_str()); };

									UpdateGuildCommandData(ChatCache[2].c_str(), ChatCache[3].c_str(), SERVER_GUILD_ID);
										
									ChatHandler(player->GetSession()).PSendSysMessage("%sGM cmd %s set to `%s`|r.", GWCOMM[guild_id].color_10.c_str(), ChatCache[2].c_str(), ChatCache[3].c_str());

									msg = GGW_MSG;
									break;
								}

								if (pGMRank != GWCOMM[SERVER_GUILD_ID].GM_admin && (GWHELP[name2].command_level == 5 || GWHELP[name2].command_level == 6))
								{
									ChatHandler(player->GetSession()).PSendSysMessage("%sGM CMD ACCESS ERROR.: |r%u", GWCOMM[guild_id].color_15.c_str(), pGMRank);
									ChatHandler(player->GetSession()).PSendSysMessage("%sCMD ERROR:|r %s %s %s %s.", GWCOMM[guild_id].color_15.c_str(), ChatCache[1].c_str(), ChatCache[2].c_str(), ChatCache[3].c_str(), ChatCache[4].c_str());
									ChatHandler(player->GetSession()).PSendSysMessage("<%sGrumbo|r>%sIt's really no big deal.. you just dont have access to some features of `GGWz`.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_1.c_str());
									break;
								}
							}
							name2++;
						} while (name2 < GWHELP.size());
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

					if (ChatCache[2] == GWCOMM[guild_id].command_set && ChatCache[3]=="")
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%scommand id      -     description", GWCOMM[guild_id].color_10.c_str());

						for (uint64 hlpsub = 0;  hlpsub < GWHELP.size(); ++hlpsub) do
						{
							if (pGuildRank == 0 && GWHELP[hlpsub].command_level <= 4)
							{
								ChatHandler(player->GetSession()).PSendSysMessage("%s%s     -     %s%s|r", GWCOMM[guild_id].color_10.c_str(), GWHELP[hlpsub].name.c_str(), GWCOMM[guild_id].color_11.c_str(), GWHELP[hlpsub].description.c_str());
							}

							if (pGMRank == GWCOMM[SERVER_GUILD_ID].GM_admin && GWHELP[hlpsub].command_level >= 5 && GWHELP[hlpsub].command_level <= 6)
							{
								ChatHandler(player->GetSession()).PSendSysMessage("%s%s - %s%s|r", GWCOMM[guild_id].color_10.c_str(), GWHELP[hlpsub].name.c_str(), GWCOMM[guild_id].color_11.c_str(), GWHELP[hlpsub].description.c_str());
							}
							hlpsub++;
						} while (hlpsub < GWHELP.size());
					}

					if (ChatCache[2] == GWCOMM[guild_id].command_set && ChatCache[3] != "")
					{
						for (uint8 name = 1; name < GWHELP.size(); ++name) do
						{
							if (ChatCache[3] == GWHELP[name].name)
							{
								if (pGuildRank == 0 && GWHELP[name].command_level <= 4)
								{
									ChatHandler(player->GetSession()).PSendSysMessage("%s%s      -      %s|r", GWCOMM[guild_id].color_10.c_str(), GWHELP[name].name.c_str(), GWHELP[name].description.c_str());
									ChatHandler(player->GetSession()).PSendSysMessage("%sExample:|r /g %s %s", GWCOMM[guild_id].color_10.c_str(), GWCOMM[guild_id].color_11.c_str(), GWHELP[name].example.c_str());
									break;
								}

								if (pGMRank == GWCOMM[SERVER_GUILD_ID].GM_admin && GWHELP[name].command_level >= 5 && GWHELP[name].command_level <= 6)
								{
									ChatHandler(player->GetSession()).PSendSysMessage("%s%s      -      %s|r", GWCOMM[guild_id].color_10.c_str(), GWHELP[name].name.c_str(), GWHELP[name].description.c_str());
									ChatHandler(player->GetSession()).PSendSysMessage("%sExample:|r /g %s%s", GWCOMM[guild_id].color_10.c_str(), GWCOMM[guild_id].color_11.c_str(), GWHELP[name].example.c_str());
									break;
								}
							}
							name++;
						} while (name < GWHELP.size());
					}
				}

// ****************************************************
// ************ CORE: Guild Master Commands ***********
// ****************************************************

// ****************************************************
// ******************* Buy commands *******************

				if (pGuildRank <= GWCOMM[guild_id].GLD_lvlb && ChatCache[1] == "buy")
				{
					if(GWCOMM[guild_id].allowed >= 1)
					{
						if (player->IsGameMaster())
						{
							ChatHandler(player->GetSession()).PSendSysMessage("%sYou Must exit GM mode first to use this command.", GWCOMM[guild_id].color_15.c_str());
						}
					
						if (GWARZ[LocId].team == SERVER_GUILD_TEAM_LOCKED_ID)
						{
							ChatHandler(player->GetSession()).PSendSysMessage("%sTHIS AREA IS OFF LIMITS.|r", GWCOMM[guild_id].color_15.c_str());
						};

						if (!player->IsGameMaster())
						{

	/*
	-- **** Location ****
	*/

							if (ChatCache[2] == GWCOMM[guild_id].loc)
							{
								if ((GWARZ[LocId].guild_id != SERVER_GUILD_ID) || (GWARZ[LocId].guild_id == guild_id))
								{
									ChatHandler(player->GetSession()).PSendSysMessage("%sYou cannot purchase this area.", GWCOMM[guild_id].color_15.c_str());
									ChatHandler(player->GetSession()).PSendSysMessage("%s%s|r owns this area.", GWCOMM[guild_id].color_14.c_str(), GWARZ[LocId].guild_name.c_str());
								}

								if (GWARZ[LocId].guild_id == SERVER_GUILD_ID)
								{
									if (GWARZ[LocId].team == 2)
									{
										if (player->GetItemCount(GWCOMM[SERVER_GUILD_ID].currency) < Zoneprice)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou do not have enough %s's.", GWCOMM[guild_id].color_15.c_str(), Currencyname.c_str());
										};

										if (player->GetItemCount(GWCOMM[SERVER_GUILD_ID].currency) >= Zoneprice) // Flag spawn is handled uniquely since i need to store its GUIDLow();
										{
											uint32 spawnflagGUID = SpawnGuildObjects(2, GWCOMM[SERVER_GUILD_ID].flag_id, pTeam_id, guild_id, Pmap, pX, pY, pZ, pO, player, LocId);

											if (!spawnflagGUID)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%sflag spawn error..|r Try again.", GWCOMM[guild_id].color_15.c_str());
											};

											if (spawnflagGUID)
											{

												player->DestroyItemCount(uint32(GWCOMM[SERVER_GUILD_ID].currency), Zoneprice, true);

			
												ChatHandler(player->GetSession()).PSendSysMessage("-%u %s's.|r", uint32(Zoneprice), Currencyname.c_str());

												if (test){ TC_LOG_INFO("server.loading", "GUILD_ID:%u FLAG_GUID:%u TEAM_ID:%u LOC_ID:%u", guild_id, spawnflagGUID, pTeam_id, LocId); };
												if (test){ TC_LOG_INFO("server.loading", "GUILD_ID:%u FLAG_GUID:%u TEAM_ID:%u LOC_ID:%u", guild_id, spawnflagGUID, pTeam_id, GWARZ[LocId].entry); };

											
												UpdateGuildLocData("guild_name", guild->GetName(), LocId);
												UpdateGuildLocData("team", ConvertNumberToString(pTeam_id), LocId);
												UpdateGuildLocData("flag_id", ConvertNumberToString(spawnflagGUID), LocId);
												UpdateGuildLocData("fs_time", ConvertNumberToString(sWorld->GetGameTime()), LocId);
												UpdateGuildLocData("guild_id", ConvertNumberToString(guild_id), LocId);

												UpdateGuildLocFloat(pX, pY, pZ, LocId);

												std::string ann = GWCOMM[guild_id].color_1 + pName + " has added location:" + str_LocId + " to your Guild Lands.";

												SendGuildMessage(GWARZ[LocId].guild_id, ann);

													if (GUILDWARZ_RANK_TYPE > 1) { CreateRankList(); };
											
													if (player->getGender() == 0)
													{
														ChatHandler(player->GetSession()).PSendSysMessage("%sCongratulations King %s. you have expanded %s's land.|r", GWCOMM[guild_id].color_14.c_str(), pName.c_str(), guild_name.c_str());
													};

													if (player->getGender() == 1)
													{
														ChatHandler(player->GetSession()).PSendSysMessage("%sCongratulations Queen %s. you have expanded %s's land.|r", GWCOMM[guild_id].color_14.c_str(), pName.c_str(), guild_name.c_str());
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

							if (GWARZ[LocId].guild_id != guild_id)
							{
								ChatHandler(player->GetSession()).PSendSysMessage("%sYour Guild does not own this land.", GWCOMM[guild_id].color_15.c_str());
							};

							if (GWARZ[LocId].guild_id == guild_id)
							{
								if (ChatCache[2] == GWCOMM[guild_id].farm)
								{
									if (player->GetItemCount(GWCOMM[SERVER_GUILD_ID].currency) < GWCOMM[SERVER_GUILD_ID].farm_cost)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sYou require more %s's.", GWCOMM[guild_id].color_15.c_str(), Currencyname.c_str());
									}

									if (player->GetItemCount(GWCOMM[SERVER_GUILD_ID].currency) >= GWCOMM[SERVER_GUILD_ID].farm_cost)
									{
										if (GWARZ[LocId].farm_count >= GWCOMM[SERVER_GUILD_ID].farm_L)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou have %u farm`s at this location.", GWCOMM[guild_id].color_15.c_str(), GWARZ[LocId].farm_count);
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou can only purchase %u farm`s per location.", GWCOMM[guild_id].color_15.c_str(), GWCOMM[SERVER_GUILD_ID].farm_L);
										};

										if (GWARZ[LocId].farm_count < GWCOMM[SERVER_GUILD_ID].farm_L)
										{
											if (GWARZ[LocId].pig_count < (GWARZ[LocId].farm_count * GWCOMM[SERVER_GUILD_ID].pig_L))
											{
												ChatHandler(player->GetSession()).PSendSysMessage("you still need to finish populating your other farms.", GWCOMM[guild_id].color_15.c_str());
											};

											if (GWARZ[LocId].pig_count >= (GWARZ[LocId].farm_count * GWCOMM[SERVER_GUILD_ID].pig_L))
											{
												uint32 spawnfarm = SpawnGuildObjects(1, GWCOMM[SERVER_GUILD_ID].farm_id, pTeam_id, guild_id, Pmap, pX, pY, pZ, pO, player, LocId);

												if (!spawnfarm)
												{
													ChatHandler(player->GetSession()).PSendSysMessage("%sFarm spawn error..|r Try again.", GWCOMM[guild_id].color_15.c_str());
												};

												if (spawnfarm)
												{
													player->DestroyItemCount(uint32(GWCOMM[SERVER_GUILD_ID].currency), GWCOMM[SERVER_GUILD_ID].farm_cost, true);

													ChatHandler(player->GetSession()).PSendSysMessage("-%u %s's.|r", GWCOMM[SERVER_GUILD_ID].farm_cost, Currencyname.c_str());

													UpdateGuildLocData("farm_count", ConvertNumberToString(GWARZ[LocId].farm_count + 1), LocId);

													std::string ann = pName + " has added a farm to location:" + str_LocId + ".";

													SendGuildMessage(GWARZ[LocId].guild_id, ann);
													msg = GGW_MSG;
												}
											}
										}
									}
								}

	/*
	-- **** Barrack ****
	*/

								if (ChatCache[2] == GWCOMM[guild_id].barrack)
								{
									if (player->GetItemCount(GWCOMM[SERVER_GUILD_ID].currency) < GWCOMM[SERVER_GUILD_ID].barrack_cost)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sYou require more %s's.", GWCOMM[guild_id].color_15.c_str(), Currencyname.c_str());
									}

									if (player->GetItemCount(GWCOMM[SERVER_GUILD_ID].currency) >= GWCOMM[SERVER_GUILD_ID].barrack_cost)
									{
										if (GWARZ[LocId].barrack_count >= uint32(GWCOMM[SERVER_GUILD_ID].farm_L * GWCOMM[SERVER_GUILD_ID].barrack_L))
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou have %u barrack\'s at this location.", GWCOMM[guild_id].color_15.c_str(), GWARZ[LocId].barrack_count);
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou can only purchase %u barrack\'s per location.", GWCOMM[guild_id].color_15.c_str(), (GWCOMM[SERVER_GUILD_ID].farm_L*GWCOMM[SERVER_GUILD_ID].barrack_L));
										};

										if (GWARZ[LocId].barrack_count < uint32(GWCOMM[SERVER_GUILD_ID].farm_L * GWCOMM[SERVER_GUILD_ID].barrack_L))
										{
											if (GWARZ[LocId].barrack_count >= GWARZ[LocId].farm_count*GWCOMM[SERVER_GUILD_ID].barrack_L)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%sEach farm supports %u barracks. You need another farm.", GWCOMM[guild_id].color_15.c_str(), GWCOMM[SERVER_GUILD_ID].barrack_L);
											};

											if (GWARZ[LocId].barrack_count < (GWARZ[LocId].farm_count*GWCOMM[SERVER_GUILD_ID].barrack_L))
											{
												uint32 spawnbarrack = SpawnGuildObjects(1, GWCOMM[SERVER_GUILD_ID].barrack_id, pTeam_id, guild_id, Pmap, pX, pY, pZ, pO, player, LocId);

												if (!spawnbarrack)
												{
													ChatHandler(player->GetSession()).PSendSysMessage("%sBarrack spawn error..|r Try again.", GWCOMM[guild_id].color_15.c_str());
												};

												if (spawnbarrack)
												{
													player->DestroyItemCount(uint32(GWCOMM[SERVER_GUILD_ID].currency), GWCOMM[SERVER_GUILD_ID].barrack_cost, true);

													ChatHandler(player->GetSession()).PSendSysMessage("-%u %s's.|r", GWCOMM[SERVER_GUILD_ID].barrack_cost, Currencyname.c_str());

													UpdateGuildLocData("barrack_count", ConvertNumberToString(GWARZ[LocId].barrack_count + 1), LocId);

													std::string ann = pName + " has added a bararck's to location:" + str_LocId + ".";

													SendGuildMessage(GWARZ[LocId].guild_id, ann);
													msg = GGW_MSG;
												}
											}
										}
									}
								}

	/*
	-- **** Hall ****
	*/

								if (ChatCache[2] == GWCOMM[guild_id].hall)
								{
									if (player->GetItemCount(GWCOMM[SERVER_GUILD_ID].currency) < GWCOMM[SERVER_GUILD_ID].hall_cost)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sEach hall costs %u %s. You dont have eough.", GWCOMM[guild_id].color_15.c_str(), GWCOMM[SERVER_GUILD_ID].hall_cost, Currencyname.c_str());
									}

									if (player->GetItemCount(GWCOMM[SERVER_GUILD_ID].currency) >= GWCOMM[SERVER_GUILD_ID].hall_cost)
									{
										if (GWARZ[LocId].hall_count >= GWCOMM[SERVER_GUILD_ID].hall_L)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou have %u hall at this location.", GWCOMM[guild_id].color_15.c_str(), GWARZ[LocId].hall_count);
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou can only have %u hall per area.", GWCOMM[guild_id].color_15.c_str(), GWCOMM[SERVER_GUILD_ID].hall_L);
										}

										if (GWARZ[LocId].hall_count < GWCOMM[SERVER_GUILD_ID].hall_L)
										{
											if (GWARZ[LocId].barrack_count == 0)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%sEach Hall require's 1 barracks per location to provide guards for defensive support.", GWCOMM[guild_id].color_15.c_str());
												ChatHandler(player->GetSession()).PSendSysMessage("%sGotta protect your HQ.", GWCOMM[guild_id].color_15.c_str());
											}

											if (GWARZ[LocId].barrack_count > 0)
											{
												uint32 spawnhall = SpawnGuildObjects(1, GWCOMM[SERVER_GUILD_ID].hall_id, pTeam_id, guild_id, Pmap, pX, pY, pZ, pO, player, LocId);

												if (!spawnhall)
												{
													ChatHandler(player->GetSession()).PSendSysMessage("%sHall spawn error..|r", GWCOMM[guild_id].color_15.c_str());
												};

												if (spawnhall)
												{
													player->DestroyItemCount(uint32(GWCOMM[SERVER_GUILD_ID].currency), GWCOMM[SERVER_GUILD_ID].hall_cost, true);

													ChatHandler(player->GetSession()).PSendSysMessage("-%u %s's.|r", GWCOMM[SERVER_GUILD_ID].hall_cost, Currencyname.c_str());

													UpdateGuildLocData("hall_count", ConvertNumberToString(GWARZ[LocId].hall_count + 1), LocId);

													std::string ann = pName + " has added a hall to location:" + str_LocId + ".";

													SendGuildMessage(GWARZ[LocId].guild_id, ann);
													msg = GGW_MSG;
												}
											}
										}
									}
								} // hall

	/*
	-- **** Pigs ****
	*/

								if (ChatCache[2] == GWCOMM[guild_id].pig)
								{
									if (player->GetItemCount(GWCOMM[SERVER_GUILD_ID].currency) < GWCOMM[SERVER_GUILD_ID].pig_cost)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sEach pig costs %u %s.", GWCOMM[guild_id].color_15.c_str(), GWCOMM[SERVER_GUILD_ID].pig_cost, Currencyname.c_str());
									}

									if (player->GetItemCount(GWCOMM[SERVER_GUILD_ID].currency) >= GWCOMM[SERVER_GUILD_ID].pig_cost)
									{
										if (GWARZ[LocId].pig_count >= uint32(GWCOMM[SERVER_GUILD_ID].pig_L * GWCOMM[SERVER_GUILD_ID].farm_L))
										{
											ChatHandler(player->GetSession()).PSendSysMessage("You have %u pigs at this location.", GWCOMM[guild_id].color_15.c_str(), GWARZ[LocId].pig_count*GWCOMM[SERVER_GUILD_ID].farm_L);
											ChatHandler(player->GetSession()).PSendSysMessage("You can only have %u pig's per farm and %u farm's per location.", GWCOMM[guild_id].color_15.c_str(), GWCOMM[SERVER_GUILD_ID].pig_L, GWCOMM[SERVER_GUILD_ID].farm_L);
										}

										if (GWARZ[LocId].farm_count == 0)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must first have a farm here before you can add pigs.", GWCOMM[guild_id].color_15.c_str());
											ChatHandler(player->GetSession()).PSendSysMessage("%sPiggies gotta live somewhere...", GWCOMM[guild_id].color_15.c_str());
										}

										if (GWARZ[LocId].pig_count >= (GWARZ[LocId].farm_count * GWCOMM[SERVER_GUILD_ID].pig_L))
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou require another farm before you can add any more pigs.", GWCOMM[guild_id].color_15.c_str());
										}

										if (GWARZ[LocId].pig_count < uint32(GWCOMM[SERVER_GUILD_ID].pig_L * GWARZ[LocId].farm_count) && GWARZ[LocId].farm_count != 0) // (GWARZ[LocId].pig_count < uint32(GWCOMM[SERVER_GUILD_ID].pig_L * GWCOMM[SERVER_GUILD_ID].farm_L) && GWARZ[LocId].farm_count != 0)
										{
											uint32 spawnpig = SpawnGuildObjects(0, GWCOMM[SERVER_GUILD_ID].pig_id, pTeam_id, guild_id, Pmap, pX, pY, pZ, pO, player, LocId);

											if (!spawnpig)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%spig spawn error..|r", GWCOMM[guild_id].color_15.c_str());
											}

											if (spawnpig)
											{
												player->DestroyItemCount(uint32(GWCOMM[SERVER_GUILD_ID].currency), uint32(GWCOMM[SERVER_GUILD_ID].pig_cost), true);

												ChatHandler(player->GetSession()).PSendSysMessage("-%u %s's.|r", GWCOMM[SERVER_GUILD_ID].pig_cost, Currencyname.c_str());

												UpdateGuildLocData("pig_count", ConvertNumberToString(GWARZ[LocId].pig_count + 1), LocId);

												std::string ann = pName + " has added a pig to location:" + str_LocId + ".";

												SendGuildMessage(GWARZ[LocId].guild_id, ann);
												msg = GGW_MSG;
											}
										}
									}
								}

	/*
	-- **** Guard ****
	*/

								if (ChatCache[2] == GWCOMM[guild_id].guard)
								{
									if (player->GetItemCount(GWCOMM[SERVER_GUILD_ID].currency) < GWCOMM[SERVER_GUILD_ID].guard_cost)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%aEach guard costs %u %s.", GWCOMM[guild_id].color_15.c_str(), GWCOMM[SERVER_GUILD_ID].guard_cost, Currencyname.c_str());
									}

									if (player->GetItemCount(GWCOMM[SERVER_GUILD_ID].currency) >= GWCOMM[SERVER_GUILD_ID].guard_cost)
									{
										if (GWARZ[LocId].guard_count >= uint16(GWCOMM[SERVER_GUILD_ID].guard_L * (GWCOMM[SERVER_GUILD_ID].barrack_L * GWCOMM[SERVER_GUILD_ID].farm_L)))
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou have %u guards at this location.", GWCOMM[guild_id].color_15.c_str(), GWARZ[LocId].guard_count);
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou can only have %u per location.", GWCOMM[guild_id].color_15.c_str(), (GWCOMM[SERVER_GUILD_ID].guard_L*GWCOMM[SERVER_GUILD_ID].farm_L));
										}

										if (GWARZ[LocId].guard_count < uint16(GWCOMM[SERVER_GUILD_ID].guard_L * (GWCOMM[SERVER_GUILD_ID].barrack_L * GWCOMM[SERVER_GUILD_ID].farm_L)))
										{
											if (GWARZ[LocId].guard_count >= (GWARZ[LocId].barrack_count * GWCOMM[SERVER_GUILD_ID].guard_L))
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%sYou must add another barracks to produce more guards.", GWCOMM[guild_id].color_15.c_str());
											}

											if (GWARZ[LocId].guard_count < (GWARZ[LocId].barrack_count * GWCOMM[SERVER_GUILD_ID].guard_L))
											{
												uint32 spawnguard = SpawnGuildObjects(0, GWCOMM[SERVER_GUILD_ID].guard_id, pTeam_id, guild_id, Pmap, pX, pY, pZ, pO, player, LocId);

												if (!spawnguard)
												{
													ChatHandler(player->GetSession()).PSendSysMessage("%sguard spawn error..|r", GWCOMM[guild_id].color_15.c_str());
												}

												if (spawnguard)
												{
													player->DestroyItemCount(uint32(GWCOMM[SERVER_GUILD_ID].currency), uint32(GWCOMM[SERVER_GUILD_ID].guard_cost), true);

													ChatHandler(player->GetSession()).PSendSysMessage("-%u %s's.|r", GWCOMM[SERVER_GUILD_ID].guard_cost, Currencyname.c_str());

													UpdateGuildLocData("guard_count", ConvertNumberToString(GWARZ[LocId].guard_count + 1), LocId);

													std::string ann = pName + " has added a guard to location:" + str_LocId + ".";

													SendGuildMessage(GWARZ[LocId].guild_id, ann);
													msg = GGW_MSG;
												}
											}
										}
									}
								} // guard

	/*
	-- **** Vendor 1 ****
	*/

								if (ChatCache[2] == GWCOMM[guild_id].vendor1)
								{
									if (!GWCOMM[SERVER_GUILD_ID].vendor1_id)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sVendor1 is NOT enabled.. Contact your Admin to enable Vendor 1", GWCOMM[guild_id].color_15.c_str());
									};

									if (GWCOMM[SERVER_GUILD_ID].vendor1_id)
									{
										if (player->GetItemCount(GWCOMM[SERVER_GUILD_ID].currency) < GWCOMM[SERVER_GUILD_ID].vendor1_cost)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sEach vendor1 costs %u %s.", GWCOMM[guild_id].color_15.c_str(), GWCOMM[SERVER_GUILD_ID].vendor1_cost, Currencyname.c_str());
										}

										if (GWARZ[LocId].vendor1_count >= (GWCOMM[SERVER_GUILD_ID].vendor1_L * GWARZ[LocId].hall_count))
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou have %u vendor1's at this location.", GWCOMM[guild_id].color_15.c_str(), GWARZ[LocId].vendor1_count);
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou can only have %u per Hall.", GWCOMM[guild_id].color_15.c_str(), GWCOMM[SERVER_GUILD_ID].vendor1_L);
										}

										if (GWARZ[LocId].vendor1_count < (GWCOMM[SERVER_GUILD_ID].vendor1_L * GWARZ[LocId].hall_count))
										{
											uint32 spawnvendor1 = SpawnGuildObjects(0, GWCOMM[SERVER_GUILD_ID].vendor1_id, pTeam_id, guild_id, Pmap, pX, pY, pZ, pO, player, LocId);

											if (!spawnvendor1)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%svendor1 spawn error..|r", GWCOMM[guild_id].color_15.c_str());
											}

											if (spawnvendor1)
											{
												player->DestroyItemCount(uint32(GWCOMM[SERVER_GUILD_ID].currency), uint32(GWCOMM[SERVER_GUILD_ID].vendor1_cost), true);

												ChatHandler(player->GetSession()).PSendSysMessage("-%u %s's.|r", GWCOMM[SERVER_GUILD_ID].vendor1_cost, Currencyname.c_str());

												UpdateGuildLocData("vendor1_count", ConvertNumberToString(GWARZ[LocId].vendor1_count + 1), LocId);

												std::string ann = pName + " has added a vendor1 to location:" + str_LocId + ".";

												SendGuildMessage(GWARZ[LocId].guild_id, ann);
												msg = GGW_MSG;
											}
										}
									}
								} // vendor 1

	/*
	-- **** Vendor 2 ****
	*/

								if (ChatCache[2] == GWCOMM[guild_id].vendor2)
								{
									if (!GWCOMM[SERVER_GUILD_ID].vendor2_id)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sVendor2 is NOT enabled..", GWCOMM[guild_id].color_15.c_str());
									}

									if (GWCOMM[SERVER_GUILD_ID].vendor2_id)
									{
										if (player->GetItemCount(GWCOMM[SERVER_GUILD_ID].currency) < GWCOMM[SERVER_GUILD_ID].vendor2_cost)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sEach vendor2 costs %u %s.", GWCOMM[guild_id].color_15.c_str(), GWCOMM[SERVER_GUILD_ID].vendor2_cost, Currencyname.c_str());
										};

										if (GWARZ[LocId].vendor2_count >= (GWCOMM[SERVER_GUILD_ID].vendor2_L * GWARZ[LocId].hall_count))
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou have %u vendor2's at this location.", GWCOMM[guild_id].color_15.c_str(), GWARZ[LocId].vendor1_count);
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou can only have %u per location.", GWCOMM[guild_id].color_15.c_str(), GWCOMM[SERVER_GUILD_ID].vendor2_L);
										};

										if (GWARZ[LocId].vendor2_count < (GWCOMM[SERVER_GUILD_ID].vendor2_L * GWARZ[LocId].hall_count))
										{
											uint32 spawnvendor2 = SpawnGuildObjects(0, GWCOMM[SERVER_GUILD_ID].vendor2_id, pTeam_id, guild_id, Pmap, pX, pY, pZ, pO, player, LocId);

											if (!spawnvendor2)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%svendor2 spawn error..|r", GWCOMM[guild_id].color_15.c_str());
											}

											if (spawnvendor2)
											{
												player->DestroyItemCount(uint32(GWCOMM[SERVER_GUILD_ID].currency), uint32(GWCOMM[SERVER_GUILD_ID].vendor2_cost), true);

												UpdateGuildLocData("vendor2_count", ConvertNumberToString(GWARZ[LocId].vendor2_count + 1), LocId);

												ChatHandler(player->GetSession()).PSendSysMessage("-%u %s's.|r", GWCOMM[SERVER_GUILD_ID].vendor2_cost, Currencyname.c_str());

												std::string ann = pName + " has added a vendor2 to location:" + str_LocId + ".";
												msg = GGW_MSG;
											}
										}
									}
								} // vendor 2

	/*
	-- **** Vendor 3 ****
	*/

								if (ChatCache[2] == GWCOMM[guild_id].vendor3)
								{
									if (!GWCOMM[SERVER_GUILD_ID].vendor3_id)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sVendor3 is NOT enabled..", GWCOMM[guild_id].color_15.c_str());
									}

									if (GWCOMM[SERVER_GUILD_ID].vendor3_id)
									{
										if (player->GetItemCount(GWCOMM[SERVER_GUILD_ID].currency) < GWCOMM[SERVER_GUILD_ID].vendor3_cost)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sEach vendor3 costs %u %s.", GWCOMM[guild_id].color_15.c_str(), GWCOMM[SERVER_GUILD_ID].vendor3_cost, Currencyname.c_str());
										};

										if (GWARZ[LocId].vendor3_count >= (GWCOMM[SERVER_GUILD_ID].vendor3_L * GWARZ[LocId].hall_count))
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou have %u vendor3's at this location.", GWCOMM[guild_id].color_15.c_str(), GWARZ[LocId].vendor1_count);
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou can only have %u per location.", GWCOMM[guild_id].color_15.c_str(), GWCOMM[SERVER_GUILD_ID].vendor2_L);
										};

										if (GWARZ[LocId].vendor3_count < GWCOMM[SERVER_GUILD_ID].vendor3_L)
										{
											uint32 spawnvendor3 = SpawnGuildObjects(0, GWCOMM[SERVER_GUILD_ID].vendor3_id, pTeam_id, guild_id, Pmap, pX, pY, pZ, pO, player, LocId);

											if (!spawnvendor3)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%svendor3 spawn error..|r", GWCOMM[guild_id].color_15.c_str());
											};

											if (spawnvendor3)
											{
												player->DestroyItemCount(uint32(GWCOMM[SERVER_GUILD_ID].currency), uint32(GWCOMM[SERVER_GUILD_ID].vendor3_cost), true);

												UpdateGuildLocData("vendor3_count", ConvertNumberToString(GWARZ[LocId].vendor3_count + 1), LocId);

												ChatHandler(player->GetSession()).PSendSysMessage("-%u %s's.|r", GWCOMM[SERVER_GUILD_ID].vendor3_cost, Currencyname.c_str());

												std::string ann = pName + " has added a vendor3 to location:" + str_LocId + ".";

												SendGuildMessage(GWARZ[LocId].guild_id, ann);
												msg = GGW_MSG;
											}
										}
									}
								} // vendor 3

	/*
	-- **** Cannon ****
	*/

								if (ChatCache[2] == GWCOMM[guild_id].cannon)
								{

									if (!GWCOMM[SERVER_GUILD_ID].cannon_id)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%scannon's NOT enabled..", GWCOMM[guild_id].color_15.c_str());
									}

									if (GWCOMM[SERVER_GUILD_ID].cannon_id)
									{
										if (player->GetItemCount(GWCOMM[SERVER_GUILD_ID].currency) < GWCOMM[SERVER_GUILD_ID].cannon_cost)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sEach cannon costs %u %s.", GWCOMM[guild_id].color_15.c_str(), GWCOMM[SERVER_GUILD_ID].cannon_cost, Currencyname.c_str());
										}

										if (player->GetItemCount(GWCOMM[SERVER_GUILD_ID].currency) >= GWCOMM[SERVER_GUILD_ID].cannon_cost)
										{
											if (GWARZ[LocId].cannon_count >= (GWCOMM[SERVER_GUILD_ID].cannon_L * GWARZ[LocId].hall_count))
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%sYou have %u %s's at this location.|r", GWCOMM[guild_id].color_15.c_str(), GWARZ[LocId].cannon_count, GWCOMM[guild_id].cannon.c_str());
												ChatHandler(player->GetSession()).PSendSysMessage("%sYou can only have %u per Hall.|r", GWCOMM[guild_id].color_15.c_str(), GWCOMM[SERVER_GUILD_ID].cannon_L);
											}

											if (GWARZ[LocId].cannon_count < (GWCOMM[SERVER_GUILD_ID].cannon_L * GWARZ[LocId].hall_count))
											{
												if (GWARZ[LocId].hall_count == 0)
												{
													ChatHandler(player->GetSession()).PSendSysMessage("%sCannons require a Hall per location.", GWCOMM[guild_id].color_15.c_str());
												}

												if (GWARZ[LocId].hall_count > 0)
												{
													uint32 spawncannon = SpawnGuildObjects(0, GWCOMM[SERVER_GUILD_ID].cannon_id, pTeam_id, guild_id, Pmap, pX, pY, pZ, pO, player, LocId);

													if (!spawncannon)
													{
														ChatHandler(player->GetSession()).PSendSysMessage("%scannon spawn error..|r", GWCOMM[guild_id].color_15.c_str());
													}

													if (spawncannon)
													{
														player->DestroyItemCount(uint32(GWCOMM[SERVER_GUILD_ID].currency), uint32(GWCOMM[SERVER_GUILD_ID].cannon_cost), true);

														ChatHandler(player->GetSession()).PSendSysMessage("-%u %s's.|r", GWCOMM[SERVER_GUILD_ID].cannon_cost, Currencyname.c_str());

														UpdateGuildLocData("cannon_count", ConvertNumberToString(GWARZ[LocId].cannon_count + 1), LocId);

														std::string ann = pName + " has added a cannon to location:" + str_LocId + ".";

														SendGuildMessage(GWARZ[LocId].guild_id, ann);

														ChatHandler(player->GetSession()).PSendSysMessage("%s!! Its a cannon !!|r", GWCOMM[guild_id].color_14.c_str());
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

								if (ChatCache[2] == GWCOMM[guild_id].vault)
								{
									if (player->GetItemCount(GWCOMM[SERVER_GUILD_ID].currency) < GWCOMM[SERVER_GUILD_ID].vault_cost)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sEach vault costs %u %s.", GWCOMM[guild_id].color_15.c_str(), GWCOMM[SERVER_GUILD_ID].vault_cost, Currencyname.c_str());
									}

									if (player->GetItemCount(GWCOMM[SERVER_GUILD_ID].currency) >= GWCOMM[SERVER_GUILD_ID].vault_cost)
									{
										if (GWARZ[LocId].vault_count >= (GWCOMM[SERVER_GUILD_ID].vault_cost * GWARZ[LocId].hall_count))
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou have %u %s's at this location.", GWCOMM[guild_id].color_15.c_str(), GWARZ[LocId].vault_count, GWCOMM[guild_id].vault.c_str());
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou can only have %u per hall.", GWCOMM[guild_id].color_15.c_str(), GWCOMM[SERVER_GUILD_ID].vault_L);
										}

										if (GWARZ[LocId].vault_count < GWCOMM[SERVER_GUILD_ID].vault_L * GWARZ[LocId].hall_count)
										{
											if (GWARZ[LocId].hall_count == 0)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%sVaults require a Hall.", GWCOMM[guild_id].color_15.c_str());
											}

											if (GWARZ[LocId].hall_count > 0)
											{
												uint32 spawnvault = SpawnGuildObjects(1, GWCOMM[SERVER_GUILD_ID].vault_id, pTeam_id, guild_id, Pmap, pX, pY, pZ, pO, player, LocId);

												if (!spawnvault)
												{
													ChatHandler(player->GetSession()).PSendSysMessage("%svault spawn error..|r", GWCOMM[guild_id].color_15.c_str());
												}

												if (spawnvault)
												{
													player->DestroyItemCount(uint32(GWCOMM[SERVER_GUILD_ID].currency), uint32(GWCOMM[SERVER_GUILD_ID].vault_cost), true);

													UpdateGuildLocData("vault_count", ConvertNumberToString(GWARZ[LocId].vault_count + 1), LocId);

													ChatHandler(player->GetSession()).PSendSysMessage("-%u %s's.|r", GWCOMM[SERVER_GUILD_ID].vault_cost, Currencyname.c_str());

													std::string ann = pName + " has added a vault to location:" + str_LocId + ".";

													SendGuildMessage(GWARZ[LocId].guild_id, ann);
													msg = GGW_MSG;
												}
											}
										}
									}
								} // vault

	/*
	-- **** MailBox ****
	*/

								if (ChatCache[2] == GWCOMM[guild_id].mailbox)
								{
									if (player->GetItemCount(GWCOMM[SERVER_GUILD_ID].currency) < GWCOMM[SERVER_GUILD_ID].mailbox_cost)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sEach mailbox costs %u %s.", GWCOMM[guild_id].color_15.c_str(), GWCOMM[SERVER_GUILD_ID].mailbox_cost, Currencyname.c_str());
									}

									if (player->GetItemCount(GWCOMM[SERVER_GUILD_ID].currency) >= (GWCOMM[SERVER_GUILD_ID].mailbox_cost * GWARZ[LocId].vault_count))
									{
										if (GWARZ[LocId].mailbox_count >= GWCOMM[SERVER_GUILD_ID].mailbox_L * GWARZ[LocId].hall_count)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou have %u %s's at this location.", GWCOMM[guild_id].color_15.c_str(), GWARZ[LocId].mailbox_count, GWCOMM[guild_id].mailbox.c_str());
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou can only have %u per hall.", GWCOMM[guild_id].color_15.c_str(), GWCOMM[SERVER_GUILD_ID].mailbox_L);
										};

										if (GWARZ[LocId].mailbox_count < (GWCOMM[SERVER_GUILD_ID].mailbox_cost * GWARZ[LocId].vault_count))
										{
											if (GWARZ[LocId].hall_count == 0)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%smailbox require a Hall.", GWCOMM[guild_id].color_15.c_str());
											};

											if (GWARZ[LocId].hall_count > 0)
											{
												uint32 spawnmailbox = SpawnGuildObjects(1, GWCOMM[SERVER_GUILD_ID].mailbox_id, pTeam_id, guild_id, Pmap, pX, pY, pZ, pO, player, LocId);

												if (!spawnmailbox)
												{
													ChatHandler(player->GetSession()).PSendSysMessage("%smailbox spawn error..|r", GWCOMM[guild_id].color_15.c_str());
												};

												if (spawnmailbox)
												{
													player->DestroyItemCount(uint32(GWCOMM[SERVER_GUILD_ID].currency), uint32(GWCOMM[SERVER_GUILD_ID].mailbox_cost), true);

													UpdateGuildLocData("mailbox_count", ConvertNumberToString(GWARZ[LocId].mailbox_count + 1), LocId);

													ChatHandler(player->GetSession()).PSendSysMessage("-%u %s's.|r", GWCOMM[SERVER_GUILD_ID].mailbox_cost, Currencyname.c_str());

													std::string ann = pName + " has added a mailbox to location:" + str_LocId + ".";

													SendGuildMessage(GWARZ[LocId].guild_id, ann);
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
							ChatHandler(player->GetSession()).PSendSysMessage("<%sGrumbo|r>:%sSpeak With Your faction Leader to gain access to ownership and development.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_1.c_str());
						else
							ChatHandler(player->GetSession()).PSendSysMessage("<%sGrumbo|r>:%sRemind your Guild Leader to Speak With Your faction Leader to gain access to ownership and development.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_1.c_str());
					}
					msg = GGW_MSG;
				} // buy commands

// ******************* Sell commands *******************

				if ((pGuildRank <= GWCOMM[guild_id].GLD_lvlb) && ChatCache[1] == "sell")
				{
					if (GWCOMM[guild_id].allowed >= 1)
					{
						if (player->IsGameMaster())
						{
							ChatHandler(player->GetSession()).PSendSysMessage("%sYou Must exit GM mode first to use this command.", GWCOMM[guild_id].color_15.c_str());
						};

						if (!player->IsGameMaster())
						{
							if (GWARZ[LocId].guild_id != guild_id)
							{
								ChatHandler(player->GetSession()).PSendSysMessage("%sYour Guild does not own this land.", GWCOMM[guild_id].color_15.c_str());
							};

							if (GWARZ[LocId].guild_id == guild_id)
							{

	/*
	-- **** Location ****
	*/

								if (ChatCache[2] == GWCOMM[guild_id].loc)
								{
									GameObject *object = player->FindNearestGameObject(GWCOMM[SERVER_GUILD_ID].flag_id + pTeam_id, 5);

									if (!object)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sYou must stand closer to your flag.|r", GWCOMM[guild_id].color_15.c_str());
									}

									if (object)
									{
										if (player->AddItem(GWCOMM[SERVER_GUILD_ID].currency, Zoneprice))
										{
											object->RemoveFromWorld();
											object->DeleteFromDB();

											UpdateGuildLocData("guild_name", SERVER_GUILD_NAME, LocId);
											UpdateGuildLocData("team", "2", LocId);
											UpdateGuildLocData("guild_id", ConvertNumberToString(SERVER_GUILD_ID), LocId);

											std::string ann = pName + " has sold location:" + str_LocId + ".";

											SendGuildMessage(GWARZ[LocId].guild_id, ann);

											if (GUILDWARZ_RANK_TYPE > 1) { CreateRankList(); };

											ChatHandler(player->GetSession()).PSendSysMessage("+%u %s's.|r", Zoneprice, Currencyname.c_str());
											msg = GGW_MSG;
										} // can add currency
									}
								} // location

	/*
	-- **** Farm ****
	*/

								if (ChatCache[2] == GWCOMM[guild_id].farm)
								{
									if (GWARZ[LocId].farm_count == 0)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sYour guild does not own a farm at this location.", GWCOMM[guild_id].color_15.c_str());
									}

									if (GWARZ[LocId].farm_count > 0)
									{
										if (GWARZ[LocId].pig_count > ((GWCOMM[SERVER_GUILD_ID].pig_L) * (GWARZ[LocId].farm_count - 1)))
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must sell |r %u%s pigs  before removing there housing.|r", GWCOMM[guild_id].color_15.c_str(), (GWARZ[LocId].pig_count - (GWCOMM[SERVER_GUILD_ID].pig_L * (GWARZ[LocId].farm_count - 1))), GWCOMM[guild_id].color_15.c_str());
										};

										if (GWARZ[LocId].pig_count <= ((GWCOMM[SERVER_GUILD_ID].pig_L) * (GWARZ[LocId].farm_count - 1)))
										{
											GameObject *go = player->FindNearestGameObject(GWCOMM[SERVER_GUILD_ID].farm_id, 5);

											if (!go)
											{
												go = player->FindNearestGameObject(GWCOMM[SERVER_GUILD_ID].farm_id + 1, 5);
											}

											if (!go)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%sYou must stand closer to a farm.|r", GWCOMM[guild_id].color_15.c_str());
											}

											if (go)
											{
												if (player->AddItem(GWCOMM[SERVER_GUILD_ID].currency, GWCOMM[SERVER_GUILD_ID].farm_cost))
												{
													go->RemoveFromWorld();
													go->DeleteFromDB();

													UpdateGuildLocData("farm_count", ConvertNumberToString(GWARZ[LocId].farm_count - 1), LocId);

													std::string ann = pName + " has sold a farm from location:" + str_LocId + ".";

													SendGuildMessage(GWARZ[LocId].guild_id, ann);

													ChatHandler(player->GetSession()).PSendSysMessage("+%u %s's.|r", GWCOMM[SERVER_GUILD_ID].farm_cost, Currencyname.c_str());
													msg = GGW_MSG;
												} // can add currency
											}
										} // enough pigs
									} // has farms
								} // farm

	/*
	-- **** Barrack ****
	*/

								if (ChatCache[2] == GWCOMM[guild_id].barrack)
								{
									if (GWARZ[LocId].barrack_count == 0)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sYour guild does not own a barrack at this location.", GWCOMM[guild_id].color_15.c_str());
									}

									if (GWARZ[LocId].barrack_count > 0)
									{
										if (GWARZ[LocId].guard_count > ((GWCOMM[SERVER_GUILD_ID].guard_L) * (GWARZ[LocId].barrack_count - 1)))
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must sell|r %u %sguards before removing there Barrack's.|r", GWCOMM[guild_id].color_15.c_str(), GWARZ[LocId].guard_count - (GWCOMM[SERVER_GUILD_ID].guard_L * (GWARZ[LocId].barrack_count - 1)), GWCOMM[guild_id].color_15.c_str());
										}

										if (GWARZ[LocId].guard_count <= ((GWCOMM[SERVER_GUILD_ID].guard_L) * (GWARZ[LocId].barrack_count - 1)))
										{
											GameObject *go = player->FindNearestGameObject(GWCOMM[SERVER_GUILD_ID].barrack_id, 5);

											if (!go)
											{
												go = player->FindNearestGameObject(GWCOMM[SERVER_GUILD_ID].barrack_id + 1, 5);
											}

											if (!go)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%sYou must stand closer to a barrack.|r", GWCOMM[guild_id].color_15.c_str());
											}

											if (go)
											{
												if (player->AddItem(GWCOMM[SERVER_GUILD_ID].currency, GWCOMM[SERVER_GUILD_ID].barrack_cost))
												{
													go->ToGameObject()->RemoveFromWorld();
													go->ToGameObject()->DeleteFromDB();

													UpdateGuildLocData("barrack_count", ConvertNumberToString(GWARZ[LocId].barrack_count - 1), LocId);

													std::string ann = pName + " has sold a barrack from location:" + str_LocId + ".";

													SendGuildMessage(GWARZ[LocId].guild_id, ann);

													ChatHandler(player->GetSession()).PSendSysMessage("+%u %s's.|r", GWCOMM[SERVER_GUILD_ID].barrack_cost, Currencyname.c_str());
													msg = GGW_MSG;
												} // can add currency
											}
										} // enough guards
									} // has barracks
								} // barrack

	/*
	-- **** Hall ****
	*/

								if (ChatCache[2] == GWCOMM[guild_id].hall)
								{
									if (GWARZ[LocId].hall_count == 0)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sYour guild does not own a hall at this location.", GWCOMM[guild_id].color_15.c_str());
									}

									if (GWARZ[LocId].hall_count > 0)
									{
										if (GWARZ[LocId].vendor1_count > 0)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must sell off your vendor1's before removing a Hall.|r", GWCOMM[guild_id].color_15.c_str());
										}
										if (GWARZ[LocId].vendor2_count > 0)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must sell off your vendor2's before removing a Hall.|r", GWCOMM[guild_id].color_15.c_str());
										}
										if (GWARZ[LocId].vendor3_count > 0)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must sell off your vendor3's before removing a Hall.|r", GWCOMM[guild_id].color_15.c_str());
										}
										if (GWARZ[LocId].cannon_count > 0)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must sell off your cannon's before removing a Hall.|r", GWCOMM[guild_id].color_15.c_str());
										}
										if (GWARZ[LocId].vault_count > 0)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must sell off your vault's before removing a Hall.|r", GWCOMM[guild_id].color_15.c_str());
										}
										if (GWARZ[LocId].mailbox_count > 0)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must sell off your mailbox's before removing a Hall.|r", GWCOMM[guild_id].color_15.c_str());
										}

										if ((GWARZ[LocId].vendor1_count == 0) && (GWARZ[LocId].vendor2_count == 0) && (GWARZ[LocId].vendor3_count == 0) && (GWARZ[LocId].cannon_count == 0) && (GWARZ[LocId].vault_count == 0) && (GWARZ[LocId].mailbox_count == 0))

										{
											GameObject *go = player->FindNearestGameObject(GWCOMM[SERVER_GUILD_ID].hall_id, 10.0);

											if (!go)
											{
												go = player->FindNearestGameObject(GWCOMM[SERVER_GUILD_ID].hall_id + 1, 10.0);
											}

											if (!go)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%sYou must stand closer to your hall.|r", GWCOMM[guild_id].color_15.c_str());
											}

											if (go)
											{
												if (player->AddItem(GWCOMM[SERVER_GUILD_ID].currency, GWCOMM[SERVER_GUILD_ID].hall_cost))
												{
													go->ToGameObject()->RemoveFromWorld();
													go->ToGameObject()->DeleteFromDB();

													UpdateGuildLocData("hall_count", ConvertNumberToString(GWARZ[LocId].hall_count - 1), LocId);

													std::string ann = pName + " has sold a hall from location:" + str_LocId + ".";

													SendGuildMessage(GWARZ[LocId].guild_id, ann);

													ChatHandler(player->GetSession()).PSendSysMessage("+%u %s's.|r", GWCOMM[SERVER_GUILD_ID].hall_cost, Currencyname.c_str());
													msg = GGW_MSG;
												} // can add currency
											} // found go
										} // find go
									} // has hall
								} // hall

	/*
	-- **** Pig ****
	*/

								if (ChatCache[2] == GWCOMM[guild_id].pig)
								{
									if (GWARZ[LocId].pig_count == 0)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sYou DONT have any pigs in this area.", GWCOMM[guild_id].color_15.c_str());
									}

									if (GWARZ[LocId].pig_count > 0)
									{
										Unit *unit = player->GetSelectedUnit();

										if (!unit)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must select a pig.|r", GWCOMM[guild_id].color_15.c_str());
										}

										if (unit)
										{
											if (unit->GetTypeId() != 3)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%sYou must select a pig.|r", GWCOMM[guild_id].color_15.c_str());
											}

											if (unit->GetTypeId() == 3)
											{
												if (unit->GetEntry() == GWCOMM[SERVER_GUILD_ID].pig_id || unit->GetEntry() == GWCOMM[SERVER_GUILD_ID].pig_id + 1)
												{
													if (player->AddItem(GWCOMM[SERVER_GUILD_ID].currency, GWCOMM[SERVER_GUILD_ID].pig_cost))
													{
														unit->RemoveFromWorld();
														unit->ToCreature()->DeleteFromDB();

														UpdateGuildLocData("pig_count", ConvertNumberToString(GWARZ[LocId].pig_count - 1), LocId);

														std::string ann = pName + " has sold a pig from location:" + str_LocId + " to the butcher shop.";

														SendGuildMessage(GWARZ[LocId].guild_id, ann);

														ChatHandler(player->GetSession()).PSendSysMessage("+%u %s's.|r", GWCOMM[SERVER_GUILD_ID].pig_cost, Currencyname.c_str());
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

								if (ChatCache[2] == GWCOMM[guild_id].guard)
								{
									if (GWARZ[LocId].guard_count == 0)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sYou DONT have any guard's in this area.", GWCOMM[guild_id].color_15.c_str());
									}

									if (GWARZ[LocId].guard_count > 0)
									{
										Unit *unit = player->GetSelectedUnit();

										if (!unit)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must select a guard.|r", GWCOMM[guild_id].color_15.c_str());
										}

										if (unit)
										{
											if (unit->GetTypeId() != 3)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%sYou must select a guard.|r", GWCOMM[guild_id].color_15.c_str());
											}

											if (unit->GetTypeId() == 3)
											{
												if (unit->GetEntry() == GWCOMM[SERVER_GUILD_ID].guard_id || unit->GetEntry() == GWCOMM[SERVER_GUILD_ID].guard_id + 1)
												{
													unit->RemoveFromWorld();
													unit->ToCreature()->DeleteFromDB();

													UpdateGuildLocData("guard_count", ConvertNumberToString(GWARZ[LocId].guard_count - 1), LocId);

													std::string ann = pName + " has released a guard from location:" + str_LocId + ".";

													SendGuildMessage(GWARZ[LocId].guild_id, ann);

													ChatHandler(player->GetSession()).PSendSysMessage("%sGuards are disposable and dont return %s's.|r", GWCOMM[guild_id].color_14.c_str(), Currencyname.c_str());
													ChatHandler(player->GetSession()).PSendSysMessage("%s%s Has released a guard.|r", GWCOMM[guild_id].color_14.c_str(), pName.c_str());
													msg = GGW_MSG;
												} // is guard
											} // is npc
										} // found npc
									} // has guard
								} //guard


	/*
	-- **** Vendor 1 ****
	*/

								if (ChatCache[2] == GWCOMM[guild_id].vendor1)
								{
									if (GWARZ[LocId].vendor1_count == 0)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sYou DONT have any vendor1's in this area.", GWCOMM[guild_id].color_15.c_str());
									}

									if (GWARZ[LocId].vendor1_count > 0)
									{
										Unit *unit = player->GetSelectedUnit();

										if (!unit)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must select a vendor1.|r", GWCOMM[guild_id].color_15.c_str());
										}

										if (unit)
										{
											if (unit->GetTypeId() != 3)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%sYou must select a vendor1.|r", GWCOMM[guild_id].color_15.c_str());
											}

											if (unit->GetTypeId() == 3)
											{
												if (unit->GetEntry() == GWCOMM[SERVER_GUILD_ID].vendor1_id || unit->GetEntry() == GWCOMM[SERVER_GUILD_ID].vendor1_id + 1)
												{
													if (player->AddItem(GWCOMM[SERVER_GUILD_ID].currency, GWCOMM[SERVER_GUILD_ID].vendor1_cost))
													{
														unit->RemoveFromWorld();
														unit->ToCreature()->DeleteFromDB();

														UpdateGuildLocData("vendor1_count", ConvertNumberToString(GWARZ[LocId].vendor1_count - 1), LocId);

														std::string ann = pName + " has sold a vendor1 from location:" + str_LocId + ".";

														SendGuildMessage(GWARZ[LocId].guild_id, ann);

														ChatHandler(player->GetSession()).PSendSysMessage("+%u %s's.|r", GWCOMM[SERVER_GUILD_ID].vendor1_cost, Currencyname.c_str());
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

								if (ChatCache[2] == GWCOMM[guild_id].vendor2)
								{
									if (GWARZ[LocId].vendor2_count == 0)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sYou DONT have any vendor2's in this area.", GWCOMM[guild_id].color_15.c_str());
									}

									if (GWARZ[LocId].vendor2_count > 0)
									{
										Unit *unit = player->GetSelectedUnit();

										if (!unit)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must select a vendor2.|r", GWCOMM[guild_id].color_15.c_str());
										}

										if (unit)
										{
											if (unit->GetTypeId() != 3)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%sYou must select a vendor2.|r", GWCOMM[guild_id].color_15.c_str());
											}

											if (unit->GetTypeId() == 3)
											{
												if (unit->GetEntry() == GWCOMM[SERVER_GUILD_ID].vendor2_id || unit->GetEntry() == GWCOMM[SERVER_GUILD_ID].vendor2_id + 1)
												{
													if (player->AddItem(GWCOMM[SERVER_GUILD_ID].currency, GWCOMM[SERVER_GUILD_ID].vendor2_cost))
													{
														unit->RemoveFromWorld();
														unit->ToCreature()->DeleteFromDB();

														UpdateGuildLocData("vendor2_count", ConvertNumberToString(GWARZ[LocId].vendor2_count - 1), LocId);

														std::string ann = pName + " has sold a vendor2 from location:" + str_LocId + ".";

														SendGuildMessage(GWARZ[LocId].guild_id, ann);

														ChatHandler(player->GetSession()).PSendSysMessage("+%u %s's.|r", GWCOMM[SERVER_GUILD_ID].vendor2_cost, Currencyname.c_str());
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

								if (ChatCache[2] == GWCOMM[guild_id].vendor3)
								{
									if (GWARZ[LocId].vendor3_count == 0)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sYou DONT have any vendor3's in this area.", GWCOMM[guild_id].color_15.c_str());
									}

									if (GWARZ[LocId].vendor3_count > 0)
									{
										Unit *unit = player->GetSelectedUnit(); //  FindNearestCreature((GWCOMM[SERVER_GUILD_ID].pig_id || GWCOMM[SERVER_GUILD_ID].pig_id + 1), 0.5);

										if (!unit)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must select a vendor3.|r", GWCOMM[guild_id].color_15.c_str());
										}

										if (unit)
										{
											if (unit->GetTypeId() != 3)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%sYou must select a vendor3.|r", GWCOMM[guild_id].color_15.c_str());
											}

											if (unit->GetTypeId() == 3)
											{
												if (unit->GetEntry() == GWCOMM[SERVER_GUILD_ID].vendor3_id || unit->GetEntry() == GWCOMM[SERVER_GUILD_ID].vendor3_id + 1)
												{
													if (player->AddItem(GWCOMM[SERVER_GUILD_ID].currency, GWCOMM[SERVER_GUILD_ID].vendor3_cost))
													{
														unit->RemoveFromWorld();
														unit->ToCreature()->DeleteFromDB();

														UpdateGuildLocData("vendor3_count", ConvertNumberToString(GWARZ[LocId].vendor3_count - 1), LocId);

														std::string ann = pName + " has sold a vendor3 from location:" + str_LocId + ".";

														SendGuildMessage(GWARZ[LocId].guild_id, ann);

														ChatHandler(player->GetSession()).PSendSysMessage("+%u %s's.|r", GWCOMM[SERVER_GUILD_ID].vendor3_cost, Currencyname.c_str());
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

								if (ChatCache[2] == GWCOMM[guild_id].cannon)
								{
									if (GWARZ[LocId].cannon_count == 0)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sYou DONT have any cannon's in this area.", GWCOMM[guild_id].color_15.c_str());
									}

									if (GWARZ[LocId].cannon_count > 0)
									{
										Unit *unit = player->GetSelectedUnit();

										if (!unit)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must select a cannon.|r", GWCOMM[guild_id].color_15.c_str());
										}

										if (unit)
										{
											if (unit->GetTypeId() != 3)
											{
												ChatHandler(player->GetSession()).PSendSysMessage("%sYou must select a cannon.|r", GWCOMM[guild_id].color_15.c_str());
											}

											if (unit->GetTypeId() == 3)
											{
												if (unit->GetEntry() == GWCOMM[SERVER_GUILD_ID].cannon_id || unit->GetEntry() == GWCOMM[SERVER_GUILD_ID].cannon_id + 1)
												{
													if (player->AddItem(GWCOMM[SERVER_GUILD_ID].currency, GWCOMM[SERVER_GUILD_ID].cannon_cost))
													{
														unit->RemoveFromWorld();
														unit->ToCreature()->DeleteFromDB();

														UpdateGuildLocData("cannon_count", ConvertNumberToString(GWARZ[LocId].cannon_count - 1), LocId);

														std::string ann = pName + " has sold a cannon from location:" + str_LocId + ".";

														SendGuildMessage(GWARZ[LocId].guild_id, ann);

														ChatHandler(player->GetSession()).PSendSysMessage("+%u %s's.|r", GWCOMM[SERVER_GUILD_ID].cannon_cost, Currencyname.c_str());
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

								if (ChatCache[2] == GWCOMM[guild_id].vault)
								{
									if (GWARZ[LocId].vault_count == 0)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sYour guild does not own a vault at this location.", GWCOMM[guild_id].color_15.c_str());
									}

									if (GWARZ[LocId].vault_count > 0)
									{
										GameObject *go = player->FindNearestGameObject(GWCOMM[SERVER_GUILD_ID].vault_id, 5);

										if (!go)
										{
											go = player->FindNearestGameObject(GWCOMM[SERVER_GUILD_ID].vault_id + 1, 5);
										}

										if (!go)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must stand closer to a vault.|r", GWCOMM[guild_id].color_15.c_str());
										}

										if (go)
										{
											if (player->AddItem(GWCOMM[SERVER_GUILD_ID].currency, GWCOMM[SERVER_GUILD_ID].vault_cost))
											{
												go->ToGameObject()->RemoveFromWorld();
												go->ToGameObject()->DeleteFromDB();

												UpdateGuildLocData("vault_count", ConvertNumberToString(GWARZ[LocId].vault_count - 1), LocId);

												std::string ann = pName + " has sold a vault from location:" + str_LocId + ".";

												SendGuildMessage(GWARZ[LocId].guild_id, ann);

												ChatHandler(player->GetSession()).PSendSysMessage("+%u %s's.|r", GWCOMM[SERVER_GUILD_ID].vault_cost, Currencyname.c_str());
												msg = GGW_MSG;
											} // can add currency
										}
									} // has vault
								} // vault

	/*
	-- **** MailBox ****
	*/

								if (ChatCache[2] == GWCOMM[guild_id].mailbox)
								{
									if (GWARZ[LocId].mailbox_count == 0)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%sYour guild does not own a mailbox at this location.", GWCOMM[guild_id].color_15.c_str());
									}

									if (GWARZ[LocId].mailbox_count > 0)
									{
										GameObject *go = player->FindNearestGameObject(GWCOMM[SERVER_GUILD_ID].mailbox_id, 5);

										if (!go)
										{
											go = player->FindNearestGameObject(GWCOMM[SERVER_GUILD_ID].mailbox_id + 1, 5);
										}

										if (!go)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sYou must stand closer to a mailbox.|r", GWCOMM[guild_id].color_15.c_str());
										}

										if (go)
										{
											if (player->AddItem(GWCOMM[SERVER_GUILD_ID].currency, GWCOMM[SERVER_GUILD_ID].mailbox_cost))
											{
												go->ToGameObject()->RemoveFromWorld();
												go->ToGameObject()->DeleteFromDB();

												UpdateGuildLocData("mailbox_count", ConvertNumberToString(GWARZ[LocId].mailbox_count - 1), LocId);

												std::string ann = pName + " has sold a mailbox from location:" + str_LocId + ".";

												SendGuildMessage(GWARZ[LocId].guild_id, ann);

												ChatHandler(player->GetSession()).PSendSysMessage("+%u %s's.|r", GWCOMM[SERVER_GUILD_ID].mailbox_cost, Currencyname.c_str());
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
							ChatHandler(player->GetSession()).PSendSysMessage("<%sGrumbo|r>:%sSpeak With Your faction Leader to gain access to ownership and development.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_1.c_str());
						else 
							ChatHandler(player->GetSession()).PSendSysMessage("<%sGrumbo|r>:%sRemind your Guild Leader to Speak With Your faction Leader to gain access to ownership and development.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_1.c_str());
					}
					msg = GGW_MSG;
				} // is sell command
/*
-- ****************************************************
-- **************** Game Master Commands **************
-- ****************************************************
*/
				if (pGMRank >= GWCOMM[SERVER_GUILD_ID].GM_minimum && player->IsGameMaster())
				{

/*
-- **** Loc Command ****
*/

					if ((ChatCache[1] == "lock") && (ChatCache[2] == GWCOMM[SERVER_GUILD_ID].loc))
					{
						UpdateGuildLocData("guild_name", SERVER_GUILD_NAME, LocId);
						UpdateGuildLocData("guild_id", ConvertNumberToString(SERVER_GUILD_ID), LocId);
						UpdateGuildLocData("team", ConvertNumberToString(SERVER_GUILD_TEAM_LOCKED_ID), LocId);
						UpdateGuildLocData("flag_id", "0", LocId);
						UpdateGuildLocData("fs_time", "0", LocId);

						ChatHandler(player->GetSession()).PSendSysMessage("%sLocation:%u succesfully|r %sLOCKED|r.", GWCOMM[SERVER_GUILD_ID].color_14.c_str(), GWARZ[LocId].entry, GWCOMM[guild_id].color_7.c_str());
						msg = GGW_MSG;
					};

/*
-- **** Reset Command ****
*/

					if (ChatCache[1] == "reset")
					{
						if (ChatCache[2] == GWCOMM[SERVER_GUILD_ID].loc)
						{
							UpdateGuildLocData("guild_name", SERVER_GUILD_NAME, LocId);
							UpdateGuildLocData("guild_id", ConvertNumberToString(SERVER_GUILD_ID), LocId);
							UpdateGuildLocData("team", ConvertNumberToString(SERVER_GUILD_TEAM), LocId);
							UpdateGuildLocData("flag_id", "0", LocId);
							UpdateGuildLocData("fs_time", "0", LocId);

							ChatHandler(player->GetSession()).PSendSysMessage("%sLocation:%u succesfully reset.|r", GWCOMM[guild_id].color_14.c_str(), GWARZ[LocId].entry);
						}

						if (ChatCache[2] == GWCOMM[SERVER_GUILD_ID].farm)
						{
							UpdateGuildLocData("farm_count", "0", LocId);

							ChatHandler(player->GetSession()).PSendSysMessage("%sLocation:%u farm count reset.|r", GWCOMM[guild_id].color_14.c_str(), GWARZ[LocId].entry);
						}

						if (ChatCache[2] == GWCOMM[SERVER_GUILD_ID].barrack)
						{
							UpdateGuildLocData("barrack_count", "0", LocId);

							ChatHandler(player->GetSession()).PSendSysMessage("%sLocation:%u barrack count reset.|r", GWCOMM[guild_id].color_14.c_str(), GWARZ[LocId].entry);
						}

						if (ChatCache[2] == GWCOMM[SERVER_GUILD_ID].hall)
						{
							UpdateGuildLocData("hall_count", "0", LocId);

							ChatHandler(player->GetSession()).PSendSysMessage("%sArea:%u hall count reset.|r", GWCOMM[guild_id].color_14.c_str(), GWARZ[LocId].entry);
						}

						if (ChatCache[2] == GWCOMM[SERVER_GUILD_ID].pig)
						{
							UpdateGuildLocData("pig_count", "0", LocId);

							ChatHandler(player->GetSession()).PSendSysMessage("%sArea:%u pig count reset.|r", GWCOMM[guild_id].color_14.c_str(), GWARZ[LocId].entry);
						}

						if (ChatCache[2] == GWCOMM[SERVER_GUILD_ID].guard)
						{
							UpdateGuildLocData("guard_count", "0", LocId);

							ChatHandler(player->GetSession()).PSendSysMessage("%sArea:%u guard count reset.|r", GWCOMM[guild_id].color_14.c_str(), GWARZ[LocId].entry);
						}

						if (ChatCache[2] == GWCOMM[SERVER_GUILD_ID].vendor1)
						{
							UpdateGuildLocData("vendor1_count", "0", LocId);

							ChatHandler(player->GetSession()).PSendSysMessage("%sArea:%u vendor1 count reset.|r", GWCOMM[guild_id].color_14.c_str(), GWARZ[LocId].entry);
						}

						if (ChatCache[2] == GWCOMM[SERVER_GUILD_ID].vendor2)
						{
							UpdateGuildLocData("vendor2_count", "0", LocId);

							ChatHandler(player->GetSession()).PSendSysMessage("%sArea:%u vendor2 count reset.|r", GWCOMM[guild_id].color_14.c_str(), GWARZ[LocId].entry);
						}

						if (ChatCache[2] == GWCOMM[SERVER_GUILD_ID].vendor3)
						{
							UpdateGuildLocData("vendor3_count", "0", LocId);
							ChatHandler(player->GetSession()).PSendSysMessage("%sArea:%u vendor3 count reset.|r", GWCOMM[guild_id].color_14.c_str(), GWARZ[LocId].entry);
						}

						if (ChatCache[2] == GWCOMM[SERVER_GUILD_ID].cannon)
						{
							UpdateGuildLocData("cannon_count", "0", LocId);

							ChatHandler(player->GetSession()).PSendSysMessage("%sArea:%u cannon count reset.|r", GWCOMM[guild_id].color_14.c_str(), GWARZ[LocId].entry);
						}

						if (ChatCache[2] == GWCOMM[SERVER_GUILD_ID].vault)
						{
							UpdateGuildLocData("vault_count", "0", LocId);

							ChatHandler(player->GetSession()).PSendSysMessage("%sArea:%u vault count reset.|r", GWCOMM[guild_id].color_14.c_str(), GWARZ[LocId].entry);
						}

						if (ChatCache[2] == GWCOMM[SERVER_GUILD_ID].mailbox)
						{
							UpdateGuildLocData("mailbox_count", "0", LocId);

							ChatHandler(player->GetSession()).PSendSysMessage("%sArea:%u mailbox count reset.|r", GWCOMM[guild_id].color_14.c_str(), GWARZ[LocId].entry);
						}

						msg = GGW_MSG;
					} // reset commands

/*
-- **** Reload Command ****
*/

					if (ChatCache[1] == "reload")
					{
						if (ChatCache[2] == GWCOMM[SERVER_GUILD_ID].table)
						{
							GWCOMM.erase(AllElements);
							GWHELP.erase(AllElements);
							GWARZ.erase(AllElements);

							LoadCommands();
							LoadHelp();
							LoadLoc();

							ChatHandler(player->GetSession()).PSendSysMessage("%sALL Grumbo'z Guild Warz Tables are Reloaded.|r", GWCOMM[guild_id].color_14.c_str());
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
							if (GWARZ[LocId].team >= SERVER_GUILD_TEAM)
							{
								ChatHandler(player->GetSession()).PSendSysMessage("%sThe Server controls This Land. !! NO NEW FLAG SPAWNED !!|r", GWCOMM[guild_id].color_15.c_str());
							}

							if (GWARZ[LocId].team < SERVER_GUILD_TEAM)
							{
								uint32 spawnflag = SpawnGuildObjects(2, GWCOMM[SERVER_GUILD_ID].flag_id, pTeam_id, guild_id, Pmap, pX, pY, pZ, pO, player, LocId);

								if (!spawnflag)
								{
									ChatHandler(player->GetSession()).PSendSysMessage("%sflag spawn error..|r", GWCOMM[guild_id].color_15);
								}

								if (spawnflag)
								{
									UpdateGuildLocFloat(pX, pY, pZ, LocId);

									UpdateGuildLocData("flag_id", ConvertNumberToString(spawnflag), LocId);
									UpdateGuildLocData("fs_time", ConvertNumberToString(sWorld->GetGameTime()), LocId);

									ChatHandler(player->GetSession()).PSendSysMessage("%sNew flag spawned for Guild Warz location:%u|r", GWCOMM[guild_id].color_14.c_str(), GWARZ[LocId].entry);
									msg = GGW_MSG;
								} // is spawned
							} // team check

						} // flag

						if (ChatCache[2] != "flag")
						{
							ChatHandler(player->GetSession()).PSendSysMessage("%sCMD ERROR:|r %s %s", GWCOMM[guild_id].color_15.c_str(), ChatCache[1], ChatCache[2]);
						}
					} // spawn

/*
-- **** Loc Details Command ****
*/

					if (ChatCache[1] == GWCOMM[SERVER_GUILD_ID].details_loc)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%s*************************************", GWCOMM[guild_id].color_9.c_str());
						ChatHandler(player->GetSession()).PSendSysMessage("%sLocation details:", GWCOMM[guild_id].color_8.c_str());
						ChatHandler(player->GetSession()).PSendSysMessage("%sLocation ID:|r %s%u|r.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWARZ[LocId].entry);

						if (GWARZ[LocId].team == 0) { ChatHandler(player->GetSession()).PSendSysMessage("%sGuild Name|r: %s%s|r.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_4.c_str(), GWARZ[LocId].guild_name.c_str()); };
						if (GWARZ[LocId].team == 1) { ChatHandler(player->GetSession()).PSendSysMessage("%sGuild Name|r: %s%s|r.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_5.c_str(), GWARZ[LocId].guild_name.c_str()); };
						if (GWARZ[LocId].team == 2) { ChatHandler(player->GetSession()).PSendSysMessage("%sGuild Name|r: %s%s|r.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_6.c_str(), GWARZ[LocId].guild_name.c_str()); };
						if (GWARZ[LocId].team == 3) { ChatHandler(player->GetSession()).PSendSysMessage("%sGuild Name|r: %s%s|r.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_7.c_str(), GWARZ[LocId].guild_name.c_str()); };

						if (GWARZ[LocId].team == 0) { ChatHandler(player->GetSession()).PSendSysMessage("%sFaction|r:%sAlliance.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_4.c_str()); };
						if (GWARZ[LocId].team == 1) { ChatHandler(player->GetSession()).PSendSysMessage("%sFaction|r:%sHorde.   |r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_5.c_str()); };
						if (GWARZ[LocId].team == 2) { ChatHandler(player->GetSession()).PSendSysMessage("%sFaction|r:%sFor Sale.|r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_6.c_str()); };
						if (GWARZ[LocId].team == 3) { ChatHandler(player->GetSession()).PSendSysMessage("%sFaction|r:%sLOCKED.  |r", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_7.c_str()); };

						ChatHandler(player->GetSession()).PSendSysMessage("%sTeam:|r%s%u|r.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWARZ[LocId].team);
						ChatHandler(player->GetSession()).PSendSysMessage("%sFarm count|r:%s%u|r.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWARZ[LocId].farm_count);
						ChatHandler(player->GetSession()).PSendSysMessage("%sBarrack count|r:%s%u|r.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWARZ[LocId].barrack_count);
						ChatHandler(player->GetSession()).PSendSysMessage("%sHall count|r:%s%u|r.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWARZ[LocId].hall_count);
						ChatHandler(player->GetSession()).PSendSysMessage("%sPig count|r:%s%u|r.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWARZ[LocId].pig_count);
						ChatHandler(player->GetSession()).PSendSysMessage("%sguard count|r:%s%u|r.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWARZ[LocId].guard_count);
						ChatHandler(player->GetSession()).PSendSysMessage("%scannon count|r:%s%u|r.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWARZ[LocId].cannon_count);

						if (GWCOMM[SERVER_GUILD_ID].vendor1_id > 0){ ChatHandler(player->GetSession()).PSendSysMessage("%svendor1 count|r:%s%u|r.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWARZ[LocId].vendor1_count); };
						if (GWCOMM[SERVER_GUILD_ID].vendor2_id > 0){ ChatHandler(player->GetSession()).PSendSysMessage("%svendor2 count|r:%s%u|r.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWARZ[LocId].vendor2_count); };
						if (GWCOMM[SERVER_GUILD_ID].vendor3_id > 0){ ChatHandler(player->GetSession()).PSendSysMessage("%svendor3 count|r:%s%u|r.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWARZ[LocId].vendor3_count); };

						ChatHandler(player->GetSession()).PSendSysMessage("%svault count:|r%s%u|r.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWARZ[LocId].vault_count);
						ChatHandler(player->GetSession()).PSendSysMessage("%smailbox count:|r%s%u|r.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWARZ[LocId].mailbox_count);
						ChatHandler(player->GetSession()).PSendSysMessage("%sflag GUIDLow:|r%s%u|r.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWARZ[LocId].flag_id);
						ChatHandler(player->GetSession()).PSendSysMessage("%sflag spawn time:|r%s%u|r.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWARZ[LocId].fs_time);
						ChatHandler(player->GetSession()).PSendSysMessage("%sGuild ID:|r%s%u|r.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), GWARZ[LocId].guild_id);
						ChatHandler(player->GetSession()).PSendSysMessage("%sGame Time|r:%s%u|r.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_2.c_str(), sWorld->GetGameTime());
						ChatHandler(player->GetSession()).PSendSysMessage("%s*************************************|r", GWCOMM[guild_id].color_9.c_str());
						msg = GGW_MSG;
					}
				} // is minimum GM
/*
-- ****************************************************
-- ********** Guild Warz teleporter system ************
-- ** a mild mutation of Grandelf1's guild teleporter *
-- ****************************************************
*/
				if (ChatCache[1] == GWCOMM[guild_id].tele)
				{
					if (ChatCache[2].c_str())
					{
						uint32 loc;

						std::istringstream(ChatCache[2]) >> loc;

						if (loc > 1 && loc <= GWARZ.size())
						{
							if (GWARZ[loc].guild_id == guild_id || player->IsGameMaster())
							{
								player->TeleportTo(uint32(GWARZ[loc].map_id), GWARZ[loc].x, GWARZ[loc].y, GWARZ[loc].z, 0.0);

								ChatHandler(player->GetSession()).PSendSysMessage("%sDeadByDawn says:|r%sTeleport to location %u complete.|r", GWCOMM[guild_id].color_2.c_str(), GWCOMM[guild_id].color_1.c_str(), loc);
								msg = GGW_MSG;
							}

							if (GWARZ[loc].guild_id != guild_id && !player->IsGameMaster())
							{
								ChatHandler(player->GetSession()).PSendSysMessage("Raider says:%sYour guild doesn't own that area.", GWCOMM[guild_id].color_15.c_str());
								ChatHandler(player->GetSession()).PSendSysMessage("Raider says:%sYou cannot teleport there.", GWCOMM[guild_id].color_15.c_str());
								msg = GGW_MSG;
							}
						}

						if (loc > GWARZ.size() || loc == 1)
						{
							ChatHandler(player->GetSession()).PSendSysMessage("Grumbo says:%sLocation %u doesn't exsist.|r", GWCOMM[guild_id].color_15.c_str(), loc);
							ChatHandler(player->GetSession()).PSendSysMessage("Grumbo says:%sTry a different id.|r", GWCOMM[guild_id].color_15.c_str());
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

		bool OnGossipHello(Player* player, GameObject* go) override // virtual
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

			uint32 LocId = GetLocationID(map_id, area_id, zone_id);

				if (!LocId)
				{
					LocId = CreateGuildLocation(map_id, area_id, zone_id, pX, pY, pZ);
				}

			uint32 loc_guild_id = GWARZ[LocId].guild_id;
			std::string loc_guild_name = GWARZ[LocId].guild_name;
			uint32 loc_guild_team = GWARZ[LocId].team;

			std::string str_LocId;
			std::stringstream convert;
			convert << LocId;
			str_LocId = convert.str();

			uint64 curr_time = sWorld->GetGameTime();
			uint64 second = 1000;
			uint64 minute = 60000;
			uint64 spawn_cooldown_timer = GWCOMM[SERVER_GUILD_ID].s_timer / second;
			uint64 total_flag_spawn_time = (GWARZ[LocId].fs_time + spawn_cooldown_timer);

			uint32 gGuid = go->GetSpawnId();

			if (test) { ChatHandler(player->GetSession()).PSendSysMessage("FLAG_TAG_EVENT triggered"); };

			if (gGuid != GWARZ[LocId].flag_id)
			{
				go->RemoveFromWorld();
				go->DeleteFromDB();

				ChatHandler(player->GetSession()).PSendSysMessage("%serror.... Phantom flag removed.|r", GWCOMM[SERVER_GUILD_ID].color_15.c_str());
				return true;
			}

			if (player->IsGameMaster())
			{
				ChatHandler(player->GetSession()).PSendSysMessage("%sYou are in GM mode. Exit GM mode to enjoy.|r", GWCOMM[SERVER_GUILD_ID].color_15.c_str());
				return true;
			}
			else
			{
				if (guild)
				{
						if (test){ TC_LOG_INFO("server.loading", "GUILD:true"); };

					uint32 guild_id = player->GetGuildId();
					std::string guild_name = guild->GetName();
					uint8 pGuildRank = player->GetRank();
					uint8 pGMRank = player->GetSession()->GetSecurity();
					uint32 GuildLeaderGUID = guild->GetLeaderGUID();

					if (guild_name != GWCOMM[guild_id].guild)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("<Grumbo>:CREATING Your Guild Warz Commands for Guild %s.", guild_name.c_str());
						CreateGuildCommands(guild_name, guild_id);
					}
					
					if (GWCOMM[guild_id].allowed >= 1)
						{
							if (guild_id == loc_guild_id || (GWCOMM[SERVER_GUILD_ID].anarchy == 0 && pTeam_id == GWARZ[LocId].team))
							{

								if (GWARZ[LocId].team == 0) ChatHandler(player->GetSession()).PSendSysMessage("%s%s|r own's this location.", GWCOMM[SERVER_GUILD_ID].color_4.c_str(), GWARZ[LocId].guild_name.c_str());
								if (GWARZ[LocId].team == 1) ChatHandler(player->GetSession()).PSendSysMessage("%s%s|r own's this location.", GWCOMM[guild_id].color_5.c_str(), GWARZ[LocId].guild_name.c_str());

								ChatHandler(player->GetSession()).PSendSysMessage("%sGrumbo'z Guild Warz System.|r", GWCOMM[SERVER_GUILD_ID].color_10.c_str());
								return true;
							}

							if (pTeam_id != GWARZ[LocId].team || (GWCOMM[SERVER_GUILD_ID].anarchy == 1 && guild_id != loc_guild_id))
							{
								if (total_flag_spawn_time > curr_time && GWCOMM[SERVER_GUILD_ID].f_timer == 1)
								{
									ChatHandler(player->GetSession()).PSendSysMessage("%s!!..Cooldown Timer in Affect..!!|r", GWCOMM[SERVER_GUILD_ID].color_15.c_str());
									return true;
								}

								if (total_flag_spawn_time <= curr_time || GWCOMM[SERVER_GUILD_ID].f_timer == 0)
								{
									if (GWCOMM[SERVER_GUILD_ID].flag_require == 1 && GWARZ[LocId].guard_count > 0)
									{
										ChatHandler(player->GetSession()).PSendSysMessage("%s!!..You must clear ALL guards..!!|r", GWCOMM[SERVER_GUILD_ID].color_15.c_str());
										return true;
									}

									if (GWARZ[LocId].guard_count == 0 || GWCOMM[SERVER_GUILD_ID].flag_require == 0)
									{

										uint32 spawnflag = SpawnGuildObjects(2, GWCOMM[SERVER_GUILD_ID].flag_id, pTeam_id, guild_id, Pmap, pX, pY, pZ, pO, player, LocId);

										if (!spawnflag)
										{
											ChatHandler(player->GetSession()).PSendSysMessage("%sflag spawn error..|r", GWCOMM[guild_id].color_15.c_str());
										}

										if (spawnflag)
										{
											go->RemoveFromWorld();
											go->DeleteFromDB();

											UpdateGuildLocData("guild_name", guild_name, LocId);
											UpdateGuildLocData("guild_id", ConvertNumberToString(guild_id), LocId);
											UpdateGuildLocData("team", ConvertNumberToString(pTeam_id), LocId);

											UpdateGuildLocData("flag_id", ConvertNumberToString(spawnflag), LocId);
											UpdateGuildLocData("fs_time", ConvertNumberToString(sWorld->GetGameTime()), LocId);

											UpdateGuildLocFloat(pX, pY, pZ, LocId);

											std::string ann = pName + " has taken location:" + str_LocId + " from ";

											if (loc_guild_team == 0) ann += (GWCOMM[SERVER_GUILD_ID].color_4 + loc_guild_name + ".");
											if (loc_guild_team == 1) ann += (GWCOMM[SERVER_GUILD_ID].color_5 + loc_guild_name + ".");

											SendGuildMessage(GWARZ[LocId].guild_id, ann);

											if (GUILDWARZ_RANK_TYPE > 1) { CreateRankList(); };

											return false;
										} // is spawned
									}
								}
							}
						}// is allowed
						else
						{
							if (GuildLeaderGUID == pGuid)
								ChatHandler(player->GetSession()).PSendSysMessage("<%sGrumbo|r>:%sSpeak With Your faction Leader to gain access to ownership and development.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_1.c_str());
							else
								ChatHandler(player->GetSession()).PSendSysMessage("<%sGrumbo|r>:%sRemind your Guild Leader to Speak With Your faction Leader to gain access to ownership and development.", GWCOMM[guild_id].color_1.c_str(), GWCOMM[guild_id].color_1.c_str());
						}
				}
				else
				{
					if (test){ TC_LOG_INFO("server.loading", "GUILD:false"); };

					if(pTeam_id != GWARZ[LocId].team)
					{
						ChatHandler(player->GetSession()).PSendSysMessage("%sGrumbo'z Guild Warz System:|r", GWCOMM[SERVER_GUILD_ID].color_1.c_str());
						ChatHandler(player->GetSession()).PSendSysMessage("%s%s own's this location %s.|r", GWCOMM[SERVER_GUILD_ID].color_1.c_str(), GWARZ[LocId].guild_name.c_str(), pName.c_str());
						ChatHandler(player->GetSession()).PSendSysMessage("%sJoin a Guild to participate in Grumbo'z Guild Warz System.|r", GWCOMM[SERVER_GUILD_ID].color_1.c_str());

						if ((pTeam_id == GWARZ[LocId].team) && (GWCOMM[GWARZ[LocId].guild_id].guild_invite == 0))
						{ 
							ChatHandler(player->GetSession()).PSendSysMessage("%sThis Guild Master has disabled the guild's auto invite system.|r", GWCOMM[SERVER_GUILD_ID].color_11.c_str()); 
						};

						ChatHandler(player->GetSession()).PSendSysMessage("%sBrought to you by Grumbo.|r", GWCOMM[SERVER_GUILD_ID].color_1.c_str());

						if (test) { ChatHandler(player->GetSession()).PSendSysMessage("TAG EVENT 0"); };
						return false;
					}

					if(pTeam_id == GWARZ[LocId].team)
					{
							if (test) { ChatHandler(player->GetSession()).PSendSysMessage("TAG EVENT 1"); };

						std::string join = GWCOMM[SERVER_GUILD_ID].color_9 + "Join the Guild `";
						
							if (GWARZ[LocId].team == 0) join += GWCOMM[SERVER_GUILD_ID].color_4 + GWARZ[LocId].guild_name + GWCOMM[SERVER_GUILD_ID].color_9 + "|r`.";
							if (GWARZ[LocId].team == 1) join += GWCOMM[SERVER_GUILD_ID].color_5 + GWARZ[LocId].guild_name + GWCOMM[SERVER_GUILD_ID].color_9 + "|r`.";

						std::string ranked = GWCOMM[SERVER_GUILD_ID].color_9 + "Ranked #" + ConvertNumberToString(GetRank(GWARZ[LocId].guild_id)) + ".";
						std::string total_locations = GWCOMM[SERVER_GUILD_ID].color_9 + "Control " + ConvertNumberToString(CalculateTotalLocations(GWARZ[LocId].guild_id)) + " Locations.";
						std::string total_value = GWCOMM[SERVER_GUILD_ID].color_9 + "Total wealth of " + ConvertNumberToString(CalculateTotalLocationsValue(GWARZ[LocId].guild_id)) + " " + Currencyname + "'s.";

							if (GWCOMM[loc_guild_id].guild_invite == 1) { AddGossipItemFor(player, 1, join, GOSSIP_SENDER_MAIN, 1); };

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

			uint64 LocId = GetLocationID(map_id, area_id, zone_id);

			std::string str_LocId;
			std::stringstream convert;
			convert << LocId;
			str_LocId = convert.str();

			Guild *guild = sGuildMgr->GetGuildById(GWARZ[LocId].guild_id);
			uint32 guild_id = GWARZ[LocId].guild_id;

			ClearGossipMenuFor(player);

			CloseGossipMenuFor(player);

			switch (actions)
			{
				case 1:
				{
					/* SQLTransaction trans(nullptr); */
					guild->AddMember(/*trans,*/ player->GetGUID(), MAX_GUILD_RANKS);

					std::string ann = GWCOMM[guild_id].color_1 + player->GetName() + " Has Joined the Guild via the 'Guild Warz Guild Invite System'.";

					SendGuildMessage(GWARZ[LocId].guild_id, ann);

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

			uint32 LocId = GetLocationID(map_id, area_id, zone_id);

			if (!LocId)
			{
				LocId = CreateGuildLocation(map_id, area_id, zone_id, pX, pY, pZ);
			}

			if ((GWARZ[LocId].guild_id != player->GetGuildId()) && (GWARZ[LocId].team < 2))
			{
				if ((GWARZ[LocId].team != player->GetTeamId()) || (GWCOMM[SERVER_GUILD_ID].anarchy == 1))
				{
					std::string Pname = player->GetName();
					std::string Pguild_name = player->GetGuild()->GetName();
					uint32 guild_id = player->GetGuild()->GetId();

					std::string ann = GWCOMM[SERVER_GUILD_ID].color_13 + player->GetName() + GWCOMM[SERVER_GUILD_ID].color_15 + " of the Guild " + GWCOMM[SERVER_GUILD_ID].color_13 + "`" + player->GetGuild()->GetName() + "`" + GWCOMM[SERVER_GUILD_ID].color_15 + " has entered location:" + GWCOMM[SERVER_GUILD_ID].color_13 + ConvertNumberToString(LocId) + GWCOMM[SERVER_GUILD_ID].color_15 + ". !Hurry!";

					SendGuildMessage(GWARZ[LocId].guild_id, ann);
				}
			}
		}
	}
};

// ************ Guild Npc/Go actions **************

void UpdateGameObject(GameObject* go, uint32 base_id, uint32 LocId)
{
	uint32 LOC_NPC_ID = (base_id + GWARZ[LocId].team);
	uint32 LOC_FACTION_ID = (84 - GWARZ[LocId].team);

	uint32 id = go->GetEntry();

	if (GWARZ[LocId].team >= SERVER_GUILD_TEAM)
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

		bool OnGossipHello(Player* player, GameObject* go) // This will show first when a player clicks on a GameObject (Gossip)
		{
			TC_LOG_INFO("server.loading", "GUILD_VAULT_TRIGGER_1");
			uint32 GoId = go->GetEntry();
			uint32 map_id = go->GetMapId();
			uint32 area_id = go->GetAreaId();
			uint32 zone_id = go->GetZoneId();

			uint32 LocId = GetLocationID(map_id, area_id, zone_id);

			uint32 lGuild_id = GWARZ[LocId].guild_id;
			uint32 pGuild_id = player->GetGuildId();
			auto TeamId = player->GetTeamId();

			if (GoId != (GWCOMM[SERVER_GUILD_ID].vault_id + GWARZ[LocId].team))
			{
				UpdateGameObject(go, GWCOMM[SERVER_GUILD_ID].vault_id, LocId);

				return false;
			}
			return true;
		}
};

class GGW_GUILD_MAILBOX : public GameObjectScript
{
public: GGW_GUILD_MAILBOX() : GameObjectScript("GGW_GUILD_MAILBOX"){ }
		
		struct GGW_GUILD_MAILBOX_AI : public GameObjectAI
		{
		GGW_GUILD_MAILBOX_AI(GameObject* go) : GameObjectAI(go) { }


		void UpdateAI(uint32 diff) // This will show first when a player clicks on a GameObject (Gossip)
		{
			uint32 map_id = go->GetMapId();
			uint32 area_id = go->GetAreaId();
			uint32 zone_id = go->GetZoneId();

			uint32 LocId = GetLocationID(map_id, area_id, zone_id);

			uint32 GoId = go->GetEntry();

			if (GoId != (GWCOMM[SERVER_GUILD_ID].mailbox_id + GWARZ[LocId].team))
			{
				UpdateGameObject(go, GWCOMM[SERVER_GUILD_ID].mailbox_id, LocId);
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
uint32 LOC_NPC_ID = (base_id + GWARZ[LocId].team);
uint32 LOC_FACTION_ID = (84 - GWARZ[LocId].team);
uint32 LOC_GUILD_ID = GWARZ[LocId].guild_id;

uint32 id = creature->GetEntry();

	if (creature->getFaction() != LOC_FACTION_ID)
	{
		if (GWARZ[LocId].team < SERVER_GUILD_TEAM)
		{
			const CreatureTemplate* mob = sObjectMgr->GetCreatureTemplate(LOC_NPC_ID);

			if (id != LOC_NPC_ID)
			{
				creature->SetEntry(LOC_NPC_ID);
				creature->SetDisplayId(mob->Modelid1);
			}


			if (creature->getFaction() != LOC_FACTION_ID)
			{
				creature->setFaction(LOC_FACTION_ID);
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
				uint32 cFaction = creature->getFaction();

				uint32 map_id = creature->GetMapId();
				uint32 area_id = creature->GetAreaId();
				uint32 zone_id = creature->GetZoneId();

				uint32 LocId = GetLocationID(map_id, area_id, zone_id);
				uint32 guild_id = GWARZ[LocId].guild_id;

				if (unit->ToPlayer())
					{
						Player* player = unit->ToPlayer();
						uint32 pFaction = player->getFaction();
	
						FactionReset(player);

						if ((creature->GetEntry() + GWARZ[LocId].team) != GWCOMM[SERVER_GUILD_ID].pig_id)
						{
							UpdateCreature(creature, GWCOMM[SERVER_GUILD_ID].pig_id, LocId);
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

				uint32 LocId = GetLocationID(map_id, area_id, zone_id);

				uint32 cGuid = creature->GetSpawnId();

				Map* map = creature->GetMap();

				float x, y, z, o;

				x = creature->GetPositionX();
				y = creature->GetPositionY();
				z = creature->GetPositionZ();
				o = creature->GetOrientation();

				if (!GGW_Creature[cGuid].LocId)
				{

					GGW_CreatureData& data = GGW_Creature[cGuid]; // Filling in the needed data to save
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

				if (GGW_Creature[cGuid].LocId != LocId)
				{
					creature->NearTeleportTo(GGW_Creature[cGuid].x, GGW_Creature[cGuid].y, GGW_Creature[cGuid].z, true);
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

				uint32 LocId = GetLocationID(map_id, area_id, zone_id);

//				uint32 Unit_LocId = GetLocationID(unit->GetMapId(), unit->GetAreaId(), unit->GetZoneId());

				if ((creature->GetEntry() + GWARZ[GGW_Creature[cGuid].LocId].team) != GWCOMM[SERVER_GUILD_ID].guard_id)
				{
					UpdateCreature(creature, GWCOMM[SERVER_GUILD_ID].guard_id, GGW_Creature[cGuid].LocId);
				}
		
				if (unit->ToPlayer())
				{
					Player* player = unit->ToPlayer();

					if (GGW_Creature[cGuid].LocId != LocId)
					{
						creature->NearTeleportTo(GGW_Creature[cGuid].x, GGW_Creature[cGuid].y, GGW_Creature[cGuid].z, true);

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

				uint32 LocId = GetLocationID(map_id, area_id, zone_id);

				if (GGW_Creature[cGuid].LocId != LocId)
				{
					creature->NearTeleportTo(GGW_Creature[cGuid].x, GGW_Creature[cGuid].y, GGW_Creature[cGuid].z, true);
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

				uint32 LocId = GetLocationID(map_id, area_id, zone_id);

//				uint32 Unit_LocId = GetLocationID(unit->GetMapId(), unit->GetAreaId(), unit->GetZoneId());

				if (GGW_Creature[cGuid].LocId != LocId)
				{
					creature->NearTeleportTo(GGW_Creature[cGuid].x, GGW_Creature[cGuid].y, GGW_Creature[cGuid].z, false);

					creature->Respawn(true);
					creature->CombatStop(true);
					creature->SetPvP(false);
					creature->ClearInCombat();
				}

				if (unit->ToPlayer())
				{
					Player *player = unit->ToPlayer();

					if (GGW_Creature[cGuid].LocId != LocId)
					{
						creature->ClearInCombat();
						creature->CombatStop(true);
						creature->SetPvP(false);
					}

					if (GGW_Creature[cGuid].LocId == LocId)
					{
						if (player->GetGuild()->GetId() > 0)
						{
							creature->Yell("You're gonna !Die! Scum!", LANG_UNIVERSAL, player);
							creature->SetPvP(true);
							creature->Attack(player, true);
							creature->SetInCombatState(true, player);

							std::string ann = "<" + GWCOMM[GWARZ[LocId].guild_id].color_8 + "Guard" + "|r>" + GWCOMM[SERVER_GUILD_ID].color_15 + "I have engaged " + GWCOMM[SERVER_GUILD_ID].color_13 + player->GetName() + GWCOMM[SERVER_GUILD_ID].color_15 + " of the Guild " + GWCOMM[SERVER_GUILD_ID].color_13 + "`" + player->GetGuild()->GetName() + "`" + GWCOMM[SERVER_GUILD_ID].color_15 + " at location:" + GWCOMM[SERVER_GUILD_ID].color_13 + ConvertNumberToString(LocId) + GWCOMM[SERVER_GUILD_ID].color_15 + ". !Hurry!";

							SendGuildMessage(GWARZ[LocId].guild_id, ann);
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
					unit->ToPlayer()->AddItem(Guard_Died_Drop_Reward, 3);

					uint32 LocId = GGW_Creature[creature->GetSpawnId()].LocId;

					creature->Yell("!You Scum! .. You killed me!", LANG_UNIVERSAL, unit);

					GGW_Creature.erase(creature->GetSpawnId());

					UpdateGuildLocData("guard_count", ConvertNumberToString(GWARZ[LocId].guard_count - 1), LocId);

					creature->RemoveFromWorld();
					creature->DeleteFromDB();
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

		if (vendor1)
		{
			uint32 map_id = creature->GetMapId();
			uint32 area_id = creature->GetAreaId();
			uint32 zone_id = creature->GetZoneId();

			uint32 LocId = GetLocationID(map_id, area_id, zone_id);

			uint32 lGuild_id = GWARZ[LocId].guild_id;
			uint32 pGuild_id = player->GetGuildId();

				if (pGuild_id != lGuild_id)
				{
					CloseGossipMenuFor(player);

					creature->Yell("You're gonna !Die! Scum!", LANG_UNIVERSAL, player);

					return false;
				}
				uint32 int_id = 0;

				for (const auto& buffNames : VENDOR1_BUFF_NAMES)
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
		if (VENDOR1_BUFF_IDS[actions])
		{
			player->AddAura(VENDOR1_BUFF_IDS[actions], player);
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

				uint32 LocId = GetLocationID(map_id, area_id, zone_id);

				if (unit->ToPlayer())
				{
					Player* player = unit->ToPlayer();

					if (unit->ToPlayer()->GetGuildId())
					{
						FactionReset(player);
					}
	
					if ((creature->GetEntry() + GWARZ[LocId].team) != GWCOMM[SERVER_GUILD_ID].vendor1_id)
					{
						UpdateCreature(creature, GWCOMM[SERVER_GUILD_ID].vendor1_id, LocId);
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

		if (vendor2)
		{
			uint32 map_id = creature->GetMapId();
			uint32 area_id = creature->GetAreaId();
			uint32 zone_id = creature->GetZoneId();

			uint32 LocId = GetLocationID(map_id, area_id, zone_id);

			uint32 lGuild_id = GWARZ[LocId].guild_id;
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

			uint32 LocId = GetLocationID(map_id, area_id, zone_id);

			if (unit->ToPlayer())
			{
				Player* player = unit->ToPlayer();

				if (unit->ToPlayer()->GetGuildId())
				{
					FactionReset(player);
				}

				if ((creature->GetEntry() + GWARZ[LocId].team) != GWCOMM[SERVER_GUILD_ID].vendor2_id)
				{
					UpdateCreature(creature, GWCOMM[SERVER_GUILD_ID].vendor2_id, LocId);
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

			if (vendor3)
			{
				uint32 map_id = creature->GetMapId();
				uint32 area_id = creature->GetAreaId();
				uint32 zone_id = creature->GetZoneId();

				uint32 LocId = GetLocationID(map_id, area_id, zone_id);

				uint32 lGuild_id = GWARZ[LocId].guild_id;
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

				uint32 LocId = GetLocationID(map_id, area_id, zone_id);

				if (unit->ToPlayer())
				{
					Player* player = unit->ToPlayer();

					if (unit->ToPlayer()->GetGuildId())
					{
						FactionReset(player);
					}

					if ((creature->GetEntry() + GWARZ[LocId].team) != GWCOMM[SERVER_GUILD_ID].vendor3_id)
					{
						UpdateCreature(creature, GWCOMM[SERVER_GUILD_ID].vendor3_id, LocId);
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

			uint32 LocId = GetLocationID(map_id, area_id, zone_id);

			uint32 lGuild_id = GWARZ[LocId].guild_id;
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

			uint32 LocId = GetLocationID(map_id, area_id, zone_id);

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

					if ((creature->GetEntry() + GWARZ[LocId].team) != GWCOMM[SERVER_GUILD_ID].cannon_id)
					{
						UpdateCreature(creature, GWCOMM[SERVER_GUILD_ID].cannon_id, LocId);
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

					UpdateGuildLocData("cannon_count", ConvertNumberToString(GWARZ[LocId].cannon_count - 1), LocId);

					creature->DeleteFromDB();
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

			if (player_guild_rank >= GWCOMM[guild_id].GLD_lvlb){ AddGossipItemFor(player, 1, "buy", GOSSIP_SENDER_MAIN, 1004); };

			if (player_guild_rank >= GWCOMM[guild_id].GLD_lvls){ AddGossipItemFor(player, 1, "sell", GOSSIP_SENDER_MAIN, 1005); };

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

						if (GWCOMM[SERVER_GUILD_ID].anarchy == 1)
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

				UpdateGuildCommandData("allowed", ConvertNumberToString(1), PLAYER_GUILD_ID);

				ChatHandler(player->GetSession()).PSendSysMessage("<%sGrumbo|r>:%sYour Guild can now aquire/develop/buy or sell any lands they may come to control during there travels.", GWCOMM[PLAYER_GUILD_ID].color_6.c_str(), GWCOMM[PLAYER_GUILD_ID].color_10.c_str());

				player->AddItem(GWCOMM[SERVER_GUILD_ID].currency, GWCOMM[SERVER_GUILD_ID].gift_count);

				ChatHandler(player->GetSession()).PSendSysMessage("<%s%s|r>:%sHere is %u %s's to get your Guild Plot started.", GWCOMM[PLAYER_GUILD_ID].color_6.c_str(), creature->GetName(), GWCOMM[PLAYER_GUILD_ID].color_10.c_str(), GWCOMM[SERVER_GUILD_ID].gift_count, Currencyname.c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("<%s%s|r>:%sNow Leave and seek out lands in the name of %s!The Horde!.", GWCOMM[PLAYER_GUILD_ID].color_6.c_str(), creature->GetName(), GWCOMM[PLAYER_GUILD_ID].color_10.c_str(), GWCOMM[PLAYER_GUILD_ID].color_15.c_str());

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
	new GGW_RankTimer;
    new GGW_LoadGWtable;
	new GGW_GuildEngine;
	new GGW_GuildPlayerOnLogin;
	new GGW_commands;
	new GGW_GUILD_FLAG;
	new GGW_Guild_Player_zone_change;
	new GGW_GUILD_VAULT;
	new GGW_GUILD_MAILBOX;
	new GGW_GUILD_PIG;
	new GGW_GUILD_GUARD;
	new GGW_GUILD_VENDOR1;
	new GGW_GUILD_VENDOR2;
	new GGW_GUILD_VENDOR3;
	new GGW_GUILD_CANNON;
	new GGW_GUILD_INFO_STATION;
	new GGW_GUILD_LEADER_QUEST_ENGINE;
}
