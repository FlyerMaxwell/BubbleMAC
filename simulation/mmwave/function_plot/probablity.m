clear all;
clc;

k = [;];
P = [];
p = 0.5;
n0 = 10.0001;
ni = n0;
times = 20;
tmp=1;

for times = 1:1:30
    for n0 = 1.0001:1:10.0001
        ni = n0;
        tmp=1;
        for i = 1:times
            n1 = floor(ni);
            n2 = ceil(ni);
            if (n1>=1)
                pi = (n2-ni)*2*n1/n0*p*(1-p)*(1-p/n0)^(n1-1)+(ni-n1)*2*n2/n0*p*(1-p)*(1-p/n0)^(n2-1);
            else
                 pi = ni*2/n0*p*(1-p);
            end
     
            tmp = tmp*(1-pi);
            ni = ni*(1-pi);
        end
        P(times, floor(n0)) = 1-tmp;
    end
end

x = [1,times];
y = [1,floor(n0)];

x1 = 1:1:times;
y1 = 1:1:floor(n0);
[X,Y] = meshgrid(y1,x1);
surf(X,Y,P);
%grid on;
% imagesc(y,x,P);
% colorbar;


% for n0 = 1.0001:1:10.0001
%     ni = n0;
%     tmp=1;
%     for i = 1:30  
%     
%         n1 = floor(ni);
%         n2 = ceil(ni);
%         if (n1>=1)
%             pi = (n2-ni)*2*n1/n0*p*(1-p)*(1-p/n0)^(n1-1)+(ni-n1)*2*n2/n0*p*(1-p)*(1-p/n0)^(n2-1);
%         else
%             pi = ni*2/n0*p*(1-p);
%         end
%     
%         tmp = tmp*(1-pi);
%         k(end+1) = i;
%         P(end+1) = 1-tmp;
%         ni = ni*(1-pi);
%     end 
%     plot(k, P, 'LineWidth', 2);
%     hold on;
%     k = [];
%     P = [];
% end
% 
% h=legend('1', '2', '3', '4', '5', '6', '7', '8', '9', '10', 'Location','SouthEast');
% set(h,'FontSize',25);
% xlabel('number', 'FontSize',25);
% ylabel('probability','FontSize',25);

% for j = 5:times
%     for p = 0.1:0.01:0.9
%     tmp=1;
%     ni=n0;
%     for i = 1:j  
%     
%         n1 = floor(ni);
%         n2 = ceil(ni);
%         if (n1>=1)
%             pi = (n2-ni)*2*n1/n0*p*(1-p)*(1-p/n0)^(n1-1)+(ni-n1)*2*n2/n0*p*(1-p)*(1-p/n0)^(n2-1);
%         else
%             pi = ni*2*1/n0*p*(1-p);
%         end
%     
%         tmp = tmp*(1-pi);
%         ni = ni*(1-pi);
%     end
%     k(end+1) = p;
%     P(end+1) = 1-tmp;
%     end
% 
%     plot(k, P, 'LineWidth', 2);
%     hold on;
%     k = [];
%     P = [];
% end
% 
% xlabel('number', 'FontSize',25);
% ylabel('probability','FontSize',25);