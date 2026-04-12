#pragma once
#include "include/es/EuroScopePlugIn.h"
#include <string>
#include <set>

namespace eddft3
{
	const std::string pluginName = "eddft3";
	const std::string pluginVersion = "0.1.0";
	const std::string pluginAuthor = "Gameagle";
	const std::string pluginCopyright = "GPL v3";

	class eddft3Plugin : public EuroScopePlugIn::CPlugIn
	{
	public:
		eddft3Plugin();
		virtual ~eddft3Plugin();

		std::set<std::string> northOps{ "AEE","AUA","DLH","CTN","UAL","LOT","SWR",
			"TAP","ITY","LHX","CLH","BTI","AAR","ACA","ANA","TAM","THA","AIC",
			"CCA","CFG","ETH","MSR","EWG","THY","TKJ","SIA","SXS","OCN","BCS",
			"DHX","BOX","FDX","TAY","GEC","DLA","LH" };

		//************************************
			// Description: Handles events on tag item updates
			// Method:    OnGetTagItem
			// FullName:  vsid::VSIDPlugin::OnGetTagItem
			// Access:    public 
			// Returns:   void
			// Qualifier:
			// Parameter: EuroScopePlugIn::CFlightPlan FlightPlan
			// Parameter: EuroScopePlugIn::CRadarTarget RadarTarget
			// Parameter: int ItemCode
			// Parameter: int TagData
			// Parameter: char sItemString[16]
			// Parameter: int * pColorCode
			// Parameter: COLORREF * pRGB
			// Parameter: double * pFontSize
			//************************************
		void OnGetTagItem(EuroScopePlugIn::CFlightPlan FlightPlan, EuroScopePlugIn::CRadarTarget RadarTarget, int ItemCode, int TagData, char sItemString[16], int* pColorCode, COLORREF* pRGB, double* pFontSize);
	};
}