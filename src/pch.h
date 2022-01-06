#pragma once

#include <boost/asio.hpp>

#ifdef _WIN32
#include <urlmon.h>
#endif

#include "gui/Configuration.h"
#include "gui/ConfigurationBackup.h"
#include "gui/gui_id.h"
#include "gui/Editor.h"
#include "gui/EscaperPanel.h"
#include "gui/ParserPanel.h"
#include "gui/LogPanel.h"
#include "gui/FilePanel.h"
#include "gui/TrayIcon.h"
#include "gui/MainPanel.h"
#include "gui/main_frame.h"

#include "Logger.h"
#include "CustomMacro.h"
#include "GuiEditor.h"
#include "StructParser.h"
#include "Database.h"
#include "GuiEditor.h"
#include "Settings.h"
#include "Server.h"
#include "Sensors.h"
#include "DirectoryBackup.h"
#include "PrintScreenSaver.h"
#include "PathSeparator.h"
#include "Session.h"
#include "SymlinkCreator.h"
#include "MacroRecorder.h"
#include "AntiLock.h"
#include "Utils.h"

#include <wx/aui/aui.h>
#include <wx/statline.h>
#include <wx/aui/aui.h>
#include <wx/clipbrd.h>
#include <wx/valnum.h>
#include <wx/spinctrl.h>
#include <wx/statline.h>
#include <wx/richtooltip.h>
#include <wx/clipbrd.h>
#include <wx/richmsgdlg.h>
#include <wx/aui/aui.h>
#include <wx/filepicker.h>
#include <wx/tglbtn.h>
#include <wx/aui/aui.h>
#include <wx/dataview.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/aui/aui.h>
#include "wx/treelist.h"
#include "wx/treectrl.h"
#include <wx/dirctrl.h>
#include <wx/xml/xml.h>
#include "wx/notifmsg.h"
#include "wx/generic/notifmsg.h"
#include <wx/filepicker.h>
#include <wx/menu.h>
#include "wx/taskbar.h"
#include <wx/wxprec.h>
#include <wx/gbsizer.h>
#include <wx/grid.h>
#include <wx/statline.h>
#include <wx/tglbtn.h>
#include <wx/filepicker.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include <wx/timectrl.h>
#include <wx/calctrl.h>
#include <wx/srchctrl.h>
#include <wx/fontpicker.h>
#include <wx/dirctrl.h>
#include <wx/artprov.h>

#include <boost/algorithm/string.hpp>
#include <boost/crc.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/range/adaptor/reversed.hpp>

#include <assert.h>

#ifdef _WIN32
#include <shellapi.h>
#include <shlobj.h>
#include <exdisp.h>
#include <shlwapi.h>
#include "Wtsapi32.h"
#endif

#include <any>
#include <iostream>
#include <fstream>
#include <array>
#include <variant>
#include <string>
#include <bitset>
#include <memory>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <future> 
#include <tuple>
#include <set>
#include <thread>
#include <codecvt>
#include <iterator>
#include <queue>
#include <deque>
#include <cstdint>
#include <stack>
#include <charconv>

#ifdef _WIN32
#include <enumser/enumser.h>
#endif
#include <lodepng/lodepng.h>

#ifndef _WIN32
	#ifndef FMT_HEADER_ONLY
		#define FMT_HEADER_ONLY
	#endif
#endif
#include <fmt/format.h>

#include "utils/AsyncSerial.h"

extern "C"
{
	#include "sha256/sha256.h"
}

#ifdef _WIN32
#include <Windows.h>
#endif