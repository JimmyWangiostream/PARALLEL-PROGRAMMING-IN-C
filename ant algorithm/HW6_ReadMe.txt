compile: mpiicc Hw6.c -o ant  -g -std=gnu99 -openmp -DINPUT_FILE="\"cities/fri26_d.txt\"" -DN=26
run: mpdboot
     mpiexec -n 4 ./ant

可能需執行數次可得結果 花費時間 2秒左右

compile: mpiicc Hw6.c -o ant  -g -std=gnu99 -openmp -DINPUT_FILE="\"cities/gr17_d.txt\"" -DN=17
run: mpdboot
     mpiexec -n 4 ./ant

可能需執行數次可得結果 花費時間 1秒左右

compile: mpiicc Hw6.c -o ant  -g -std=gnu99 -openmp -DINPUT_FILE="\"cities/dantzig42_d.txt\"" -DN=42
run: mpdboot
     mpiexec -n 4 ./ant

可能需執行數次可得結果 花費時間 10秒左右

compile: mpiicc Hw6.c -o ant  -g -std=gnu99 -openmp -DINPUT_FILE="\"cities/att48_d.txt\"" -DN=48
run: mpdboot
     mpiexec -n 4 ./ant


可能需執行數次可得結果 花費時間 10秒左右