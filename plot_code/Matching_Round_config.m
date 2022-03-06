clear all;
clc;

metric = ["OCR" "ATP" "DTP"];
for n=1:1:3
    figure;
    hold on;
   % path1 = sprintf('/home/sjg/vanet1.0_old/vanet1.0/simulation_result/CDF/speed_60.csv');
    path1 = sprintf('/home/sjg/vanet1.0_old/vanet1.0/simulation_result/CDF/metric%d/result_199_4_60_11_1_40_4_53169.csv', n);
    path2 = sprintf('/home/sjg/vanet1.0_old/vanet1.0/simulation_result/CDF/metric%d/result_199_4_60_11_2_40_4_53169.csv', n);
    path3 = sprintf('/home/sjg/vanet1.0_old/vanet1.0/simulation_result/CDF/metric%d/result_199_4_60_11_3_40_4_53169.csv', n);
    path4 = sprintf('/home/sjg/vanet1.0_old/vanet1.0/simulation_result/CDF/metric%d/result_199_4_60_11_4_40_4_53169.csv', n);
    
    fid1 = fopen(path1,'rt'); 
    fid2 = fopen(path2,'rt');
    fid3 = fopen(path3,'rt');
    fid4 = fopen(path4,'rt'); 

    C1 = textscan(fid1, '%f');
    C2 = textscan(fid2, '%f');
    C3 = textscan(fid3, '%f');
    C4 = textscan(fid4, '%f');

    fclose(fid1); %之后把数据扫描进C这个变量中,我们是按照浮点类型的形式来处理我们的数据的
    fclose(fid2);
    fclose(fid3); %之后把数据扫描进C这个变量中,我们是按照浮点类型的形式来处理我们的数据的
    fclose(fid4);


    data1 = deal(C1{1});
    data2 = deal(C2{1});
    data3 = deal(C3{1});
    data4 = deal(C4{1});


    %plot(,2,n);

    h1=cdfplot(data1);
    set(h1,'LineStyle','-.','color',[0 0 1],'Linewidth',3);
    hold on;
    h2=cdfplot(data2);
    set(h2,'LineStyle',':','color',[0.85 0.33 0.1],'Linewidth',3);
    hold on;
    h3=cdfplot(data3);
    set(h3,'LineStyle','-','color','k','Linewidth',3);
    hold on;
    h4=cdfplot(data4);
    set(h4,'LineStyle','--','color',[1 0 1],'Linewidth',3);
    hold on;
    
    
    set(gca,'FontSize',18, 'Fontname', 'Times New Roman', 'Box','on');
    set(gcf,'unit','normalized','position',[0.15,0.15,0.24,0.34]);
    set(gca,'position',[0.1,0.1,0.85,0.85] );
 
    legend('\itK=1','\itK=2','\itK=3','\itK=4', 'Location', 'SouthEast','Fontsize',16);
     
    ylim([0 1]);
    yticks(0:0.2:1);
    

    xlim([0 1]);
    xticks(0:0.2:1);
    set(gca,'xticklabel',{'0','20%' '40%' '60%', '80%','100%'})
    
    yticks(0:0.2:1);
     xlabel(metric(n),'FontSize',20);
     ylabel('CDF','FontSize',20);
     title_name = ' ';
     title(title_name);
% 
      save_path1 = sprintf('/home/sjg/vanet1.0_old/vanet1.0/simulation_result/tmp/');
      save_path2 = sprintf('NDrounds_metric%d', n);
      saveas(gcf,[save_path1,save_path2,'.epsc'])
      saveas(gcf,[save_path1,save_path2,'.fig'])
end