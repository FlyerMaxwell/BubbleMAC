clear all;
clc;

N=5000;

x1= 1:1:N;
x2= 1:1:N;
x3= 1:1:N;


fid1 = fopen('/home/sjg/vanet1.0_old/vanet1.0/simulation_result/TOTAL_LINK/total_link_300_1_12_10_1_10_900.csv','rt');
C1 = textscan(fid1, '%d', N);
fclose(fid1);
data1 = deal(C1{1});

fid2 = fopen('/home/sjg/vanet1.0_old/vanet1.0/simulation_result/TOTAL_LINK/total_link_300_1_22_10_1_10_900.csv','rt');
C2 = textscan(fid2, '%d', N);
fclose(fid2);
data2 = deal(C2{1});

fid3 = fopen('/home/sjg/vanet1.0_old/vanet1.0/simulation_result/TOTAL_LINK/total_link_300_1_72_10_1_10_900.csv','rt');
C3 = textscan(fid3, '%d', N);
fclose(fid3);
data3 = deal(C3{1});


plot(x1, data1, '-*',x2, data2, '-o',x3, data3, '-x');

h=legend('random','weight1','optimal','Location','SouthEast');
set(h,'FontSize',25);


clear all;
clc;