cp ..\super_list.c; gcc -x c++ test.cpp -x c super_list.c -o a.exe -Ofast -mprfchw -march=native -DNDEBUG -flto -lstdc++ -g
