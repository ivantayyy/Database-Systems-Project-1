========== Installation and Operation Guide ==============
1. Data files (.tsv) are placed in ./data folder

2. Executable program is placed in ./executable folder

3. (Optional) Place custom data file (.tsv) into ./data folder

4. Execute Database.exe in /executable to start the application

5. Once in the program, select data file by inputting its corresponding item number.

6. Follow on-screen instructions to insert data into virtual disk

7. Navigate through menu to execute desired functions.

================== Code Compilation Guide =================
Requirements:
    - MinGW inclusive of boost library
    - Can be downloaded from https://nuwen.net/mingw.html, setup according to stated instuctions.
    - Set IDE toolchain and compiler to MinGW

Important:
    - In CMakeLists.txt, ensure that boost directory corresponds to MinGW Path:
        set(Boost_INCLUDE_DIR C:/MinGW/include/)
        set(Boost_LIBRARY_DIR C:/MinGW/lib/)
