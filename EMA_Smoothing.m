
%%% Exponential Moving Average Implemented in Matlab

%% EMA simulation based on
% http://stockcharts.com/school/doku.php?id=chart_school:technical_indicators:moving_averages
% http://osdir.com/ml/python.matplotlib.general/2005-04/msg00044.html

a = [100:0.001:200];
b = [50:0.001:100];
input = [a b];
n = 5;
weight = 1/(n+1);

count = 1;
output(numel(input)+1) = 0.0;
for k = input
    count = count + 1;
    output(count) = (k - output(count-1)) * weight + output(count-1);
end

Xi = 1:numel(input);
Xo = 1:numel(output);

plot(Xi, input, 'go', Xo, output, 'ro')
