 #define Pro_cnt INT ( Line8:設置 producer 個數 預設為2)
 #define Con_cnt INT ( Line9:設置 consumer 個數 預設為3)  
  sprintf(dirname,"PATH STRING");  ( LINE29 & LINE62 設置目標 dir 名稱 //default ex: "/home/C14041141/txtdata")  !!可能需要更改路徑名稱!!
  fp = fopen("YOURKEYWORD.txt","r"); (LINE 85 打開所命名 keyword 檔  //default ex: "key.txt") !!放在與 HW5-2.c 的同個目錄

 compile:  gcc -o b.out -fopenmp HW5-2.c

 run:  ./b.out
  
 結果: 
       =====keyword======: key.txt 中的 keyword
       =====Producer_readline=====:哪一個 producer 讀txtdata中的txt檔 哪一行
       ======Result of Finding=====:每個keyword 在txtdata中找到幾個
       ===========Spending Time============:花費時間