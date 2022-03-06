 clear all;
clc;

max_theta = 5;
max_EIRP = 28.0; %dBm
max_RX_gain = 8 %dBi
noise_power = -80.655; %dBm
car_number = [40 60 80 100];
ratio = [];

for num = car_number
    path1 = sprintf('/home/sjg/vanet1.0_old/vanet1.0/simulation_result/CDF/car_number_%d.csv',num);
    fid1 = fopen(path1,'rt');
    C1 = textscan(fid1, '%f, %f');
    mat_C1 = cell2mat(C1);
    %mat_C1(:,2) = mat_C1(:,2)
    
    for alpha = 5:1:30   %alpha: TX beamforming width
        EIRP = max_EIRP-10*log10(alpha/max_theta);
        for beta = 5:1:30  %beta: RX beamforming width 
            RX_gain = max_RX_gain-10*log10(beta/max_theta);
            scan_angle = alpha;
            path_loss = 17.7*log10(mat_C1(:,1))+70+15*mat_C1(:,1)/1000;            %dB
            misalign_angle = abs(mat_C1(:,2)-round(mat_C1(:,2)/scan_angle)*scan_angle); %degree
            misalign_loss = 10*0.3*misalign_angle.^2/alpha^2+10*0.3*misalign_angle.^2/beta^2
            SNR = EIRP+RX_gain-path_loss-misalign_loss-noise_power;
            
            ratio(beta-4, alpha-4) = sum(SNR(:)>6)/length(SNR);
        end
    end
    figure;
    X = 10:2:60;
    Y = 10:2:60;
    s = surf(X, Y, ratio);
    
    %tmp_title = sprintf('Traffic Density: %d', num);
    %title(tmp_title,'FontSize',20);
    x1 = xlabel('TX Beam Width \alpha', 'FontSize',24);
    y1 = ylabel('RX Beam Width \beta', 'FontSize', 24);
    
    view([0,0,1]);
    s.EdgeColor = 'none';
    colorbar;
    colorbar('Ticks', 0:0.05:1);
    hold on;
    h = contour3(X,Y,ratio, [0.95, 0.95], 'r', 'LineWidth', 3);
    
    xlim([10 60])
    xticks(10:5:60)
    ylim([10 60])
    yticks(10:5:60)

    %set(gca,'XTickLabelRotation',15)
    set(gca,'FontSize',24, 'Fontname', 'Times New Roman', 'Box','on');
    set(gcf,'unit','normalized','position',[0.15,0.15,0.24,0.34]);
    set(gca,'position',[0.11,0.1,0.75,0.85] );


    save_path1 = sprintf('/home/sjg/vanet1.0_old/vanet1.0/simulation_result/tmp/');
    save_path2 = sprintf('beamwidth_density%d', num);
    %exportgraphics(gcf, [save_path1,save_path2,'.png']);
    saveas(gcf,[save_path1,save_path2,'.epsc'])
    saveas(gcf,[save_path1,save_path2,'.fig'])

end