 #define Pro_cnt INT ( Line8:�]�m producer �Ӽ� �w�]��2)
 #define Con_cnt INT ( Line9:�]�m consumer �Ӽ� �w�]��3)  
  sprintf(dirname,"PATH STRING");  ( LINE29 & LINE62 �]�m�ؼ� dir �W�� //default ex: "/home/C14041141/txtdata")  !!�i��ݭn�����|�W��!!
  fp = fopen("YOURKEYWORD.txt","r"); (LINE 85 ���}�ҩR�W keyword ��  //default ex: "key.txt") !!��b�P HW5-2.c ���P�ӥؿ�

 compile:  gcc -o b.out -fopenmp HW5-2.c

 run:  ./b.out
  
 ���G: 
       =====keyword======: key.txt ���� keyword
       =====Producer_readline=====:���@�� producer Ūtxtdata����txt�� ���@��
       ======Result of Finding=====:�C��keyword �btxtdata�����X��
       ===========Spending Time============:��O�ɶ