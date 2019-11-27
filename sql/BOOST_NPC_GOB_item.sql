-- // these are created from an Ace based Core. so you may need to change some of the column names.
-- // i slimmed them down to just the required data but you still may need to change column names.

REPLACE INTO `creature_template` (`entry`, `modelid1`, `name`, `subname`, `IconName`, `minlevel`, `maxlevel`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `scale`, `rank`, `dmgschool`, `baseattacktime`, `rangeattacktime`, `unit_class`, `unit_flags`, `family`, `type`, `AIName`, `DamageModifier`, `RegenHealth`, `flags_extra`, `ScriptName`) VALUES 
(49000, 193, 'Guild War\'z Pig', 'Alliance', NULL, 85, 85, 35, 0, 1, 1.14286, 1, 0, 1, 0, 0, 0, 0, 0, 0, '', 0, 1, 0, 'GGW_GUILD_PIG'),
(49001, 744, 'Guild War\'z Pig', 'Horde', NULL, 85, 85, 35, 0, 1, 1.14286, 1, 0, 1, 0, 0, 0, 0, 0, 0, '', 0, 1, 0, 'GGW_GUILD_PIG'),
(49002, 26725, 'Guild War\'z Guard', 'Alliance', 'NULL', 85, 85, 84, 524288, 1.5, 2.5, 1, 4, 4, 1200, 0, 1, 520, 17, 7, 'AggressorAI', 154355, 1, 33024, 'GGW_GUILD_GUARD'),
(49003, 26725, 'Guild War\'z Guard', 'Horde', 'NULL', 85, 85, 83, 524288, 1.5, 2.5, 1, 4, 4, 1200, 0, 1, 520, 17, 7, 'AggressorAI', 154355, 1, 33024, 'GGW_GUILD_GUARD'),
(49004, 18239, 'Buff Vendor 1', 'Guild War\'z', 'Buy', 80, 80, 35, 129, 1, 1.14286, 1, 0, 1, 0, 0, 0, 0, 0, 0, '', 0, 1, 0, 'GGW_GUILD_VENDOR1'),
(49005, 1322, 'Buff Vendor 1', 'Guild War\'z', 'Buy', 80, 80, 35, 129, 1, 1.14286, 1, 0, 1, 0, 0, 0, 0, 0, 0, '', 0, 1, 0, 'GGW_GUILD_VENDOR1'),
(49006, 10722, 'Fun Vendor 2', 'Guild War\'z', 'Buy', 80, 80, 35, 128, 1, 1.14286, 1, 0, 1, 0, 0, 0, 0, 0, 0, '', 0, 1, 0, 'GGW_GUILD_VENDOR2'),
(49007, 3608, 'Fun Vendor 2', 'Guild War\'z', 'Buy', 80, 80, 35, 128, 1, 1.14286, 1, 0, 1, 0, 0, 0, 0, 0, 0, '', 0, 1, 0, 'GGW_GUILD_VENDOR2'),
(49008, 3527, 'Gear Vendor 3', 'Guild War\'z', 'Buy', 80, 80, 35, 128, 1, 1.14286, 1, 0, 1, 0, 0, 0, 0, 0, 0, '', 0, 1, 0, 'GGW_GUILD_VENDOR3'),
(49009, 1321, 'Gear Vendor 3', 'Guild War\'z', 'Buy', 80, 80, 35, 128, 1, 1.14286, 1, 0, 1, 0, 0, 0, 0, 0, 0, '', 0, 1, 0, 'GGW_GUILD_VENDOR3');

REPLACE INTO `creature_template` (`entry`, `difficulty_entry_1`, `modelid1`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `scale`, `rank`, `dmgschool`, `baseattacktime`, `rangeattacktime`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `type`, `type_flags`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `DamageModifier`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `flags_extra`, `ScriptName`) VALUES 
(49010,32795,27101, 'Anti-personnel Cannon', '', 'vehichleCursor',0,80,80,84,16777216,1,1,1,1,0,2000,0,1,32772,2048,8,0,9,262176,160,0,0, 'NullAI',0,1,8.64198,1,1,134343,0,1,344276858, 2, 'GGW_GUILD_CANNON'),
(49011,32795,27101, 'Anti-personnel Cannon', '', 'vehichleCursor',0,80,80,83,16777216,1,1,1,1,0,2000,0,1,32772,2048,8,0,9,262176,160,0,0, 'NullAI',0,1,8.64198,1,1,134343,0,1,344276858, 2, 'GGW_GUILD_CANNON');

