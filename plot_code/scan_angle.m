clear all;
clc;

%%%%%%%%%%%Variable%%%%%%%%%%%
dT = 15;
theta = 1;
angle_w = 10;
T = [10000, 20000, 30000];
distance = 50;
%max_SNR = 64;
Noise = -80.655;  %dBm
k = 2;

Y = [];
X = [];
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


for  Ts = T
for theta = 1:1:20
    path_loss = 17.7*log10(distance)+70+15*distance/1000; %dB  
    misalign_loss = -10*log(10^(-0.15/2*theta^2/angle_w^2)); %dB 
    SNR_dB = 30-Noise-path_loss-misalign_loss; %dB
    MAX_SNR_dB = 30-Noise-path_loss;
    SNR = 10^(SNR_dB/10);
    MAX_SNR = 10^(MAX_SNR_dB/10);
    
    r = (1-k*(4*180*dT)/(Ts*theta))*log2(1+SNR)/log2(1+MAX_SNR);
    Y(end+1)=SNR_dB/MAX_SNR_dB;
    X(end+1)=theta;
end
plot(X,Y, 'LineWidth', 2);

X=[];
Y=[];
hold on;
end

