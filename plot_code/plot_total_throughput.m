clear all;
clc;

traffic_density = [40 60 80 100];
pro = [3000 70000 2000];
index =1;
for car_num = traffic_density
    raw_data = [];
    matplot1 = [];
    matplot2 = [];
    matplot3 = [];
    mat_plot = [];
    for pro_index = pro
        dataA = [];
        mat1 = [];
        for i=0:1:99
            pro_num = pro_index+i
            path1 = sprintf('/home/sjg/vanet1.0_old/vanet1.0/simulation_result/Matching/density_%d_%d.csv', car_num, pro_num);
            fid1 = fopen(path1,'rt');
            C1 = textscan(fid1, '%f');
            fclose(fid1);
            data1 = deal(C1{1});
            data1 = reshape(data1,[200,10]);
            dataA = [dataA,data1];
        end
        avg1 = mean(dataA,2);
        low1 = prctile(dataA,5,2);
        high1 = prctile(dataA,95,2);
        
        lower1 = avg1-low1;
        upper1 = high1-avg1;
        mat1 = [avg1 lower1 upper1];
        
        raw_data = [raw_data, mat1];
    end
    
    raw_data = raw_data/(1024^3)*8*(10^6);
    for i=1:1:4
         mat_plot = [mat_plot;raw_data(i*20,:)];
    end

    figure;

    hold on;
    box on;

    X= [1 2 3 4];
    p1=bar(mat_plot(:,1:3:7));
% 
% 
%     set(p1(1),'FaceColor',[1 1 0.07]);
%     set(p1(2),'FaceColor',[0.06 1 1]);
% 
% 
% 
    ylabel('Average Throughput(Gbps)','FontSize',20);
    xlabel('Matching Round','FontSize',20);
    tmp_title = sprintf('Traffic Density: %d', car_num);
    title(tmp_title,'FontSize',20);
    
    set(gca,'YGrid','on')
    xticks([1 2 3 4])
    set(gca,'XTicklabel',{'20', '40', '60', '80'});
    %set(gca,'XTickLabelRotation',15)
    set(gca,'FontSize',18, 'Fontname', 'Times New Roman', 'Box','on');
    set(gcf,'unit','normalized','position',[0.15,0.15,0.24,0.34]);
    set(gca,'position',[0.1,0.1,0.85,0.85] );
    
    ylim([0 5]);
    yticks(0:1:5);
    set(gca,'YGrid','on')
    
    hold on;
    
    e11 = errorbar(X-0.225, mat_plot(:,1), mat_plot(:,2), mat_plot(:,3),'k', 'Linestyle', 'None', 'LineWidth', 2);
    e12 = errorbar(X, mat_plot(:,4), mat_plot(:,5), mat_plot(:,6),'k', 'Linestyle', 'None', 'LineWidth', 2);
    e13 = errorbar(X+0.225, mat_plot(:,7), mat_plot(:,8), mat_plot(:,9),'k', 'Linestyle', 'None', 'LineWidth', 2);

    legend(p1,'SDWM','COMNET','Random','Orientation','vertical','Location','North','Fontsize',14);
    lineWidth=1.5
    capSize =5
    set(e11,'Color',[0 0 0],'CapSize',capSize,'LineWidth',lineWidth)
    set(e12,'Color',[0 0 0],'CapSize',capSize,'LineWidth',lineWidth)
    set(e13,'Color',[0 0 0],'CapSize',capSize,'LineWidth',lineWidth)
%     
     save_path1 = sprintf('/home/sjg/vanet1.0_old/vanet1.0/simulation_result/tmp/');
     save_path2 = sprintf('MATCHcompare_%d', car_num);
%     exportgraphics(gcf, [save_path1,save_path2,'.png']);
     saveas(gcf,[save_path1,save_path2,'.epsc'])
     saveas(gcf,[save_path1,save_path2,'.fig'])
end