#ifndef GRUMBOZ_GUILD_WARZ_H
#define GRUMBOZ_GUILD_WARZ_H

struct Commands
{
	std::string guild;
	uint8 team;
	uint32 guild_id;
	std::string commands;
	std::string info_loc;
	std::string list_loc;
	std::string tele;
	std::string version;
    uint8 GLD_lvlb;
    uint8 GLD_lvls;
	std::string respawn_flag;
	std::string details_loc;
	std::string table;
    uint8 GM_admin;
    uint8 GM_minimum;
	uint64 currency;
    uint64 pig_payz;
    uint64 pig_payz_timer;
    uint32 gift_count;
    uint8 flag_require;
	std::string Server;
	std::string command_set;
	uint8 anarchy;
	uint8 f_timer;
	uint64 s_timer;
	uint8 guild_invite;
	std::string loc;
    uint32 loc_cost;
	uint64 flag_id;
	std::string farm;
	uint16 farm_cost;
	uint16 farm_L;
	uint64 farm_id;
	std::string barrack;
	uint16 barrack_cost;
	uint16 barrack_L;
	uint64 barrack_id;
	std::string hall;
	uint16 hall_cost;
	uint16 hall_L;
	uint64 hall_id;
	std::string pig;
	uint16 pig_cost;
	uint16 pig_L;
	uint64 pig_id;
	std::string guard;
	uint16 guard_cost;
	uint16 guard_L;
	uint64 guard_id;
	std::string vendor1;
	uint16 vendor1_cost;
	uint16 vendor1_L;
	uint64 vendor1_id;
	std::string vendor2;
	uint16 vendor2_cost;
	uint16 vendor2_L;
	uint64 vendor2_id;
	std::string vendor3;
	uint16 vendor3_cost;
	uint16 vendor3_L;
	uint64 vendor3_id;
	std::string cannon;
	uint16 cannon_cost;
	uint16 cannon_L;
	uint64 cannon_id;
	std::string vault;
	uint16 vault_cost;
	uint16 vault_L;
	uint64 vault_id;
	std::string mailbox;
	uint16 mailbox_cost;
	uint16 mailbox_L;
	uint64 mailbox_id;
	std::string setup;
	std::string color_1;
	std::string color_2;
	std::string color_3;
	std::string color_4;
	std::string color_5;
	std::string color_6;
	std::string color_7;
	std::string color_8;
	std::string color_9;
	std::string color_10;
	std::string color_11;
	std::string color_12;
	std::string color_13;
	std::string color_14;
	std::string color_15;
	uint8 allowed;
};

struct Help
{
    uint32 entry;
    std::string name;
	std::string description;
	std::string example;
    uint8 command_level;
};

struct LocData
{
	uint32 entry;
	uint32 map_id;
	uint32 area_id;
	uint32 zone_id;
	std::string guild_name;
    uint8 team;
    float x;
    float y;
    float z;
	uint32 farm_count;
	uint32 barrack_count;
	uint32 hall_count;
	uint32 pig_count;
	uint32 guard_count;
	uint32 vendor1_count;
	uint32 vendor2_count;
	uint32 vendor3_count;
	uint32 cannon_count;
	uint32 vault_count;
	uint32 mailbox_count;
	uint32 flag_id;
	uint64 fs_time;
    uint32 guild_id;
};

struct rank_info
{
	uint32 guild_id;
	uint8 team;
	uint32 total_gross_worth;
	std::string name;
};

struct GGW_PlayerData
{
	uint32 faction;
	uint8 team;
	bool faction_checker;
};

struct GGW_CreatureData
{
	uint32 LocId;
	uint32 map_id;
	uint32 area_id;
	uint32 zone_id;
	float x;
	float y;
	float z;
	float o;
	Map* map;
};

class TC_GAME_API GGW
{
private:
		GGW();
		~GGW();

public:
	static GGW* instance();

