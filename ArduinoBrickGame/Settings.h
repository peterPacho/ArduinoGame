#pragma once

struct s_sett
{
	bool vibrations = false;
	bool sound = false;
	bool id = false;
} SETTINGS;

void saveSettings()
{
	eeprom_update_block((void *)&SETTINGS, NULL, sizeof(SETTINGS));
}

void loadSettings()
{
	eeprom_read_block((void *)&SETTINGS, NULL, sizeof(SETTINGS));
}

void defaultSettings()
{
	s_sett def;
	eeprom_update_block((void *)&def, NULL, sizeof(def));
	loadSettings();
}