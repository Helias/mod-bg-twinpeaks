-- acore_string
DELETE FROM `acore_string` WHERE entry BETWEEN 1230 AND 1244;
INSERT INTO `acore_string` (`entry`,`content_default`) VALUES
(1230,'The Battle for Twin Peaks begins in 2 minutes.'),
(1231,'The Battle for Twin Peaks begins in 1 minute.'),
(1232,'The Battle for Twin Peaks begins in 30 seconds. Prepare yourselves!'),
(1233,'The Battle for Twin Peaks has begun!'),
(1234,'$N captured the Horde flag!'),
(1235,'$N captured the Alliance flag!'),
(1236,'The Horde flag was dropped by $N!'),
(1237,'The Alliance flag was dropped by $N!'),
(1238,'The Alliance Flag was returned to its base by $N!'),
(1239,'The Horde Flag was returned to its base by $N!'),
(1240,'The Horde flag was picked up by $N!'),
(1241,'The Alliance flag was picked up by $N!'),
(1242,'The flags are now placed at their bases.'),
(1243,'The Alliance flag is now placed at its base.'),
(1244,'The Horde flag is now placed at its base.');

-- battleground_template
DELETE FROM `battleground_template` WHERE id=108;
INSERT INTO `battleground_template` (`id`,`MinPlayersPerTeam`,`MaxPlayersPerTeam`,`MinLvl`,`MaxLvl`,`AllianceStartLoc`,`AllianceStartO`,`HordeStartLoc`,`HordeStartO`,`StartMaxDist`,`Weight`,`ScriptName`,`Comment`) VALUES
(108,5,10,85,85,1726,2.57218,1727,6.16538,120,1,'','Twin Peaks');

-- gameobjects data
DELETE FROM `gameobject_template` WHERE entry IN (207075, 207076, 206653, 206654, 206655, 208205, 208206, 208207, 208208, 208209);
INSERT INTO `gameobject_template` (`entry`,`type`,`displayId`,`name`,`IconName`,`castBarCaption`,`unk1`,`size`,`Data0`,`Data1`,`Data2`,`Data3`,`Data4`,`Data5`,`Data6`,`Data7`,`Data8`,`Data9`,`Data10`,`Data11`,`Data12`,`Data13`,`Data14`,`Data15`,`Data16`,`Data17`,`Data18`,`Data19`,`Data20`,`Data21`,`Data22`,`Data23`,`AIName`,`ScriptName`,`VerifiedBuild`) VALUES
(207075,8,9507,'Cozy Fire','','','',0.92,4,10,2061,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'','',1),
(207076,8,8661,'Cookpot','','','',0.42,4,10,2061,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'','',1),
(206653,0,10122,'Twin Peaks Alliance Gate 3 and 4','','','',1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'','',1),
(206654,0,10123,'Twin Peaks Alliance Gate 2','','','',1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'','',1),
(206655,0,10124,'Twin Peaks Alliance Gate 1','','','',1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'','',1),
(208205,0,10442,'Twin Peaks Horde Gate 1','','','',1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'','',1),
(208206,0,10443,'Twin Peaks Horde Gate 2 and 3','','','',1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'','',1),
(208207,0,10444,'Twin Peaks Horde Gate 4','','','',1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'','',1),
(208208,26,5912,'Twin Peaks Alliance Flag','','','',2.5,0,8623,23383,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'','',12340),
(208209,26,5913,'Twin Peaks Horde Flag','','','',2.5,0,8624,23384,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'','',12340);

DELETE FROM `gameobject_template_addon` WHERE entry IN (207075, 207076, 206655, 206654, 206653, 208205, 208206, 208207, 208208, 208209);
INSERT INTO `gameobject_template_addon` (`entry`,`faction`,`flags`,`mingold`,`maxgold`) VALUES
(206655,1375,32,0,0),
(206654,1375,32,0,0),
(206653,1375,32,0,0),
(208205,1375,32,0,0),
(208206,1375,32,0,0),
(208207,1375,32,0,0),
(208208,1913,0,0,0),
(208209,2058,0,0,0),
(207075,0,0,0,0),
(207076,0,0,0,0);

