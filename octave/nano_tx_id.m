clear('all');
close('all');
clc();

COMProcPath = "COMProc.exe";

[status, output] = system(sprintf('%s HALLO?', COMProcPath))

