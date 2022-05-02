clear all;
clc;

traffic_density = [40 60 80 100]
magic_number = [10000 50000 60000 70000 80000];
linespec = ['^', '*', '+', 'x','d'];
for car_num = traffic_density
    ii=1;
    raw_data = [];
    matplot1 = [];
    matplot2 = [];
    mat_plot = [];
    figure;
    for pro_index = magic_number
        dataA = zeros(200,1);
        for i=0:1:99
            pro_num = pro_index+i;
            path1 = sprintf('/home/sjg/vanet1.0_old/vanet1.0/simulation_result/Matching/density_%d_%d.csv', car_num, pro_num);
            fid1 = fopen(path1,'rt');
            C1 = textscan(fid1, '%f');
            fclose(fid1);
            data1 = deal(C1{1});
            data1 = reshape(data1,[200,10]);
            data1 = mean(data1,2);
            dataA = dataA+data1;
        end
        dataA = dataA/100/(1024^3)*8*(10^6);
        dataA = dataA(1:50);
        semilogx(dataA, linespec(ii),'LineWidth', 2);
        ii = ii+1;
        %plot(dataA, 'LineWidth', 2)
        hold on;
        raw_data = [raw_data, dataA];
    end
     xlim([1 50])
     xticks([1 10 50])
     ylim([0 4.5])
     yticks(0:1:4)
     ylabel('Mean Capacity (Gbps)','FontSize',24);
     xlabel('# negotiate slot','FontSize',24);
     %tmp_title = sprintf('Traffic Density: %d', car_num);
     %title(tmp_title, 'FontSize', 10);

     set(gca,'FontSize',24, 'Fontname', 'Times New Roman', 'Box','on');
     set(gcf,'unit','normalized','position',[0.15,0.15,0.24,0.34]);
     set(gca,'position',[0.11,0.1,0.85,0.85] );
% 
    legend('1', '5', '6', '7', '8','Orientation','vertical','Location','SouthEast','Fontsize',20);
% 
% 
%     ylim([0 1])
%     yticks(0:0.1:1)
%     set(gca,'YGrid','on')

     save_path1 = sprintf('/home/sjg/vanet1.0_old/vanet1.0/simulation_result/tmp/');
     save_path2 = sprintf('MagicNum_%d', car_num);
%     %exportgraphics(gcf, [save_path1,save_path2,'.png']);
%     saveas(gcf,[save_path1,save_path2,'.epsc'])
     saveas(gcf,[save_path1,save_path2,'.fig'])
end