DELETE FROM `gameobject` WHERE guid IN (187448, 187449, 402303);
INSERT INTO `gameobject` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`position_x`,`position_y`,`position_z`,`orientation`,`rotation0`,`rotation1`,`rotation2`,`rotation3`,`spawntimesecs`,`animprogress`,`state`,`ScriptName`,`VerifiedBuild`) VALUES
(187448,207075,726,0,0,1,1,1986.05,420.546,-20.7072,5.14872,0,0,0.390731,0.920505,7200,255,1,'',0),
(187449,207076,726,0,0,1,1,1953.24,394.69,-9.54223,0.549777,0,0,0.390731,0.920505,7200,255,1,'',0);

-- game_graveyard
DELETE FROM `game_graveyard` WHERE ID IN (1726, 1727, 1728, 1729, 1749, 1750);
INSERT INTO `game_graveyard` (`ID`, `Map`, `x`, `y`, `z`, `Comment`) VALUES
(1726,726,2142.6,175.918,43.6513,"Twin Peaks - Alliance Entrance"),
(1727,726,1549.538,346.9549,1.285329,"Twin Peaks - Horde Entrance"),
(1728,726,1561.16,212.76,13.8615,"Twin Peaks - Horde Graveyard (Base)"),
(1729,726,2168.1,332.038,34.6959,"Twin Peaks - Alliance Graveyard (Base)"),
(1749,726,1877.15,439.111,-4.00017,"Twin Peaks - Alliance Graveyard (Center)"),
(1750,726,1816.87,160.083,1.80644,"Twin Peaks - Horde Graveyard (Center)");

-- area trigger
DELETE FROM `areatrigger` WHERE entry IN (5904, 5905, 5906, 5907, 5908, 5909, 5910, 5911, 5914, 5916, 5917, 5918, 5920, 5921, 6803, 6804, 6805, 6806);
INSERT INTO `areatrigger` (`entry`,`map`,`x`,`y`,`z`,`radius`,`length`,`width`,`height`,`orientation`) VALUES
(5904,726,2119.05,190.865,44.0558,5,0.3333,0.3333,0.3333,0),
(5905,726,1578.09,344.069,2.41768,5,0.3333,0.3333,0.3333,0),
(5906,726,2176.17,226.554,43.7637,4,0,0,0,0),
(5907,726,1544.05,303.741,0.663854,4,0,0,0,0),
(5908,726,1754.16,242.052,-13.9863,4,0,0,0,0),
(5909,726,1951.39,383.95,-10.5183,4,0,0,0,0),
(5910,726,1737.69,435.859,-7.80297,4,0,0,0,0),
(5911,726,1933.13,226.578,-17.0168,4,0,0,0,0),
(5914,726,2136.63,237.644,43.7999,0,82.35,10,56.02,5.747),
(5916,726,2098.52,203.977,43.4028,0,84.69,26.53,56.02,1.02),
(5917,726,2100.05,137.571,39.8552,0,82.35,23.83,56.02,5.747),
(5918,726,1590,294.613,0.69735,0,82.35,23.83,56.02,6.178),
(5920,726,1595.89,347.719,1.36971,0,80.11,14.59,56.02,1.48),
(5921,726,1559.92,388.009,-4.8754,0,19.87,14.06,56.02,6.178),
(6803,726,1820.85,156.363,9,0,43,29,18,0),
(6804,726,1879.68,442.125,0,13.6763,5.332,11.1,0,0),
(6805,726,1552,212.507,18,0,57,37,12,1.608),
(6806,726,2180,330,45,0,35,49,25,0.3152);

-- achievements IDs
-- (5226, 5227, 5231, 5552, 5229, 5221, 5222, 5220, 5219, 5223, 5216, 5213, 5214, 5211, 5208, 5230, 5215, 5209, 5210, 5228)

