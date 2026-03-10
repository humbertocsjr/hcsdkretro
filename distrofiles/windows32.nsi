; HC SDK for Retro Computing - Simple Installer
; Script de instalação NSIS

;--------------------------------
; Configurações básicas

Name "HC SDK for Retro Computing v___VERSION___"
OutFile "../../hcsdk-win-installer.exe"
InstallDir "$PROGRAMFILES\HC SDK for Retro Computing"
InstallDirRegKey HKLM "Software\HC SDK for Retro Computing" "Install_Dir"
RequestExecutionLevel admin

;--------------------------------
; Interface (Inglês)

!include "MUI2.nsh"

; Ícones
!define MUI_ICON "hcsdk.ico"
!define MUI_UNICON "hcsdk.ico"

; Páginas do instalador
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "LICENSE.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; Páginas do desinstalador
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Idioma (Inglês)
!insertmacro MUI_LANGUAGE "English"

;--------------------------------
; Seção principal

Section "Install"

  SetOutPath $INSTDIR
  
  ; Copiar toda a pasta do projeto
  File /r "*.*"
  
  ; Criar atalho no menu Iniciar com o ícone personalizado
  CreateDirectory "$SMPROGRAMS\HC SDK for Retro Computing"
  CreateShortcut "$SMPROGRAMS\HC SDK for Retro Computing\Set Environment.lnk" \
                 "$INSTDIR\setenv.bat" \
                 "" \
                 "$INSTDIR\hcsdk.ico" \
                 0
  
  ; Criar atalho para desinstalação
  CreateShortcut "$SMPROGRAMS\HC SDK for Retro Computing\Uninstall.lnk" \
                 "$INSTDIR\uninstall.exe"
  
  ; Escrever informações de desinstalação
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HCSDKRetro" \
                   "DisplayName" "HC SDK for Retro Computing"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HCSDKRetro" \
                   "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HCSDKRetro" \
                   "DisplayIcon" "$INSTDIR\hcsdk.ico"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HCSDKRetro" \
                   "Publisher" "HC Software"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HCSDKRetro" \
                   "DisplayVersion" "___VERSION___"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HCSDKRetro" \
                     "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HCSDKRetro" \
                     "NoRepair" 1
  
  ; Criar desinstalador
  WriteUninstaller "$INSTDIR\uninstall.exe"
  
SectionEnd

;--------------------------------
; Seção de desinstalação

Section "Uninstall"

  ; Remover atalhos do menu Iniciar
  RMDir /r "$SMPROGRAMS\HC SDK for Retro Computing"
  
  ; Remover diretório de instalação
  RMDir /r "$INSTDIR"
  
  ; Remover chaves do registro
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HCSDKRetro"
  DeleteRegKey HKLM "Software\HC SDK for Retro Computing"

SectionEnd