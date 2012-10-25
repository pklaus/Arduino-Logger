%%% Script for Matlab
%%% Helps to size the serial reference resistors when using
%%% an NTC to measure temperatures with an ADC.

% Properties of the NTC:
B=3625; R0 = 10000; T0 = 25+273;

% Properties of the measurement circuit:
Rref = 500; Uvcc = 4.6; ADCbits = 10;

% Map the measurement range of the ADC:
Um = [0:(Uvcc/(2^ADCbits-1.)):Uvcc];
% remove the first and last element:
Um = Um(2:end-1);

% Calculate the distinguishable resistance values of the NTC:
Rt = Rref*(Uvcc ./Um - 1); % by measuring the voltage accross the ref res.
%Rt = Rref ./ (Uvcc ./ Um - 1); % measuring the voltage across the NTC

% Calculate the temperature values belonging to those NTC resistances:
T = 1./(log(Rt/R0)/B + 1/T0 ) - 273.15;

% Plot the temperatures versus their differences
scatter(T(1:end-1),diff(T));

% Plot the Temperatures vs. discrete ADC values
%scatter([1:2^ADCbits-2],T)
