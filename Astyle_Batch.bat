rem @echo on
for /R %%f in (*.cpp;*.c;*.cc;*.h;*.hpp) do (astyle.exe -A2 -U -p -c -n "%%f")
for /R %%f in (*.orig) do (del /Q "%%f")
