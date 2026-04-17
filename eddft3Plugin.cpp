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
	colorOk = getColorFromSettings("ok");
	colorWarn = getColorFromSettings("warn");
	colorCaution = getColorFromSettings("caution");
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

eddft3::color eddft3::eddft3Plugin::getColorFromSettings(const std::string_view& colorType)
{
	std::string setting;
	try
	{
		if (colorType == "ok")
		{
			if (const char* tmpColor = this->GetDataFromSettings("okColor"); tmpColor != nullptr)
				setting = tmpColor;
			else setting = "000.255.000"; // default bright green
		}
		else if (colorType == "warn")
		{
			if (const char* tmpColor = this->GetDataFromSettings("warnColor"); tmpColor != nullptr)
				setting = tmpColor;
			else setting = "255.255.000"; // default yellow
		}
		else if (colorType == "caution")
		{
			if (const char* tmpColor = this->GetDataFromSettings("cautionColor"); tmpColor != nullptr)
				setting = tmpColor;
			else setting = "255.000.000"; // default red
		}
		else
		{
			eddft3::Logger::log("Invalid color type requested: " + std::string(colorType));
			return { 0,0,0 };
		}
	}
	catch (const std::exception& e)
	{
		eddft3::Logger::log("Error retrieving color setting for " + std::string(colorType) + ": " + e.what());
		return { 0,0,0 };
	}
	

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

	eddft3::Logger::log("Color for " + std::string(colorType) + " set to: " + std::to_string(settingColor.r) + ":" + std::to_string(settingColor.g) + ":" + std::to_string(settingColor.b));
	return settingColor;
}

bool eddft3::eddft3Plugin::isSouthApp(const std::string_view& last)
{
	static const std::regex starPattern(R"([BC](?=/|$))");

	bool starMatch = std::regex_match(last.begin(), last.end(), starPattern);

	return last.ends_with("25L") || last.ends_with("07R")/* || starMatch*/;
}

void eddft3::eddft3Plugin::OnGetTagItem(EuroScopePlugIn::CFlightPlan FlightPlan, EuroScopePlugIn::CRadarTarget RadarTarget, int ItemCode, int TagData, char sItemString[16], int* pColorCode, COLORREF* pRGB, double* pFontSize)
{
	if (!FlightPlan.IsValid() || !RadarTarget.IsValid())
		return;

	if (ItemCode == eddft3::TAG_ITEM_DFT3_HINT)
	{
		*pColorCode = EuroScopePlugIn::TAG_COLOR_RGB_DEFINED;
		double distance = FlightPlan.GetCorrelatedRadarTarget().GetPosition().GetPosition().DistanceTo(eddfPosition);

		if (FlightPlan.GetFlightPlanData().GetDestination() == std::string("EDDF") /*&& distance <= 80.0*/)
		{
			std::string callsign(FlightPlan.GetCallsign());

			// skip operator landing in the north
			if (northOps.find(callsign.substr(0, 3)) == northOps.end())
			{
				std::string_view route = FlightPlan.GetFlightPlanData().GetRoute();

				size_t lastCharPos = route.find_last_not_of(' ');

				if (lastCharPos == std::string_view::npos)
				{
					eddft3::Logger::log("Route is empty or only contains spaces for flight " + callsign);
					return;
				}

				route.remove_suffix(route.size() - lastCharPos - 1);

				size_t lastSpacePos = route.find_last_of(' ');

				std::string_view lastRoutePart = (lastSpacePos == std::string_view::npos) ? route : route.substr(lastSpacePos + 1);

				if (!isSouthApp(std::string(lastRoutePart.begin(), lastRoutePart.end())))
				{
					
					*pRGB = RGB(colorCaution.r, colorCaution.g, colorCaution.b);
					strcpy_s(sItemString, 16, "S");
				}
				else
				{
					*pRGB = RGB(colorOk.r, colorOk.g, colorOk.b);
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
		std::vector<std::string> parameters;
		auto commandSplit = command | std::ranges::views::split(' ');
		for (auto part : commandSplit)
		{
			if (std::string_view(part.begin(), part.end()) == std::string_view(".dft3")) continue;
			parameters.emplace_back(part.begin(), part.end());
		}

		if (std::find(parameters.begin(), parameters.end(), "log") != parameters.end())
		{
			if (!eddft3::Logger::isRunning())
				eddft3::Logger::initialize();
			else
				eddft3::Logger::shutdown();

			return true;
		}
		else if(std::find(parameters.begin(), parameters.end(), "rgb") != parameters.end())
		{
			if(parameters.size() < 3)
			{
				eddft3::Logger::log("Insufficient parameters for RGB command. Command was: " + command);
				return true;
			}
			
			std::string rgbValue = parameters.back();
			std::string rgbMode = parameters[parameters.size() - 2];		

			static const std::regex rgbPattern(R"(^\s*\d{3}:\d{3}:\d{3}\s*$)");

			if (rgbMode != "ok" && rgbMode != "warn" && rgbMode != "caution")
			{
				eddft3::Logger::log("Invalid RGB mode. Expected 'ok', 'warn', or 'caution'. Current value: " + rgbMode);
				return true;
			}

			if (!std::regex_match(rgbValue, rgbPattern))
			{
				eddft3::Logger::log("Invalid RGB command format. Expected format: .dft3 rgb R:G:B (e.g. .dft3 rgb 255:000:000 for red). Current value: " + rgbValue);
				return true;
			}

			auto colorParts = rgbValue | std::ranges::views::split(':');

			int idx = 0;
			color tmpColor;

			for (auto part : colorParts)
			{
				int value = 0;
				auto first = &*part.begin();
				auto last = first + std::ranges::distance(part);

				auto [_, ec] = std::from_chars(first, last, value);

				if (ec == std::errc{} && value >= 0 && value <= 255)
				{
					if (idx == 0) tmpColor.r = (uint8_t)value;
					if (idx == 1) tmpColor.g = (uint8_t)value;
					if (idx == 2) tmpColor.b = (uint8_t)value;
				}

				idx++;
			}

			std::string formattedColor = std::format("{:03}.{:03}.{:03}", tmpColor.r, tmpColor.g, tmpColor.b);

			// premature saving of settings - will be prompted on ES closure
			if (rgbMode == "ok")
			{
				colorOk = tmpColor;
				eddft3::Logger::log("Setting OK color to: " + formattedColor);
				this->SaveDataToSettings("okColor", "south RWY ok color", formattedColor.c_str());
			}
			else if (rgbMode == "warn")
			{
				colorWarn = tmpColor;
				eddft3::Logger::log("Setting WARN color to: " + formattedColor);
				this->SaveDataToSettings("warnColor", "south RWY warning color", formattedColor.c_str());
			}
			else if (rgbMode == "caution")
			{
				colorCaution = tmpColor;
				eddft3::Logger::log("Setting CAUTION color to: " + formattedColor);
				this->SaveDataToSettings("cautionColor", "south RWY caution color", formattedColor.c_str());
			}

			return true;
		}
		else
		{
			DisplayUserMessage("eddft3", "eddft3", ("Unkown command: " + command).c_str(), true, true, true, true, false);
			eddft3::Logger::log("Unknown command parameter. Command was: " + command);
			return true;
		}
	}

	return false;
}

eddft3::eddft3Plugin* pEddft3Plugin = nullptr;

// Diese Funktion wird von EuroScope aufgerufen, wenn das Plugin geladen wird
void __declspec(dllexport) EuroScopePlugInInit(EuroScopePlugIn::CPlugIn** ppPlugInInstance)
{
	// eddft3::Logger::initialize(); // - disabled 
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