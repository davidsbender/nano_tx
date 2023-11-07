set btl="C:\Users\bende\Desktop\nano_tx\firmware\BOOTLOADER.X\dist\default\production\BOOTLOADER.X.production.hex"
set app="C:\Users\bende\Desktop\nano_tx\firmware\NANO_TX.X\dist\default\production\NANO_TX.X.production.hex"
set hexmate="C:\Program Files\Microchip\MPLABX\v6.05\mplab_platform\bin\hexmate.exe"
%hexmate% %btl% %app% -logfile=log.hxl