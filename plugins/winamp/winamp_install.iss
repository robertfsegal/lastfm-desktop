; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[CustomMessages]
Version=2.1.0.9

[Setup]
OutputBaseFilename=WinampPluginSetup_2.1.0.9
; setup.exe version
VersionInfoVersion=1.1.0.1
VersionInfoTextVersion=1.1.0.1
AppName=Last.fm Winamp Plugin
AppVerName=Last.fm Winamp Plugin {cm:Version}
VersionInfoDescription=Last.fm Winamp Plugin Installer
AppPublisher=Last.fm
AppPublisherURL=http://www.last.fm
AppSupportURL=http://www.last.fm
AppUpdatesURL=http://www.last.fm
AppCopyright=Copyright 2008 Last.fm Ltd (c)
DefaultDirName="{pf}\Winamp\plugins"
UsePreviousAppDir=yes
UninstallFilesDir={commonappdata}\Last.fm\Client\UninstWA
OutputDir=.
Compression=lzma
SolidCompression=yes
DirExistsWarning=no
DisableReadyPage=yes
; Keep this the same across versions, even if they're incompatible. That will ensure
; uninstallation works fine after many upgrades. Can't use GUID as it'll break backward
; compatibility.
AppId=Audioscrobbler Winamp Plugin
CreateUninstallRegKey=no

[Registry]
; The name of the final subkey here must match the one in plugins.data
Root: HKLM; Subkey: "Software\Last.fm\Client\Plugins\wa2"; ValueType: string; ValueName: "Version"; ValueData: "{cm:Version}"; Flags: uninsdeletekey
Root: HKLM; Subkey: "Software\Last.fm\Client\Plugins\wa2"; ValueType: string; ValueName: "Name"; ValueData: "Winamp"; Flags: uninsdeletekey

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[InstallDelete]
; Delete any old plugins present
Type: files; Name: "{app}\gen_audioscrobbler.dll"
Type: files; Name: "{app}\ml_audioscrobbler.dll"

[UninstallDelete]
; Need to specify both here as we can't be sure the final filename is ml_
Type: files; Name: "{app}\gen_wa2_scrobbler.dll"
Type: files; Name: "{app}\ml_wa2_scrobbler.dll"

; For legacy reasons
Type: files; Name: "{app}\AudioScrobbler.log.txt"

; Try and delete the localappdata log for the case where the user running the uninstaller is the same as the plugin user
Type: files; Name: "{localappdata}\Last.fm\Client\WinampPlugin.log"

[Files]
Source: "Release\ml_wa2_scrobbler.dll"; DestDir: "{app}"; DestName: "{code:GetDestName}"; Flags: ignoreversion
Source: "..\..\Updater.exe"; DestDir: "{code:ExtractFileDir|{reg:HKLM\Software\Last.fm\Client,Path|{sd}}}"; Flags: onlyifdestfileexists

[Run]
; Nothing here. Now taken care of in CurStepChanged.

[Code]
function GetDestName(Param: String): String;
var
  ml_file: String;
  ml_file_exists: Boolean;
begin
  ml_file := ExpandConstant('{app}\gen_ml.dll');

  ml_file_exists := FileExists(ml_file);

  if (ml_file_exists = FALSE) then
  begin
      Result := 'gen_wa2_scrobbler.dll';
  end
  else
  begin
    Result := 'ml_wa2_scrobbler.dll';
  end
end;

procedure CurStepChanged(CurStep: TSetupStep);
var
  batfile: String;
  batcontent: String;
  uninstaller: String;
  alreadyAdded: Boolean;
  cmdToAdd: String;
begin
  if (CurStep = ssPostInstall) then
  begin
    //MsgBox('postinstall', mbInformation, MB_OK);

    batfile := ExpandConstant('{commonappdata}\Last.fm\Client\uninst2.bat');
    LoadStringFromFile(batfile, batcontent);
    //MsgBox('loaded string: ' + batcontent, mbInformation, MB_OK);

    uninstaller := ExpandConstant('{uninstallexe}');
    //MsgBox('uninstaller pre-OEM: ' + uninstaller, mbInformation, MB_OK);

    alreadyAdded := (Pos(uninstaller, batcontent) <> 0);
    if (alreadyAdded = False) then
    begin
      cmdToAdd := uninstaller + #13#10;
      //MsgBox('not present, will add: ' + cmdToAdd, mbInformation, MB_OK);

      SaveStringToFile(batfile, cmdToAdd, True)
    end;

  end;
  
end;