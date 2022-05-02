clear all;
close all;
clc;

metric=2;
matplot1 = [];

for car_num=40:20:100
    path1 = sprintf('/home/sjg/vanet1.0_old/vanet1.0/simulation_result/CDF/metric%d/result_199_%d_11_2_20_1_278.csv', metric, car_num);
    path2 = sprintf('/home/sjg/vanet1.0_old/vanet1.0/simulation_result/CDF/metric%d/result_199_%d_21_2_20_1_278.csv', metric, car_num);
    path3 = sprintf('/home/sjg/vanet1.0_old/vanet1.0/simulation_result/CDF/metric%d/result_199_%d_31_2_20_1_278.csv', metric, car_num);
    path4 = sprintf('/home/sjg/vanet1.0_old/vanet1.0/simulation_result/CDF/metric%d/result_199_%d_41_2_20_1_278.csv', metric, car_num);
    
    fid1 = fopen(path1,'rt'); 
    fid2 = fopen(path2,'rt');
    fid3 = fopen(path3,'rt');
    fid4 = fopen(path4,'rt'); 

    C1 = textscan(fid1, '%f');
    C2 = textscan(fid2, '%f');
    C3 = textscan(fid3, '%f');
    C4 = textscan(fid4, '%f');
    
    mat_C1 = cell2mat(C1);
    mat_C2 = cell2mat(C2);
    mat_C3 = cell2mat(C3);
    mat_C4 = cell2mat(C4);
    
    mat_C = [mat_C1 mat_C2 mat_C3 mat_C4];
    
    ave1 = mean(mat_C, 1);
    b1 = prctile(mat_C,25,1);
    c1 = prctile(mat_C,75,1);
    lower1 = ave1-b1;
    upper1 = c1-ave1;
    mat1=[ave1 lower1 upper1];
    matplot1 = [matplot1;mat1];
end

%% Plot old method
figure

hold on
box on

p1=bar(matplot1(:,1:4));
%p2=bar(matplot2(:,1:5));


set(p1(4),'FaceColor',[1 1 0.07])
set(p1(4),'FaceColor',[0.06 1 1])



ylabel('NEER')
set(gca,'YGrid','on')
xticks([1 2 3 4])
set(gca,'XTicklabel',{'40','60','80','100'});
%set(gca,'XTickLabelRotation',15)
set(gca,'FontSize',18, 'Fontname', 'Times New Roman', 'Box','on');
set(gcf,'unit','normalized','position',[0.15,0.15,0.24,0.32]);
set(gca,'position',[0.11,0.1,0.85,0.85] );





e11 = errorbar((1:size(matplot1,1))-0.272,matplot1(:,1),...
    matplot1(:,5), matplot1(:,9), 'k', 'Linestyle', 'None', 'LineWidth', 2);
e12 = errorbar((1:size(matplot1,1))-0.089,matplot1(:,2),...
    matplot1(:,6), matplot1(:,10), 'k', 'Linestyle', 'None', 'LineWidth', 2);
e13 = errorbar((1:size(matplot1,1))+0.089,matplot1(:,3),...
    matplot1(:,7), matplot1(:,11), 'k', 'Linestyle', 'None', 'LineWidth', 2);
e14 = errorbar((1:size(matplot1,1))+0.272,matplot1(:,4),...
    matplot1(:,8), matplot1(:,12), 'k', 'Linestyle', 'None', 'LineWidth', 2);


legend(p1,'COMNET','Random','JSAC17','Optimal','Orientation','vertical','Location','North','Fontsize',16);


lineWidth=1.5
capSize =5
set(e11,'Color',[0 0 0],'CapSize',capSize,'LineWidth',lineWidth)
set(e12,'Color',[0 0 0],'CapSize',capSize,'LineWidth',lineWidth)
set(e13,'Color',[0 0 0],'CapSize',capSize,'LineWidth',lineWidth)
set(e14,'Color',[0 0 0],'CapSize',capSize,'LineWidth',lineWidth)