REPLACE INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`) VALUES 
(49010, 0, 49872, 12340),
(49011, 0, 49872, 12340);

REPLACE INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES 
(49010, 60682, 1, 0),
(49011, 60682, 1, 0);

REPLACE INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `IconName`, `size`, `Data0`, `Data1`, `Data2`, `Data3`, `Data4`, `Data5`, `Data6`, `Data7`, `Data8`, `Data9`, `Data10`, `Data11`, `Data12`, `Data13`, `Data14`, `Data15`, `Data16`, `Data17`, `Data18`, `Data19`, `Data20`, `Data21`, `Data22`, `Data23`, `ScriptName`) VALUES 
(187432, 10, 5651, 'Alliance Guild Flag', 'interact', 4, 0, 0, 0, 3000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'GGW_GUILD_FLAG'),
(187433, 10, 5652, 'Horde Guild Flag', 'interact', 4, 0, 0, 0, 3000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'GGW_GUILD_FLAG'),
(500000, 10, 8060, 'Alliance Guild farm', 'Interact', 0.6, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ''),
(500001, 10, 8060, 'Horde Guild farm', 'Interact', 0.6, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ''),
(500002, 10, 8057, 'Alliance Guild Barracks', 'Interact', 0.3, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ''),
(500003, 10, 8057, 'Horde Guild Barracks', 'Interact', 0.3, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ''),
(500004, 33, 7900, 'Tiff\'s Guild Hall', 'Interact', 0.3, 5000, 35074, 0, 19669, 1, 25000, 1, 1, 0, 19672, 1, 1, 0, 0, 19675, 0, 0, 0, 39, 0, 0, 0, 19678, 0, ''),
(500005, 33, 7878, 'Grumbo\'z Guild Hall', 'Interact', 0.3, 5000, 35074, 0, 19669, 1, 25000, 1, 1, 0, 19672, 1, 1, 0, 0, 19675, 0, 0, 0, 39, 0, 0, 0, 19678, 0, ''),
(500006, 34, 7613, 'Alliance Guild Vault', '', 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'GGW_GUILD_VAULT'),
(500007, 34, 7613, 'Horde Guild Vault', '', 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'GGW_GUILD_VAULT'),
(500008, 19, 7605, 'Alliance Guild Mailbox', '', 0.5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'GGW_GUILD_MAILBOX'),
(500009, 19, 7605, 'Horde Guild Mailbox', '', 0.5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'GGW_GUILD_MAILBOX');

REPLACE INTO `gameobject_template_addon` (`entry`, `faction`, `flags`, `mingold`, `maxgold`) VALUES 
(187431, 35, 0, 0, 0),
(187432, 35, 0, 0, 0),
(500000, 84, 6553632, 0, 0),
(500001, 83, 6553632, 0, 0),
(500002, 84, 6553632, 0, 0),
(500003, 83, 6553632, 0, 0),
(500004, 84, 6553632, 0, 0),
(500005, 83, 6553632, 0, 0),
(500006, 84, 0, 0, 0),
(500007, 83, 0, 0, 0),
(500008, 84, 0, 0, 0),
(500009, 83, 0, 0, 0);

REPLACE INTO `item_template` (`entry`, `class`, `subclass`, `SoundOverrideSubclass`, `name`, `displayid`, `Quality`, `Flags`, `FlagsExtra`, `BuyCount`, `BuyPrice`, `SellPrice`, `InventoryType`, `AllowableClass`, `AllowableRace`, `maxcount`, `stackable`, `bonding`, `description`, `RequiredDisenchantSkill`, `ScriptName`) VALUES 
(62006, 0, 8, -1, 'Guild Coin', 34104, 4, 0, 0, 1, 500000000, 500000000, 0, -1, -1, 2147483647, 2147483647, 0, 'Grumbo\'z Guild Warz', -1, '');

REPLACE INTO `npc_vendor` (`entry`, `slot`, `item`, `maxcount`, `incrtime`, `ExtendedCost`, `VerifiedBuild`) VALUES 
-- Vendor2
(49006, 1, 7734, 0, 0, 0, 0),(49006, 2, 6948, 0, 0, 0, 0),
(49006, 3, 49912, 0, 0, 0, 0),(49006, 4, 34498, 0, 0, 0, 0),
(49006, 5, 46693, 0, 0, 0, 0),(49006, 6, 34499, 0, 0, 0, 0),
(49006, 7, 35557, 0, 0, 0, 0),(49006, 8, 37431, 0, 0, 0, 0),
(49006, 9, 17202, 0, 0, 0, 0),(49006, 10, 21038, 0, 0, 0, 0),
(49006, 11, 46783, 0, 0, 0, 0),

(49007, 1, 7734, 0, 0, 0, 0),(49007, 2, 6948, 0, 0, 0, 0),
(49007, 3, 49912, 0, 0, 0, 0),(49007, 4, 34498, 0, 0, 0, 0),
(49007, 5, 46693, 0, 0, 0, 0),(49007, 6, 34499, 0, 0, 0, 0),
(49007, 7, 35557, 0, 0, 0, 0),(49007, 8, 37431, 0, 0, 0, 0),
(49007, 9, 17202, 0, 0, 0, 0),(49007, 10, 21038, 0, 0, 0, 0),
(49007, 11, 46783, 0, 0, 0, 0),

-- Vendor3
(49008, 1, 32837, 0, 0, 0, 0),(49008, 2, 32838, 0, 0, 0, 0),
(49008, 3, 22736, 0, 0, 0, 0),(49008, 4, 19019, 0, 0, 0, 0),
(49008, 5, 51858, 0, 0, 0, 0),(49008, 6, 24550, 0, 0, 0, 0),
(49008, 7, 2000, 0, 0, 0, 0),(49008, 8, 50730, 0, 0, 0, 0),
(49008, 9, 50070, 0, 0, 0, 0),(49008, 10, 34196, 0, 0, 0, 0),
(49008, 11, 30906, 0, 0, 0, 0),

(49009, 1, 32837, 0, 0, 0, 0),(49009, 2, 32838, 0, 0, 0, 0),
(49009, 3, 22736, 0, 0, 0, 0),(49009, 4, 19019, 0, 0, 0, 0),
(49009, 5, 51858, 0, 0, 0, 0),(49009, 6, 24550, 0, 0, 0, 0),
(49009, 7, 2000, 0, 0, 0, 0),(49009, 8, 50730, 0, 0, 0, 0),
(49009, 9, 50070, 0, 0, 0, 0),(49009, 10, 34196, 0, 0, 0, 0);

-- -- -- -- -- -- --
-- Quest support  --
-- -- -- -- -- -- --
REPLACE INTO `item_template` (`entry`, `class`, `subclass`, `name`, `displayid`, `Quality`, `Flags`, `FlagsExtra`, `InventoryType`, `AllowableClass`, `AllowableRace`, `ItemLevel`, `maxcount`, `bonding`, `description`, `PageText`, `LanguageID`, `PageMaterial`, `startquest`, `sheath`, `ScriptName`, `flagsCustom`, `VerifiedBuild`) VALUES 
(65000, 15, 0, 'Leather Bound Letter From King Varian Wrynn.', 16065, 1, 32, 0, 0, -1, -1, 0, 0, 1, 'A leather bound note containing a summons from King Varian Wrynn.', 0, 0, 4, 52000, 0, '', 0, 12340),
(65001, 15, 0, 'Leather Bound Letter from WarChief Thrall.', 63437, 1, 32, 0, 0, -1, -1, 0, 0, 1, 'A leather bound note containing a summons from Thrall the Warchief.', 0, 0, 2, 52001, 0, '', 0, 12340),
(65002, 15, 0, 'Leather Bound Summons', 16065, 1, 32, 0, 0, -1, -1, 0, 0, 1, 'A leather bound summons from King Varian Wrynn.', 5000, 0, 4, 0, 0, '', 0, 12340),
(65003, 15, 0, 'Leather Bound Summons', 63437, 1, 32, 0, 0, -1, -1, 0, 0, 1, 'A leather bound summons from Thrall the Warchief.', 5001, 0, 2, 0, 0, '', 0, 12340),
(65004, 15, 0, 'Guild Warz Development Permit.', 16065, 1, 32, 0, 0, -1, -1, 0, 0, 1, 'A leather bound Development Permit from King Varian Wrynn.', 5002, 0, 4, 0, 0, '', 0, 12340),
(65005, 15, 0, 'Guild Warz Development Permit.', 63437, 1, 32, 0, 0, -1, -1, 0, 0, 1, 'A leather bound Development Permit from Thrall the Warchief.', 5003, 0, 2, 0, 0, '', 0, 12340);

REPLACE INTO `page_text` (`ID`, `Text`, `NextPageID`, `VerifiedBuild`) VALUES 
(5000, 'Guild Master $N. I King Varian Wrynn, herby summon you Keep to appear befor me in StormWind.', 0, 12340),
(5001, 'Guild Master $N. I WarChief Thrall, herby summon you to appear befor me in Orgrimmar.', 0, 12340),
(5002, 'Guild Master $N. I King Varian Wrynn, herby give permit to the Guild under $N`s command to own and build on any land they may aquire in there travels.', 0, 12340),
(5003, 'Guild Master $N. I WarChief Thrall, herby give permit to the Guild under $N`s command to own, build and sell any land they may aquire in there travels.', 0, 12340);

