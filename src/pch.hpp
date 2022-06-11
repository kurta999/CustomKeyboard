#pragma once

#include <boost/asio.hpp>

#ifdef _WIN32
#include <urlmon.h>
#endif

#include "gui/CanPanel.hpp"
#include "gui/Configuration.hpp"
#include "gui/ConfigurationBackup.hpp"
#include "gui/gui_id.hpp"
#include "gui/Editor.hpp"
#include "gui/EscaperPanel.hpp"
#include "gui/ParserPanel.hpp"
#include "gui/LogPanel.hpp"
#include "gui/FilePanel.hpp"
#include "gui/TrayIcon.hpp"
#include "gui/MainPanel.hpp"
#include "gui/MainFrame.hpp"

#include "Logger.hpp"
#include "CustomMacro.hpp"
#include "GuiEditor.hpp"
#include "StructParser.hpp"
#include "DatabaseImpl.hpp"
#include "DatabaseLogic.hpp"
#include "GuiEditor.hpp"
#include "Settings.hpp"
#include "Server.hpp"
#include "Sensors.hpp"
#include "DirectoryBackup.hpp"
#include "PrintScreenSaver.hpp"
#include "PathSeparator.hpp"
#include "Session.hpp"
#include "SerialForwarder.hpp"
#include "SymlinkCreator.hpp"
#include "MacroRecorder.hpp"
#include "AntiLock.hpp"
#include "TerminalHotkey.hpp"
#include "SerialPort.hpp"
#include "CanSerialPort.hpp"
#include "CryptoPrice.hpp"
#include "CanEntryHandler.hpp"
#include "Utils.hpp"

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
#include <boost/algorithm/string_regex.hpp>
#include <boost/algorithm/hex.hpp>
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

#include "utils/AsyncSerial.hpp"

extern "C"
{
	#include "sha256/sha256.h"
}

#ifdef _WIN32
#include <Windows.h>
#endif