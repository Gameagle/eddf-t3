#include "pch.h"
#include "eddft3Plugin.h"
#include "logger.h"

#include <regex>
#include <ranges>
#include <charconv>
#include <format>

eddft3::eddft3Plugin::eddft3Plugin() : EuroScopePlugIn::CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE,
	eddft3::pluginName.c_str(),
	eddft3::pluginVersion.c_str(),
	eddft3::pluginAuthor.c_str(),
	eddft3::pluginCopyright.c_str())
{
	RegisterTagItemType("RWY Hint", TAG_ITEM_DFT3_HINT);
	DisplayUserMessage("Message", "eddft3", std::string("Version: " + pluginVersion + " loaded.").c_str(), true, true, true, false, false);

	eddfPosition = getEddfPosition();
	warnColor = getColorFromSettings();
}

eddft3::eddft3Plugin::~eddft3Plugin()
{
	DisplayUserMessage("Message", "eddft3", "Unloaded successfully", true, true, true, false, false);
}

EuroScopePlugIn::CPosition eddft3::eddft3Plugin::getEddfPosition()
{
	EuroScopePlugIn::CPosition pos;

	for (EuroScopePlugIn::CSectorElement airport = this->SectorFileElementSelectFirst(EuroScopePlugIn::SECTOR_ELEMENT_AIRPORT);
		airport.IsValid();
		airport = this->SectorFileElementSelectNext(airport, EuroScopePlugIn::SECTOR_ELEMENT_AIRPORT))
	{
		eddft3::Logger::log("Checking airport: " + std::string(airport.GetName()));

		if (airport.GetName() == std::string("EDDF"))
		{
			airport.GetPosition(&pos, 0);

			eddft3::Logger::log("EDDF position found: " + std::to_string(pos.m_Latitude) + ", " + std::to_string(pos.m_Longitude));
			break;
		}
	}
	return pos;
}

eddft3::color eddft3::eddft3Plugin::getColorFromSettings()
{
	std::string setting = this->GetDataFromSettings("warnColor");
	color settingColor{ 0,0,0 };
	static const std::regex pattern(R"(^\d{3}\.\d{3}\.\d{3}$)");

	if (std::regex_match(setting, pattern))
	{
		auto parts = setting | std::ranges::views::split('.');

		int idx = 0;
		for (auto part : parts)
		{
			int value = 0;
			auto first = &*part.begin();
			auto last = first + std::ranges::distance(part);

			auto [_, ec] = std::from_chars(first, last, value);

			if (ec == std::errc{} && value >= 0 && value <= 255)
			{
				if (idx == 0) settingColor.r = (uint8_t)value;
				if (idx == 1) settingColor.g = (uint8_t)value;
				if (idx == 2) settingColor.b = (uint8_t)value;
			}
			idx++;
		}
	}
	else eddft3::Logger::log("Invalid color setting format. Expected format: RRR.GGG.BBB (e.g. 255.000.000 for red). Current value: " + setting);

	return settingColor;
}

void eddft3::eddft3Plugin::OnGetTagItem(EuroScopePlugIn::CFlightPlan FlightPlan, EuroScopePlugIn::CRadarTarget RadarTarget, int ItemCode, int TagData, char sItemString[16], int* pColorCode, COLORREF* pRGB, double* pFontSize)
{
	if (!FlightPlan.IsValid() || !RadarTarget.IsValid())
		return;

	if (ItemCode == eddft3::TAG_ITEM_DFT3_HINT)
	{
		double distance = FlightPlan.GetCorrelatedRadarTarget().GetPosition().GetPosition().DistanceTo(eddfPosition);

		if (FlightPlan.GetFlightPlanData().GetDestination() == std::string("EDDF") && distance <= 80.0)
		{
			std::string callsign(FlightPlan.GetCallsign());

			if (northOps.find(callsign.substr(0, 3)) == northOps.end())
			{
				std::string route = FlightPlan.GetFlightPlanData().GetRoute();

				if (!route.ends_with("25L") && !route.ends_with("07R"))
				{
					*pColorCode = EuroScopePlugIn::TAG_COLOR_RGB_DEFINED;
					*pRGB = RGB(warnColor.r, warnColor.g, warnColor.b);
					strcpy_s(sItemString, 16, "S");
				}
			}
		}
	}
}

bool eddft3::eddft3Plugin::OnCompileCommand(const char* sCommandLine)
{
	std::string command(sCommandLine);

	if (command.starts_with(".dft3"))
	{
		if (command.find("log") != std::string::npos)
		{
			if (!eddft3::Logger::isRunning())
				eddft3::Logger::initialize();
			else
				eddft3::Logger::shutdown();

			return true;
		}

		if(command.find("rgb") != std::string::npos)
		{
			auto colorCommand = command | std::ranges::views::split(' ');

			auto it = colorCommand.begin();
			auto lastCmd = *it;
			for (auto part : colorCommand) { lastCmd = part; }

			std::string lastCmdStr(lastCmd.begin(), lastCmd.end());

			static const std::regex rgbPattern(R"(^\s*\d{3}:\d{3}:\d{3}\s*$)");

			if (!std::regex_match(lastCmdStr, rgbPattern))
			{
				eddft3::Logger::log("Invalid RGB command format. Expected format: .dft3 rgb R:G:B (e.g. .dft3 rgb 255:0:0 for red). Current value: " + lastCmdStr);
				return true;
			}

			auto colorParts = lastCmd | std::ranges::views::split(':');

			int idx = 0;
			for (auto part : colorParts)
			{
				int value = 0;
				auto first = &*part.begin();
				auto last = first + std::ranges::distance(part);

				auto [_, ec] = std::from_chars(first, last, value);

				if (ec == std::errc{} && value >= 0 && value <= 255)
				{
					if (idx == 0) warnColor.r = (uint8_t)value;
					if (idx == 1) warnColor.g = (uint8_t)value;
					if (idx == 2) warnColor.b = (uint8_t)value;
				}

				idx++;
			}

			// premature saving of settings - will be prompted on ES closure
			std::string formattedColor = std::format("{:03}.{:03}.{:03}", warnColor.r, warnColor.g, warnColor.b);
			this->SaveDataToSettings("warnColor", "south RWY warning color", formattedColor.c_str());

			return true;
		}
	}

	return false;
}

eddft3::eddft3Plugin* pEddft3Plugin = nullptr;

// Diese Funktion wird von EuroScope aufgerufen, wenn das Plugin geladen wird
void __declspec(dllexport) EuroScopePlugInInit(EuroScopePlugIn::CPlugIn** ppPlugInInstance)
{
	eddft3::Logger::initialize();
	*ppPlugInInstance = pEddft3Plugin = new eddft3::eddft3Plugin();
}

// Diese Funktion wird beim Entladen aufgerufen
void __declspec(dllexport) EuroScopePlugInExit(void)
{
	eddft3::Logger::log("eddft3 shutdown...");
	if (pEddft3Plugin != nullptr)
	{
		delete pEddft3Plugin;
		pEddft3Plugin = nullptr;
	}
	eddft3::Logger::shutdown();
}