#pragma once

#include <string>
#include <set>

namespace eddft3
{
	const std::string pluginName = "eddft3";
	const std::string pluginVersion = "0.1.0";
	const std::string pluginAuthor = "Gameagle";
	const std::string pluginCopyright = "GPL v3";

	struct color {
		int r, g, b;
	};

	class eddft3Plugin : public EuroScopePlugIn::CPlugIn
	{
	public:
		eddft3Plugin();
		virtual ~eddft3Plugin();

	private:
		std::set<std::string> northOps{ "AEE","AUA","DLH","CTN","UAL","LOT","SWR",
			"TAP","ITY","LHX","CLH","BTI","AAR","ACA","ANA","TAM","THA","AIC",
			"CCA","CFG","ETH","MSR","EWG","THY","TKJ","SIA","SXS","OCN","BCS",
			"DHX","BOX","FDX","TAY","GEC","DLA","LH" };

		EuroScopePlugIn::CPosition eddfPosition;
		color warnColor{ 255, 0, 0 };

		//************************************
		// Description: Retrieves the position of EDDF from the sector file
		// Method:    getEddfPosition
		// FullName:  eddft3::eddft3Plugin::getEddfPosition
		// Access:    private 
		// Returns:   EuroScopePlugIn::CPosition
		// Qualifier:
		//************************************
		EuroScopePlugIn::CPosition getEddfPosition();

		//************************************
		// Description: Retrieves the color for warnings from the plugin settings, or defaults if not set
		// Method:    getColorFromSettings
		// FullName:  eddft3::eddft3Plugin::getColorFromSettings
		// Access:    private 
		// Returns:   eddft3::color
		// Qualifier:
		//************************************
		color getColorFromSettings();

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

		//************************************
		// Description: Handles custom commands for the plugin, such as toggling logging or setting RGB values
		// Method:    OnCompileCommand
		// FullName:  eddft3::eddft3Plugin::OnCompileCommand
		// Access:    private 
		// Returns:   bool
		// Qualifier:
		// Parameter: const char * sCommandLine
		//************************************
		bool OnCompileCommand(const char* sCommandLine);
	};
}