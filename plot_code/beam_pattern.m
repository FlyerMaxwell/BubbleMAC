%plot 3GPP beam model
clear all;
clc;

theta1 = -60:1:60;
theta1 = theta1/180*pi
theta2 = [-180:-90, 90:180];
theta2 = theta2/180*pi
% figure;
% for omega = 10
%     gain = 10.^(-0.3*theta.^2/omega^2);
%     plot(theta, gain);
%     hold on;
% end

figure;
omega = 10/180*pi;
scan_angle =0;

    gain = 10.^(-0.3*(theta1-scan_angle).^2/omega^2);
    polarplot(theta1, gain);
    hold on;
    gain = 10.^(-0.3*(theta1*180/pi-scan_angle).^2/(omega*180/pi)^2);
    h1 =polarplot(theta1, gain);
    
    hold on;
    h2 =polarplot(theta2, repelem(0.1,182));
    
    set(h1, 'color', 'k', 'LineWidth', 2)
    set(h2, 'color', 'k', 'LineWidth', 2)
%set(gcf,'unit','normalized','position',[0.15,0.15,0.24,0.24]);
save_path1 = sprintf('/home/sjg/vanet1.0_old/vanet1.0/simulation_result/tmp/');
save_path2 = sprintf('beam_pattern');
%saveas(gcf,[save_path1,save_path2,'.epsc'])
%saveas(gcf,[save_path1,save_path2,'.fig'])

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%calculate power under different beam width
% power_1 = [];
% omega2 = 5:1:60;
% for i = omega2;
%     fun = @(x,c) 0.5*10.^(-0.6*x.^2/c.^2);
%     total_power = integral(@(x) fun(x,i/180*pi),-pi,pi);
%     power_1(end+1) = total_power;
% end
% figure;
% plot(omega2, power_1);