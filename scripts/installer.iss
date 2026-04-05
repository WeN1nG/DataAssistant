; ============================================
; DataAssistant Inno Setup Installation Script v1.0
; ============================================
; Purpose: Create Windows installer for DataAssistant
; Requirement: Inno Setup 6.0+
; Output: ReleaseTemp/installer/
; ============================================

#define MyAppName "DataAssistant"
#define MyAppVersion "0.1"
#define MyAppPublisher "DataAssistant Team"
#define MyAppExeName "PersonalDateAssisant.exe"
#define MyAppAssocName "DataAssistant Schedule File"
#define MyAppAssocExt ".dat"
#define MyAppAssocKey StringChange(MyAppAssocName, " ", "") + MyAppAssocExt

[Setup]
; Application basic info
AppId={{E8F9A2B1-C4D3-4E5F-8A7B-9C0D1E2F3A4B}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppName}
DisableProgramGroupPage=yes
; License file (if needed)
; LicenseFile=..\LICENSE.md
; Output configuration - OUTPUT TO ReleaseTemp/installer
OutputDir=..\ReleaseTemp\installer
OutputBaseFilename=DataAssistant_Setup_v{#MyAppVersion}
SetupIconFile=..\icon.ico
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
; Windows version requirement
MinVersion=10.0
; Privilege elevation
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
; Installer UI settings
WizardImageFile=compiler:WizModernImage.bmp
WizardSmallImageFile=compiler:WizModernSmallImage.bmp
; Allow user to modify installation directory
AllowNoIcons=yes
; Uninstall settings
UninstallDisplayIcon={app}\{#MyAppExeName}
UninstallDisplayName={#MyAppName}

[Languages]
Name: "chinesesimplified"; MessagesFile: "compiler:Languages\ChineseSimplified.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 6.1; Check: not IsAdminInstallMode

[Files]
; Main program and all dependencies - SOURCE FROM ReleaseTemp/deploy
Source: "..\ReleaseTemp\deploy\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: quicklaunchicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[Registry]
; Application registry entries
Root: HKCU; Subkey: "Software\{#MyAppName}"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\{#MyAppName}"; ValueType: string; ValueName: "InstallPath"; ValueData: "{app}"
Root: HKCU; Subkey: "Software\{#MyAppName}"; ValueType: string; ValueName: "Version"; ValueData: "{#MyAppVersion}"

[Code]
// Initialize setup
function InitializeSetup(): Boolean;
begin
  Result := True;
end;

// Pre-installation check
function PrepareToInstall(var NeedsRestart: Boolean): String;
begin
  Result := '';
end;

// Uninstall cleanup
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usPostUninstall then
  begin
    // Cleanup user data directory (optional)
    // if MsgBox('Do you want to delete user data and settings?', mbConfirmation, MB_YESNO) = IDYES then
    // begin
    //   DelTree(ExpandConstant('{userappdata}\DataAssistant'), True, True, True);
    // end;
  end;
end;