REPLACE INTO `quest_template` (`ID`, `QuestType`, `QuestLevel`, `MinLevel`, `StartItem`, `Flags`, `RewardItem1`, `RewardAmount1`, `LogTitle`, `LogDescription`, `QuestDescription`, `AreaDescription`, `QuestCompletionLog`, `RequiredItemId1`, `RequiredItemCount1`, `ObjectiveText1`, `ObjectiveText2`, `ObjectiveText3`, `ObjectiveText4`, `VerifiedBuild`) VALUES 
(52000, 2, 80, 80, 65002, 8192, 65004, 1, 'Guild Master Summons.', 'Appear befor King Varian Wrynn.', 'Take this summons to StormWind Keep and speak with King Varian Wrynn.', 'StormWind', 'Quest Completion Log', 65002, 1, 'Head to StormWind Keep and speak with King Varian Wrynn.', 'OBJ TXT 2', 'OBJ TXT 3', 'OBJ TXT 4', 0),
(52001, 2, 80, 80, 65003, 270336, 65005, 1, 'Guild Master Summons.', 'Appear befor Thrall the Warchief.', 'Take this summons to Grommash Hold and speak with Thrall the WarChief.', 'Orgrimmar', 'Head to Grommash Hold in Orgrimmar and speak with Thrall the WarChief.', 65003, 1, 'OBJ TXT 1', 'OBJ TXT 2', 'OBJ TXT 3', 'OBJ TXT 4', 0);

