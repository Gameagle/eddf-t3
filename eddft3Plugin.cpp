#include "pch.h"

#include "eddft3Plugin.h"

eddft3::eddft3Plugin::eddft3Plugin() : EuroScopePlugIn::CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE,
	eddft3::pluginName.c_str(),
	eddft3::pluginVersion.c_str(),
	eddft3::pluginAuthor.c_str(),
	eddft3::pluginCopyright.c_str())
{
	RegisterTagItemType("RWY Hint", TAG_ITEM_DFT3_HINT);
	DisplayUserMessage("Message", "eddft3", std::string("Version: " + pluginVersion + " loaded.").c_str(), true, true, true, false, false);
}

eddft3::eddft3Plugin::~eddft3Plugin()
{
	DisplayUserMessage("Message", "eddft3", "Unloaded successfully", true, true, true, false, false);
}

void eddft3::eddft3Plugin::OnGetTagItem(EuroScopePlugIn::CFlightPlan FlightPlan, EuroScopePlugIn::CRadarTarget RadarTarget, int ItemCode, int TagData, char sItemString[16], int* pColorCode, COLORREF* pRGB, double* pFontSize)
{
	if (!FlightPlan.IsValid() || !RadarTarget.IsValid())
		return;

	if (ItemCode == eddft3::TAG_ITEM_DFT3_HINT)
	{
		if (FlightPlan.GetFlightPlanData().GetDestination() == std::string("EDDF"))
		{
			std::string callsign(sItemString);

			if (northOps.find(callsign.substr(0, 3)) == northOps.end())
			{
				std::string route = FlightPlan.GetFlightPlanData().GetRoute();

				if (!route.ends_with("25L") && !route.ends_with("07R"))
				{
					*pColorCode = EuroScopePlugIn::TAG_COLOR_RGB_DEFINED;
					*pRGB = RGB(255, 0, 0);
					strcpy_s(sItemString, 16, "S");
				}
			}
		}
	}
}

eddft3::eddft3Plugin* pEddft3Plugin = nullptr;

// Diese Funktion wird von EuroScope aufgerufen, wenn das Plugin geladen wird
void __declspec(dllexport) EuroScopePlugInInit(EuroScopePlugIn::CPlugIn** ppPlugInInstance)
{
	*ppPlugInInstance = pEddft3Plugin = new eddft3::eddft3Plugin();
}

// Diese Funktion wird beim Entladen aufgerufen
void __declspec(dllexport) EuroScopePlugInExit(void)
{
	if (pEddft3Plugin != nullptr)
	{
		delete pEddft3Plugin;
		pEddft3Plugin = nullptr;
	}
}