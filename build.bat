@echo off

del nx.exe
del nx.obj
cl  -D_CRT_SECURE_NO_WARNINGS=1 ^
    -DNOMINMAX=1 ^
    -O2 ^
    -nologo -MT -Gm- -GR- -EHsc -W4 ^
    main.c ^
    -Fenx.exe
