UPDATE item_template SET socketColor_1 = 0, socketColor_2 = 0, socketColor_3 = 0, socketBonus = 0, GemProperties = 0 WHERE class = 2 OR class = 4;

DELETE FROM `gameobject_template` WHERE `entry`=441150;
INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `IconName`, `castBarCaption`, `unk1`, `size`, `Data0`, `Data1`, `Data2`, `Data3`, `Data4`, `Data5`, `Data6`, `Data7`, `Data8`, `Data9`, `Data10`, `Data11`, `Data12`, `Data13`, `Data14`, `Data15`, `Data16`, `Data17`, `Data18`, `Data19`, `Data20`, `Data21`, `Data22`, `Data23`, `AIName`, `ScriptName`, `VerifiedBuild`) VALUES (441150, 2, 8304, 'Mystic Anvil', '', '', '', 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 'DynamicSocketsGameObjectScript', 0);