REPLACE INTO `quest_template_addon` (`ID`, `ProvidedItemCount`, `SpecialFlags`) VALUES 
(52000, 1, 1),
(52001, 1, 1);

REPLACE INTO `quest_offer_reward` (`ID`, `Emote1`, `Emote2`, `Emote3`, `Emote4`, `EmoteDelay1`, `EmoteDelay2`, `EmoteDelay3`, `EmoteDelay4`, `RewardText`, `VerifiedBuild`) VALUES 
(52000, 0, 0, 0, 0, 0, 0, 0, 0, 'I have called you befor me to discuss some issues our Kingdom is facing... Due to thinned ranks and personel, we cannot protect our lands ... at ALL. I need you and your Guild to go out explore and claim any land your Guild is capable of siezing. I will give ownership and permits for developement. If your Guild becomes weak and cannot hold its lands .. Well maybe a stronger Guild should challange your Guild for them with battle. !Now go forth and claim lands in the name of the Alliance!', 12340),
(52001, 0, 0, 0, 0, 0, 0, 0, 0, 'I have called you befor me to discuss some issues We, the Horde, are facing. Due to thinned ranks and personel, we cannot protect our lands ... at ALL. I need you and your Guild to go out explore and claim any land your Guild is capable of siezing. I will give ownership and permits for developement. If your Guild becomes weak and cannot hold its lands .. Well maybe a stronger Guild should take some from your Guild with force!. !Now go forth and claim lands in the name of the Horde!', 12340);

REPLACE INTO `creature_template` (`entry`, `modelid1`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `scale`, `rank`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `type`, `type_flags`, `mingold`, `maxgold`, `AIName`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `DamageModifier`, `ExperienceModifier`, `RacialLeader`, `RegenHealth`, `mechanic_immune_mask`, `ScriptName`, `VerifiedBuild`) VALUES 
(49014, 28127, 'King Varian Wrynn', 'King of Stormwind', '', 0, 83, 83, 2, 1733, 3, 1, 1.42857, 1, 3, 2000, 2000, 1, 1, 1, 0, 2048, 7, 76, 700000, 750000, '', 1, 400, 20, 1, 35, 1, 1, 1, 619397119, 'GGW_GUILD_LEADER_QUEST_ENGINE', 12340),
(49015, 4527, 'Thrall', 'Warchief', 'Speak', 0, 83, 83, 2, 1734, 3, 1, 1.14286, 1, 3, 2000, 2000, 1, 1, 2, 64, 2048, 7, 12, 700000, 750000, '', 1, 400, 30, 1, 35, 1, 1, 1, 619397119, 'GGW_GUILD_LEADER_QUEST_ENGINE', 12340);

REPLACE INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `VerifiedBuild`) VALUES 
(1000000, 49014, 0, 1519, 1519, 1, 1, 28127, 0, -8521.7539, 434.0248, 106.0520, 2.248629, 0, 5, 0, 8, 0, 0, 0, 0, 0, 0),
(1000001, 49015, 1, 1637, 1637, 1, 1, 4527, 0, 1927.4399, -4229.9599, 42.3202, 4.76595, 0, 5, 0, 8, 0, 0, 0, 0, 0, 0);

REPLACE INTO `creature_questender` (`id`, `quest`) VALUES 
(49014, 52000),
(49015, 52001);
