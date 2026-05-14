#pragma once

#include "logger.h"

#include "include/es/EuroScopePlugIn.h"
#include <string>
#include <string_view>
#include <set>

namespace eddft3
{
	const std::string pluginName = "eddft3";
	const std::string pluginVersion = "0.2.1";
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
		std::set<std::string> southOps{ "AAL", "ALK", "ASL", "BAW", "CAL", "CES", "CPA", "CSN",
			"DAH", "DAL", "EIN", "ETD", "FHY", "GFA", "HVN", "HYM", "HYS", "JAL", "KAC", "KAL",
			"KZR", "LBT", "MEA", "MFX", "MGL", "MNE", "MSC", "NMA", "OMA", "PGT", "PUE", "QNT",
			"QTR", "RAM", "RJA", "SEU", "SVA", "TUA", "TWB", "UAE", "UBD", "AEA", "AFR", "ANE",
			"BUC", "EJU", "FIN", "IBE", "ICE", "KLM", "ROT", "RZO", "SAS", "SEH", "TUI", "CAO",
			"NCA", "LCO", "CSS", "CSC", "TAR", "LZB"};

		std::set<std::string> acftAllRwys{ "B703","B712","B721","B722","B731","B732","B733","B734",
			"B734","B735","B736","B737","B738","B739","B37M","B38M","B39M","B3XM","B752","B753",
			"B762","B763","B764","B772","B77L","B773","B77W","B779","B788","B789","B78X","A306",
			"A30B","A310","A318","A319","A320","A321","A32A","A32B","A32C","A32D","A19N","A20N",
			"A21N","A332","A333","A337","A338","A339","A342","A343","A345","A346","A359","A35K",
			"AT43","AT45","AT46","AT72","AT73","AT75","AT76","BCS1","BCS3","CRJ7","CRJ9","CRJX",
			"DH8C","DH8D","E170","E175","E75L","E75S","E190","E195","E290","E295","F70","F100",
			"MD81","MD82","MD83","MD87","MD88","MD90","RJ1H","RJ70","RJ85","B461","B462","B563" };

		EuroScopePlugIn::CPosition eddfPosition;
		color colorOk;
		color colorWarn;
		color colorCaution;

		std::set<std::string> activeAppAtc;
		std::set<std::string> ackFpln;

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
		color getColorFromSettings(const std::string_view& colorType);

		//************************************
		// Description: Determines if a flight plan is supposed to be on the southern app
		// Method:    isSouthApp
		// FullName:  eddft3::eddft3Plugin::isSouthApp
		// Access:    private 
		// Returns:   bool
		// Qualifier:
		// Parameter: const std::string_view & last
		//************************************
		bool isSouthApp(const std::string_view& last);

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
		// Description: Handles custom function calls
		// Method:    OnFunctionCall
		// FullName:  eddft3::eddft3Plugin::OnFunctionCall
		// Access:    private 
		// Returns:   void
		// Qualifier:
		// Parameter: int FunctionId
		// Parameter: const char * sItemString
		// Parameter: POINT Pt
		// Parameter: RECT Area
		//************************************
		void OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area);

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

		//************************************
		// Description: Handles updates to controller positions
		// Method:    OnControllerPositionUpdate
		// FullName:  eddft3::eddft3Plugin::OnControllerPositionUpdate
		// Access:    private 
		// Returns:   void
		// Qualifier:
		// Parameter: EuroScopePlugIn::CController Controller
		//************************************
		void OnControllerPositionUpdate(EuroScopePlugIn::CController Controller);

		//************************************
		// Description: Handles controller disconnections
		// Method:    OnControllerDisconnect
		// FullName:  eddft3::eddft3Plugin::OnControllerDisconnect
		// Access:    private 
		// Returns:   void
		// Qualifier:
		// Parameter: EuroScopePlugIn::CController Controller
		//************************************
		inline void OnControllerDisconnect(EuroScopePlugIn::CController Controller)
		{
			if(Controller.IsValid()) activeAppAtc.erase(Controller.GetCallsign());
		};

		//************************************
		// Description: Handles flight plan disconnections
		// Method:    OnFlightPlanDisconnect
		// FullName:  eddft3::eddft3Plugin::OnFlightPlanDisconnect
		// Access:    private 
		// Returns:   void
		// Qualifier:
		// Parameter: EuroScopePlugIn::CFlightPlan FlightPlan
		//************************************
		inline void OnFlightPlanDisconnect(EuroScopePlugIn::CFlightPlan FlightPlan)
		{
			if (FlightPlan.IsValid() && FlightPlan.GetFlightPlanData().GetDestination() == std::string("EDDF")) ackFpln.erase(FlightPlan.GetCallsign());
		}

		//************************************
		// Description: Handles updates to flight plan data
		// Method:    OnFlightPlanFlightPlanDataUpdate
		// FullName:  eddft3::eddft3Plugin::OnFlightPlanFlightPlanDataUpdate
		// Access:    private 
		// Returns:   void
		// Qualifier:
		// Parameter: EuroScopePlugIn::CFlightPlan FlightPlan
		//************************************
		inline void OnFlightPlanFlightPlanDataUpdate(EuroScopePlugIn::CFlightPlan FlightPlan)
		{
			if (FlightPlan.IsValid() && FlightPlan.GetFlightPlanData().GetDestination() == std::string("EDDF"))
			{
				size_t ackCount = ackFpln.erase(FlightPlan.GetCallsign());
				if (ackCount > 0) eddft3::Logger::log("Removed from ackFpln list (FPLN Data Update): " + std::string(FlightPlan.GetCallsign()));
			}
		}

		//************************************
		// Description: Handles updates on RadarTargets
		// Method:    OnRadarTargetPositionUpdate
		// FullName:  eddft3::eddft3Plugin::OnRadarTargetPositionUpdate
		// Access:    private 
		// Returns:   void
		// Qualifier:
		// Parameter: EuroScopePlugIn::CRadarTarget RadarTarget
		//************************************
		inline void OnRadarTargetPositionUpdate(EuroScopePlugIn::CRadarTarget RadarTarget)
		{
			if (RadarTarget.IsValid() && RadarTarget.GetCorrelatedFlightPlan().GetFlightPlanData().GetDestination() == std::string("EDDF"))
			{
				if (RadarTarget.GetGS() <= 50)
				{
					size_t ackCount = ackFpln.erase(RadarTarget.GetCallsign());
					if (ackCount > 0) eddft3::Logger::log("Removed from ackFpln list (RadarTarget Update): " + std::string(RadarTarget.GetCallsign()));
				}
			}
		}	
	};
}