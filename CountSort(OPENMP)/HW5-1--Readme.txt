#define Range INT ( Line 5:設置 Range 為 矩陣 a 中int的範圍 為 0~(Range-1) )
#define N INT  ( Line 6:設置 N 為 矩陣 a 大小 )
#define threadnum INT ( Line 7:設置 threadnum 為 thread 個數 )


compile: gcc -o c.out -fopenmp HW5-1.cpp

run:  ./c.out

結果:
	=========New Matrix a========: 新創建的矩陣 a[N]
        ==========Count_sort Matix a===========:經過count_sort後的 a[N]
        ==========Spending time===========:花費時間