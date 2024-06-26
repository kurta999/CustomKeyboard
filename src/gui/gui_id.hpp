#pragma once

constexpr int WINDOW_SIZE_X = 1024;
constexpr int WINDOW_SIZE_Y = 768;

enum
{
	ID_Help = 1,
	ID_About,
	ID_Quit,
	ID_10msTimer,
	ID_100msTimer,
	ID_CheckKeypress,
	ID_FilePicker,
	ID_FilePickerMapConverter,
	ID_FilePickerCanScript,
	ID_DestroyAll,
	ID_CanLoadTxList,
	ID_CanSaveTxList,
	ID_CanLoadRxList,
	ID_CanSaveRxList,
	ID_CanLoadMapping,
	ID_CanSaveMapping,
	ID_CanSaveAll,
	ID_CmdExecutorSave,
	ID_BsecSaveCache,
	ID_SaveEverything,
	ID_AppBindListMain,
	ID_MacroDetails,
	ID_BackupPanel,
	ID_EditorPropGrid,
	ID_DirList,
	Id_Macro_AddNewApplication,
	Id_Macro_AddNewMacroKey,
	Id_Macro_Rename,
	Id_Macro_Delete,
	Id_MacroDetails_AddNew,
	Id_MacroDetails_Clone,
	Id_MacroDetails_Delete,
	Id_MacroDetails_MoveUp,
	Id_MacroDetails_MoveDown,
	Id_Backup_AddNew,
	Id_Backup_Delete,
	ID_CanSenderMoreInfo,
	ID_CanSenderLogForFrame,
	ID_CanSenderEditStyle,
	ID_CanSenderRemoveRxFrame,
	ID_CanSenderEditLogLevel,
	ID_CanSenderEditFavourites,
	ID_CmdExecutorEdit,
	ID_CmdExecutorChangeCommandIcon,
	ID_CmdExecutorDuplicate,
	ID_CmdExecutorMoveUp,
	ID_CmdExecutorMoveDown,
	ID_CmdExecutorAdd,
	ID_CmdExecutorAddSeparator,
	ID_CmdExecutorReload,
	ID_CmdExecutorDelete,	
	ID_CmdExecutorAddCol,
	ID_CmdExecutorDeleteCol,
	ID_CmdExecutorEditPageName,
	ID_CmdExecutorChangeIcon,
	ID_CmdExecutorAddPage,
	ID_CmdExecutorDeletePage,
	ID_CmdExecutorDuplicatePageAfter,
	ID_CmdExecutorDuplicatePageBefore,
	ID_CanLogLevelSpinCtrl,
	ID_ModbusShowBits,
	ID_ModbusDataEdit,
	ID_ModbusDataDec,
	ID_ModbusDataHex,
	ID_ModbusDataBin,
	ID_ModbusEditFavourites,
	ID_AlarmTrigger,
	ID_AlarmCancel,
};