	//	creatorz
	uint32 CreateGuildLocation(uint32 map_id, uint32 area_id, uint32 zone_id, float pX, float pY, float pZ);
	uint32 CreateGuildCommands(std::string guild_name, uint32 guild_id);
	uint32 SpawnGuildObjects(uint8 type, uint32 flat_id, uint32 team_id, uint32 guild_id, Map *map, float x, float y, float z, float o, Player *player, uint32 LocId);

// loaderz
	bool LoadCommands();
	bool LoadHelp();
	bool LoadLoc();

//	getterz
	static uint32 GetLocationID(uint32 map_id, uint32 area_id, uint32 zone_id);

//	setterz
	static void SetCreatureFactionToLocation(Creature* creature, Unit* unit, uint32 LocId);

//	updaterz
	static void UpdateGuildLocData(std::string column_target, std::string new_data, uint32 loc_id);
	static void UpdateGuildLocFloat(float x, float y, float z, uint32 loc_id);
	static void UpdateGuildCommandData(std::string column_target, std::string new_data, uint32 guild_id);
	static void UpdateGameObject(GameObject* go, uint32 base_id, uint32 LocId);
	static void UpdateCreature(Creature* creature, uint32 base_id, uint32 LocId);

// misc
	static void SendGuildMessage(uint32 guild_id, std::string msg);
	static void PigPayz(Player* player);
	static void FactionReset(Player* player);
	uint64 GGW_ConvertStringToNumber(std::string arg);
	std::string ConvertNumberToString(uint64 numberX);
	uint32 GetTotalPigs(uint32 guild_id);
	uint32 CalculateTotalLocations(uint32 guild_id);
	uint32 CalculateLocationValue(uint32 loc_id);
	uint64 CalculateTotalLocationsValue(uint32 guild_id);
	uint32 GetRank(uint32 guild_id);
	bool UpdateRankEntry(uint32 guild_id, uint32 total_gross_worth, uint8 team);
	static void BuildRankList();
	bool CreateRankList();

	// defining most Globals now
	// hard stored  settings //
	float core_version = 6.85f;
	float table_version = 2.88f;
	float pigpayz_version = 2.50f;
	float tele_version = 1.50f;
	float pvp_version = 4.88f;
	float vendor_version = 1.55f;

	float GW_version = ((table_version + core_version + pigpayz_version + tele_version + pvp_version + vendor_version) / 4);


	uint8 MAX_GUILD_RANKS = 255;
	uint8 SERVER_GUILD_TEAM_LOCKED_ID = 3;
	uint8 SERVER_GUILD_TEAM = 2;
	uint32 SERVER_GUILD_ID = 0;
	uint64 Guard_Died_Drop_Reward = 20558; // wsg's not yet added
										   
	// pre-define global core tables
	std::unordered_map<uint32, Commands>GWCOMM;
	std::unordered_map<uint32, Help>GWHELP;
	std::unordered_map<uint32, LocData>GWARZ;
	std::unordered_map<uint32, GGW_CreatureData>GGW_Creature;
	std::map<uint32, rank_info>GW_Ranks;
	std::map<uint32, uint32>GW_RANKS;
    std::string Guild_Warz_DB;
    std::string commands_db;
    std::string help_db;
    std::string zones_db;
    std::string ranking_db;

	// pre-defining other Global's for later
	uint32 GUILDWARZ_PLAYER_CHECK_TIMER;
	uint32 GUILDWARZ_PIGPAYZ_VALUE;
	uint32 GUILDWARZ_PIGPAYZ_TIMER;
	uint32 GUILDWARZ_RANK_TYPE;
	uint32 GUILDWARZ_RANKING_TIMER;
	uint32 GUILDWARZ_RANKING_MAX;

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

	uint32 Currencyid;
	std::string Currencyname;

private:
};

#define sGGW GGW::instance()
#endif // GRUMBOZ_GUILD_WARZ_H_INCLUDED
