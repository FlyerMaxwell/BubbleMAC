clear all;
close all;
clc;

metric_name = ["OCR" "ATP" "DTP"];
for metric=1:1:3;
    matplot1 = [];
    density = [40 60 80 100];
    for data_num = density
        path1 = sprintf('/home/sjg/vanet1.0_old/vanet1.0/simulation_result/CDF/metric%d/result_199_4_%d_11_3_40_4_53169.csv', metric,data_num);
        path2 = sprintf('/home/sjg/vanet1.0_old/vanet1.0/simulation_result/CDF/metric%d/result_199_4_%d_2_3_40_4_53169.csv', metric, data_num);
        path3 = sprintf('/home/sjg/vanet1.0_old/vanet1.0/simulation_result/CDF/metric%d/result_199_4_%d_3_3_40_4_53169.csv', metric, data_num);
 
        fid1 = fopen(path1,'rt'); 
        fid2 = fopen(path2,'rt');
        fid3 = fopen(path3,'rt');

        C1 = textscan(fid1, '%f');
        C2 = textscan(fid2, '%f');
        C3 = textscan(fid3, '%f');

        mat_C1 = cell2mat(C1);
        mat_C2 = cell2mat(C2);
        mat_C3 = cell2mat(C3);

        mat_C = [mat_C1 mat_C2 mat_C3];

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
%     cdfplot(mat_C1);
%     hold on;
%     cdfplot(mat_C2);
%     hold on;
%     cdfplot(mat_C3);
%     legend('COMNET', 'Random', '802.11ad')
    hold on
    box on
    
    p1=bar(matplot1(:,1:3));

    tmp_title = metric_name(metric);
    ylabel(metric_name(metric),'FontSize',20);
    xlabel('Traffic Density (vpl)','FontSize',20);
    %title(tmp_title,'FontSize',20);
    
    set(gca,'YGrid','on')
    xticks([1 2 3 4])
    set(gca,'XTicklabel',{'15', '20', '25', '30'});
    %set(gca,'XTickLabelRotation',15)
    set(gca,'FontSize',18, 'Fontname', 'Times New Roman', 'Box','on');
    set(gcf,'unit','normalized','position',[0.15,0.15,0.24,0.34]);
    set(gca,'position',[0.1,0.1,0.85,0.85] );
    
    ylim([0 1]);
    yticks(0:0.2:1);
    set(gca,'YGrid','on')
    
    hold on;
    
    
    e11 = errorbar((1:size(matplot1,1))-0.225,matplot1(:,1),...
        matplot1(:,4), matplot1(:,7), 'k', 'Linestyle', 'None', 'LineWidth', 2);
    e12 = errorbar((1:size(matplot1,1)),matplot1(:,2),...
        matplot1(:,5), matplot1(:,8), 'k', 'Linestyle', 'None', 'LineWidth', 2);
    e13 = errorbar((1:size(matplot1,1))+0.225,matplot1(:,3),...
        matplot1(:,6), matplot1(:,9), 'k', 'Linestyle', 'None', 'LineWidth', 2);

    
    
    legend(p1,'mmV2V','ROP','802.11ad','Orientation','vertical','Location','NorthEast','Fontsize',16);
    
    
    lineWidth=3
    capSize =5
    set(e11,'Color',[0 0 0],'CapSize',capSize,'LineWidth',lineWidth)
    set(e12,'Color',[0 0 0],'CapSize',capSize,'LineWidth',lineWidth)
    set(e13,'Color',[0 0 0],'CapSize',capSize,'LineWidth',lineWidth)
    
          save_path1 = sprintf('/home/sjg/vanet1.0_old/vanet1.0/simulation_result/result_image/');
          save_path2 = sprintf('PROcompare_metric%d', metric);
          saveas(gcf,[save_path1,save_path2,'.epsc'])
          saveas(gcf,[save_path1,save_path2,'.fig'])
end